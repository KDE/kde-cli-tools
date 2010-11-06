/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#include <config-runtime.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>
#if defined(HAVE_SYS_WAIT_H)
#include <sys/wait.h>
#endif

#include <QFileInfo>
#include <QtCore/QBool>
#include <QFile>

#include <QtDBus/QtDBus>

#include <kdebug.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <kstartupinfo.h>
#include <kdefakes.h>
#include <kwindowsystem.h>

#include <kdesu/defaults.h>
#include <kdesu/su.h>
#include <kdesu/client.h>

#include "sudlg.h"

#define ERR strerror(errno)

QByteArray command;
const char *Version = "1.0";

// NOTE: if you change the position of the -u switch, be sure to adjust it
// at the beginning of main()
#if 0
// D-BUS has no equivalent
QByteArray dcopNetworkId()
{
    QByteArray result;
    result.resize(1025);
    QFile file(DCOPClient::dcopServerFile());
    if (!file.open(QIODevice::ReadOnly))
        return "";
    int i = file.readLine(result.data(), 1024);
    if (i <= 0)
        return "";
    result.data()[i-1] = '\0'; // strip newline
    return result;
}
#endif

static int startApp();

int main(int argc, char *argv[])
{
    // FIXME: this can be considered a poor man's solution, as it's not
    // directly obvious to a gui user. :)
    // anyway, i vote against removing it even when we have a proper gui
    // implementation.  -- ossi

    QByteArray duser = qgetenv("ADMIN_ACCOUNT");
    if (duser.isEmpty())
        duser = "root";

    KAboutData aboutData("kdesu", 0, ki18n("KDE su"),
            Version, ki18n("Runs a program with elevated privileges."),
            KAboutData::License_Artistic,
            ki18n("Copyright (c) 1998-2000 Geert Jansen, Pietro Iglio"));
    aboutData.addAuthor(ki18n("Geert Jansen"), ki18n("Maintainer"),
            "jansen@kde.org", "http://www.stack.nl/~geertj/");
    aboutData.addAuthor(ki18n("Pietro Iglio"), ki18n("Original author"),
            "iglio@fub.it");
    aboutData.setProgramIconName("dialog-password");

    KCmdLineArgs::init(argc, argv, &aboutData);

    // NOTE: if you change the position of the -u switch, be sure to adjust it
    // at the beginning of main()
    KCmdLineOptions options;
    options.add("+command", ki18n("Specifies the command to run"));
    options.add("c <command>", ki18n("Specifies the command to run"));
    options.add("f <file>", ki18n("Run command under target uid if <file> is not writable"));
    options.add("u <user>", ki18n("Specifies the target uid"), duser);
    options.add("n", ki18n("Do not keep password"));
    options.add("s", ki18n("Stop the daemon (forgets all passwords)"));
    options.add("t", ki18n("Enable terminal output (no password keeping)"));
    options.add("p <prio>", ki18n("Set priority value: 0 <= prio <= 100, 0 is lowest"), "50");
    options.add("r", ki18n("Use realtime scheduling"));
    options.add("noignorebutton", ki18n("Do not display ignore button"));
    options.add("i <icon name>", ki18n("Specify icon to use in the password dialog"));
    options.add("d", ki18n("Do not show the command to be run in the dialog"));
#ifdef Q_WS_X11
    /* KDialog originally used --embed for attaching the dialog box.  However this is misleading and so we changed to --attach.
     * For consistancy, we silently map --embed to --attach */
    options.add("attach <winid>", ki18nc("Transient means that the kdesu app will be attached to the app specified by the winid so that it is like a dialog box rather than some separate program", "Makes the dialog transient for an X app specified by winid"));
    options.add("embed <winid>");
#endif
    KCmdLineArgs::addCmdLineOptions(options);

    //KApplication::disableAutoDcopRegistration();
    // kdesu doesn't process SM events, so don't even connect to ksmserver
    QByteArray session_manager = qgetenv( "SESSION_MANAGER" );
    unsetenv( "SESSION_MANAGER" );
    KApplication app;
    // but propagate it to the started app
    setenv( "SESSION_MANAGER", session_manager.data(), 1 );

    {
#ifdef Q_WS_X11
        KStartupInfoId id;
        id.initId( kapp->startupId());
        id.setupStartupEnv(); // make DESKTOP_STARTUP_ID env. var. available again
#endif
    }

    int result = startApp();

    if (result == 127)
    {
        KMessageBox::sorry(0, i18n("Cannot execute command '%1'.", QString::fromLocal8Bit(command)));
    }

    return result;
}

