/*
  SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
  SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>

  SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "main.h"

#include <config-kde-cli-tools.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QIcon>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KAboutData>
#include <KActivities/ResourceInstance>
#include <KAuthorized>
#include <KCModuleProxy>
#include <KLocalizedString>
#include <KPluginMetaData>
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 92)
#include <KServiceTypeTrader>
#endif
#include <KStartupInfo>
#include <kworkspace.h>

#include <algorithm>
#include <iostream>

inline QVector<KPluginMetaData> findKCMsMetaData()
{
    QVector<KPluginMetaData> metaDataList = KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings_qwidgets"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/kinfocenter"));
    return metaDataList;
}
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 92)
static KService::List listModules()
{
    // First condition is what systemsettings does, second what kinfocenter does, make sure this is kept in sync
    // We need the exist calls because otherwise the trader language aborts if the property doesn't exist and the second part of the or is not evaluated
    KService::List services =
        KServiceTypeTrader::self()->query(QStringLiteral("KCModule"),
                                          QStringLiteral("(exist [X-KDE-System-Settings-Parent-Category] and [X-KDE-System-Settings-Parent-Category] != '') or "
                                                         "(exist [X-KDE-ParentApp] and [X-KDE-ParentApp] == 'kinfocenter')"));

    auto it = std::remove_if(services.begin(), services.end(), [](const KService::Ptr &service) {
        return !KAuthorized::authorizeControlModule(service->menuId());
    });
    services.erase(it, services.end());

    std::stable_sort(services.begin(), services.end(), [](const KService::Ptr s1, const KService::Ptr s2) {
        return QString::compare(s1->desktopEntryName(), s2->desktopEntryName(), Qt::CaseInsensitive) < 0;
    });

    return services;
}

static KService::Ptr locateModule(const QString &module)
{
    QString path = module;

    if (!path.endsWith(QLatin1String(".desktop"))) {
        path += QStringLiteral(".desktop");
    }

    KService::Ptr service = KService::serviceByStorageId(path);
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
#endif

bool KCMShell::isRunning()
{
    const QString owner = QDBusConnection::sessionBus().interface()->serviceOwner(m_serviceName);
    if (owner == QDBusConnection::sessionBus().baseService()) {
        return false; // We are the one and only.
    }

    qDebug() << "kcmshell5 with modules" << m_serviceName << "is already running.";

    QDBusInterface iface(m_serviceName, QStringLiteral("/KCModule/dialog"), QStringLiteral("org.kde.KCMShellMultiDialog"));
    QDBusReply<void> reply = iface.call(QStringLiteral("activate"), KStartupInfo::startupId());
    if (!reply.isValid()) {
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

    connect(this, &KCMShellMultiDialog::currentPageChanged, this, [](KPageWidgetItem *newPage, KPageWidgetItem *oldPage) {
        Q_UNUSED(oldPage);
        KCModuleProxy *activeModule = newPage->widget()->findChild<KCModuleProxy *>();
        if (activeModule) {
            KActivities::ResourceInstance::notifyAccessed(QUrl(QLatin1String("kcm:") + activeModule->metaData().pluginId()),
                                                          QStringLiteral("org.kde.systemsettings"));
        }
    });
}

void KCMShellMultiDialog::activate(const QByteArray &asn_id)
{
#ifdef HAVE_X11
    setAttribute(Qt::WA_NativeWindow, true);
    KStartupInfo::setNewStartupId(windowHandle(), asn_id);
#endif
}

void KCMShell::setServiceName(const QString &dbusName)
{
    m_serviceName = QLatin1String("org.kde.kcmshell_") + dbusName;
    QDBusConnection::sessionBus().registerService(m_serviceName);
}

void KCMShell::waitForExit()
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    watcher->addWatchedService(m_serviceName);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &KCMShell::appExit);
    exec();
}

void KCMShell::appExit(const QString &appId, const QString &oldName, const QString &newName)
{
    Q_UNUSED(appId);
    Q_UNUSED(newName);

    if (!oldName.isEmpty()) {
        qDebug() << appId << "closed, quitting.";
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
    KAboutData aboutData(QStringLiteral("kcmshell5"), //
                         i18n("System Settings Module"),
                         QLatin1String(PROJECT_VERSION),
                         i18n("A tool to start single system settings modules"),
                         KAboutLicense::GPL,
                         i18n("(c) 1999-2016, The KDE Developers"));

    aboutData.addAuthor(i18n("Frans Englich"), i18n("Maintainer"), QStringLiteral("frans.englich@kde.org"));
    aboutData.addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    aboutData.addAuthor(i18n("Matthias Hoelzer-Kluepfel"), QString(), QStringLiteral("hoelzer@kde.org"));
    aboutData.addAuthor(i18n("Matthias Elter"), QString(), QStringLiteral("elter@kde.org"));
    aboutData.addAuthor(i18n("Matthias Ettrich"), QString(), QStringLiteral("ettrich@kde.org"));
    aboutData.addAuthor(i18n("Waldo Bastian"), QString(), QStringLiteral("bastian@kde.org"));
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
        std::cout << i18n("--lang is deprecated. Please set the LANGUAGE environment variable instead").toLocal8Bit().constData() << std::endl;
    }

    if (parser.isSet(QStringLiteral("list"))) {
        std::cout << i18n("The following modules are available:").toLocal8Bit().constData() << '\n';

        QVector<KPluginMetaData> plugins = findKCMsMetaData();
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 92)
        const KService::List services = listModules();
        for (const auto &service : services) {
            const QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("/kservices5/") + service->entryPath());
            plugins << KPluginMetaData::fromDesktopFile(file);
        }
#endif
        int maxLen = 0;

        for (const auto &plugin : plugins) {
            const int len = plugin.pluginId().size();
            maxLen = std::max(maxLen, len);
        }

        for (const auto &plugin : plugins) {
            QString comment = plugin.description();
            if (comment.isEmpty()) {
                comment = i18n("No description available");
            }

            const QString entry = QStringLiteral("%1 - %2").arg(plugin.pluginId().leftJustified(maxLen, QLatin1Char(' ')), comment);

            std::cout << entry.toLocal8Bit().constData() << '\n';
        }

        std::cout << std::endl;

        return 0;
    }

    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp();
        return -1;
    }

    QString serviceName;
    QList<KPluginMetaData> metaDataList;

    QStringList args = parser.positionalArguments();
    args.removeDuplicates();
    for (const QString &arg : args) {
        KPluginMetaData data(arg);
        if (data.isValid()) {
            metaDataList << data;
        } else {
            // Look in the namespaces for systemsettings/kinfocenter
            const static auto knownKCMs = findKCMsMetaData();
            const QStringList possibleIds{arg, QStringLiteral("kcm_") + arg, QStringLiteral("kcm") + arg};
            bool foundKCM = std::any_of(knownKCMs.begin(), knownKCMs.end(), [&possibleIds, &metaDataList, &arg, &serviceName](const KPluginMetaData &data) {
                bool idMatches = possibleIds.contains(data.pluginId());
                if (idMatches) {
                    metaDataList << data;
                    if (!serviceName.isEmpty()) {
                        serviceName += QLatin1Char('_');
                    }
                    serviceName += arg;
                }
                return idMatches;
            });
            if (foundKCM) {
                continue;
            }
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 92)
            KService::Ptr service = locateModule(arg);
            if (!service) {
                service = locateModule(QStringLiteral("kcm_") + arg);
            }
            if (!service) {
                service = locateModule(QStringLiteral("kcm") + arg);
            }

            if (service) {
                if (!serviceName.isEmpty()) {
                    serviceName += QLatin1Char('_');
                }
                serviceName += arg;

                const QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("/kservices5/") + service->entryPath());
                auto data = KPluginMetaData::fromDesktopFile(file);
                metaDataList << data;
            } else {
                std::cerr << i18n("Could not find module '%1'. See kcmshell5 --list for the full list of modules.", arg).toLocal8Bit().constData() << std::endl;
            }
#endif
        }
    }

    /* Check if this particular module combination is already running */
    app.setServiceName(serviceName);
    if (app.isRunning()) {
        app.waitForExit();
        return 0;
    }

    KPageDialog::FaceType ftype = KPageDialog::Plain;

    const int modCount = metaDataList.count();
    if (modCount == 0) {
        return -1;
    }

    if (modCount > 1) {
        ftype = KPageDialog::List;
    }

    KCMShellMultiDialog *dlg = new KCMShellMultiDialog(ftype);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    if (parser.isSet(QStringLiteral("caption"))) {
        dlg->setWindowTitle(parser.value(QStringLiteral("caption")));
    } else if (modCount == 1) {
        dlg->setWindowTitle(metaDataList.constFirst().name());
    }

    const QStringList moduleArgs = parser.value(QStringLiteral("args")).split(QRegularExpression(QStringLiteral(" +")));
    for (const KPluginMetaData &m : std::as_const(metaDataList)) {
        dlg->addModule(m, moduleArgs);
    }

    if (parser.isSet(QStringLiteral("icon"))) {
        dlg->setWindowIcon(QIcon::fromTheme(parser.value(QStringLiteral("icon"))));
    } else {
        dlg->setWindowIcon(QIcon::fromTheme(metaDataList.constFirst().iconName()));
    }

    if (app.desktopFileName() == QLatin1String("org.kde.kcmshell5")) {
        const QString path = metaDataList.constFirst().metaDataFileName();

        if (path.endsWith(QLatin1String(".desktop"))) {
            app.setDesktopFileName(path);
        } else {
            app.setDesktopFileName(metaDataList.constFirst().pluginId());
        }
    }

    dlg->show();

    app.exec();

    return 0;
}
// vim: sw=4 et sts=4
