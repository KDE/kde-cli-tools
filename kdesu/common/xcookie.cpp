/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 */

/**
 * X cookie extraction.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <qcstring.h>
#include <qglobal.h>

#include "xcookie.h"


Cookie::Cookie(): m_bOk(false)
{    
    char *display = getenv("DISPLAY");
    if (display == 0L)
	return;

    // Get the cookie from the output of "xauth list"
    QCString cmd;
    cmd.sprintf("xauth list %s", display);

    FILE *f;
    if (!(f = popen(cmd, "r"))) {
	qWarning("popen(\"xauth\"): %s", strerror(errno));
	return;
    }

    // Parse the first line of output.
    char buf[500];
    if (fgets(buf, 499, f) == 0L) {
	qWarning("Cookie::Cookie(): %s", strerror(errno));
	pclose(f); return;
    }

    QCString output = buf;

    // only handle complete lines
    if (output.find('\n') == -1) {
	qWarning("Cookie::Cookie(): line too long");
	pclose(f); return;
    }

    output = output.simplifyWhiteSpace();
    int n1 = output.find(' ');
    int n2 = output.find(' ', n1+1);
    if ((n1 == -1) || (n2 == -1)) {
	qWarning("Cookie::Cookie(): parse error 1");
	pclose(f); return;
    }

    m_Proto = output.mid(n1+1, n2-n1-1);
    m_Cookie = output.mid(n2+1);
    if (m_Proto.isEmpty() || m_Cookie.isEmpty()) {
	qWarning("Cookie::Cookie(): parse error 2");
	pclose(f); return;
    }

    m_bOk = true;
    pclose(f);
}

