/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * secure.cpp: Peer credentials for a UNIX socket.
 */

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "secure.h"
#include "debug.h"


/**
 * Under Linux, Socket_security is supported.
 */

#if defined(SO_PEERCRED) 

SocketSecurity::SocketSecurity(int sockfd)
{
    ksize_t len = sizeof(struct ucred);
    if (getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &len) < 0) {
	xerror("getsockopt(SO_PEERCRED): %s");
	return; 
    }

    ok = true;
}

#else


/**
 * The default version does nothing.
 */

SocketSecurity::SocketSecurity(int sockfd)
{
    warning("Using void socket security");

    // This passes the test made in handler.cpp
    cred.uid = getuid();
    ok = true;
}

#endif 
