/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * 
 */

#ifndef __SSH_h_Included__
#define __SSH_h_Included__

#include <qcstring.h>
#include "process.h"

/**
 * Execute a remote command, using ssh.
 */

class SshProcess
    : public PtyProcess
{
public:
    SshProcess(QCString host=0, QCString user=0, QCString command=0);
    ~SshProcess();

    /** Set the target host. */
    void setHost(QCString host) { m_Host = host; }

    /** Set the target user. */
    void setUser(QCString user) { m_User = user; }

    /** 
     * Check if the current user@host needs a password. 
     * @return The prompt for the password if a password is required. A null
     * string otherwise.
     */
    QString checkNeedPassword();

    /**
     * Check if the stub is installed and if the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /** Execute the command. */
    int exec(const char *password, int check=0);

protected:
    QCString display();
    QCString displayAuth();
    QCStringList dcopServer();
    QCStringList dcopAuth();
    QCStringList iceAuth();

private:
    QCString dcopForward();
    int ConverseSsh(const char *password, int check);

    int m_dcopPort, m_dcopSrv;
    QString m_Prompt;
    QCString m_Host, m_User;
};

#endif
