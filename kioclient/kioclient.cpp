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

#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
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
#include <QTimer>
#include <krun.h>
#include <QtDBus/QtDBus>
#include <kcomponentdata.h>
#include <iostream>

static const char appName[] = "kioclient";
static const char programName[] = I18N_NOOP("KIO Client");
static const char description[] = I18N_NOOP("Command-line tool for network-transparent operations");
static const char version[] = "2.0";

bool ClientApp::m_ok = true;
static bool s_interactive = true;
static KIO::JobFlags s_jobFlags = KIO::DefaultFlags;

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
              "            #   document, or it may be a *.desktop file.\n"
              "            #   'url' can be an executable, too.\n").toLocal8Bit());
    puts(i18n("  kioclient move 'src' 'dest'\n"
              "            # Moves the URL 'src' to 'dest'.\n"
              "            #   'src' may be a list of URLs.\n").toLocal8Bit());
    puts(i18n("            #   'dest' may be \"trash:/\" to move the files\n"
              "            #   to the trash.\n").toLocal8Bit());
    puts(i18n("            #   the short version kioclient mv\n"
              "            #   is also available.\n\n").toLocal8Bit());
    puts(i18n("  kioclient download ['src']\n"
              "            # Copies the URL 'src' to a user-specified location'.\n"
              "            #   'src' may be a list of URLs, if not present then\n"
              "            #   a URL will be requested.\n\n").toLocal8Bit());
    puts(i18n("  kioclient copy 'src' 'dest'\n"
              "            # Copies the URL 'src' to 'dest'.\n"
              "            #   'src' may be a list of URLs.\n").toLocal8Bit());
    puts(i18n("            #   the short version kioclient cp\n"
              "            #   is also available.\n\n").toLocal8Bit());
    puts(i18n("  kioclient cat 'url'\n"
              "            # Writes out the contents of 'url' to stdout\n\n").toLocal8Bit());
    puts(i18n("  kioclient ls 'url'\n"
              "            # Lists the contents of the directory 'url' to stdout\n\n").toLocal8Bit());
    puts(i18n("  kioclient remove 'url'\n"
              "            # Removes the URL\n"
              "            #   'url' may be a list of URLs.\n").toLocal8Bit());
    puts(i18n("            #   the short version kioclient rm\n"
              "            #   is also available.\n\n").toLocal8Bit());

    puts(i18n("*** Examples:\n"
              "  kioclient exec file:/root/Desktop/cdrom.desktop \"Mount default\"\n"
              "             // Mounts the CDROM\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec file:/home/weis/data/test.html\n"
              "             // Opens the file with default binding\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec file:/home/weis/data/test.html Netscape\n"
              "             // Opens the file with netscape\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec ftp://localhost/\n"
              "             // Opens new window with URL\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec file:/root/Desktop/emacs.desktop\n"
              "             // Starts emacs\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec file:/root/Desktop/cdrom.desktop\n"
              "             // Opens the CDROM's mount directory\n\n").toLocal8Bit());
    puts(i18n("  kioclient exec .\n"
              "             // Opens the current directory. Very convenient.\n\n").toLocal8Bit());
}
#endif

int main( int argc, char **argv )
{
  KCmdLineArgs::init(argc, argv, appName, 0, ki18n(programName), version, ki18n(description), KCmdLineArgs::CmdLineArgNone);


  KCmdLineOptions options;
  options.add("noninteractive", ki18n("Non-interactive use: no message boxes"));
  #if !defined(KIOCLIENT_AS_KDEOPEN)
  options.add("overwrite", ki18n("Overwrite destination if it exists (for copy and move)"));
  #endif
  #if defined(KIOCLIENT_AS_KDEOPEN)
  options.add("+url", ki18n("file or URL"));
  #elif defined(KIOCLIENT_AS_KDECP)
  options.add("+src", ki18n("Source URL or URLs"));
  options.add("+dest", ki18n("Destination URL"));
  #elif defined(KIOCLIENT_AS_KDEMV)
  options.add("+src", ki18n("Source URL or URLs"));
  options.add("+dest", ki18n("Destination URL"));
  #elif defined(KIOCLIENT_AS_KIOCLIENT)
  options.add("commands", ki18n("Show available commands"));
  options.add("+command", ki18n("Command (see --commands)"));
  options.add("+[URL(s)]", ki18n("Arguments for command"));
  #endif

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
        fputs( i18nc("@info:shell", "%1: Syntax error, not enough arguments\n", appName).toLocal8Bit(), stderr );
        ::exit(1);
    }
    if (max && (count > max))
    {
        fputs( i18nc("@info:shell", "%1: Syntax error, too many arguments\n", appName).toLocal8Bit(), stderr );
        ::exit(1);
    }
}

bool ClientApp::kde_open(const KUrl& url, const QString& mimeType, bool allowExec)
{
    if ( mimeType.isEmpty() ) {
        kDebug() << url;
        KRun * run = new KRun( url, 0 );
        run->setRunExecutables(allowExec);
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
    KIO::Job * job = KIO::copy( srcLst, args->url(argc - 1), s_jobFlags );
    if ( !s_interactive )
        job->setUiDelegate( 0 );
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
    this->exec();
    return m_ok;
}

void ClientApp::slotEntries(KIO::Job* job, const KIO::UDSEntryList& list)
{
    KUrl url = static_cast<KIO::ListJob*>( job )->url();
    KIO::UDSEntryList::ConstIterator it=list.begin();
    for (; it != list.end(); ++it) {
        // For each file...
        QString name = (*it).stringValue( KIO::UDSEntry::UDS_NAME );
        std::cout << qPrintable(name) << std::endl;
    }
}

bool ClientApp::doList( int firstArg )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KUrl dir = args->url(firstArg);
    KIO::Job * job = KIO::listDir(dir, KIO::HideProgressInfo);
    if ( !s_interactive )
        job->setUiDelegate(0);
    connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
            SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
    connect(job, SIGNAL(result(KJob *)), this, SLOT(slotResult(KJob *)));
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

    KIO::Job * job = KIO::move( srcLst, args->url(argc - 1), s_jobFlags );
    if ( !s_interactive )
        job->setUiDelegate( 0 );
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
    this->exec();
    return m_ok;
}

