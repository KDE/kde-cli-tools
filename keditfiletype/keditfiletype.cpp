/* This file is part of the KDE project
   Copyright (C) 2000, 2007 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 or at your option version 3 as published by
   the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Own
#include "keditfiletype.h"
#include "mimetypewriter.h"

// Qt
#include <QApplication>
#include <QBoxLayout>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFile>
#include <QPushButton>

// KDE
#include <kaboutdata.h>
#include <kbuildsycocaprogressdialog.h>
#include <klocalizedstring.h>

#include <ksycoca.h>
#include <kservicetypeprofile.h>

#include <kwindowsystem.h>

// Local
#include "filetypedetails.h"
#include "typeslistitem.h"

FileTypeDialog::FileTypeDialog(MimeTypeData *mime)
    : QDialog(nullptr)
    , m_mimeTypeData(mime)
{
    init();
}

FileTypeDialog::~FileTypeDialog()
{
    delete m_details;
}

void FileTypeDialog::init()
{
    m_details = new FileTypeDetails(this);
    m_details->setMimeTypeData(m_mimeTypeData);
    connect(m_details, &FileTypeDetails::changed, this, &FileTypeDialog::clientChanged);

    m_buttonBox = new QDialogButtonBox;
    m_buttonBox->setStandardButtons(
        QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(m_buttonBox->button(
                QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &FileTypeDialog::save);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // This code is very similar to kcdialog.cpp
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_details);
    layout->addWidget(m_buttonBox);
    // TODO setHelp()

    setApplyButtonEnabled(false);

    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)),
            SLOT(slotDatabaseChanged(QStringList)));
}

void FileTypeDialog::setApplyButtonEnabled(bool enabled)
{
    m_buttonBox->button(QDialogButtonBox::Apply)->setEnabled(enabled);
}

void FileTypeDialog::save()
{
    if (m_mimeTypeData->isDirty()) {
        const bool servicesDirty = m_mimeTypeData->isServiceListDirty();
        if (m_mimeTypeData->sync()) {
            MimeTypeWriter::runUpdateMimeDatabase();
        }
        if (servicesDirty) {
            KBuildSycocaProgressDialog::rebuildKSycoca(this);
        }
        // Trigger reparseConfiguration of filetypesrc in konqueror
        QDBusMessage message
            = QDBusMessage::createSignal(QStringLiteral("/KonqMain"),
                                         QStringLiteral("org.kde.Konqueror.Main"),
                                         QStringLiteral("reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);
    }
}

void FileTypeDialog::accept()
{
    save();
    QDialog::accept();
}

void FileTypeDialog::clientChanged(bool state)
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(state);
    m_buttonBox->button(QDialogButtonBox::Apply)->setEnabled(state);
}

void FileTypeDialog::slotDatabaseChanged(const QStringList &changedResources)
{
    qDebug() << changedResources;
    if (changedResources.contains(QStringLiteral("xdgdata-mime"))  // changes in mimetype definitions
        || changedResources.contains(QStringLiteral("services"))) {   // changes in .desktop files
        m_details->refresh();
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral(
                                                     "preferences-desktop-filetype-association")));

    KAboutData aboutData(QStringLiteral("keditfiletype"), i18n("File Type Editor"),
                         QLatin1String(PROJECT_VERSION),
                         i18n(
                             "KDE file type editor - simplified version for editing a single file type"),
                         KAboutLicense::GPL,
                         i18n("(c) 2000, KDE developers"));
    aboutData.addAuthor(i18n("Preston Brown"), QString(), QStringLiteral("pbrown@kde.org"));
    aboutData.addAuthor(i18n("David Faure"), QString(), QStringLiteral("faure@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("parent"),
        i18n("Makes the dialog transient for the window specified by winid"),
    QStringLiteral("winid")));
    parser.addPositionalArgument(QStringLiteral("mimetype"),
                                 i18n("File type to edit (e.g. text/html)"));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.positionalArguments().count() == 0) {
        parser.showHelp();
    }

    QMimeDatabase db;
    QString arg = parser.positionalArguments().first();
    MimeTypeData *mimeTypeData = nullptr;
    const bool createType = arg.startsWith(QLatin1Char('*'));
    if (createType) {
        QString mimeString = QStringLiteral("application/x-kdeuser%1");
        QString mimeTypeName;
        int inc = 0;
        bool ok = false;
        do {
            ++inc;
            mimeTypeName = mimeString.arg(inc);
            ok = !db.mimeTypeForName(mimeTypeName).isValid();
        } while (!ok);

        QStringList patterns;
        if (arg.length() > 2) {
            patterns << arg.toLower() << arg.toUpper();
        }
        QString comment;
        if (arg.startsWith(QLatin1String("*.")) && arg.length() >= 3) {
            const QString type = arg.mid(3).prepend(arg[2].toUpper());
            comment = i18n("%1 File", type);
        }

        mimeTypeData = new MimeTypeData(mimeTypeName, true); // new mimetype
        mimeTypeData->setComment(comment);
        mimeTypeData->setPatterns(patterns);
    } else {
        const QString mimeTypeName = arg;
        QMimeType mime = db.mimeTypeForName(mimeTypeName);
        if (!mime.isValid()) {
            qCritical() << "Mimetype" << mimeTypeName << "not found";
            return 1;
        }

        mimeTypeData = new MimeTypeData(mime);
    }

    FileTypeDialog dlg(mimeTypeData);
    if (parser.isSet(QStringLiteral("parent"))) {
        bool ok;
        long id = parser.value(QStringLiteral("parent")).toLong(&ok);
        if (ok) {
            dlg.setAttribute(Qt::WA_NativeWindow, true);
            KWindowSystem::setMainWindow(dlg.windowHandle(), WId(id));
        }
    }
    if (!createType) {
        dlg.setWindowTitle(i18n("Edit File Type %1", mimeTypeData->name()));
    } else {
        dlg.setWindowTitle(i18n("Create New File Type %1", mimeTypeData->name()));
        dlg.setApplyButtonEnabled(true);
    }

    dlg.show(); // non-modal

    return app.exec();
}
