/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 *
 * kdesud.cpp: KDE su daemon. Offers "keep password" functionality to kde su.
 *
 * The socket /tmp/kdesud_$(uid)_$(display) is used for communication with
 * client programs.
 *
 * The protocol: Client initiates the connection. All commands and responses
 * are terminated by a newline.
 *
 *   Client                     Server     Description
 *   ------                     ------     -----------
 *
 *   PASS <pass> <timeout>      OK         Set password for commands in
 *                                         this session. Password is
 *                                         valid for <timeout> seconds.
 *
 *   USER <user>                OK         Set the target user [required]
 *
 *   EXEC <command>             OK         Execute command <command>. If
 *                              NO         <command> has been executed
 *                                         before (< timeout) no PASS
 *                                         command is needed.
 *                                              
 *   DEL <command>              OK         Delete password for command
 *                              NO         <command>.
 *
 *   PING                       OK         Ping the server (diagnostics).
 */


#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <pwd.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <qglobal.h>

#include "repo.h"
#include "handler.h"
#include "client.h"
#include "kdesu.h"

// Globals

Repository *repo;
int _show_dbg = 0;
int _show_wrn = 1;
const char *Version = VERSION;
const char *Help = "Try `kdesud -h' for more information.";
const char *Email = "<g.t.jansen@stud.tue.nl";
QCString sock;


// Unlink the socket atexit()

void cleanup(void)
{
    unlink(sock);
}

// Global signal handler

void signal_exit(int sig)
{
    fprintf(stderr, "Exiting on signal %d\n", sig); 
    exit(1);
}

// SIGCHLD handler

void sigchld_handler(int)
{
    int status;
    pid_t pid;

    while (1) {
	pid = waitpid((pid_t) -1, &status, WNOHANG);
	if (pid <= 0)
	    break;
	qDebug("PID %d exited", (int) pid);
    }
}


/**
 * Creates an AF_UNIX socket in /tmp, mode 0600.
 */

int create_socket()
{
    int sockfd;
    char *display;
    ksize_t addrlen;

    display = getenv("DISPLAY");
    if (!display) {
	qWarning("DISPLAY is not set");
	return -1;
    }

    sock.sprintf("/tmp/kdesud_%d_%s", (int) getuid(), display);

    if (!access(sock, R_OK|W_OK)) {
	KDEsuClient client;
	if (client.ping() == -1) {
	    qWarning("stale socket exists");
	    if (unlink(sock)) {
		qWarning("Could not delete stale socket");
		return -1;
	    }
	} else {
	    qWarning("kdesud is already running");
	    return -1;
	}

    }

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
	qWarning("socket(): %s", strerror(errno));
	return -1;
    }
    qDebug("Created socket: %s", (const char *) sock);

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock);
    addrlen = SUN_LEN(&addr);
    if (bind(sockfd, (struct sockaddr *)&addr, addrlen) < 0) {
	qWarning("bind(): %s", strerror(errno));
	return -1;
    }

    atexit(cleanup);

    struct linger lin;
    lin.l_onoff = lin.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *) &lin,
		   sizeof(linger)) < 0) {
	qWarning("setsockopt(SO_LINGER): %s", strerror(errno));
	return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
		   sizeof(opt)) < 0) {
	qWarning("setsockopt(SO_REUSEADDR): %s", strerror(errno));
	return -1;
    }
    opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *) &opt,
		   sizeof(opt)) < 0) {
	qWarning("setsockopt(SO_KEEPALIVE): %s", strerror(errno));
	return -1;
    }

    chmod(sock, 0600);
    qDebug("Chmod to 0600");

    return sockfd;
}


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


/**
 * Main program
 */

int main(int argc, char *argv[])
{
    qInstallMsgHandler(msgHandler);

    int c;
    while ((c = getopt(argc, argv, "hvdq")) != -1) {
	switch (c) {
	case 'h':
	    printf("kdesud [OPTIONS]...\n");
	    printf("KDE su daemon (password keeper for KDE su).\n");
	    printf("\n");
	    printf("Options:\n");
	    printf("  -d      Give debug information\n");
	    printf("  -q      Be quiet (no warnings)\n");
	    printf("  -v      Show verion information\n");
	    printf("\n");
	    printf("Please report bugs to %s\n", Email);
	    exit(0);
	case 'v':
	    printf("kdesud version %s\n", Version);
	    printf("\n");
	    printf("  Copyright (c) 1999 Geert Jansen %s\n", Email);
	    printf("\n");
	    exit(0);
	case 'd':
	    _show_dbg++;
	    break;
	case 'q':
	    _show_wrn = 0;
	    break;
	case '?': default:
	    printf("kdesud: invalid option\n");
	    printf("%s\n", Help);
	    exit(1);
	}
    }

    // Set core dump size to 0
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
	qWarning("setrlimit(): %s", strerror(errno));
	exit(1);
    }

    // Create the Unix socket.
    int sockfd = create_socket();
    if (sockfd < 0)
	exit(1);
    if (listen(sockfd, 1) < 0) {
	qWarning("listen(): %s", strerror(errno));
	exit(1);
    }

    // Allocate the repository 
    repo = new Repository;

    // connection handlers
    ConnectionHandler *handler[FD_SETSIZE];

    // Signal handlers 
    struct sigaction sa;
    sa.sa_handler = signal_exit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGHUP, &sa, 0L);
    sigaction(SIGINT, &sa, 0L);
    sigaction(SIGTERM, &sa, 0L);
    sigaction(SIGQUIT, &sa, 0L);

    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, 0L);

    // Main execution loop 

    ksize_t addrlen;
    struct sockaddr_un clientname;
    struct timeval tv;

    fd_set tmp_fds, active_fds;
    FD_ZERO(&active_fds);
    FD_SET(sockfd, &active_fds);
    while (1) {

	tmp_fds = active_fds;
	tv.tv_sec = 5; tv.tv_usec = 0;
	if (select(FD_SETSIZE, &tmp_fds, 0L, 0L, &tv) < 0) {
	    if (errno == EINTR)
		continue;
	    qFatal("select(): %s", strerror(errno));
	}
	
	repo->expire();

	for (int i=0; i<FD_SETSIZE; i++) {
	    if (!FD_ISSET(i, &tmp_fds)) 
		continue;

	    if (i == sockfd) {
		// new connection
		int fd;
		addrlen = 64;
		fd = accept(sockfd, (struct sockaddr *) &clientname, &addrlen);
		if (fd < 0) {
		    qWarning("accept(): %s", strerror(errno));
		    continue;
		}
		handler[fd] = new ConnectionHandler(fd);
		FD_SET(fd, &active_fds);
		qDebug("Accepted new connection on fd %d", fd);
		continue;
	    }

	    // handle alreay established connection
	    if (handler[i]->handle() < 0) {
		delete handler[i];
		close(i);
		FD_CLR(i, &active_fds);
	    }
	}
    }
    qWarning("???");
}

