/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * client.h: defines for KDEsuClient, a client to access kdesud.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <string>

/**
 * A client class to access kdesud, the KDE su daemon.
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
    int exec(const char *command);

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
    int setUser(const char *user=0L);

    /**
     * Remove a command and it's password from kdesud.
     * @param command The command to remove.
     * @return zero on success, -1 on an error
     */
    int delCommand(const char *command);

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

    enum Commands {
	Cmd_pass, Cmd_exec,
	Cmd_user, Cmd_del, 
	Cmd_stop, Cmd_ping
    };

    int sockfd;
    string sock;

    int command(int code, const char *arg1=0, int arg2=0);
    string escape(const char *str);
};

