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

#include <iostream>

#include <QtCore/QFile>
#include <QtGui/QIcon>

#include <QtDBus/QtDBus>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kcmdlineargs.h>
#include <kcmoduleinfo.h>
#include <kcmoduleloader.h>
#include <kcmoduleproxy.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kservicetypetrader.h>
#include <kstartupinfo.h>
#include <kglobal.h>
#include <kicon.h>

#include "main.moc"

using namespace std;

KService::List m_modules;

static int debugArea() {
    static int s_area = KDebug::registerArea("kcmshell");
    return s_area;
}

static void listModules()
{
  const KService::List services = KServiceTypeTrader::self()->query( "KCModule", "[X-KDE-ParentApp] == 'kcontrol' or [X-KDE-ParentApp] == 'kinfocenter'" );
  for( KService::List::const_iterator it = services.begin();
       it != services.end(); ++it)
  {
      const KService::Ptr s = (*it);
      if (!KAuthorized::authorizeControlModule(s->menuId()))
          continue;
      m_modules.append(s);
  }
}

static KService::Ptr locateModule(const QString& module)
{
    QString path = module;

    if (!path.endsWith(QLatin1String(".desktop")))
        path += ".desktop";

    KService::Ptr service = KService::serviceByStorageId( path );
    if (!service) {
        return KService::Ptr();
    }

    if (!service->hasServiceType("KCModule")) {
        // Not a KCModule. E.g. "kcmshell4 akonadi" finds services/kresources/kabc/akonadi.desktop, unrelated.
        return KService::Ptr();
    }

    if ( service->noDisplay() ) {
        kDebug(debugArea()) << module << " should not be loaded.";
        return KService::Ptr();
    }

    return service;
}

bool KCMShell::isRunning()
{
    QString owner = QDBusConnection::sessionBus().interface()->serviceOwner(m_serviceName);
    if( owner == QDBusConnection::sessionBus().baseService() )
        return false; // We are the one and only.

    kDebug(debugArea()) << "kcmshell4 with modules '" <<
        m_serviceName << "' is already running." << endl;

    QDBusInterface iface(m_serviceName, "/KCModule/dialog", "org.kde.KCMShellMultiDialog");
    QDBusReply<void> reply = iface.call("activate", kapp->startupId());
    if (!reply.isValid())
    {
        kDebug(debugArea()) << "Calling D-Bus function dialog::activate() failed.";
        return false; // Error, we have to do it ourselves.
    }

    return true;
}

KCMShellMultiDialog::KCMShellMultiDialog(KPageDialog::FaceType dialogFace, QWidget *parent)
    : KCMultiDialog(parent)
{
    setFaceType(dialogFace);
    setModal(true);

    QDBusConnection::sessionBus().registerObject("/KCModule/dialog", this, QDBusConnection::ExportScriptableSlots);
}

void KCMShellMultiDialog::activate( const QByteArray& asn_id )
{
    kDebug(debugArea()) ;

#ifdef Q_WS_X11
    KStartupInfo::setNewStartupId( this, asn_id );
#endif
}

void KCMShell::setServiceName(const QString &dbusName )
{
    m_serviceName = QLatin1String( "org.kde.kcmshell_" ) + dbusName;
    QDBusConnection::sessionBus().registerService(m_serviceName);
}

void KCMShell::waitForExit()
{
    kDebug(debugArea());

    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    watcher->addWatchedService(m_serviceName);
    connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            SLOT(appExit(QString,QString,QString)));
    exec();
}

void KCMShell::appExit(const QString &appId, const QString &oldName, const QString &newName)
{
    Q_UNUSED(appId);
    Q_UNUSED(newName);
    kDebug(debugArea());

    if (!oldName.isEmpty())
    {
        kDebug(debugArea()) << "'" << appId << "' closed, dereferencing.";
        KGlobal::deref();
    }
}

