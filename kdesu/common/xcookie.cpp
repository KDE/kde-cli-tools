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
#include <ctype.h>

#include "xcookie.h"
#include "debug.h"


Cookie::Cookie(): mOk(false)
{    
    const char *display = getenv("DISPLAY");
    if (display == 0L)
	return;

    // Get the cookie from the output of "xauth list"
    char *cmd = new char[strlen(display)+20];
    strcpy(cmd, "xauth list ");
    strcat(cmd, display);

    FILE *f;
    if (!(f = popen(cmd, "r"))) {
	xerror("popen(\"xauth\"): %s");
	return;
    }

    // Parse the first line of output.
    char buf[500];
    if (fgets(buf, 499, f)) {

	// only handle complete lines
	int len = strlen(buf);
	if (buf[len-1] != '\n') {
	    error("line too long");
	    goto out;
	}
	buf[--len] = '\000';

	// skip display
	int i=0;
	while ((i<len) && !isspace(buf[i]))
	    i++;
	while ((i<len) && isspace(buf[i]))
	    i++;
	if (i == len)
	    goto out;

	// protocol
	int j=i;
	while ((j<len) && !isspace(buf[j]))
	    j++;
	if (j == len)
	    goto out;
	mProto = new char[j-i+1];
	strncpy(mProto, buf+i, j-i);
	mProto[j-i] = '\000';
	
	// cookie
	i=j;
	while ((i<len) && isspace(buf[i]))
	    i++;
	if (i == len)
	    goto out;
	mCookie = new char[len-i+1];
	strncpy(mCookie, buf+i, len-i);
	mCookie[len-i] = '\000';

	mOk = true;
    } 
	    
out:
    delete[] cmd;
    pclose(f);
}

