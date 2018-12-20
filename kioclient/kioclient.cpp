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

#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <kio/listjob.h>
#include <kio/transferjob.h>
#ifndef KIOCORE_ONLY
#include <KIO/JobUiDelegate>
#include <KPropertiesDialog>
#include <KService>
#include <KRun>
#include <KMimeTypeTrader>
#endif
#include <KAboutData>
#include <KLocalizedString>

#include <QDBusConnection>
#include <QCommandLineParser>
#include <QFileDialog>
#include <QDebug>
#include <iostream>

bool ClientApp::m_ok = true;
static bool s_interactive = true;
static KIO::JobFlags s_jobFlags = KIO::DefaultFlags;

static QUrl makeURL(const QString &urlArg)
{
    return QUrl::fromUserInput(urlArg, QDir::currentPath());
}

static QList<QUrl> makeUrls(const QStringList& urlArgs)
{
    QList<QUrl> ret;
    foreach(const QString& url, urlArgs) {
        ret += makeURL(url);
    }
    return ret;
}

#ifdef KIOCLIENT_AS_KIOCLIENT5
static void usage()
{
    puts(i18n("\nSyntax:\n").toLocal8Bit().constData());
    puts(i18n("  kioclient openProperties 'url'\n"
              "            # Opens a properties menu\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient exec 'url' ['mimetype']\n"
              "            # Tries to open the document pointed to by 'url', in the application\n"
              "            #   associated with it in KDE. You may omit 'mimetype'.\n"
              "            #   In this case the mimetype is determined\n"
              "            #   automatically. Of course URL may be the URL of a\n"
              "            #   document, or it may be a *.desktop file.\n"
              "            #   'url' can be an executable, too.\n").toLocal8Bit().constData());
    puts(i18n("  kioclient move 'src' 'dest'\n"
              "            # Moves the URL 'src' to 'dest'.\n"
              "            #   'src' may be a list of URLs.\n").toLocal8Bit().constData());
    puts(i18n("            #   'dest' may be \"trash:/\" to move the files\n"
              "            #   to the trash.\n").toLocal8Bit().constData());
    puts(i18n("            #   the short version kioclient mv\n"
              "            #   is also available.\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient download ['src']\n"
              "            # Copies the URL 'src' to a user-specified location'.\n"
              "            #   'src' may be a list of URLs, if not present then\n"
              "            #   a URL will be requested.\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient copy 'src' 'dest'\n"
              "            # Copies the URL 'src' to 'dest'.\n"
              "            #   'src' may be a list of URLs.\n").toLocal8Bit().constData());
    puts(i18n("            #   the short version kioclient cp\n"
              "            #   is also available.\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient cat 'url'\n"
              "            # Writes out the contents of 'url' to stdout\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient ls 'url'\n"
              "            # Lists the contents of the directory 'url' to stdout\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient remove 'url'\n"
              "            # Removes the URL\n"
              "            #   'url' may be a list of URLs.\n").toLocal8Bit().constData());
    puts(i18n("            #   the short version kioclient rm\n"
              "            #   is also available.\n\n").toLocal8Bit().constData());

    puts(i18n("*** Examples:\n").toLocal8Bit().constData());
    puts(i18n("  kioclient exec file:/home/weis/data/test.html\n"
              "             // Opens the file with default binding\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient exec ftp://localhost/\n"
              "             // Opens new window with URL\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient exec file:/root/Desktop/emacs.desktop\n"
              "             // Starts emacs\n\n").toLocal8Bit().constData());
    puts(i18n("  kioclient exec .\n"
              "             // Opens the current directory. Very convenient.\n\n").toLocal8Bit().constData());
}
#endif

int main( int argc, char **argv )
{
#ifdef KIOCORE_ONLY
  QCoreApplication app(argc, argv);
#else
  QApplication app(argc, argv);
#endif

  KLocalizedString::setApplicationDomain("kioclient5");

  QString appName = QStringLiteral("kioclient");
  QString programName = i18n("KIO Client");
  QString description = i18n("Command-line tool for network-transparent operations");
  QString version = QLatin1String(PROJECT_VERSION);
  KAboutData data(appName, programName, version, description, KAboutLicense::LGPL_V2);
  KAboutData::setApplicationData(data);

  QCommandLineParser parser;
  data.setupCommandLine(&parser);
  parser.addOption(QCommandLineOption(QStringLiteral("noninteractive"), i18n("Non-interactive use: no message boxes. If you don't want a "
                                                             "graphical connection, use --platform offscreen")));

  #if !defined(KIOCLIENT_AS_KDEOPEN)
  parser.addOption(QCommandLineOption(QStringLiteral("overwrite"), i18n("Overwrite destination if it exists (for copy and move)")));
  #endif

  #if defined(KIOCLIENT_AS_KDEOPEN)
  parser.addPositionalArgument(QStringLiteral("url"), i18n("file or URL"), i18n("urls..."));
  #elif defined(KIOCLIENT_AS_KDECP5)
  parser.addPositionalArgument(QStringLiteral("src"), i18n("Source URL or URLs"), i18n("urls..."));
  parser.addPositionalArgument(QStringLiteral("dest"), i18n("Destination URL"), i18n("url"));
  #elif defined(KIOCLIENT_AS_KDEMV5)
  parser.addPositionalArgument(QStringLiteral("src"), i18n("Source URL or URLs"), i18n("urls..."));
  parser.addPositionalArgument(QStringLiteral("dest"), i18n("Destination URL"), i18n("url"));
  #elif defined(KIOCLIENT_AS_KIOCLIENT5)
  parser.addOption(QCommandLineOption(QStringLiteral("commands"), i18n("Show available commands")));
  parser.addPositionalArgument(QStringLiteral("command"), i18n("Command (see --commands)"), i18n("command"));
  parser.addPositionalArgument(QStringLiteral("URLs"), i18n("Arguments for command"), i18n("urls..."));
  #endif

//   KCmdLineArgs::addTempFileOption();

  parser.process(app);
  data.processCommandLine(&parser);

#ifdef KIOCLIENT_AS_KIOCLIENT5
  if ( argc == 1 || parser.isSet(QStringLiteral("commands")) )
  {
    puts(parser.helpText().toLocal8Bit().constData());
    puts("\n\n");
    usage();
    return 0;
  }
#endif

  ClientApp client;
  return client.doIt(parser) ? 0 /*no error*/ : 1 /*error*/;
}

