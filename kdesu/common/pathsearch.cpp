/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * pathsearch.cpp: Find programs in PATH.
 */

#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "pathsearch.h"


PathSearch::PathSearch()
{
    char *p = getenv("PATH");
    if ((p == 0L) || (*p == 0))
	return;

    string pathstr = p;
    string::size_type n=0, m=0;
    while ((m = pathstr.find(':', n)) != string::npos) {
	path.push_back(pathstr.substr(n, (m-n)));
	n = m+1;
    }
    if (n < pathstr.size())
	path.push_back(pathstr.substr(n, m));
}

const char *PathSearch::locate(const char *prog, int mode)
{
    string file;
    for (it = path.begin(); it != path.end(); it++) {
	file = *it + "/" + prog;
	if (access(file.c_str(), mode) == 0)
	    return file.c_str();
    }

    return 0L;
}

