/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * client.h: client to access kdesud.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <qcstring.h>

/**
 * A client class to access kdesud, the KDE su daemon.
 * It can execute commands for you, and store variables.
 */

class KDEsuClient {
public:
    KDEsuClient();
    ~KDEsuClient();

    /**
     * Execute a command through kdesud. If a password is already set, the 
     * command and password are stored and the command can be executed later
     * on without having to supply a password. 
     *
     * @param command The command to execute.
     * @return Zero on success, -1 on failure.
     */
    int exec(QCString command);

    /**
     * Set root's password, lasts one session.
     *
     * @param pass Root's password.
     * @param timeout The time that a password will live.
     * @return Zero on success, -1 on failure.
     */
    int setPass(const char *pass, int timeout);

    /**
     * Set the target user.
     */
    int setUser(QCString user);

    /**
     * Set the target host (optional).
     */
    int setHost(QCString host);

    /**
     * Remove a command and it's password from kdesud.
     * @param command The command to remove.
     * @return zero on success, -1 on an error
     */
    int delCommand(QCString command);

    /**
     * Set a persistent variable.
     * @param key The name of the variable.
     * @param value Its value.
     */
    int setVar(QCString key, QCString value);

    /**
     * Get a persistent variable.
     * @param key The name of the variable.
     * @return Its value.
     */
    QCString getVar(QCString key);

    /**
     * Ping kdesud. This can be used for diagnostics.
     * @return Zero on success, -1 on failure
     */
    int ping();

    /**
     * Stop the daemon.
     */
    int stopServer();

    /**
     * Try to start up kdesud
     */
    int startServer();

    /**
     * re(connect) to kdesud
     */
    int connect();

private:

    int sockfd;
    QCString sock;

    int command(QCString cmd, QCString *result=0L);
    QCString escape(QCString str);
};

