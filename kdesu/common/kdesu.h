/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __KDEsu_h_included__
#define __KDEsu_h_included__

const int defTimeout = 120*60;
const int defEchoMode = 0;
const int defKeep = false;

/**
 * I got a bug report from a user that SUN_LEN does not exist in his Linux
 * 2.0 system. This definition is taken from glibc 2.0.6
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef SUN_LEN

#include <config.h>
#include <string.h>

#define SUN_LEN(ptr) ((ksize_t) (((struct sockaddr_un *) 0)->sun_path)         \
	                      + strlen ((ptr)->sun_path))   
#endif // SUN_LEN

#endif
