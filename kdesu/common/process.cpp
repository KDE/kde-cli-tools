/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
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
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

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


PtyProcess::PtyProcess()
{
    m_bTerminal = false;
    m_bErase = false;
    m_bSycoca = true;
    m_bXOnly = false;
    m_pPTY = 0L;
}


int PtyProcess::init()
{
    delete m_pPTY;
    m_pPTY = new PTY();
    m_Fd = m_pPTY->getpt();
    if (m_Fd < 0)
	return -1;
    if ((m_pPTY->grantpt() < 0) || (m_pPTY->unlockpt() < 0)) {
	kDebugWarning("%s: Master setup failed.", ID);
	m_Fd = -1;
	return -1;
    }
    m_TTY = m_pPTY->ptsname();
    m_pCookie = new KCookie();
    return 0;
}


PtyProcess::~PtyProcess()
{
    delete m_pPTY;
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

int PtyProcess::WaitSlave()
{
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0) {
	kDebugError("%s: Could not open slave tty", ID);
	return -1;
    }

    struct termios tio;
    struct timeval tv;
    while (1) {
	if (tcgetattr(slave, &tio) < 0) {
	    kDebugPError("%s: tcgetattr()", ID);
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


int PtyProcess::disableLocalEcho()
{
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0) {
	kDebugError("%s: Could not open slave tty.", ID);
	return -1;
    }
    struct termios tio;
    if (tcgetattr(slave, &tio) < 0) {
	kDebugPError("%s: tcgetattr()", ID);
	close(slave); return -1;
    }
    tio.c_lflag &= ~ECHO;
    if (tcsetattr(slave, TCSANOW, &tio) < 0) {
	kDebugPError("%s: tcsetattr()", ID);
	close(slave); return -1;
    }
    close(slave);
    return 0;
}


QCString PtyProcess::commaSeparatedList(QStringList lst)
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

int PtyProcess::ConverseStub(bool check_only)
{
    char buf[256];
    int nbytes, pos;

    // This makes parsing a lot easier.
    disableLocalEcho();

    QCString inbuf, line;
    while (1) {
	nbytes = read(m_Fd, buf, 255);
	if (nbytes == -1) {
	    if (errno == EINTR) continue;
	    else return -1;
	}
	if (nbytes == 0)
	    return -1;
	buf[nbytes] = '\000';

	inbuf += buf;
	while ((pos = inbuf.find('\n')) != -1) {
	    line = inbuf.left(pos);
	    inbuf.remove(0, pos+1);
	    if (!strcmp(line, "kdesu_stub")) {
		if (check_only)
		    write(m_Fd, "stop\n", 5);
		else
		    write(m_Fd, "ok\n", 3);
	    } else if (!strcmp(line, "display")) {
		QCString str = display().latin1();
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(line, "display_auth")) {
		QCString str = displayAuth().latin1();
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(line, "dcopserver")) {
		QCString str = commaSeparatedList(dcopServer());
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(line, "dcop_auth")) {
		QCString str = commaSeparatedList(dcopAuth());
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(line, "ice_auth")) {
		QCString str = commaSeparatedList(iceAuth());
		write(m_Fd, str, str.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(line, "command")) {
		write(m_Fd, m_Command, m_Command.length());
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(line, "path")) {
		char *path = getenv("PATH");
		if (path)
		    write(m_Fd, path, strlen(path));
		write(m_Fd, "\n", 1);
	    } else if (!strcmp(line, "build_sycoca")) {
		if (m_bXOnly)
		    write(m_Fd, "no\n", 3);
		else if (m_bSycoca)
		    write(m_Fd, "yes\n", 4);
		else
		    write(m_Fd, "check\n", 6);
	    } else if (!strcmp(line, "end")) {
		return 0;
	    } else {
		kDebugWarning("%s: Unknown request: -->%s<--", ID, buf);
		return -1;
	    }
	}
    }
    return 0;
}

/*
 * Copy output to stdout until the child process exists, or a line of output
 * matches `m_Exit'.
 * We have to use waitpid() to test for exit. Merely waiting for EOF on the
 * pty does not work, because the target process may have children still
 * attached to the terminal.
 */

int PtyProcess::waitForChild(bool echo)
{
    char buf[256];
    int pos, ret, nbytes, state, retval = 1;
    struct timeval tv;

    fd_set fds;
    FD_ZERO(&fds);

    echo |= m_bTerminal;
    QCString inbuf, line;
    while (1) {
	tv.tv_sec = 1; tv.tv_usec = 0;
	FD_SET(m_Fd, &fds);
	ret = select(m_Fd+1, &fds, 0L, 0L, &tv);
	if (ret == -1) {
	    if (errno == EINTR) continue;
	    else {
		kDebugPError("%s: select()", ID);
		return -1;
	    }
	}

	// Buffer data to complete lines.
	if (ret > 0) {
	    nbytes = read(m_Fd, buf, 255);
	    if (nbytes > 0) {
		buf[nbytes] = '\0';
		inbuf += buf;
	    }
	}
	while ((pos = inbuf.find('\n')) != -1) {
	    line = inbuf.left(pos+1);
	    inbuf.remove(0, pos+1);
	    if (!m_Exit.isEmpty() && !strnicmp(line, m_Exit, m_Exit.length()))
		kill(m_Pid, SIGTERM);
	    if (echo)
		fputs(line, stdout);
	}

	// Check if the process is still alive
	ret = waitpid(m_Pid, &state, WNOHANG);
	if (ret < 0) {
	    kDebugPError("%s: waitpid()", ID);
	    break;
	}
	if (ret == m_Pid) {
	    if (WIFEXITED(state))
		retval = WEXITSTATUS(state);
	    break;
	}
    }

    return -retval;
}
   
/*
 * SetupTTY: Creates a new session. The filedescriptor "fd" cannot be closed
 * until the slave pty is opened. This fd is actually also connected to it,
 * and it makes sure the peer doesn't get EIO when reading before we opened
 * it.
 */

int PtyProcess::SetupTTY(int fd)
{    
    // Reset signal handlers
    for (int sig = 1; sig < NSIG; sig++)
	signal(sig, SIG_DFL);
    signal(SIGHUP, SIG_IGN);

    // Close all file handles
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    for (int i = 0; i < (int)rlp.rlim_cur; i++)
	if (i != fd) close(i); 

    // Create a new session.
    setsid();

    // Open slave. This will make it our controlling terminal
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0) {
	kDebugPError("%s: Could not open slave side", ID);
	return -1;
    }
    close(fd);

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

    // Disable OPOST processing. Otherwise, '\n' are (on Linux at least)
    // translated to '\r\n'.
    struct termios tio;
    if (tcgetattr(0, &tio) < 0) {
	kDebugPError("%s: tcgetattr()", ID);
	return -1;
    }
    tio.c_oflag &= ~OPOST;
    if (tcsetattr(0, TCSANOW, &tio) < 0) {
	kDebugPError("%s: tcsetattr()", ID);
	return -1;
    }

    return 0;
}

