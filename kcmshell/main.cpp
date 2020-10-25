/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2004 Frans Englich <frans.englich@telia.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "main.h"

#include <config-kde-cli-tools.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDBusInterface>
#include <QDebug>
#include <QIcon>
#include <QStandardPaths>

#include <KAboutData>
#include <KAuthorized>

#include <KActivities/ResourceInstance>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KLocalizedString>
#include <KQuickAddons/QtQuickSettings>
#include <KServiceTypeTrader>
#include <KStartupInfo>
#include <kworkspace.h>

#include <iostream>

using namespace std;

KService::List m_modules;

static bool caseInsensitiveLessThan(const KService::Ptr s1, const KService::Ptr s2)
{
    const int compare = QString::compare(s1->desktopEntryName(),
                                         s2->desktopEntryName(),
                                         Qt::CaseInsensitive);
    return (compare < 0);
}

static void listModules()
{
    // First condition is what systemsettings does, second what kinfocenter does, make sure this is kept in sync
    // We need the exist calls because otherwise the trader language aborts if the property doesn't exist and the second part of the or is not evaluated
    const KService::List services = KServiceTypeTrader::self()->query( QStringLiteral("KCModule"), QStringLiteral("(exist [X-KDE-System-Settings-Parent-Category] and [X-KDE-System-Settings-Parent-Category] != '') or (exist [X-KDE-ParentApp] and [X-KDE-ParentApp] == 'kinfocenter')") );
    for( KService::List::const_iterator it = services.constBegin(); it != services.constEnd(); ++it) {
        const KService::Ptr s = (*it);
        if (!KAuthorized::authorizeControlModule(s->menuId()))
            continue;
        m_modules.append(s);
    }

    std::stable_sort(m_modules.begin(), m_modules.end(), caseInsensitiveLessThan);
}

static KService::Ptr locateModule(const QString& module)
{
    QString path = module;

    if (!path.endsWith(QLatin1String(".desktop")))
        path += QStringLiteral(".desktop");

    KService::Ptr service = KService::serviceByStorageId( path );
    if (!service) {
        return KService::Ptr();
    }

    if (!service->hasServiceType(QStringLiteral("KCModule"))) {
        // Not a KCModule. E.g. "kcmshell5 akonadi" finds services/kresources/kabc/akonadi.desktop, unrelated.
        return KService::Ptr();
    }

    if (service->noDisplay()) {
        qDebug() << module << "should not be loaded.";
        return KService::Ptr();
    }

    return service;
}

bool KCMShell::isRunning()
{
    const QString owner = QDBusConnection::sessionBus().interface()->serviceOwner(m_serviceName);
    if (owner == QDBusConnection::sessionBus().baseService()) {
        return false; // We are the one and only.
    }

    qDebug() << "kcmshell5 with modules '" << m_serviceName << "' is already running.";

    QDBusInterface iface(m_serviceName, QStringLiteral("/KCModule/dialog"), QStringLiteral("org.kde.KCMShellMultiDialog"));
    QDBusReply<void> reply = iface.call(QStringLiteral("activate"), KStartupInfo::startupId());
    if (!reply.isValid())
    {
        qDebug() << "Calling D-Bus function dialog::activate() failed.";
        return false; // Error, we have to do it ourselves.
    }

    return true;
}

KCMShellMultiDialog::KCMShellMultiDialog(KPageDialog::FaceType dialogFace, QWidget *parent)
    : KCMultiDialog(parent)
{
    setFaceType(dialogFace);
    setModal(false);

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/KCModule/dialog"), this, QDBusConnection::ExportScriptableSlots);

    connect(this, &KCMShellMultiDialog::currentPageChanged,
            this, [](KPageWidgetItem *newPage,KPageWidgetItem *oldPage) {
                Q_UNUSED(oldPage);
                KCModuleProxy *activeModule = newPage->widget()->findChild<KCModuleProxy *>();
                if (activeModule) {
                    KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("kcm:") + activeModule->moduleInfo().service()->storageId()),
                            QStringLiteral("org.kde.systemsettings"));
                }
            });
}

void KCMShellMultiDialog::activate(const QByteArray& asn_id)
{
#ifdef HAVE_X11
    setAttribute(Qt::WA_NativeWindow, true);
    KStartupInfo::setNewStartupId(windowHandle(), asn_id);
#endif
}

void KCMShell::setServiceName(const QString &dbusName)
{
    m_serviceName = QLatin1String( "org.kde.kcmshell_" ) + dbusName;
    QDBusConnection::sessionBus().registerService(m_serviceName);
}

void KCMShell::waitForExit()
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    watcher->addWatchedService(m_serviceName);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged,
            this, &KCMShell::appExit);
    exec();
}

void KCMShell::appExit(const QString &appId, const QString &oldName, const QString &newName)
{
    Q_UNUSED(appId);
    Q_UNUSED(newName);

    if (!oldName.isEmpty())
    {
        qDebug() << "'" << appId << "' closed, quitting.";
        qApp->quit();
    }
}

