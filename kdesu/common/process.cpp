/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * This file contains code from TEShell.C of the KDE konsole. 
 * Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de> 
 *
 * process.cpp: Execute a program as another user with "class SuProcess".
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>
#include <pwd.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/time.h>

#if defined(__SVR4) && defined(sun)
#include <stropts.h>
#include <sys/stream.h>
#endif

#include <qglobal.h>
#include <qcstring.h>

#include <kdebug.h>
#include <kstddirs.h>

#include "process.h"
#include "pty.h"
#include "kcookie.h"

#ifdef __GNUC__
#define ID __PRETTY_FUNCTION__
#else
#define ID __FILE__
#endif

SuProcess::SuProcess(const char *command)
{
    m_Command = command;
    m_User = "root";
    m_bTerminal = false;
    m_bErase = false;
    m_bSycoca = true;
}


/**
 * Execute a command with su(1).
 */

int SuProcess::exec(char *password, bool check_only)
{
    // Open a pty/tty pair.
    PTY *pty = new PTY();
    m_Fd = pty->getpt();
    if (m_Fd < 0)
	return -1;
    if ((pty->grantpt() < 0) || (pty->unlockpt() < 0)) {
	kDebugWarning("%s: Master setup failed.", ID);
	return -1;
    }
    m_TTY = pty->ptsname();

    if ((m_Pid = fork()) == -1) {
	kDebugWarning("%s: fork(): %m", ID);
	return -1;
    }
    if (m_Pid) {
	if (ConverseSU(password) < 0) {
	    kDebugWarning("%s: Conversation with su failed", ID);
	    return -1;
	} 
	if (m_bErase) {
	    int len = strlen(password);
	    for (int i=0; i<len; i++)
		password[i] = '\000';
	}
	if (!check_only && (ConverseStub() < 0)) {
	    kDebugWarning("%s: Converstation with kdesu_stub failed", ID);
	    return -1;
	}
	int ret = waitForChild(check_only || m_bTerminal);
	delete pty;
	return ret;

    } else {

	QString stub = KStandardDirs::findExe("kdesu_stub");
	if (stub.isEmpty()) {
	    kDebugError("%s: kdesu_stub not found", ID);
	    _exit(1);
	}
	if (check_only)
	    stub += " --verify";

	if (SetupTTY() < 0)
	    _exit(1);

	// Terminal output will go through the tty from now on. It is visible
	// only if the user uses the '-t' switch.

	execl(_PATH_SU, "su", m_User.data(), "-c", stub.latin1(), 0L);

	// Exec failed..
	kDebugWarning("%s: execl(\"%s\"): %m", ID, _PATH_SU);
	_exit(1);
    }

    return 0;
}

/*
 * Wait until the terminal is set into no echo mode. At least one su 
 * (RH6 w/ Linux-PAM patches) sets noecho mode AFTER writing the Password: 
 * prompt, using TCSAFLUSH. This flushes the terminal I/O queues, possibly 
 * taking the password  with it. So we wait until no echo mode is set 
 * before writing the password.
 * Note that this is done on the slave fd. While Linux allows tcgetattr() on
 * the master side, Solaris doesn't.
 */

int SuProcess::WaitSlave()
{
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0) {
	kDebugWarning("Could not open slave side: %m");
	return -1;
    }

    struct termios tio;
    struct timeval tv;
    while (1) {
	if (tcgetattr(slave, &tio) < 0) {
	    kDebugWarning("%s: tcgetattr(): %m", ID);
	    close(slave);
	    return -1;
	}
	if (tio.c_lflag & ECHO) {
	    kDebugInfo("%s: Echo mode still on.", ID);
	    // sleep 1/10 sec
	    tv.tv_sec = 0; tv.tv_usec = 100000;
	    select(slave, 0L, 0L, 0L, &tv);
	    continue;
	}
	break;
    }
    close(slave);
    return 0;
}
	
/*
 * Conversation with su: feed the password.
 */

int SuProcess::ConverseSU(const char *password)
{	
    char buf[256]; 
    int nbytes, i;
    int state = 0;

    while (state < 2) {
	nbytes = read(m_Fd, buf, 255);
	if (nbytes == -1) {
	    if (errno == EINTR) continue;
	    else {
		kDebugError("%s: read(): %m", ID);
		return -1;
	    }
	}
	if (nbytes == 0)
	    return -1;
	buf[nbytes] = '\000';
	switch (state) {
	case 0:
	    // Write password
	    if (strchr(buf, ':')) {
		WaitSlave();
		write(m_Fd, password, strlen(password));
		write(m_Fd, "\n", 1);
		state++;
	    } 
	    break;
	case 1:
	    // Wait for newline
	    for (i=0; (i<nbytes) && (buf[i] != '\n'); i++);
	    if (i < nbytes)
		state++;
	    break;
	}
    }
    return 0;
}


