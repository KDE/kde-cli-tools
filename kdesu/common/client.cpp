/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * client.cpp: A client for kdesud.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include <qglobal.h>
#include <qcstring.h>

#include "kdesu.h"
#include "process.h"
#include "client.h"
#include "pathsearch.h"


KDEsuClient::KDEsuClient()
{
    sockfd = -1;
    char *dpy = getenv("DISPLAY");
    if (dpy == 0L) {
	qWarning("DISPLAY is not set");
	return;
    }
    sock.sprintf("/tmp/kdesud_%d_%s", (int) getuid(), dpy);
    connect();
}


KDEsuClient::~KDEsuClient()
{
    if (sockfd >= 0)
	close(sockfd);
}


int KDEsuClient::connect()
{
    if (sockfd >= 0)
	close(sockfd);
    if (access(sock, R_OK|W_OK)) {
	sockfd = -1;
	return -1;
    }

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
	qWarning("KDEsuClient::connect(): socket(): %s", strerror(errno));
	return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock);
    if (::connect(sockfd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0) {
	qWarning("KDEsuClient::connect(): connect(): %s", strerror(errno));
	close(sockfd); sockfd = -1;
	return -1;
    }
    return 0;
}


QCString KDEsuClient::escape(const char *str)
{
    QCString copy(str);

    int n = 0;
    while ((n = copy.find("\\", n)) != -1) {
	copy.insert(n, '\\');
	n += 2;
    }
    n = 0;
    while ((n = copy.find("\"", n)) != -1) {
	copy.insert(n, '\\');
	n += 2;
    }

    copy.prepend("\"");
    copy.append("\"");

    return copy;
}


int KDEsuClient::command(int code, const char *arg1, int arg2)
{
    if (sockfd < 0)
	return -1;

    QCString cmd;

    switch (code) {
    case Cmd_pass:
	cmd.sprintf("PASS %s %d\n", (const char *) escape(arg1), arg2);
	break;
    case Cmd_user:
	cmd.sprintf("USER %s\n", (const char *) escape(arg1));
	break;
    case Cmd_exec:
	cmd.sprintf("EXEC %s\n", (const char *) escape(arg1));
	break;
    case Cmd_del:
	cmd.sprintf("DEL %s\n", (const char *) escape(arg1));
	break;
    case Cmd_ping:
	cmd = "PING\n";
	break;
    case Cmd_stop:
	cmd = "STOP\n";
	break;
    }

    if (send(sockfd, cmd, cmd.length(), 0) != (int) cmd.length())
	return -1;
    
    char buf[100];
    int nbytes = recv(sockfd, buf, 99, 0);
    if (nbytes <= 0) {
	qWarning("KDEsuClient::command(): no reply");
	return -1;
    }
    buf[nbytes] = '\000';

    QString reply = buf;
    if (reply != "OK\n")
	return -1;

    return 0;
}


int KDEsuClient::setPass(const char *pass, int timeout)
{
    return command(Cmd_pass, pass, timeout);
}

int KDEsuClient::setUser(const char *user)
{
    if (user == 0L)
	user = "root";

    return command(Cmd_user, user);
}

int KDEsuClient::exec(const char *cmd)
{
    return command(Cmd_exec, cmd);
}

int KDEsuClient::delCommand(const char *cmd)
{
    return command(Cmd_del, cmd);
}

int KDEsuClient::ping()
{
    return command(Cmd_ping);
}

int KDEsuClient::stopServer()
{
    return command(Cmd_stop);
}

int KDEsuClient::startServer()
{
    PathSearch PS;

    QCString daemon = PS.locate("kdesud");
    if (daemon.isEmpty()) {
	qWarning("KDEsuClient::startServer(): kdesud not found -- no password keeping");
	return -1;
    }
    struct stat sbuf;
    if (stat(daemon, &sbuf) < 0) {
	qWarning("KDEsuClient::startServer(): stat(\"%s\"): %s", 
		(const char *) daemon, strerror(errno));
	return -1;
    }
    if (!(sbuf.st_mode & S_ISGID)) {
	qWarning("KDEsuClient::startServer(): kdesud not setgid -- no password keeping");
	return -1;
    }

    UserProcess proc("kdesud &");
    return proc.exec();
}
