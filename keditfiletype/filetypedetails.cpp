/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000, 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

// Own
#include "filetypedetails.h"

// Qt
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

// KDE
#include <KIconButton>
#include <KIconDialog>
#include <KLineEdit>
#include <KLocalizedString>

// Local
#include "kservicelistwidget.h"
#include "typeslistitem.h"

FileTypeDetails::FileTypeDetails(QWidget *parent)
    : QWidget(parent)
    , m_mimeTypeData(nullptr)
    , m_item(nullptr)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);

    m_mimeTypeLabel = new QLabel(this);
    m_mimeTypeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    topLayout->addWidget(m_mimeTypeLabel, 0, Qt::AlignCenter);

    QString wtstr;
    QWidget *firstWidget = new QWidget(this);
    QVBoxLayout *firstLayout = new QVBoxLayout(firstWidget);
    topLayout->addWidget(firstWidget);

    QHBoxLayout *hBox = new QHBoxLayout();
    firstLayout->addLayout(hBox);

    iconButton = new KIconButton(firstWidget);
    iconButton->setIconType(KIconLoader::Desktop, KIconLoader::MimeType);
    connect(iconButton, &KIconButton::iconChanged, this, &FileTypeDetails::updateIcon);
    iconButton->setWhatsThis(
        i18n("This button displays the icon associated"
             " with the selected file type. Click on it to choose a different icon."));
    iconButton->setFixedSize(70, 70);
    iconLabel = nullptr;
    hBox->addWidget(iconButton);

    QGroupBox *gb = new QGroupBox(i18n("Filename Patterns"), firstWidget);
    hBox->addWidget(gb);

    hBox = new QHBoxLayout(gb);

    extensionLB = new QListWidget(gb);
    connect(extensionLB, &QListWidget::itemSelectionChanged, this, &FileTypeDetails::enableExtButtons);
    hBox->addWidget(extensionLB);

    extensionLB->setFixedHeight(extensionLB->minimumSizeHint().height());

    extensionLB->setWhatsThis(
        i18n("This box contains a list of patterns that can be"
             " used to identify files of the selected type. For example, the pattern *.txt is"
             " associated with the file type 'text/plain'; all files ending in '.txt' are recognized"
             " as plain text files."));

    QVBoxLayout *vbox = new QVBoxLayout();
    hBox->addLayout(vbox);

    addExtButton = new QPushButton(i18n("Add..."), gb);
    addExtButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    addExtButton->setEnabled(false);
    connect(addExtButton, &QAbstractButton::clicked, this, &FileTypeDetails::addExtension);
    vbox->addWidget(addExtButton);
    addExtButton->setWhatsThis(i18n("Add a new pattern for the selected file type."));

    removeExtButton = new QPushButton(i18n("Remove"), gb);
    removeExtButton->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    removeExtButton->setEnabled(false);
    connect(removeExtButton, &QAbstractButton::clicked, this, &FileTypeDetails::removeExtension);
    vbox->addWidget(removeExtButton);
    removeExtButton->setWhatsThis(i18n("Remove the selected filename pattern."));

    vbox->addStretch(1);

    gb->setFixedHeight(gb->minimumSizeHint().height());

    description = new KLineEdit(firstWidget);
    description->setClearButtonEnabled(true);
    connect(description, &QLineEdit::textChanged, this, &FileTypeDetails::updateDescription);

    QHBoxLayout *descriptionBox = new QHBoxLayout;
    descriptionBox->addWidget(new QLabel(i18n("Description:"), firstWidget));
    descriptionBox->addWidget(description);
    firstLayout->addLayout(descriptionBox);

    wtstr = i18n(
        "You can enter a short description for files of the selected"
        " file type (e.g. 'HTML Page'). This description will be used by applications"
        " like Konqueror to display directory content.");
    description->setWhatsThis(wtstr);

    serviceListWidget = new KServiceListWidget(KServiceListWidget::SERVICELIST_APPLICATIONS, firstWidget);
    connect(serviceListWidget, &KServiceListWidget::changed, this, &FileTypeDetails::changed);
    connect(serviceListWidget, &KServiceListWidget::multiApply, this, &FileTypeDetails::multiApply);
    firstLayout->addWidget(serviceListWidget, 5);
}

void FileTypeDetails::updateRemoveButton()
{
    removeExtButton->setEnabled(extensionLB->count() > 0);
}

void FileTypeDetails::updateIcon(const QString &icon)
{
    if (!m_mimeTypeData) {
        return;
    }

    m_mimeTypeData->setUserSpecifiedIcon(icon);

    if (m_item) {
        m_item->setIcon(icon);
    }

    Q_EMIT changed(true);
}

void FileTypeDetails::updateDescription(const QString &desc)
{
    if (!m_mimeTypeData) {
        return;
    }

    m_mimeTypeData->setComment(desc);

    Q_EMIT changed(true);
}

void FileTypeDetails::addExtension()
{
    if (!m_mimeTypeData) {
        return;
    }

    bool ok;
    QString ext = QInputDialog::getText(this, i18n("Add New Extension"), i18n("Extension:"), QLineEdit::Normal, QStringLiteral("*."), &ok);
    if (ok) {
        extensionLB->addItem(ext);
        QStringList patt = m_mimeTypeData->patterns();
        patt += ext;
        m_mimeTypeData->setPatterns(patt);
        updateRemoveButton();
        Q_EMIT changed(true);
    }
}

void FileTypeDetails::removeExtension()
{
    if (extensionLB->currentRow() == -1) {
        return;
    }
    if (!m_mimeTypeData) {
        return;
    }
    QStringList patt = m_mimeTypeData->patterns();
    patt.removeAll(extensionLB->currentItem()->text());
    m_mimeTypeData->setPatterns(patt);
    delete extensionLB->takeItem(extensionLB->currentRow());
    updateRemoveButton();
    Q_EMIT changed(true);
}

void FileTypeDetails::setMimeTypeData(MimeTypeData *mimeTypeData, TypesListItem *item)
{
    m_mimeTypeData = mimeTypeData;
    m_item = item; // can be 0
    Q_ASSERT(mimeTypeData);
    m_mimeTypeLabel->setText(i18n("File type %1", mimeTypeData->name()));
    if (iconButton) {
        iconButton->setIcon(mimeTypeData->icon());
        iconButton->setToolTip(mimeTypeData->icon());
    } else {
        iconLabel->setPixmap(QIcon::fromTheme(mimeTypeData->icon()).pixmap(KIconLoader::SizeLarge));
    }
    description->setText(mimeTypeData->comment());
    extensionLB->clear();
    addExtButton->setEnabled(true);
    removeExtButton->setEnabled(false);

    serviceListWidget->setMimeTypeData(mimeTypeData);

    extensionLB->addItems(mimeTypeData->patterns());
}

void FileTypeDetails::enableExtButtons()
{
    removeExtButton->setEnabled(true);
}

void FileTypeDetails::refresh()
{
    if (!m_mimeTypeData) {
        return;
    }

    // Called when ksycoca has been updated -> refresh data, then widgets
    m_mimeTypeData->refresh();
    setMimeTypeData(m_mimeTypeData, m_item);
}

void FileTypeDetails::allowMultiApply(bool allow)
{
    serviceListWidget->allowMultiApply(allow);
}

#include "moc_filetypedetails.cpp"
