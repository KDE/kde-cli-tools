/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */


/**
 * PTY compatibility routines. This class tries to emulate a UNIX98 PTY API
 * on various platforms.
 */
#ifndef KDESU_PTY_H
#define KDESU_PTY_H

class PTY {

public:
    /**
     * Construct a PTY object.
     */
    PTY();

    /**
     * Destructs the object. The PTY is closed if it is still open.
     */
    ~PTY();

    /**
     * Allocate a pty.
     * @return A filedescriptor to the master side.
     */
    int getpt();

    /**
     * Grant access to the slave side.
     * @return Zero if succesfull, < 0 otherwise.
     */
    int grantpt();

    /**
     * Unlock the slave side.
     * @return Zero if successful, < 0 otherwise.
     */
    int unlockpt();

    /**
     * Get the slave name.
     * @return A pointer to the slave's name in the filesystem.
     */
    const char *ptsname();
    
private:

    int ptyfd;

    char ptyname[50];
    char ttyname[50];
};
    
#endif    
