/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
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

#include "process.h"
#include "pty.h"
#include "xcookie.h"
#include "pathsearch.h"


SuProcess::SuProcess(const char *command)
{
    m_Command = command;
    m_User = "root";
    m_bTerminal = false;
    m_bErase = false;
}


/**
 * Execute a command with su(1).
 */

int SuProcess::exec(char *password, bool check_only)
{
    // Open a pty/tty pair.
    PTY *pty = new PTY();
    int master = pty->getpt();
    if (master < 0)
	return -1;
    if ((pty->grantpt() < 0) || (pty->unlockpt() < 0)) {
	qWarning("SuProcess::exec(): master setup failed");
	return -1;
    }

    pid_t pid;
    if ((pid = fork()) == -1) {
	qWarning("SuProcess::exec(): fork(): %s", strerror(errno));
	return -1;
    }

    if (pid) {
	// Parent: Have the necessary conversations.

	if (ConverseSU(master, password, pty->ptsname()) < 0) {
	    qWarning("SuProcess::exec(): Conversation with su failed");
	    return -1;
	} 
	if (m_bErase) {
	    int len = strlen(password);
	    for (int i=0; i<len; i++)
		password[i] = 'x';
	}
	if (!check_only && (ConverseStub(master, m_Command) < 0)) {
	    qWarning("SuProcess::exec(): Converstation with kdesu_stub failed");
	    return -1;
	}
	CopyOutput(master, check_only || m_bTerminal);

	int ret;
	waitpid(pid, &ret, 0);

	delete pty;

    	if (WIFEXITED(ret) && (!WEXITSTATUS(ret)))
	   return 0;
	return -1;

    } else {

	const char *prog;
	if (check_only) 
	    prog = "true";
	else
	    prog = "kdesu_stub";

	PathSearch PS;
	QCString path = PS.locate(prog);
	if (path.isEmpty()) {
	    qWarning("SuProcess::exec(): %s not found", prog);
	    _exit(1);
	}

	if (SetupTTY(pty->ptsname()) < 0)
	    _exit(1);

	// Terminal output will go through the tty from now on. It is visible
	// only if the user uses the '-t' switch.

	execl(_PATH_SU, "su", (const char *) m_User, "-c", 
		(const char *) path, NULL);

	// Exec failed..
	qWarning("SuProcess::exec(): execl(\"%s\"): %s", _PATH_SU, 
		strerror(errno));
	_exit(1);
    }

    return 0;
}

/**
 * Copy output to stdout until end of file or error.
 */
int SuProcess::CopyOutput(int fd, bool echo)
{
    char buf[256];
    int nbytes;
    while (1) {
	nbytes = read(fd, buf, 255);
	if (nbytes == -1) {
	    if (errno == EINTR) continue;
	    else break;
	}
	if (nbytes == 0)
	    break;
	if (echo)
	    fwrite(buf, sizeof(char), nbytes, stdout);
    }
    return 0;
}


/**
 * Conversation with su: feed the password.
 */
