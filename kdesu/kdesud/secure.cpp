/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * secure.cpp: Peer credentials for a UNIX socket.
 */

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <kdebug.h>
#include <ksockaddr.h>
#include "secure.h"


/**
 * Under Linux, Socket_security is supported.
 */

#if defined(SO_PEERCRED)

SocketSecurity::SocketSecurity(int sockfd)
{
    ksocklen_t len = sizeof(struct ucred);
    if (getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &len) < 0) {
	kdError() << "getsockopt(SO_PEERCRED) " << perror << endl;
	return;
    }

    ok = true;
}

#else
# if defined(HAVE_GETPEEREID)
SocketSecurity::SocketSecurity(int sockfd)
{
    uid_t euid;
    gid_t egid;
    if (getpeereid(sockfd, &euid, &egid) == 0) {
	cred.uid = euid;
	cred.gid = egid;
	cred.pid = -1;
	ok = true;
    }
}

# else


/**
 * The default version does nothing.
 */

SocketSecurity::SocketSecurity(int sockfd)
{
    static bool warned_him = FALSE;

    if (!warned_him) {
        kdWarning() << "Using void socket security. Please add support for your" << endl;
        kdWarning() << "platform to kdesu/kdesud/secure.cpp" << endl;
        warned_him = TRUE;
    }

    // This passes the test made in handler.cpp
    cred.uid = getuid();
    ok = true;
}

# endif
#endif
