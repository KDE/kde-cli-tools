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
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <pwd.h>

#include <queue>
#include <map>
#include <fstream>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "kdesud.h"
#include "repo.h"
#include "handler.h"
#include "debug.h"
#include "client.h"
#include "kdesu.h"

// Globals

Repository *repo;
int _show_dbg = 0;
int _show_wrn = 1;
const char *Version = VERSION;
const char *Help = "Try `kdesud -h' for more information.";
const char *Email = "<g.t.jansen@stud.tue.nl";
string sock;


// Unlink the socket atexit()

void cleanup(void)
{
    unlink(sock.c_str());
}

// Global signal handler

void signal_exit(int sig)
{
    error("Exiting on signal %d", sig);
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
	debug("PID %d exited", (int) pid);
    }
}

// Erase a <string>

void clear(string &s, string::size_type npos)
{
    if (npos == 0)
	npos = s.size();
    s.replace(0, npos, npos, 'x');
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
	error("DISPLAY is not set");
	return -1;
    }

    sock = "/tmp/kdesud_";
    char buf[20];
    sprintf(buf, "%d_", (int) getuid());
    sock += buf; sock += display;

    if (!access(sock.c_str(), R_OK|W_OK)) {
	KDEsuClient client;
	if (client.ping() == -1) {
	    warning("stale socket exists");
	    if (unlink(sock.c_str())) {
		error("Could not delete stale socket");
		return -1;
	    }
	} else {
	    error("kdesud is already running");
	    return -1;
	}

    }

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
	xerror("socket(): %s");
	return -1;
    }
    debug("Created socket: %s", sock.c_str());

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock.c_str());
    addrlen = SUN_LEN(&addr);
    if (bind(sockfd, (struct sockaddr *)&addr, addrlen) < 0) {
	xerror("bind(): %s");
	return -1;
    }

    atexit(cleanup);

    struct linger lin;
    lin.l_onoff = lin.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, 
		   static_cast<char*>((char *)&lin), 
		   sizeof(linger)) < 0) {
	xerror("setsockopt(SO_LINGER): %s");
	return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
		   static_cast<char*>((char *)&opt), 
		   sizeof(opt)) < 0) {
	xerror ("setsockopt(SO_REUSEADDR): %s");
	return -1;
    }
    opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, 
		   static_cast<char*>((char *)&opt),
		   sizeof(opt)) < 0) {
	xerror("setsockopt(SO_KEEPALIVE): %s");
	return -1;
    }

    chmod(sock.c_str(), 0600);
    debug("Chmod to 0600");

    return sockfd;
}


/**
 * Main program
 */

int main(int argc, char *argv[])
{
    int sockfd, i;
    struct timeval tv;
    struct sigaction sa;

    int c;

    while ((c = getopt(argc, argv, "hvdq")) != -1) {
	switch (c) {
	case 'h':
	    cerr << "kdesud [OPTIONS]...\n";
	    cerr << "KDE su daemon (password keeper for KDE su).\n";
	    cerr << endl;
	    cerr << "Options:\n";
	    cerr << "  -d      Give debug information\n";
	    cerr << "  -q      Be quiet (no warnings)\n";
	    cerr << "  -v      Show verion information\n";
	    cerr << endl;
	    cerr << "Please report bugs to " << Email << endl;
	    exit(0);
	case 'v':
	    cerr << "kdesud version " << Version << endl;
	    cerr << endl;
	    cerr << "  Copyright (c) 1999 Geert Jansen " << Email << endl;
	    cerr << endl;
	    exit(0);
	case 'd':
	    _show_dbg++;
	    break;
	case 'q':
	    _show_wrn = 0;
	    break;
	case '?': default:
	    cerr << "kdesud: invalid option" << endl;
	    cerr << Help << endl;
	    exit(1);
	}
    }

    // Set core dump size to 0
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
	xerror("setrlimit(): %s");
	exit(1);
    }

    // Create the Unix socket.
    sockfd = create_socket();
    if (sockfd < 0)
	exit(1);
    if (listen(sockfd, 1) < 0) {
	xerror("listen(): %s");
	exit(1);
    }

    // Allocate the repository 
    repo = new Repository;

    // connection handlers
    vector<ConnectionHandler *> handler(FD_SETSIZE);

    // Signal handlers 
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

    // Expire at least every 5 seconds 
    tv.tv_sec = 5; tv.tv_usec = 0;

    // Main execution loop 

    ksize_t addrlen;
    struct sockaddr_un clientname;

    fd_set tmp_fds, active_fds;
    FD_ZERO(&active_fds);
    FD_SET(sockfd, &active_fds);
    while (1) {

	tmp_fds = active_fds;
	if (select(FD_SETSIZE, &tmp_fds, 0L, 0L, &tv) < 0) {
	    if (errno == EINTR)
		continue;
	    xerror("select(): %s");
	    exit(1);
	}
	
	tv.tv_sec = 5; tv.tv_usec = 0;

	repo->expire();

	for (i=0; i<FD_SETSIZE; i++) {
	    if (!FD_ISSET(i, &tmp_fds)) 
		continue;

	    if (i == sockfd) {
		// new connection
		int fd;
		addrlen = 64;
		fd = accept(sockfd, (struct sockaddr *) &clientname, &addrlen);
		if (fd < 0) {
		    xerror("accept(): %s");
		    exit(1);
		}
		handler[fd] = new ConnectionHandler(fd);
		FD_SET(fd, &active_fds);
		debug("Accepted new connection on fd %d", fd);
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
    warning("???");
}

