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
#include "kpassdlg.h"
#include "process.h"
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
    if (!change_uid) {
	UserProcess proc(command);
	return proc.exec();
    }

    // Try to exec the command with kdesud.
    bool keep = true;
    if (args->isSet("n"))
	keep = false;
    bool terminal = args->isSet("t");
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
    if (setrlimit(RLIMIT_CORE, &rlim)) {
	kDebugFatal("rlimit(): %s", strerror(errno));
	exit(1);
    }

    // From  here, we need the GUI: create a KApplication
    KApplication *app = new KApplication;

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

    KDEsuDialog *dlg = new KDEsuDialog(command, user, 
	    ((keep&&!terminal) ? 1+keep_cfg : 0));
    dlg->setEchoMode(echo_mode);
    dlg->exec();

    char *pass = new char[KPasswordEdit::PassLen];
    switch (dlg->result()) {
    case KDEsuDialog::Accepted:
	strcpy(pass, dlg->getPass());
	change_uid = true;
	break;
    case KDEsuDialog::Rejected:
	return 0;
    case KDEsuDialog::AsUser:
	change_uid = false;
	break;
    }
    
    keep = dlg->keepPass();
    delete dlg;

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

