/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
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
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <qglobal.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "repo.h"
#include "handler.h"
#include "client.h"
#include "kdesu.h"

#include <X11/X.h>
#include <X11/Xlib.h>

// Globals

Repository *repo;
const char *Version = "1.01";
QCString sock;
Display *x11Display;


// Borrowed from kdebase/kaudio/kaudioserver.cpp

int xio_errhandler(Display *)
{
    kDebugError("kdesud: Fatal IO error, exiting...");
    exit(1);
}

int initXconnection()
{
    x11Display = XOpenDisplay(NULL);
    if (x11Display != 0L) {
	XSetIOErrorHandler(xio_errhandler);
	XCreateSimpleWindow(x11Display, DefaultRootWindow(x11Display), 
		0, 0, 1, 1, 0,
		BlackPixelOfScreen(DefaultScreenOfDisplay(x11Display)),
		BlackPixelOfScreen(DefaultScreenOfDisplay(x11Display)));
	return XConnectionNumber(x11Display);
    } else {
	kDebugWarning("kdesud: Can't connect to the X Server.");
	kDebugWarning("kdesud: Might not terminate at end of session.");
	return -1;
    }
}

// Global signal handler

void signal_exit(int sig)
{
    kDebugError("Exiting on signal %d", sig); 
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
	kDebugInfo("kdesud: PID %d exited", (int) pid);
    }
}

// Cleanup atexit()

void cleanup(void)
{
    unlink(sock);
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
	kDebugWarning("DISPLAY is not set");
	return -1;
    }

    sock.sprintf("/tmp/kdesud_%d_%s", (int) getuid(), display);

    if (!access(sock, R_OK|W_OK)) {
	KDEsuClient client;
	if (client.ping() == -1) {
	    kDebugWarning("stale socket exists");
	    if (unlink(sock)) {
		kDebugWarning("Could not delete stale socket");
		return -1;
	    }
	} else {
	    kDebugWarning("kdesud is already running");
	    return -1;
	}

    }

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
	kDebugPError("socket()");
	return -1;
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock);
    addrlen = SUN_LEN(&addr);
    if (bind(sockfd, (struct sockaddr *)&addr, addrlen) < 0) {
	kDebugPError("bind()");
	return -1;
    }
    atexit(cleanup);

    struct linger lin;
    lin.l_onoff = lin.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *) &lin,
		   sizeof(linger)) < 0) {
	kDebugPError("setsockopt(SO_LINGER)");
	return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
		   sizeof(opt)) < 0) {
	kDebugPError("setsockopt(SO_REUSEADDR)");
	return -1;
    }
    opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *) &opt,
		   sizeof(opt)) < 0) {
	kDebugPError("setsockopt(SO_KEEPALIVE)");
	return -1;
    }
    chmod(sock, 0600);
    return sockfd;
}


/**
 * Main program
 */

int main(int argc, char *argv[])
{
    KAboutData aboutData("kdesud", I18N_NOOP("KDE su daemon"),
	    Version, I18N_NOOP("Daemon used by kdesu"),
	    KAboutData::License_Artistic,
	    "Copyright (c) 1999,2000 Geert Jansen");
    aboutData.addAuthor("Geert Jansen", I18N_NOOP("Author"),
	    "jansen@kde.org", "http://www.stack.nl/~geertj/");
    KCmdLineArgs::init(argc, argv, &aboutData);

    // Set core dump size to 0
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
	kDebugPError("setrlimit()");
	exit(1);
    }

    // Create the Unix socket.
    int sockfd = create_socket();
    if (sockfd < 0)
	exit(1);
    if (listen(sockfd, 1) < 0) {
	kDebugPError("listen()");
	exit(1);
    }

    // Ok, we're accepting connections. Fork to the background.
    pid_t pid = fork();
    if (pid == -1) {
	kDebugPError("fork()");
	exit(1);
    }
    if (pid)
	exit(0);

    // Make sure we exit when the display gets closed.
    int x11Fd = initXconnection();

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

    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0L);

    // Main execution loop 

    ksize_t addrlen;
    struct sockaddr_un clientname;
    struct timeval tv;

    fd_set tmp_fds, active_fds;
    FD_ZERO(&active_fds);
    FD_SET(sockfd, &active_fds);
    if (x11Fd != -1)
	FD_SET(x11Fd, &active_fds);
    while (1) {

	tmp_fds = active_fds;
	tv.tv_sec = 5; tv.tv_usec = 0;
	if (select(FD_SETSIZE, &tmp_fds, 0L, 0L, &tv) < 0) {
	    if (errno == EINTR)
		continue;
	    kDebugPError("select()");
	    exit(1);
	}
	
	repo->expire();

	for (int i=0; i<FD_SETSIZE; i++) {
	    if (!FD_ISSET(i, &tmp_fds)) 
		continue;

	    if (i == x11Fd) {
		// Discard X events
		XEvent event_return;
		if (x11Display)
		    XNextEvent(x11Display, &event_return);
	    }

	    if (i == sockfd) {
		// new connection
		int fd;
		addrlen = 64;
		fd = accept(sockfd, (struct sockaddr *) &clientname, &addrlen);
		if (fd < 0) {
		    kDebugPError("accept()");
		    continue;
		}
		handler[fd] = new ConnectionHandler(fd);
		FD_SET(fd, &active_fds);
		kDebugInfo("kdesud: Accepted new connection on fd %d", fd);
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
    kDebugWarning("???");
}

