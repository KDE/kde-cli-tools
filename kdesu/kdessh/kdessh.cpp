/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#include <config.h>

#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <qstring.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "kdesu.h"
#include "ssh.h"
#include "sshdlg.h"
#include "client.h"


const char *Version = "1.0";

static KCmdLineOptions options[] = {
    { "+host", I18N_NOOP("Specifies the remote host"), 0 }, 
    { "+command", I18N_NOOP("The command to run."), 0 },
    { "u <user>", I18N_NOOP("Specifies the target uid"), 0 },
    { "n", I18N_NOOP("Do not keep password."), 0 },
    { "s", I18N_NOOP("Stop the daemon (forgets all passwords)."), 0 },
    { "t", I18N_NOOP("Enable terminal output (no password keeping)."), 0 },
    { 0, 0, 0 }
};


int main(int argc, char *argv[])
{
    KAboutData aboutData("kdessh", I18N_NOOP("KDE ssh"),
	    Version, I18N_NOOP("Runs a program on a remote host."),
	    KAboutData::License_Artistic, 
	    "Copyright (c) 2000 Geert Jansen");
    aboutData.addAuthor("Geert Jansen", I18N_NOOP("Maintainer"),
	    "jansen@kde.org", "http://www.stack.nl/~geertj/");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    // Stop daemon and exit?
    if (args->isSet("s")) {
	KDEsuClient client;
	if (client.ping() == -1)
	    kDebugFatal("Daemon not running -- nothing to stop");
	if (client.stopServer() != -1) {
	    kDebugInfo("Daemon stopped\n");
	    exit(0);
	}
	kDebugFatal("Could not stop daemon");
	exit(1);
    }

    // Get remote userid
    QCString user = args->getOption("u");
    if (user.isNull()) {
	struct passwd *pw = getpwuid(getuid());
	if (pw == 0L) {
	    kDebugFatal("You don't exist!");
	    exit(1);
	}
	user = pw->pw_name;
    }

    // Get remote host, command
    if (args->count() < 2)
	KCmdLineArgs::usage(i18n("No command or host specified!"));
    QCString host = args->arg(0);
    QCString command = args->arg(1);
    for (int i=2; i<args->count(); i++) {
	command += " ";
	command += args->arg(i);
    }

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

    // Try to exec the command with kdesud?
    bool keep = !args->isSet("n");
    bool terminal = args->isSet("t");
    if (keep && !terminal && !just_started) {
	client.setHost(host);
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

    KApplication *app = new KApplication;

    // Read configuration
    KConfig *config = new KConfig("kdesurc");
    config->setGroup("Passwords");
    int pw_timeout = config->readNumEntry("KeepPasswordTimeout", defTimeout);

    SshProcess proc(host, user);
    QString prompt = proc.checkNeedPassword();
    QCString password;

    if (!prompt.isEmpty()) {
	int k = keep && !terminal;
	int res = KDEsshDialog::getPassword(password, host, user, 
		command, prompt, &k);
	if (res == KDEsshDialog::Rejected)
	    exit(0);
	if (keep)
	    keep = k;
    } else 
	keep = 0; 

    delete app;

    if (just_started && have_daemon) {
	client.connect();
	if (client.ping() == -1) {
	    kDebugWarning("Could not connect to daemon.");
	    have_daemon = false;
	}
    }

    // Run command

    if (keep && have_daemon) {
	client.setUser(user);
	client.setHost(host);
	client.setPass(password, pw_timeout);
	return client.exec(command);
    } else {
	SshProcess proc(host, user, command);
	QCString key = host + "*" + user + "*ksycoca";
	if (client.getVar(key) == "yes")
	    proc.setBuildSycoca(false);
	else
	    client.setVar(key, "yes");
	proc.setTerminal(terminal);
	proc.setErase(true);
	return proc.exec(password);
    }
}

