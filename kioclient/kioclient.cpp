/* This file is part of the KDE project
   Copyright (C) 1999-2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kioclient.h"

#include <ktoolinvocation.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <kcmdlineargs.h>
#include <kpropertiesdialog.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurlrequesterdialog.h>
#include <kmessagebox.h>
#include <kmimetypetrader.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kservice.h>
#include <kstaticdeleter.h>
#include <QTimer>
#include <krun.h>
#include <QtDBus/QtDBus>
#include <kcomponentdata.h>

static const char appName[] = "kioclient";
static const char programName[] = I18N_NOOP("KIO Client");
static const char description[] = I18N_NOOP("Command-line tool for network-transparent operations");
static const char version[] = "2.0";

#if defined(KIOCLIENT_AS_KDEOPEN)
static const KCmdLineOptions options[] =
{
    { "noninteractive", I18N_NOOP("Non interactive use: no message boxes"), 0},
    { "+urls", I18N_NOOP("url or urls"), 0 },
    KCmdLineLastOption
};
#elif defined(KIOCLIENT_AS_KDECP)
static const KCmdLineOptions options[] =
{
    { "noninteractive", I18N_NOOP("Non interactive use: no message boxes"), 0},
    { "+src", I18N_NOOP("Source url or urls"), 0 },
    { "+dest", I18N_NOOP("Destination url"), 0 },
    KCmdLineLastOption
};
#elif defined(KIOCLIENT_AS_KDEMV)
static const KCmdLineOptions options[] =
{
    { "noninteractive", I18N_NOOP("Non interactive use: no message boxes"), 0},
    { "+src", I18N_NOOP("Source url or urls"), 0 },
    { "+dest", I18N_NOOP("Destination url"), 0 },
    KCmdLineLastOption
};
#elif defined(KIOCLIENT_AS_KIOCLIENT)
static const KCmdLineOptions options[] =
{
    { "noninteractive", I18N_NOOP("Non interactive use: no message boxes"), 0},
    { "commands", I18N_NOOP("Show available commands"), 0},
    { "+command", I18N_NOOP("Command (see --commands)"), 0},
    { "+[URL(s)]", I18N_NOOP("Arguments for command"), 0},
    KCmdLineLastOption
};
#endif

bool ClientApp::m_ok = true;
static bool s_interactive = true;

#ifdef KIOCLIENT_AS_KIOCLIENT
static void usage()
{
    KCmdLineArgs::enable_i18n();
    puts(i18n("\nSyntax:\n").toLocal8Bit());
    puts(i18n("  kioclient openProperties 'url'\n"
              "            # Opens a properties menu\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec 'url' ['mimetype']\n"
              "            # Tries to open the document pointed to by 'url', in the application\n"
              "            #   associated with it in KDE. You may omit 'mimetype'.\n"
              "            #   In this case the mimetype is determined\n"
              "            #   automatically. Of course URL may be the URL of a\n"
              "            #   document, or it may be a *.desktop file.\n").toLocal8Bit());
    puts(i18n("  kioclient move 'src' 'dest'\n"
              "            # Moves the URL 'src' to 'dest'.\n"
              "            #   'src' may be a list of URLs.\n").toLocal8Bit());
    //puts(i18n("            #   'dest' may be \"trash:/\" to move the files\n"
    //            "            #   in the trash bin.\n\n").toLocal8Bit());
    puts(i18n("  kioclient download ['src']\n"
              "            # Copies the URL 'src' to a user specified location'.\n"
              "            #   'src' may be a list of URLs, if not present then\n"
              "            #   a URL will be requested.\n\n").toLocal8Bit());
    puts(i18n("  kioclient copy 'src' 'dest'\n"
              "            # Copies the URL 'src' to 'dest'.\n"
              "            #   'src' may be a list of URLs.\n\n").toLocal8Bit());

    puts(i18n("*** Examples:\n"
              "  kioclient exec file:/root/Desktop/cdrom.desktop \"Mount default\"\n"
              "             // Mounts the CD-ROM\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec file:/home/weis/data/test.html\n"
              "             // Opens the file with default binding\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec file:/home/weis/data/test.html Netscape\n"
              "             // Opens the file with netscape\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec ftp://localhost/\n"
              "             // Opens new window with URL\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec file:/root/Desktop/emacs.desktop\n"
              "             // Starts emacs\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec file:/root/Desktop/cdrom.desktop\n"
              "             // Opens the CD-ROM's mount directory\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec .\n"
              "             // Opens the current directory. Very convenient.\n\n").toLocal8Bit());
}
#endif

int main( int argc, char **argv )
{
  KCmdLineArgs::init(argc, argv, appName, programName, description, version, false);

  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs::addTempFileOption();

#ifdef KIOCLIENT_AS_KIOCLIENT
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( argc == 1 || args->isSet("commands") )
  {
    usage();
    return 0;
  }
#endif

  return ClientApp::doIt() ? 0 /*no error*/ : 1 /*error*/;
}

bool krun_has_error = false;

void ClientApp::delayedQuit()
{
    // Quit in 2 seconds. This leaves time for KRun to pop up
    // "app not found" in KProcessRunner, if that was the case.
    QTimer::singleShot( 2000, this, SLOT(deref()) );
    // don't access the KRun instance later, it will be deleted after calling slots
    if( static_cast< const KRun* >( sender())->hasError())
        krun_has_error = true;
}

static void checkArgumentCount(int count, int min, int max)
{
    if (count < min)
    {
        fputs( i18n("Syntax Error: Not enough arguments\n").toLocal8Bit(), stderr );
        ::exit(1);
    }
    if (max && (count > max))
    {
        fputs( i18n("Syntax Error: Too many arguments\n").toLocal8Bit(), stderr );
        ::exit(1);
    }
}