QCString SuProcess::commaSeparatedList(QStringList lst)
{
    if (lst.count() == 0)
	return QCString("");

    QStringList::Iterator it = lst.begin();
    QCString str = (*it).latin1();
    for (it++; it!=lst.end(); it++) {
	str += ',';
	str += (*it).latin1();
    }
    return str;
}
    
/*
 * Conversation with kdesu_stub. This is how we pass the authentication
 * tokens (X11 and DCOP), the X11 display, the DCOP server and the command
 * to the target process. kdesu_stub is already running with target 
 * priviliges.
 */

int SuProcess::ConverseStub()
{
    bool got_header = false, done = false;
    char buf[256];
    int nbytes;
    KCookie c;

    while (!done) {
	nbytes = read(m_Fd, buf, 255);
	if (nbytes == -1) {
	    if (errno == EINTR) continue;
	    else return -1;
	}
	if (nbytes == 0)
	    return -1;
	buf[nbytes] = '\000';

	if (buf[nbytes-1] != '\n') {
	    kDebugWarning("%s: Request from kdesu_stub too long.", ID);
	    return -1;
	}

	char *req = buf, *ptr;
	while ((ptr = strchr(req, '\n'))) {
	    *ptr = '\000';

	    // Require a header first.
	    if (!got_header) {
		if (!strcmp(req, "kdesu_stub"))
		    got_header = true;
		req = ptr+1;
		continue;
	    }

	    // Requests:
	    if (!strcmp(req, "display")) {
		QCString str = c.display().latin1();
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(req, "display_auth")) {
		QCString str = c.displayAuth().latin1();
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(req, "dcopserver")) {
		QCString str = commaSeparatedList(c.dcopServer());
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(req, "dcop_auth")) {
		QCString str = commaSeparatedList(c.dcopAuth());
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(req, "ice_auth")) {
		QCString str = commaSeparatedList(c.iceAuth());
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(req, "command")) {
		write(m_Fd, m_Command, m_Command.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(req, "path")) {
		char *path = getenv("PATH");
		if (path)
		    write(m_Fd, path, strlen(path));
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(req, "build_sycoca")) {
		if (m_bSycoca)
		    write(m_Fd, "yes\n", 4);
		else
		    write(m_Fd, "no\n", 3);
	    } else if (!strcmp(req, "end")) {
		done = true;
		break;
	    } else {
		kDebugWarning("%s: Unknown request: %s", ID, req);
		return -1;
	    }

	    req = ptr+1;
	}
    }

    return 0;
}

/*
 * Copy output to stdout until the child process exists.
 * We really have to test with waitpid() here. Merely waiting for EOF on the
 * pty does not work, because the target process may have children still
 * attached to the terminal.
 * I'd prefer doing this asynchronously with SIGCHLD, but I think its
 * cleaner not to mess with global state variables.
 */

int SuProcess::waitForChild(bool echo)
{
    char buf[256];
    int ret, nbytes, state = 1;
    struct timeval tv;

    fd_set fds;
    FD_ZERO(&fds);

    while (1) {
	tv.tv_sec = 0; tv.tv_usec = 500000;
	FD_SET(m_Fd, &fds);
	ret = select(m_Fd+1, &fds, 0L, 0L, &tv);
	if (ret == -1) {
	    if (errno == EINTR) continue;
	    else {
		kDebugError("%s: select(): %m", ID);
		return -1;
	    }
	}
	if (ret > 0) {
	    nbytes = read(m_Fd, buf, 255);
	    if ((nbytes > 0) && echo)
		fwrite(buf, sizeof(char), nbytes, stdout);
	}
	ret = waitpid(m_Pid, &state, WNOHANG);
	if (ret < 0) {
	    kDebugError("%s: waitpid(): %m", ID);
	    break;
	}
	if (ret == m_Pid) {
	    if (WIFEXITED(state))
		state = WEXITSTATUS(state);
	    break;
	}
    }

    return -state;
}
   
/*
 * SetupTTY: Creates a new session.
 */

int SuProcess::SetupTTY()
{    
    // Reset signal handlers
    for (int sig = 1; sig < NSIG; sig++)
	signal(sig, SIG_DFL);
    signal(SIGHUP, SIG_IGN);

    // Close all file handles
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    for (int i = 0; i < (int)rlp.rlim_cur; i++)
	close(i); 

    // Open slave. This will make it our controlling terminal
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0) {
	kDebugWarning("%s: Could not open slave side: %m", ID);
	return -1;
    }

    // Create a new session.
    setsid();

#if defined(__SVR4) && defined(sun)

    // Solaris STREAMS environment.
    // Push these modules to make the stream look like a terminal.
    ioctl(slave, I_PUSH, "ptem");
    ioctl(slave, I_PUSH, "ldterm");

#endif

    // Connect stdin, stdout and stderr
    dup2(slave, 0); dup2(slave, 1); dup2(slave, 2);
    if (slave > 2) 
	close(slave);

    return 0;
}

