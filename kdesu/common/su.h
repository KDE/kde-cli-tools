/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __SU_h_Included__
#define __SU_h_Included__

#include <qcstring.h>
#include "process.h"

class QStringList;

/**
 * Execute under a different uid, using su to gain privileges.
 */

class SuProcess
    : public PtyProcess
{
public:
    SuProcess(QCString user=0, QCString command=0);
    ~SuProcess();

    /** Set the target user. */
    void setUser(QCString user) { m_User = user; }

    /** Execute the command. This will wait for the command to finish. */
    int exec(char *password, int check=0);

    /** 
     * Check if the stub is installed and the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

protected:
    QStringList dcopServer();

private:
    int ConverseSU(const char *password);

    QCString m_User;
};

#endif
