/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#include <config.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <qstring.h>
#include <qfileinfo.h>
#include <qglobal.h>

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


// Globals
int _show_dbg = 0;
int _show_wrn = 1;
const char *Version = VERSION;
const char *TryHelp = "Try `kdesu -h' for more information.\n";
const char *Email = "<g.t.jansen@stud.tue.nl>";
const char *Usage = "Usage: kdesu [USER] [-ntqd] [-f FILE] -c COMMAND [ARG1 [ARG2 [...]]]\n"
		    "       kdesu -v|-h|-s\n";

/*
 * Message handler
 */
void msgHandler(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
	if (_show_dbg)
	    fprintf(stderr, "Debug: %s\n", msg);
	break;

    case QtWarningMsg:
	if (_show_wrn)
	    fprintf(stderr, "Warning: %s\n", msg);
	break;

    case QtFatalMsg:
	fprintf(stderr, "Fatal: %s\n", msg);
	exit(1);
    }
}


// Main program

int main(int argc, char *argv[])
{
    qInstallMsgHandler(msgHandler);

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
	printf(Usage);
	printf(TryHelp);
	exit(1);
    }
    if (s.at(0) != '-') {
	struct passwd *pw = getpwnam(s.latin1());
	if (pw == 0L)
	    qFatal("User %s does not exist", s.latin1());
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
	    printf(Usage); printf(TryHelp);
	    exit(1);
	}
	bool next = false;
	for (int j=1; j<s.length(); j++) {
	    switch (s.at(j).latin1()) {
	    case 'f':
		file = parm.get(i++);
		if (file.isEmpty()) {
		    printf(Usage); printf(TryHelp);
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
		printf("kdesu version %s\n", Version);
		printf("\n");
		printf("  Copyright (C) 1998 Pietro Iglio <iglio@fub.it>\n");
		printf("  Copyright (C) 1999 Geert Jansen %s", Email);
		printf("\n");
		exit(0);
	    case 'h':
		printf(Usage);
		printf("Runs a command as another user.\n");
		printf("\n");
		printf("Options:\n");
		printf("  -c COMMAND    Run command COMMAND. The entire command\n");
		printf("                line has to be passed as a single argument.\n");
		printf("  -f FILE       Run command as root if file specified by FILE\n");
		printf("                is not writeable under current uid.\n");
		printf("  -n            Do not keep password\n");
		printf("  -s            Stop the daemon (forgets all passwords)\n"); 
		printf("  -t            Enable terminal output (no password keeping).\n");
		printf("  -q            Be quiet (shows no warnings)\n");
		printf("  -d            Show debug information\n");
		printf("  -v            Show version information\n");
		printf("\n");
		printf("Please report bugs to %s\n", Email);
		exit(0);
	     case 's':
		{
		    KDEsuClient client;
		    if (client.ping() == -1)
			qFatal("Daemon not running -- nothing to stop");
		    if (client.stopServer() != -1) {
			printf("Daemon stopped\n");
			return 0;
		    }
		    qFatal("Could not stop daemon");
		}
	    }
	    if (next)
		break;
	}
	s = parm.get(i++);
	if (s.isEmpty()) {
	    printf(Usage); printf(TryHelp);
	    exit(1);
	}
    }

    // Parse command
    command = parm.get(i++);
    if (command.isEmpty()) {
	printf(Usage); printf(TryHelp);
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
	if (!fi.exists())
	    qFatal("File does not exist: %s", file.latin1());
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
	    keep = (client.startServer() != -1);
    }

    // Set core dump size to 0 because we will have 
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim))
	qFatal("rlimit(): %s", strerror(errno));

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

    KPasswordDlg *pwDlg = new KPasswordDlg(txt, command, user, 
	    ((keep&&!terminal) ? 1+keep_cfg : 0));
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
	    qWarning("Cannot connect to daemon -- not keeping password");
    } 

    SuProcess proc;
    proc.setCommand(command);
    proc.setUser(user);
    proc.setTerminal(terminal);
    proc.setErase(true);
    return proc.exec(pass);
}