int main(int _argc, char *_argv[])
{
    const bool qpaVariable = qEnvironmentVariableIsSet("QT_QPA_PLATFORM");
    KWorkSpace::detectPlatform(_argc, _argv);
    KCMShell app(_argc, _argv);
    if (!qpaVariable) {
        // don't leak the env variable to processes we start
        qunsetenv("QT_QPA_PLATFORM");
    }
    KLocalizedString::setApplicationDomain("kcmshell5");
    KQuickAddons::QtQuickSettings::init();

    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    KAboutData aboutData(QStringLiteral("kcmshell5"), i18n("System Settings Module"),
                         QLatin1String(PROJECT_VERSION),
                         i18n("A tool to start single system settings modules"),
                         KAboutLicense::GPL,
                         i18n("(c) 1999-2016, The KDE Developers"));

    aboutData.addAuthor(i18n("Frans Englich"), i18n("Maintainer"), QStringLiteral("frans.englich@kde.org"));
    aboutData.addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    aboutData.addAuthor(i18n("Matthias Hoelzer-Kluepfel"),QString(), QStringLiteral("hoelzer@kde.org"));
    aboutData.addAuthor(i18n("Matthias Elter"),QString(), QStringLiteral("elter@kde.org"));
    aboutData.addAuthor(i18n("Matthias Ettrich"),QString(), QStringLiteral("ettrich@kde.org"));
    aboutData.addAuthor(i18n("Waldo Bastian"),QString(), QStringLiteral("bastian@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.addOption(QCommandLineOption(QStringLiteral("list"), i18n("List all possible modules")));
    parser.addPositionalArgument(QStringLiteral("module"), i18n("Configuration module to open"));
    parser.addOption(QCommandLineOption(QStringLiteral("lang"), i18n("Specify a particular language"), QLatin1String("language")));
    parser.addOption(QCommandLineOption(QStringLiteral("silent"), i18n("Do not display main window")));
    parser.addOption(QCommandLineOption(QStringLiteral("args"), i18n("Arguments for the module"), QLatin1String("arguments")));
    parser.addOption(QCommandLineOption(QStringLiteral("icon"), i18n("Use a specific icon for the window"), QLatin1String("icon")));
    parser.addOption(QCommandLineOption(QStringLiteral("caption"), i18n("Use a specific caption for the window"), QLatin1String("caption")));

    parser.parse(app.arguments());
    aboutData.processCommandLine(&parser);

    parser.process(app);

    const QString lang = parser.value(QStringLiteral("lang"));
    if (!lang.isEmpty()) {
        cout << i18n("--lang is deprecated. Please set the LANGUAGE environment variable instead").toLocal8Bit().data() << endl;
    }

    if (parser.isSet(QStringLiteral("list"))) {
        cout << i18n("The following modules are available:").toLocal8Bit().data() << endl;

        listModules();

        int maxLen=0;

        for (KService::List::ConstIterator it = m_modules.constBegin(); it != m_modules.constEnd(); ++it) {
            int len = (*it)->desktopEntryName().length();
            if (len > maxLen)
                maxLen = len;
        }

        for (KService::List::ConstIterator it = m_modules.constBegin(); it != m_modules.constEnd(); ++it) {
            QString entry(QStringLiteral("%1 - %2"));

            entry = entry.arg((*it)->desktopEntryName().leftJustified(maxLen, QLatin1Char(' ')))
                    .arg(!(*it)->comment().isEmpty() ? (*it)->comment()
                                                     : i18n("No description available"));

            cout << entry.toLocal8Bit().data() << endl;
        }
        return 0;
    }

    if (parser.positionalArguments().count() < 1) {
        parser.showHelp();
        return -1;
    }

    QString serviceName;
    KService::List modules;
    for (int i = 0; i < parser.positionalArguments().count(); i++) {
        const QString arg = parser.positionalArguments().at(i);
        KService::Ptr service = locateModule(arg);
        if (!service) {
            service = locateModule(QStringLiteral("kcm_") + arg);
        }
        if (!service) {
            service = locateModule(QStringLiteral("kcm") + arg);
        }

        if (service) {
            modules.append(service);
            if (!serviceName.isEmpty()) {
                serviceName += QLatin1Char('_');
            }
            serviceName += arg;
        } else {
            cerr << i18n("Could not find module '%1'. See kcmshell5 --list for the full list of modules.", arg).toLocal8Bit().constData() << endl;
        }
    }

    /* Check if this particular module combination is already running */
    app.setServiceName(serviceName);
    if (app.isRunning()) {
        app.waitForExit();
        return 0;
    }

    KPageDialog::FaceType ftype = KPageDialog::Plain;

    if (modules.count() < 1) {
        return -1;
    } else if (modules.count() > 1) {
        ftype = KPageDialog::List;
    }

    QStringList moduleArgs;
    const QString x = parser.value(QStringLiteral("args"));
    moduleArgs << x.split(QRegExp(QStringLiteral(" +")));

    KCMShellMultiDialog *dlg = new KCMShellMultiDialog(ftype);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    if (parser.isSet(QStringLiteral("caption"))) {
        dlg->setWindowTitle(parser.value(QStringLiteral("caption")));
    } else if (modules.count() == 1) {
        dlg->setWindowTitle(modules.first()->name());
    }

    for (KService::List::ConstIterator it = modules.constBegin(); it != modules.constEnd(); ++it) {
        dlg->addModule(*it, nullptr, moduleArgs);
    }

    if (parser.isSet(QStringLiteral("icon"))) {
        dlg->setWindowIcon(QIcon::fromTheme(parser.value(QStringLiteral("icon"))));
    } else if (!parser.isSet(QStringLiteral("icon")) && !modules.isEmpty()) {
        const QString iconName = KCModuleInfo(modules.first()).icon();
        dlg->setWindowIcon( QIcon::fromTheme(iconName) );
    }

    if (modules.count() == 1 && app.desktopFileName() == QLatin1String("org.kde.kcmshell5")) {
        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                    QStringLiteral("kservices5/%1.desktop").arg(modules.first()->desktopEntryName()));
        if (!path.isEmpty()) {
            app.setDesktopFileName(path);
        }
    }

    dlg->show();

    app.exec();

    return 0;
}
// vim: sw=4 et sts=4
