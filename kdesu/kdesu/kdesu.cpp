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

#include "kdesu.h"
#include "su.h"
#include "sudlg.h"
#include "client.h"


const char *Version = "1.0";

static KCmdLineOptions options[] = {
    { "+command", I18N_NOOP("Specifies the command to run."), 0},
    { "f <file>", I18N_NOOP("Run command under target uid if <file> is not writeable."), "" },
    { "u <user>", I18N_NOOP("Specifies the target uid"), "root" },
    { "n", I18N_NOOP("Do not keep password."), 0 },
    { "s", I18N_NOOP("Stop the daemon (forgets all passwords)."), 0 },
    { "t", I18N_NOOP("Enable terminal output (no password keeping)."), 0 },
    { 0, 0, 0 }
};


int main(int argc, char *argv[])
{
    KAboutData aboutData("kdesu", I18N_NOOP("KDE su"),
	    Version, I18N_NOOP("Runs a program under a different uid."),
	    KAboutData::License_Artistic, 
	    "Copyright (c) 1998-2000 Geert Jansen, Pietro Iglio");
    aboutData.addAuthor("Geert Jansen", I18N_NOOP("Maintainer"),
	    "jansen@kde.org", "http://www.stack.nl/~geertj/");
    aboutData.addAuthor("Pietro Iglio", I18N_NOOP("Original author"),
	    "iglio@fub.it");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    // Stop daemon and exit?
    if (args->isSet("s")) {
	KDEsuClient client;
	if (client.ping() == -1)
	    kDebugFatal("Daemon not running -- nothing to stop");
	if (client.stopServer() != -1) {
	    printf("Daemon stopped\n");
	    exit(0);
	}
	kDebugFatal("Could not stop daemon");
	exit(1);
    }

    // Configure found su?
    if (!strcmp(__PATH_SU, "false")) {
	kDebugFatal("su was not found by configure");
	exit(1);
    }

    // Get target uid
    QCString user = args->getOption("u");
    struct passwd *pw = getpwnam(user);
    if (pw == 0L) {
	kDebugFatal("User %s does not exist", (const char *) user);
	exit(1);
    }
    uid_t uid = pw->pw_uid;
    bool change_uid = (getuid() != uid);

    // Get command
    if (args->count() == 0)
	KCmdLineArgs::usage(i18n("No command specified!"));
    QCString command = args->arg(0);
    for (int i=1; i<args->count(); i++) {
	command += " ";
	command += args->arg(i);
    }

    // If file is not writeable, change uid
    QString file = QFile::decodeName(args->getOption("f"));
    if (!file.isEmpty()) {
	if (file.at(0) != '/') {
	    KStandardDirs dirs;
	    dirs.addKDEDefaults();
	    file = dirs.findResource("config", file);
	    if (file.isEmpty()) {
		kDebugFatal("Config file not found: %s", file.latin1());
		exit(1);
	    }
	}
	QFileInfo fi(file);
	if (!fi.exists()) {
	    kDebugFatal("File does not exist: %s", file.latin1());
	    exit(1);
	}
	change_uid = !fi.isWritable();
    }

    // Don't change uid if we're don't need to.
    if (!change_uid)
	return system(command);

    // Check for daemon and start if necessary
    bool just_started = false;
    bool have_daemon = true;
    KDEsuClient client;
    if (client.ping() == -1) {
	just_started = true;
	if (client.startServer() == -1) {
	    kDebugWarning("Could not start daemon, reduced functionality.");
	    have_daemon = false;
	}
    }

    // Try to exec the command with kdesud.
    bool keep = !args->isSet("n");
    bool terminal = args->isSet("t");
    if (keep && !terminal && !just_started) {
	client.setUser(user);
	if (client.exec(command) != -1)
	    return 0;
    }

    // Set core dump size to 0 because we will have 
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim)) {
	kDebugFatal("rlimit(): %m");
	exit(1);
    }

    // From  here, we need the GUI: create a KApplication
    KApplication *app = new KApplication;

    // Read configuration
    KConfig *config = KGlobal::config();
    config->setGroup("Passwords");
    int timeout = config->readNumEntry("Timeout", defTimeout);

     // Start the dialog
    QCString password;
    int k = keep  && !terminal;
    int ret = KDEsuDialog::getPassword(password, user, command, &k);
    if (ret == KDEsuDialog::Rejected)
	exit(0);
    if (ret == KDEsuDialog::AsUser)
	change_uid = false;

    // This destroys the Qt event loop and makes sure the dialog goes away.
    delete app;

    if (!change_uid)
	return system(command);

    if (just_started && have_daemon) {
	client.connect();
	if (client.ping() == -1) {
	    kDebugWarning("Could not connect to daemon.");
	    have_daemon = false;
	}
    }

    // Run command

    if (k && have_daemon) {
	client.setUser(user);
	client.setPass(password, timeout);
	return client.exec(command);
    } else {
	SuProcess proc;
	proc.setTerminal(terminal);
	proc.setErase(true);
	proc.setUser(user);
	proc.setCommand(command);
	return proc.exec(password);
    }
}