static bool krun_has_error = false;

void ClientApp::delayedQuit()
{
#ifndef KIOCORE_ONLY
    // don't access the KRun instance later, it will be deleted after calling slots
    if( static_cast< const KRun* >( sender())->hasError())
        krun_has_error = true;
#endif
}

static void checkArgumentCount(int count, int min, int max)
{
    if (count < min)
    {
        fputs( i18nc("@info:shell", "%1: Syntax error, not enough arguments\n", qAppName()).toLocal8Bit().constData(), stderr );
        ::exit(1);
    }
    if (max && (count > max))
    {
        fputs( i18nc("@info:shell", "%1: Syntax error, too many arguments\n", qAppName()).toLocal8Bit().constData(), stderr );
        ::exit(1);
    }
}

#ifndef KIOCORE_ONLY
bool ClientApp::kde_open(const QUrl& url, const QString& mimeType, bool allowExec)
{
    if ( mimeType.isEmpty() ) {
        KRun * run = new KRun( url, nullptr );
        run->setRunExecutables(allowExec);
        QObject::connect( run, SIGNAL( finished() ), this, SLOT( delayedQuit() ));
        QObject::connect( run, SIGNAL( error() ), this, SLOT( delayedQuit() ));
        qApp->exec();
        return !krun_has_error;
    } else {
        return KRun::runUrl(url, mimeType, nullptr, KRun::RunFlags(KRun::RunExecutables));
    }
}
#endif

bool ClientApp::doCopy( const QStringList& urls )
{
    QList<QUrl> srcLst(makeUrls(urls));
    QUrl dest = srcLst.takeLast();
    KIO::Job * job = KIO::copy( srcLst, dest, s_jobFlags );
    if ( !s_interactive ) {
        job->setUiDelegate( nullptr );
        job->setUiDelegateExtension( nullptr );
    }
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
    qApp->exec();
    return m_ok;
}

void ClientApp::slotEntries(KIO::Job*, const KIO::UDSEntryList& list)
{
    KIO::UDSEntryList::ConstIterator it=list.begin();
    for (; it != list.end(); ++it) {
        // For each file...
        QString name = (*it).stringValue( KIO::UDSEntry::UDS_NAME );
        std::cout << qPrintable(name) << std::endl;
    }
}

bool ClientApp::doList( const QStringList& urls )
{
    QUrl dir = makeURL(urls.first());
    KIO::Job * job = KIO::listDir(dir, KIO::HideProgressInfo);
    if ( !s_interactive ) {
        job->setUiDelegate( nullptr );
        job->setUiDelegateExtension( nullptr );
    }
    connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
            SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
    connect(job, SIGNAL(result(KJob *)), this, SLOT(slotResult(KJob *)));
    qApp->exec();
    return m_ok;
}

bool ClientApp::doMove( const QStringList& urls )
{
    QList<QUrl> srcLst(makeUrls(urls));
    QUrl dest = srcLst.takeLast();
    KIO::Job * job = KIO::move( srcLst, dest, s_jobFlags );
    if ( !s_interactive ) {
        job->setUiDelegate( nullptr );
        job->setUiDelegateExtension( nullptr );
    }
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
    qApp->exec();
    return m_ok;
}

bool ClientApp::doRemove( const QStringList& urls )
{
    KIO::Job * job = KIO::del( makeUrls(urls), s_jobFlags );
    if ( !s_interactive ) {
        job->setUiDelegate( nullptr );
        job->setUiDelegateExtension( nullptr );
    }
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
    qApp->exec();
    return m_ok;
}

