/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __Secure_h_included__
#define __Secure_h_included__

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>

#ifndef HAVE_STRUCT_UCRED

// `struct ucred' is not defined in glibc 2.0.

struct ucred {
    int32_t   pid;
    int32_t   uid;
    int32_t   gid;
};

#endif // HAVE_STRUCT_UCRED


/**
 * The Socket_security class autheticates the peer for you. It provides
 * the process-id, user-id and group-id plus the MD5 sum of the connected
 * binary.
 */

class SocketSecurity {
public:
    SocketSecurity(int fd);

    /** Returns the peer's process-id. */
    int peerPid() { if (!ok) return -1; return cred.pid; }

    /** Returns the peer's user-id */
    int peerUid() { if (!ok) return -1; return cred.uid; }

    /** Returns the peer's group-id */
    int peerGid() { if (!ok) return -1; return cred.gid; }

private:
    bool ok;
    struct ucred cred;
};

#endif
