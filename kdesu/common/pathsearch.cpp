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

#include <qvaluelist.h>
#include <qcstring.h>

#include "pathsearch.h"


PathSearch::PathSearch()
{
    QCString path = getenv("PATH");
    if (path.isEmpty())
	return;

    int n=0, m=0;
    while ((m = path.find(':', n)) != -1) {
	m_Path.append(path.mid(n, m - n));
	n = m+1;
    }

    if (n < (int) path.length())
	m_Path.append(path.mid(n));
}

QCString PathSearch::locate(const char *prog, int mode)
{
    QCString file;
    QValueList<QCString>::ConstIterator it;
    for (it = m_Path.begin(); it != m_Path.end(); it++) {
	file = *it + "/" + prog;
	if (access(file, mode) == 0)
	    return file;
    }

    return QCString();
}

