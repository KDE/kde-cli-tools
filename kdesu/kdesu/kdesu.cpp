/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#include <config.h>

#include <iostream>
#include <string>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <qstring.h>
#include <qfileinfo.h>

#include <kglobal.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstartparams.h>

#include "kdesu.h"
#include "kpassdlg.h"
#include "process.h"
#include "client.h"
#include "debug.h"


// Globals
int _show_dbg = 0;
int _show_wrn = 1;
const char *Version = VERSION;
const char *TryHelp = "Try `kdesu -h' for more information.\n";
const char *Email = "<g.t.jansen@stud.tue.nl>";
const char *Usage = "Usage: kdesu [USER] [-ntqd] [-f FILE] -c COMMAND [ARG1 [ARG2 [...]]]\n"
		    "       kdesu -v|-h|-s\n";

// Main program

int main(int argc, char *argv[])
{
    int c;
    QString command, file;
    bool keep = true;
    bool terminal = false;

    KStartParams parm(argc, argv);
    QString s;

    // Parse user name argument
    uid_t uid; 
    QString user;
    int i=0;
    s = parm.get(i++);
    if (s.isEmpty()) {
	cout << Usage << TryHelp << endl;
	exit(1);
    }
    if (s.at(0) != '-') {
	struct passwd *pw = getpwnam(s.latin1());
	if (pw == 0L) {
	    error("User %s does not exist", s.latin1());
	    exit(1);
	}
	user = s;
	uid = pw->pw_uid;
	s = parm.get(i++);
    } else {
	user = "root";
	uid = 0;
    }

    // Parse options
    while (s != "-c") {
	if (s.at(0) != '-') {
	    cout << Usage << TryHelp << endl;
	    exit(1);
	}
	bool next = false;
	for (int j=1; j<s.length(); j++) {
	    switch (s.at(j).latin1()) {
	    case 'f':
		file = parm.get(i++);
		if (file.isEmpty()) {
		    cout << Usage << TryHelp << endl;
		    exit(1);
		}
		next = true;
		break;
	    case 't':
		terminal = true;
		break;
	    case 'n':
		keep = false;
		break;
	    case 'q':
		_show_dbg = 0;
		_show_wrn = 0;
		break;
	    case 'd':
		_show_dbg++;
		break;

	    // Exiting comand from here
	    case 'v':
		cerr << "kdesu version " << Version << endl;
		cerr << endl;
		cerr << "  Copyright (C) 1998 Pietro Iglio <iglio@fub.it>\n";
		cerr << "  Copyright (C) 1999 Geert Jansen " << Email << endl;
		cerr << endl;
		exit(0);
	    case 'h':
		cerr << Usage;
		cerr << "Runs a command as another user.\n";
		cerr << endl;
		cerr << "Options:\n";
		cerr << "  -c COMMAND    Run command COMMAND. The entire command\n";
		cerr << "                line has to be passed as a single argument.\n";
		cerr << "  -f FILE       Run command as root if file specified by FILE\n";
		cerr << "                is not writeable under current uid.\n";
		cerr << "  -n            Do not keep password\n";
		cerr << "  -s            Stop the daemon (forgets all passwords)\n"; 
		cerr << "  -t            Enable terminal output (no password keeping).\n";
		cerr << "  -q            Be quiet (shows no warnings)\n";
		cerr << "  -d            Show debug information\n";
		cerr << "  -v            Show version information\n";
		cerr << endl;
		cerr << "Please report bugs to " << Email << endl;
		exit(0);
	     case 's':
		{
		    KDEsuClient client;
		    if (client.ping() == -1) {
			error("Daemon not running -- nothing to stop");
			exit(1);
		    }
		    if (client.stopServer() != -1) {
			cout << "Daemon stopped\n";
			return 0;
		    }
		    error("Could not stop daemon");
		    exit(1);
		}
	    }
	    if (next)
		break;
	}
	s = parm.get(i++);
	if (s.isEmpty()) {
	    cout << Usage << TryHelp << endl;
	    exit(1);
	}
    }

    // Parse command
    command = parm.get(i++);
    if (command.isEmpty()) {
	cout << Usage << TryHelp << endl;
	exit(1);
    }
    while (!(s = parm.get(i++)).isEmpty()) {
	command += " ";
	command += s;
    }
    
    // Don't use su if we're don't need to.
    bool change_uid = (getuid() != uid);

    // If file is not writeable, change uid
    if (change_uid && !file.isEmpty()) {
	if (file.at(0) != '/') {
	    KStandardDirs dirs;
	    dirs.addKDEDefaults();
	    file = dirs.findResource("config", file);
	}
	QFileInfo fi(file);
	if (!fi.exists()) {
	    error("File does not exist: %s", file.latin1());
	    exit(1);
	}
	change_uid = !fi.isWritable();
    }

    if (!change_uid) {
	UserProcess proc(command);
	return proc.exec();
    }

    // Try to exec the command with kdesud.
    if (keep && !terminal) {
	KDEsuClient client;
	if (client.ping() != -1) {
	    client.setUser(user);
	    if (client.exec(command) != -1)
		return 0;
	} else
	    // The user has to enter a password and this very probably 
	    // gives enough time to start up the daemon.
	    client.startServer();
    }

    // Set core dump size to 0 because we will have 
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim)) {
	xerror("rlimit(): %s");
	exit(1);
    }

    // From  here, we need the GUI: create a KApplication
    KApplication *app = new KApplication(argc, argv, "kdesu");

    // Read configuration
    KConfig *config = KGlobal::config();
    config->setGroup("Common");
    QString val = config->readEntry("EchoMode", "x");
    int echo_mode;
    if (val == "OneStar")
	echo_mode = OneStar;
    else if (val == "ThreeStars")
	echo_mode = ThreeStars;
    else if (val == "NoStars")
	echo_mode = NoStars;
    else
	echo_mode = defEchomode;
    bool keep_cfg = config->readBoolEntry("KeepPassword", defKeep);
    int pw_timeout = config->readNumEntry("KeepPasswordTimeout", defTimeout);

     // Start the dialog
    QString txt;
    if (user == "root")
	txt = i18n("The action you requested needs root priviliges.\n"
		   "Please enter root's password below or click\n"
		   "Ignore to continue with your current priviliges.");
    else
	txt = i18n("The action you requested needs additional priviliges.\n"
		   "Please enter the password for \"%1\" below or click\n"
		   "Ignore to continue with your current privileges.").arg(user);

    KPasswordDlg *pwDlg = new KPasswordDlg(txt, command, user, ((keep&&!terminal) ? 1+keep_cfg : 0));
    pwDlg->setEchoMode(echo_mode);
    pwDlg->exec();

    char *pass = new char[KPasswordEdit::PassLen];
    switch (pwDlg->result()) {
    case KPasswordDlg::Accepted:
	strcpy(pass, pwDlg->getPass());
	change_uid = true;
	break;
    case KPasswordDlg::Rejected:
	return 0;
    case KPasswordDlg::AsUser:
	change_uid = false;
	break;
    }
    
    keep = pwDlg->keepPass();
    delete pwDlg;

    // This destroys the Qt event loop and makes sure the dialog goes away.
    delete app;

    if (!change_uid) {
	UserProcess proc(command);
	return proc.exec();
    }

    // Change uid
    if (keep) {
	KDEsuClient client;
	if (client.ping() != -1) {
	    client.setUser(user);
	    client.setPass(pass, pw_timeout);
	    return client.exec(command);
	} else
	    error("Cannot connect to daemon -- not keeping password");
    } 

    SuProcess proc;
    proc.setCommand(command);
    proc.setUser(user);
    proc.setTerminal(terminal);
    proc.setErase(true);
    return proc.exec(pass);
}