bool ClientApp::doIt(const QCommandLineParser& parser)
{
    const int argc = parser.positionalArguments().count();
    checkArgumentCount(argc, 1, 0);

    if ( !parser.isSet( QStringLiteral("noninteractive") ) ) {
        s_interactive = false;
        s_jobFlags = KIO::HideProgressInfo;
    }
#if !defined(KIOCLIENT_AS_KDEOPEN)
    if (parser.isSet(QStringLiteral("overwrite"))) {
        s_jobFlags |= KIO::Overwrite;
    }
#endif

#ifdef KIOCLIENT_AS_KDEOPEN
    return kde_open(makeURL(parser.positionalArguments().at(0)), QString(), false);
#elif defined(KIOCLIENT_AS_KDECP5)
    checkArgumentCount(argc, 2, 0);
    return doCopy(parser.positionalArguments());
#elif defined(KIOCLIENT_AS_KDEMV5)
    checkArgumentCount(argc, 2, 0);
    return doMove(parser.positionalArguments());
#else
    // Normal kioclient mode
    QString command = parser.positionalArguments().first();
#ifndef KIOCORE_ONLY
    if ( command == QLatin1String("openProperties") )
    {
        checkArgumentCount(argc, 2, 2); // openProperties <url>
        QUrl url = makeURL(parser.positionalArguments().last());
        KPropertiesDialog * p = new KPropertiesDialog(url, nullptr /*no parent*/ );
        QObject::connect( p, SIGNAL( destroyed() ), qApp, SLOT( quit() ));
        QObject::connect( p, SIGNAL( canceled() ), this, SLOT( slotDialogCanceled() ));
        p->show();
        qApp->exec();
        return m_ok;
    }
    else
#endif
        if ( command == QLatin1String("cat") )
    {
        checkArgumentCount(argc, 2, 2); // cat <url>
        QUrl url = makeURL(parser.positionalArguments().last());
        KIO::TransferJob* job = KIO::get(url, KIO::NoReload, s_jobFlags);
        if ( !s_interactive ) {
            job->setUiDelegate( nullptr );
            job->setUiDelegateExtension( nullptr );
        }
        connect(job, SIGNAL(data(KIO::Job*,QByteArray) ), this, SLOT(slotPrintData(KIO::Job*,QByteArray)));
        connect(job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
        qApp->exec();
        return m_ok;
    }
#ifndef KIOCORE_ONLY
    else if ( command ==QLatin1String( "exec") )
    {
        checkArgumentCount(argc, 2, 3);
        return kde_open( makeURL(parser.positionalArguments()[1]),
                             argc == 3 ? parser.positionalArguments().last() : QString(),
                             true );
    }
#endif
    else if ( command == QLatin1String("download") )
    {
        checkArgumentCount(argc, 0, 0);
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        QList<QUrl> srcLst = makeUrls(args);

        if (srcLst.isEmpty())
            return m_ok;
        QUrl dsturl = QFileDialog::getSaveFileUrl(nullptr, i18n("Destination where to download the files"), (!srcLst.isEmpty()) ? QUrl() : srcLst.first() );

        if (dsturl.isEmpty()) // canceled
            return m_ok; // AK - really okay?
        KIO::Job * job = KIO::copy( srcLst, dsturl, s_jobFlags );
        if ( !s_interactive ) {
            job->setUiDelegate( nullptr );
            job->setUiDelegateExtension( nullptr );
        }
        connect( job, SIGNAL( result( KJob * ) ), qApp, SLOT( slotResult( KJob * ) ) );
        qApp->exec();
        return m_ok;
    }
    else if ( command == QLatin1String("copy") || command == QLatin1String("cp") )
    {
        checkArgumentCount(argc, 3, 0); // cp <src> <dest>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doCopy(args);
    }
    else if ( command == QLatin1String("move") || command == QLatin1String("mv") )
    {
        checkArgumentCount(argc, 3, 0); // mv <src> <dest>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doMove(args);
    }
    else if ( command == QLatin1String("list") || command == QLatin1String("ls") )
    {
        checkArgumentCount(argc, 2, 2); // ls <url>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doList(args);
    }
    else if ( command == QLatin1String("remove") || command == QLatin1String("rm") )
    {
        checkArgumentCount(argc, 2, 0); // rm <url>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doRemove(args);
    }
    else
    {
        fputs( i18nc("@info:shell", "%1: Syntax error, unknown command '%2'\n", qAppName(), command).toLocal8Bit().data(), stderr );
        return false;
    }
    Q_UNREACHABLE();
#endif
}

void ClientApp::slotResult( KJob * job )
{
    if (job->error()) {
#ifndef KIOCORE_ONLY
        if (s_interactive) {
            static_cast<KIO::Job*>(job)->uiDelegate()->showErrorMessage();
        } else
#endif
        {
            qWarning() << job->errorString();

        }
    }
    m_ok = !job->error();
    if (qApp->topLevelWindows().isEmpty()) {
        qApp->quit();
    } else {
        qApp->setQuitOnLastWindowClosed(true);
    }
}

void ClientApp::slotDialogCanceled()
{
    m_ok = false;
    qApp->quit();
}

void ClientApp::slotPrintData(KIO::Job*, const QByteArray &data)
{
    if (!data.isEmpty())
        std::cout.write(data.constData(), data.size());
}

ClientApp::ClientApp()
    : QObject()
{
}

