/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __KDESu_h_included__
#define __KDESu_h_included__

enum echoModes { OneStar, ThreeStars, NoStars };

const int defEchomode= OneStar;
const bool defKeep = false;
const int defTimeout = 120*60;

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