int SuProcess::ConverseSU(int fd, const char *password, 
	const char *tty)
{	
    char buf[256]; 
    int nbytes, i;
    int state = 0;

    while (state < 2) {
	nbytes = read(fd, buf, 255);
	if (nbytes == -1) {
	    if (errno == EINTR) continue;
	    else return -1;
	}
	if (nbytes == 0)
	    return -1;
	buf[nbytes] = '\000';

	switch (state) {
	case 0:
	    // Write password
	    if (strchr(buf, ':')) {
		WaitSlave(tty);
		write(fd, password, strlen(password));
		write(fd, "\n", 1);
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


/**
 * Conversation with kdesu_stub. This is how we pass the cookie, DISPLAY,
 * and command to the target process. kdesu_stub is already running with
 * target priviliges.
 */

int SuProcess::ConverseStub(int fd, const char *command)
{
    char buf[256];
    int nbytes;

    Cookie C;

    while (1) {
	nbytes = read(fd, buf, 255);
	if (nbytes == -1) {
	    if (errno == EINTR) continue;
	    else return -1;
	}
	if (nbytes == 0)
	    return -1;
	buf[nbytes] = '\000';

	if (buf[nbytes-1] != '\n') {
	    qWarning("SuProcess:ConverseStub(): request from kdesu_stub too long");
	    return -1;
	}
	buf[nbytes-1] = '\000';
	if (buf[nbytes-2] == '\r')
	    buf[nbytes-2] = '\000';
	
	if (!strcmp(buf, "cookie")) {
	    // X cookie
	    qDebug("SuProcess::ConverseStub(): request for cookie");
	    if (C.cookie())
		write(fd, C.cookie(), strlen(C.cookie()));
	    write(fd, "\n", 1);
	} else if (!strcmp(buf, "display")) {
	    // X Display
	    qDebug("SuProcess::ConverseStub(): request for display");
	    char *ptr = getenv("DISPLAY");
	    if (ptr)
		write(fd, ptr, strlen(ptr));
	    write(fd, "\n", 1);
	} else if (!strcmp(buf, "proto")) {
	    // X cookie protocol
	    qDebug("SuProcess::ConverseStub(): request for proto");
	    if (C.proto())
		write(fd, C.proto(), strlen(C.proto()));
	    write(fd, "\n", 1);
	} else if (!strcmp(buf, "path")) {
	    // X Display
	    qDebug("SuProcess::ConverseStub(): request for PATH");
	    char *ptr = getenv("PATH");
	    if (ptr)
		write(fd, ptr, strlen(ptr));
	    write(fd, "\n", 1);
	} else if (!strcmp(buf, "command")) {
	    // Target command
	    qDebug("SuProcess::ConverseStub(): request for command");
	    write(fd, command, strlen(command));
	    write(fd, "\n", 1);
	} else if (!strcmp(buf, "end")) {
	    // end the conversation
	    qDebug("SuProcess::ConverseStub(): Ending stub conversation");
	    break;
	} else {
	    qWarning("SuProcess::ConverseStub(): unknown request: %s", buf);
	    return -1;
	}
    }

    return 0;
}

    
    
/**
 * WaitSlave: Wait until the terminal is set into no echo mode.
 *	      At least one su (RH6 w/ Linux-PAM patches) sets noecho mode 
 *	      AFTER writing the "Password: " prompt, using TCSAFLUSH. This 
 *	      flushes the terminal I/O queues, possibly taking the password 
 *	      with it. So we wait until no echo mode is set before writing
 *	      the password.
 */

int SuProcess::WaitSlave(const char *ttyname)
{
    struct termios tio;
    struct timeval tv;

    int fd = open(ttyname, O_RDWR|O_NOCTTY);
    if (fd < 0)
	return -1;

    while (1) {
	if (tcgetattr(fd, &tio) < 0) {
	    qWarning("SuProcess::WaitSlave(): tcgetattr(): %s",
		    strerror(errno));
	    close(fd);
	    return -1;
	}
	if (tio.c_lflag & ECHO) {
	    qDebug("SuProcess::WaitSlave(): echo mode still on");
	    // sleep 1/10 sec
	    tv.tv_sec = 0; tv.tv_usec = 100000;
	    select(fd, 0L, 0L, 0L, &tv);
	    continue;
	}
	qDebug("SuProcess::WaitSlave(): echo mode off");
	break;
    }
    close(fd);
    return 0;
}
	    
/**
 * SetupTTY: Creates a new session.
 */

int SuProcess::SetupTTY(const char *ttyname)
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

    // Create a new session.
    setsid();

    // Open the pty slave. This will make it our controlling terminal.
    int fd = open(ttyname, O_RDWR);
    if (fd < 0)
	return -1;

#if defined(__SVR4) && defined(sun)

    // Solaris STREAMS environment.
    // Push these modules to make the stream look like a terminal.
    ioctl(fd, I_PUSH, "ptem");
    ioctl(fd, I_PUSH, "ldterm");

#endif

    // Connect stdin, stdout and stderr
    dup2(fd, 0); dup2(fd, 1); dup2(fd, 2);
    if (fd > 2) 
	close(fd);

    return 0;
}


UserProcess::UserProcess(const char *command)
	: m_Command(command)
{
}

int UserProcess::exec()
{
    if (m_Command.isEmpty())
	return -1;

    pid_t pid;
    if ((pid = fork()) == -1) {
	qWarning("UserProcess::exec(): fork(): %s", strerror(errno));
	return -1;
    }

    if (pid) {
	// Parent
	int ret;
	waitpid(pid, &ret, 0);
    	if (WIFEXITED(ret) && !WEXITSTATUS(ret))
	    return 0;
	return -1;
    } else {
	execl("/bin/sh", "sh", "-c", (const char *) m_Command, NULL);
	qWarning("UserProcess::exec(): exec(\"/bin/sh\"): %s",
		strerror(errno));
	_exit(1);
    }
}


