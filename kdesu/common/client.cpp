/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * client.cpp: A client for kdesud.
 */

#include <iostream>
#include <string>

#include <stdio.h> // sprintf
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "kdesu.h"
#include "process.h"
#include "client.h"
#include "debug.h"
#include "pathsearch.h"


KDEsuClient::KDEsuClient()
{
    sockfd = -1;
    char *dpy = getenv("DISPLAY");
    if (dpy == 0L) {
	error("DISPLAY is not set");
	return;
    }
    char uid[20];
    sprintf(uid, "%d_", (int) getuid());
    sock = string("/tmp/kdesud_") + uid + dpy;
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
    if (access(sock.c_str(), R_OK|W_OK)) {
	sockfd = -1;
	return -1;
    }

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
	xerror("socket(): %s");
	return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock.c_str());
    if (::connect(sockfd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0) {
	xerror("connect(): %s");
	close(sockfd); sockfd = -1;
	return -1;
    }
    return 0;
}


string KDEsuClient::escape(const char *str)
{
    string copy = str;

    string::size_type n = 0;
    while ((n = copy.find_first_of("\"\\", n)) != string::npos) {
	copy.insert(n, 1, '\\');
	n += 2;
    }
    copy.insert(0, "\"");
    copy += '\"';

    return copy;
}


int KDEsuClient::command(int code, const char *arg1, int arg2)
{
    if (sockfd < 0)
	return -1;

    string cmd;
    char buf[100];

    switch (code) {
    case Cmd_pass:
	sprintf(buf, "%d", arg2);
	cmd = string("PASS ") + escape(arg1) + ' ' + buf + '\n';
	break;
    case Cmd_user:
	cmd = string("USER ") + escape(arg1) + '\n';
	break;
    case Cmd_exec:
	cmd = string("EXEC ") + escape(arg1) + '\n';
	break;
    case Cmd_del:
	cmd = string("DEL ") + escape(arg1) + '\n';
	break;
    case Cmd_ping:
	cmd = "PING\n";
	break;
    case Cmd_stop:
	cmd = "STOP\n";
	break;
    }

    if (send(sockfd, cmd.c_str(), cmd.size(), 0) != (int) cmd.size())
	return -1;
    
    string reply;
    int nbytes;
    while ((nbytes = recv(sockfd, buf, 99, 0)) > 0) {
	buf[nbytes] = '\000';
	reply += buf;
	if (reply.find('\n') != string::npos)
	    break;
    }

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

    const char *daemon = PS.locate("kdesud");
    if (daemon == 0L) {
	error("kdesud not found -- password keeping disabled");
	return -1;
    }
    struct stat sbuf;
    if (stat(daemon, &sbuf) < 0) {
	xerror("stat(%s): %s", daemon);
	return -1;
    }
    if (!(sbuf.st_mode & S_ISGID)) {
	error("kdesud not setgid -- password keeping disabled");
	return -1;
    }

    UserProcess proc("kdesud &");
    return proc.exec();
}
