/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */
#ifndef __Pathsearch_h_Included__
#define __Pathsearch_h_Included__

#include <unistd.h>

#include <qvaluelist.h>
#include <qcstring.h>

/**
 * A class to search PATH for an executable.
 */
class PathSearch
{
public:
    /** Contruct a PathSearch object. */
    PathSearch();

    /**
     * Locate a program in PATH.
     * @param prog The desired program.
     * @param mode The desised mode (default: F_OK). The modes are the 
     * same as with access().
     */
    QCString locate(const char *prog, int mode=F_OK);

private:
    QValueList<QCString> m_Path;
};

#endif