bool ClientApp::kde_open( const KUrl& url, const QString& mimeType )
{
    if ( mimeType.isEmpty() ) {
        kDebug() << k_funcinfo << url << endl;
        KRun * run = new KRun( url, 0 );
        QObject::connect( run, SIGNAL( finished() ), this, SLOT( delayedQuit() ));
        QObject::connect( run, SIGNAL( error() ), this, SLOT( delayedQuit() ));
        this->exec();
        return !krun_has_error;
    } else {
        KUrl::List urls;
        urls.append( url );
        const KService::List offers = KMimeTypeTrader::self()->query(
            mimeType, QLatin1String( "Application" ) );
        if (offers.isEmpty()) return 1;
        KService::Ptr serv = offers.first();
        return KRun::run( *serv, urls, 0 );
    }
}

bool ClientApp::doCopy( int firstArg )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int argc = args->count();
    KUrl::List srcLst;
    for ( int i = firstArg; i <= argc - 2; i++ )
      srcLst.append( args->url(i) );
    KIO::Job * job = KIO::copy( srcLst, args->url(argc - 1) );
    if ( !s_interactive )
        job->setUiDelegate( 0 );
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
    this->exec();
    return m_ok;
}

bool ClientApp::doMove( int firstArg )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int argc = args->count();
    KUrl::List srcLst;
    for ( int i = firstArg; i <= argc - 2; i++ )
      srcLst.append( args->url(i) );

    KIO::Job * job = KIO::move( srcLst, args->url(argc - 1) );
    if ( !s_interactive )
        job->setUiDelegate( 0 );
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
    this->exec();
    return m_ok;
}

bool ClientApp::doIt()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int argc = args->count();
    checkArgumentCount(argc, 1, 0);

    if ( !args->isSet( "ninteractive" ) ) {
        s_interactive = false;
    }

    kDebug() << "Creating ClientApp" << endl;
    int fake_argc = 0;
    char** fake_argv = 0;
    ClientApp app( fake_argc, fake_argv );
    KComponentData componentData("kioclient"); // needed by KIO's internal use of KConfig
    app.setApplicationName(componentData.componentName());
    app.setQuitOnLastWindowClosed( false );

    // KIO needs dbus (for uiserver communication)
    extern void qDBusBindToApplication();
    qDBusBindToApplication();
    if (!QDBusConnection::sessionBus().isConnected())
        kFatal(101) << "Session bus not found" << endl;


#ifdef KIOCLIENT_AS_KDEOPEN
    app.kde_open( args->url(0), QByteArray() );
    return true;
#elif defined(KIOCLIENT_AS_KDECP)
    checkArgumentCount(argc, 2, 0);
    return app.doCopy(0);
#elif defined(KIOCLIENT_AS_KDEMV)
    checkArgumentCount(argc, 2, 0);
    return app.doMove(0);
#else
    // Normal kioclient mode
    const QByteArray command = args->arg(0);
    if ( command == "openProperties" )
    {
        checkArgumentCount(argc, 2, 2);
        KPropertiesDialog * p = new KPropertiesDialog( args->url(1) );
        QObject::connect( p, SIGNAL( destroyed() ), &app, SLOT( quit() ));
        QObject::connect( p, SIGNAL( canceled() ), &app, SLOT( slotDialogCanceled() ));
        p->show();
        app.exec();
        return m_ok;
    }
    else if ( command == "exec" )
    {
        checkArgumentCount(argc, 2, 3);
        app.kde_open( args->url( 1 ),
                      argc == 3 ? QString::fromLocal8Bit( args->arg( 2 ) ) : QString() );
    }
    else if ( command == "download" )
    {
        checkArgumentCount(argc, 0, 0);
        KUrl::List srcLst;
        if (argc == 1) {
            while(true) {
                KUrl src = KUrlRequesterDialog::getUrl();
                if (!src.isEmpty()) {
                    if (!src.isValid()) {
                        KMessageBox::error(0, i18n("Unable to download from an invalid URL."));
                        continue;
                    }
                    srcLst.append(src);
                }
                break;
            }
        } else {
            for ( int i = 1; i <= argc - 1; i++ )
                srcLst.append( args->url(i) );
        }
        if (srcLst.count() == 0)
            return m_ok;
        QString dst =
            KFileDialog::getSaveFileName( (argc<2) ? (QString::null) : (args->url(1).fileName()) );
        if (dst.isEmpty()) // canceled
            return m_ok; // AK - really okay?
        KUrl dsturl;
        dsturl.setPath( dst );
        KIO::Job * job = KIO::copy( srcLst, dsturl );
        if ( !s_interactive )
            job->setUiDelegate( 0 );
        connect( job, SIGNAL( result( KJob * ) ), &app, SLOT( slotResult( KJob * ) ) );
        app.exec();
        return m_ok;
    }
    else if ( command == "copy" )
    {
        checkArgumentCount(argc, 2, 0);
        return app.doCopy( 1 );
    }
    else if ( command == "move" )
    {
        checkArgumentCount(argc, 2, 0);
        return app.doMove( 1 );
    }
    else
    {
        fprintf( stderr, "%s", i18n("Syntax Error: Unknown command '%1'\n", QString::fromLocal8Bit(command)).toLocal8Bit().data() );
        return false;
    }
    return true;
#endif
}

void ClientApp::slotResult( KJob * job )
{
    if (job->error() && s_interactive)
        static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
    m_ok = !job->error();
    quit();
}

void ClientApp::slotDialogCanceled()
{
    m_ok = false;
    quit();
}

void ClientApp::deref()
{
    KGlobal::deref();
}

#include "kioclient.moc"
