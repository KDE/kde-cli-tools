/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#include <config.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <qstring.h>
#include <qfileinfo.h>
#include <qglobal.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>

#include <kdesu/defaults.h>
#include <kdesu/su.h>
#include <kdesu/client.h>

#include "sudlg.h"

#define ERR strerror(errno)

QCString command;
const char *Version = "1.0";

static KCmdLineOptions options[] = {
    { "+command", I18N_NOOP("Specifies the command to run."), 0 },
    { "c <command>", I18N_NOOP("Specifies the command to run."), "" },
    { "f <file>", I18N_NOOP("Run command under target uid if <file> is not writeable."), "" },
    { "u <user>", I18N_NOOP("Specifies the target uid"), "root" },
    { "n", I18N_NOOP("Do not keep password."), 0 },
    { "s", I18N_NOOP("Stop the daemon (forgets all passwords)."), 0 },
    { "t", I18N_NOOP("Enable terminal output (no password keeping)."), 0 },
    { "p <prio>", I18N_NOOP("Set priority value: 0 <= prio <= 100, 0 is lowest."), "50" },
    { "r", I18N_NOOP("Use realtime scheduling."), 0 },
    { 0, 0, 0 }
};


int main(int argc, char *argv[])
{
    KAboutData aboutData("kdesu", I18N_NOOP("KDE su"),
	    Version, I18N_NOOP("Runs a program with elevated privileges."),
	    KAboutData::License_Artistic, 
	    "Copyright (c) 1998-2000 Geert Jansen, Pietro Iglio");
    aboutData.addAuthor("Geert Jansen", I18N_NOOP("Maintainer"),
	    "jansen@kde.org", "http://www.stack.nl/~geertj/");
    aboutData.addAuthor("Pietro Iglio", I18N_NOOP("Original author"),
	    "iglio@fub.it");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication *app = new KApplication;

    // Stop daemon and exit?
    if (args->isSet("s")) 
    {
	KDEsuClient client;
	if (client.ping() == -1)
	{
	    kdError(1206) << "Daemon not running -- nothing to stop\n";
	    exit(1);
	}
	if (client.stopServer() != -1) 
	{
	    kdDebug(1206) << "Daemon stopped\n";
	    exit(0);
	}
	kdError(1206) << "Could not stop daemon\n";
	exit(1);
    }

    // Get target uid
    QCString user = args->getOption("u");
    QCString auth_user = user;
    struct passwd *pw = getpwnam(user);
    if (pw == 0L) 
    {
	kdError(1206) << "User " << user << " does not exist\n";
	exit(1);
    }
    bool change_uid = (getuid() != pw->pw_uid);

    // If file is writeable, do not change uid
    QString file = QFile::decodeName(args->getOption("f"));
    if (change_uid && !file.isEmpty()) 
    {
	if (file.at(0) != '/') 
	{
	    KStandardDirs dirs;
	    dirs.addKDEDefaults();
	    file = dirs.findResource("config", file);
	    if (file.isEmpty()) 
	    {
		kdError(1206) << "Config file not found: " << file << "\n";
		exit(1);
	    }
	}
	QFileInfo fi(file);
	if (!fi.exists()) 
	{
	    kdError(1206) << "File does not exist: " << file << "\n";
	    exit(1);
	}
	change_uid = !fi.isWritable();
    }

    // Get priority/scheduler
    QCString tmp = args->getOption("p");
    bool ok;
    int priority = tmp.toInt(&ok);
    if (!ok || (priority < 0) || (priority > 100)) 
    {
	KCmdLineArgs::usage(i18n("Illegal priority: %1").arg(tmp));
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
    command = args->getOption("c");
    for (int i=0; i<args->count(); i++)
    {
	command += " ";
	command += args->arg(i);
    }
    }
    else {
    if( args->count() == 0 )
    {
	KCmdLineArgs::usage(i18n("No command specified!"));
	exit(1);
    }
    command = args->arg(0); 
    for (int i=1; i<args->count(); i++) 
    {
	command += " ";
	command += args->arg(i);
    }}

//  CJM - Test lines remove when working
//    kdWarning(1206) << args->count();
//    kdWarning(1206) << command;

    // Don't change uid if we're don't need to.
    if (!change_uid)
	return system(command);

    // Check for daemon and start if necessary
    bool just_started = false;
    bool have_daemon = true;
    KDEsuClient client;
    if (!client.isServerSGID())
    {
	kdWarning(1206) << "Daemon not safe (not sgid), not using it.\n";
	have_daemon = false;
    } else if (client.ping() == -1) 
    {
	if (client.startServer() == -1)
	{
	    kdWarning(1206) << "Could not start daemon, reduced functionality.\n";
	    have_daemon = false;
	}
	just_started = true;
    }

    // Try to exec the command with kdesud.
    bool keep = !args->isSet("n") && have_daemon;
    bool terminal = args->isSet("t");
    if (keep && !terminal && !just_started) 
    {
	client.setPriority(priority);
	client.setScheduler(scheduler);
	if (client.exec(command, user) != -1)
	    return 0;
    }

    // Set core dump size to 0 because we will have 
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim)) 
    {
	kdError(1206) << "rlimit(): " << ERR << "\n";
	exit(1);
    }

    // Read configuration
    KConfig *config = KGlobal::config();
    config->setGroup("Passwords");
    int timeout = config->readNumEntry("Timeout", defTimeout);

    // Check if we need a password
    SuProcess proc;
    proc.setUser(auth_user);
    int needpw = proc.checkNeedPassword();
    if (needpw < 0)
    {
	QString err = i18n("Su returned with an error!\n");
	KMessageBox::error(0L, err);
	exit(1);
    }
    if (needpw == 0)
    {
	keep = 0;
	kdDebug() << "Don't need password!!\n";
    }

    // Start the dialog
    QCString password;
    if (needpw)
    {
	KDEsuDialog *dlg = new KDEsuDialog(user, auth_user, keep && !terminal);
	dlg->addLine(i18n("Command:"), command);
	if ((priority != 50) || (scheduler != SuProcess::SchedNormal)) 
	{
	    QString prio;
	    if (scheduler == SuProcess::SchedRealtime)
		prio += i18n("realtime: ");
	    prio += QString("%1/100").arg(priority);
	    dlg->addLine(i18n("Priority:"), prio);
	}
	int ret = dlg->exec();
	if (ret == KDEsuDialog::Rejected)
	    exit(0);
	if (ret == KDEsuDialog::AsUser)
	    change_uid = false;
	password = dlg->password();
	keep = dlg->keep();
	delete dlg;
    }

    // Some events may need to be handled (like a button animation)
    app->processEvents();

    if (!change_uid)
	return system(command);

    // Run command

    if (keep && have_daemon) 
    {
	client.setPass(password, timeout);
	client.setPriority(priority);
	client.setScheduler(scheduler);
	return client.exec(command, user);
    } else 
    {
	SuProcess proc;
	proc.setTerminal(terminal);
	proc.setErase(true);
	proc.setUser(user);
	proc.setPriority(priority);
	proc.setScheduler(scheduler);
	proc.setCommand(command);
	return proc.exec(password);
    }
}