bool ClientApp::doRemove( int firstArg )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int argc = args->count();
    KUrl::List srcLst;
    for ( int i = firstArg; i < argc; i++ )
      srcLst.append( args->url(i) );
    KIO::Job * job = KIO::del( srcLst, s_jobFlags );
    if ( !s_interactive )
        job->setUiDelegate( 0 );
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
    this->exec();
    return m_ok;
}

bool ClientApp::doIt()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    const int argc = args->count();
    checkArgumentCount(argc, 1, 0);

    if ( !args->isSet( "ninteractive" ) ) {
        s_interactive = false;
        s_jobFlags = KIO::HideProgressInfo;
    }
#if !defined(KIOCLIENT_AS_KDEOPEN)
    if (args->isSet("overwrite")) {
        s_jobFlags |= KIO::Overwrite;
    }
#endif

    kDebug() << "Creating ClientApp";
    int fake_argc = 0;
    char** fake_argv = 0;
    ClientApp app( fake_argc, fake_argv );
    KComponentData componentData("kioclient"); // needed by KIO's internal use of KConfig
    app.setApplicationName(componentData.componentName());
    KGlobal::ref();
    KGlobal::setAllowQuit(true);

    // KIO needs dbus (for uiserver communication)
    extern void qDBusBindToApplication();
    qDBusBindToApplication();
    if (!QDBusConnection::sessionBus().isConnected())
        kFatal(101) << "Session bus not found" ;

#ifdef KIOCLIENT_AS_KDEOPEN
    return app.kde_open(args->url(0), QByteArray(), false);
#elif defined(KIOCLIENT_AS_KDECP)
    checkArgumentCount(argc, 2, 0);
    return app.doCopy(0);
#elif defined(KIOCLIENT_AS_KDEMV)
    checkArgumentCount(argc, 2, 0);
    return app.doMove(0);
#else
    // Normal kioclient mode
    const QByteArray command = args->arg(0).toLocal8Bit();
    if ( command == "openProperties" )
    {
        checkArgumentCount(argc, 2, 2); // openProperties <url>
        KPropertiesDialog * p = new KPropertiesDialog( args->url(1), 0 /*no parent*/ );
        QObject::connect( p, SIGNAL( destroyed() ), &app, SLOT( quit() ));
        QObject::connect( p, SIGNAL( canceled() ), &app, SLOT( slotDialogCanceled() ));
        p->show();
        app.exec();
        return m_ok;
    }
    else if ( command == "cat" )
    {
        checkArgumentCount(argc, 2, 2); // cat <url>
        KIO::TransferJob* job = KIO::get(args->url(1), KIO::NoReload, s_jobFlags);
        if ( !s_interactive )
            job->setUiDelegate( 0 );
        connect(job, SIGNAL(data(KIO::Job*,QByteArray) ), &app, SLOT(slotPrintData(KIO::Job*,QByteArray)));
        connect( job, SIGNAL( result( KJob * ) ), &app, SLOT( slotResult( KJob * ) ) );
        app.exec();
        return m_ok;
    }
    else if ( command == "exec" )
    {
        checkArgumentCount(argc, 2, 3);
        return app.kde_open( args->url( 1 ),
                             argc == 3 ? args->arg( 2 ) : QString(),
                             true );
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
            KFileDialog::getSaveFileName( (argc<2) ? QString() : (args->url(1).fileName()) );
        if (dst.isEmpty()) // canceled
            return m_ok; // AK - really okay?
        KUrl dsturl;
        dsturl.setPath( dst );
        KIO::Job * job = KIO::copy( srcLst, dsturl, s_jobFlags );
        if ( !s_interactive )
            job->setUiDelegate( 0 );
        connect( job, SIGNAL( result( KJob * ) ), &app, SLOT( slotResult( KJob * ) ) );
        app.exec();
        return m_ok;
    }
    else if ( command == "copy" || command == "cp" )
    {
        checkArgumentCount(argc, 3, 0); // cp <src> <dest>
        return app.doCopy( 1 );
    }
    else if ( command == "move" || command == "mv" )
    {
        checkArgumentCount(argc, 3, 0); // mv <src> <dest>
        return app.doMove( 1 );
    }
    else if ( command == "list" || command == "ls" )
    {
        checkArgumentCount(argc, 2, 2); // ls <url>
        return app.doList( 1 );
    }
    else if ( command == "remove" || command == "rm" )
    {
        checkArgumentCount(argc, 2, 0); // rm <url>
        return app.doRemove( 1 );
    }
    else
    {
        fputs( i18nc("@info:shell", "%1: Syntax error, unknown command '%2'\n", appName, QString::fromLocal8Bit(command)).toLocal8Bit().data(), stderr );
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

void ClientApp::slotPrintData(KIO::Job*, const QByteArray &data)
{
    if (!data.isEmpty())
        std::cout.write(data.constData(), data.size());
}

ClientApp::ClientApp(int &argc, char **argv )
    : QApplication( argc, argv, s_interactive /*-> bool GUIenabled*/ )
{
}

#include "kioclient.moc"