extern "C" KDE_EXPORT int kdemain(int _argc, char *_argv[])
{
    KAboutData aboutData( "kcmshell", 0, ki18n("KDE Control Module"),
                          0,
                          ki18n("A tool to start single KDE control modules"),
                          KAboutData::License_GPL,
                          ki18n("(c) 1999-2004, The KDE Developers") );

    aboutData.addAuthor(ki18n("Frans Englich"), ki18n("Maintainer"), "frans.englich@kde.org");
    aboutData.addAuthor(ki18n("Daniel Molkentin"), KLocalizedString(), "molkentin@kde.org");
    aboutData.addAuthor(ki18n("Matthias Hoelzer-Kluepfel"),KLocalizedString(), "hoelzer@kde.org");
    aboutData.addAuthor(ki18n("Matthias Elter"),KLocalizedString(), "elter@kde.org");
    aboutData.addAuthor(ki18n("Matthias Ettrich"),KLocalizedString(), "ettrich@kde.org");
    aboutData.addAuthor(ki18n("Waldo Bastian"),KLocalizedString(), "bastian@kde.org");

    KCmdLineArgs::init(_argc, _argv, &aboutData);

    KCmdLineOptions options;
    options.add("list", ki18n("List all possible modules"));
    options.add("+module", ki18n("Configuration module to open"));
    options.add("lang <language>", ki18n("Specify a particular language"));
    options.add("silent", ki18n("Do not display main window"));
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KCMShell app;

    const KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    const QString lang = args->getOption("lang");
    if( !lang.isEmpty() ) {
        KGlobal::setLocale(new KLocale(aboutData.catalogName(), lang));
    }

    if (args->isSet("list"))
    {
        cout << i18n("The following modules are available:").toLocal8Bit().data() << endl;

        listModules();

        int maxLen=0;

        for( KService::List::ConstIterator it = m_modules.constBegin(); it != m_modules.constEnd(); ++it)
        {
            int len = (*it)->desktopEntryName().length();
            if (len > maxLen)
                maxLen = len;
        }

        for( KService::List::ConstIterator it = m_modules.constBegin(); it != m_modules.constEnd(); ++it)
        {
            QString entry("%1 - %2");

            entry = entry.arg((*it)->desktopEntryName().leftJustified(maxLen, ' '))
                         .arg(!(*it)->comment().isEmpty() ? (*it)->comment()
                                 : i18n("No description available"));

            cout << entry.toLocal8Bit().data() << endl;
        }
        return 0;
    }

    if (args->count() < 1)
    {
        args->usage();
        return -1;
    }

    QString serviceName;
    KService::List modules;
    for (int i = 0; i < args->count(); i++)
    {
        const QString arg = args->arg(i);
        KService::Ptr service = locateModule(arg);
        if (!service) {
            service = locateModule("kcm_" + arg);
        }
        if (!service) {
            service = locateModule("kcm" + arg);
        }

        if (service) {
            modules.append(service);
            if( !serviceName.isEmpty() )
                serviceName += '_';
            serviceName += args->arg(i);
        } else {
            fprintf(stderr, "%s\n", i18n("Could not find module '%1'. See kcmshell4 --list for the full list of modules.", arg).toLocal8Bit().constData());
        }
    }

    /* Check if this particular module combination is already running */
    app.setServiceName(serviceName);
    if( app.isRunning() ) {
        app.waitForExit();
        return 0;
    }

    KPageDialog::FaceType ftype = KPageDialog::Plain;

    if (modules.count() < 1) {
        return 0;
    } else if (modules.count() > 1) {
        ftype = KPageDialog::List;
    }

    KCMShellMultiDialog *dlg = new KCMShellMultiDialog(ftype);
    KCmdLineArgs *kdeargs = KCmdLineArgs::parsedArgs("kde");
    if (kdeargs && kdeargs->isSet("caption")) {
        dlg->setCaption(QString());
        kdeargs->clear();
    } else if (modules.count() == 1) {
        dlg->setCaption(modules.first()->name());
    }

    for (KService::List::ConstIterator it = modules.constBegin(); it != modules.constEnd(); ++it)
        dlg->addModule(*it);

    if ( !args->isSet( "icon" ) && modules.count() == 1)
    {
        QString iconName = KCModuleInfo(modules.first()).icon();
        dlg->setWindowIcon( KIcon(iconName) );
    }
    dlg->exec();
    delete dlg;

    return 0;
}
// vim: sw=4 et sts=4