static int startApp()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    // Stop daemon and exit?
    if (args->isSet("s"))
    {
        KDEsuClient client;
        if (client.ping() == -1)
        {
            kError(1206) << "Daemon not running -- nothing to stop\n";
            exit(1);
        }
        if (client.stopServer() != -1)
        {
            kDebug(1206) << "Daemon stopped\n";
            exit(0);
        }
        kError(1206) << "Could not stop daemon\n";
        exit(1);
    }

    QString icon;
    if ( args->isSet("i"))
	icon = args->getOption("i");

    bool prompt = true;
    if ( args->isSet("d"))
	prompt = false;

    // Get target uid
    QByteArray user = args->getOption("u").toLocal8Bit();
    QByteArray auth_user = user;
    struct passwd *pw = getpwnam(user);
    if (pw == 0L)
    {
        kError(1206) << "User " << user << " does not exist\n";
        exit(1);
    }
    bool change_uid = (getuid() != pw->pw_uid);

    // If file is writeable, do not change uid
    QString file = args->getOption("f");
    if (change_uid && !file.isEmpty())
    {
        if (file.at(0) != '/')
        {
            KStandardDirs dirs;
            file = dirs.findResource("config", file);
            if (file.isEmpty())
            {
                kError(1206) << "Config file not found: " << file << "\n";
                exit(1);
            }
        }
        QFileInfo fi(file);
        if (!fi.exists())
        {
            kError(1206) << "File does not exist: " << file << "\n";
            exit(1);
        }
        change_uid = !fi.isWritable();
    }

    // Get priority/scheduler
    QString tmp = args->getOption("p");
    bool ok;
    int priority = tmp.toInt(&ok);
    if (!ok || (priority < 0) || (priority > 100))
    {
        KCmdLineArgs::usageError(i18n("Illegal priority: %1", tmp));
        exit(1);
    }
    int scheduler = SuProcess::SchedNormal;
    if (args->isSet("r"))
        scheduler = SuProcess::SchedRealtime;
    if ((priority > 50) || (scheduler != SuProcess::SchedNormal))
    {
        change_uid = true;
        auth_user = "root";
    }

    // Get command
    if (args->isSet("c"))
    {
        command = args->getOption("c").toLocal8Bit();
        for (int i=0; i<args->count(); i++)
        {
            QString arg = args->arg(i);
	    if(!arg.isEmpty()) {
		QChar q('\'');
		arg.replace(q, "'\\''").prepend(q).append(q);
	    }
            command += ' ';
            command += QFile::encodeName(arg);
        }
    }
    else
    {
        if( args->count() == 0 )
        {
            KCmdLineArgs::usageError(i18n("No command specified."));
            exit(1);
        }
        command = args->arg(0).toLocal8Bit();
        for (int i=1; i<args->count(); i++)
        {
            QString arg = args->arg(i);
	    if(!arg.isEmpty()) {
		QChar q('\'');
		arg.replace(q, "'\\''").prepend(q).append(q);
	    }
            command += ' ';
            command += QFile::encodeName(arg);
        }
    }

    // Don't change uid if we're don't need to.
    if (!change_uid)
    {
        int result = system(command);
        result = WEXITSTATUS(result);
        return result;
    }

    // Check for daemon and start if necessary
    bool just_started = false;
    bool have_daemon = true;
    KDEsuClient client;
    if (!client.isServerSGID())
    {
        kWarning(1206) << "Daemon not safe (not sgid), not using it.\n";
        have_daemon = false;
    }
    else if (client.ping() == -1)
    {
        if (client.startServer() == -1)
        {
            kWarning(1206) << "Could not start daemon, reduced functionality.\n";
            have_daemon = false;
        }
        just_started = true;
    }

    // Try to exec the command with kdesud.
    bool keep = !args->isSet("n") && have_daemon;
    bool terminal = args->isSet("t");
    bool withIgnoreButton = args->isSet("ignorebutton");
    int winid = -1;
    bool attach = args->isSet("attach");
    if(attach) {
        winid = args->getOption("attach").toInt(&attach, 0);  //C style parsing.  If the string begins with "0x", base 16 is used; if the string begins with "0", base 8 is used; otherwise, base 10 is used.
        if(!attach)
            kWarning(1206) << "Specified winid to attach to is not a valid number";
    } else if(args->isSet("embed")) {
        /* KDialog originally used --embed for attaching the dialog box.  However this is misleading and so we changed to --attach.
         * For consistancy, we silently map --embed to --attach */
        attach = true;
        winid = args->getOption("embed").toInt(&attach, 0);  //C style parsing.  If the string begins with "0x", base 16 is used; if the string begins with "0", base 8 is used; otherwise, base 10 is used.
        if(!attach)
            kWarning(1206) << "Specified winid to attach to is not a valid number";
    }


    QList<QByteArray> env;
    QByteArray options;
    env << ( "DESKTOP_STARTUP_ID=" + kapp->startupId());

    if (pw->pw_uid)
    {
       // Only propagate KDEHOME for non-root users,
       // root uses KDEROOTHOME

       // Translate the KDEHOME of this user to the new user.
       QString kdeHome = KGlobal::dirs()->relativeLocation("home", KGlobal::dirs()->localkdedir());
       if (kdeHome[0] != '/')
          kdeHome.prepend("~/");
       else
          kdeHome.clear(); // Use default

       env << ("KDEHOME="+ QFile::encodeName(kdeHome));
    }

    KUser u;
    env << (QByteArray) ("KDESU_USER=" + u.loginName().toLocal8Bit());

    if (keep && !terminal && !just_started)
    {
        client.setPriority(priority);
        client.setScheduler(scheduler);
        int result = client.exec(command, user, options, env);
        if (result == 0)
        {
           result = client.exitCode();
           return result;
        }
    }

    // Set core dump size to 0 because we will have
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim))
    {
        kError(1206) << "rlimit(): " << ERR << "\n";
        exit(1);
    }

    // Read configuration
    KConfigGroup config(KGlobal::config(), "Passwords");
    int timeout = config.readEntry("Timeout", defTimeout);

    // Check if we need a password
    SuProcess proc;
    proc.setUser(auth_user);
    int needpw = proc.checkNeedPassword();
    if (needpw < 0)
    {
        QString err = i18n("Su returned with an error.\n");
        KMessageBox::error(0L, err);
        exit(1);
    }
    if (needpw == 0)
    {
        keep = 0;
        kDebug() << "Don't need password!!\n";
    }

    // Start the dialog
    QString password;
    if (needpw)
    {
#ifdef Q_WS_X11
        KStartupInfoId id;
        id.initId( kapp->startupId());
        KStartupInfoData data;
        data.setSilent( KStartupInfoData::Yes );
        KStartupInfo::sendChange( id, data );
#endif
        KDEsuDialog dlg(user, auth_user, keep && !terminal, icon, withIgnoreButton);
        if (prompt)
            dlg.addCommentLine(i18n("Command:"), command);
        if (defKeep)
            dlg.setKeepPassword(true);

        if ((priority != 50) || (scheduler != SuProcess::SchedNormal))
        {
            QString prio;
            if (scheduler == SuProcess::SchedRealtime)
                prio += i18n("realtime: ");
            prio += QString("%1/100").arg(priority);
            if (prompt)
                dlg.addCommentLine(i18n("Priority:"), prio);
        }

	//Attach dialog
#ifdef Q_WS_X11
	if(attach)
            KWindowSystem::setMainWindow(&dlg, (WId)winid);
#endif
        int ret = dlg.exec();
        if (ret == KDEsuDialog::Rejected)
        {
#ifdef Q_WS_X11
            KStartupInfo::sendFinish( id );
#endif
            exit(1);
        }
        if (ret == KDEsuDialog::AsUser)
            change_uid = false;
        password = dlg.password();
        keep = dlg.keepPassword();
#ifdef Q_WS_X11
        data.setSilent( KStartupInfoData::No );
        KStartupInfo::sendChange( id, data );
#endif
    }

    // Some events may need to be handled (like a button animation)
    kapp->processEvents();

    // Run command
    if (!change_uid)
    {
        int result = system(command);
        result = WEXITSTATUS(result);
        return result;
    }
    else if (keep && have_daemon)
    {
        client.setPass(password.toLocal8Bit(), timeout);
        client.setPriority(priority);
        client.setScheduler(scheduler);
        int result = client.exec(command, user, options, env);
        if (result == 0)
        {
            result = client.exitCode();
            return result;
        }
    } else
    {
        SuProcess proc;
        proc.setTerminal(terminal);
        proc.setErase(true);
        proc.setUser(user);
        proc.setEnvironment(env);
        proc.setPriority(priority);
        proc.setScheduler(scheduler);
        proc.setCommand(command);
        int result = proc.exec(password.toLocal8Bit());
        return result;
    }
    return -1;
}

