/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 */

#ifndef __Process_h_included__
#define __Process_h_included__


#include <qcstring.h>
#include <qstringlist.h>


/**
 * Execute a program as root, using su to gain privileges.
 */

class SuProcess
{
public:
    /**
     * Construct a RootProcess object.
     * @param command The command to execute, including arguments, shell
     * redirection, etc.
     */
    SuProcess(const char *command=0L);

    /**
     * Set the command
     */
    void setCommand(const char *command) { m_Command = command; }

    /**
     * Enable terminal output.
     * @param bool True if terminal output is wanted, false otherwise.
     */
    void setTerminal(bool terminal) { m_bTerminal = terminal; }

    /**
     * If set to true, the password is overwritten as soon as it is used.
     */
    void setErase(bool erase) { m_bErase = erase; }

    /**
     * Set the user to su to.
     */
    void setUser(const char *user) { m_User = user; }

    /**
     * Execute the command. This will wait for the command to finish. If
     * this is not desired, put the '&' character after the command.
     * @param password Root's password.
     */
    int exec(char *password, bool check_only=false);

    /**
     * Check the password. This is dony by trying to executing "true".
     */
    int checkPass(char *pass) { return exec(pass, true); }

private:
    int SetupTTY(const char *ttyname);
    int WaitSlave(const char *ttyname);
    int ConverseSU(int fd, const char *password, const char *tty);
    int ConverseStub(int fd, const char *command);
    int CopyOutput(int fd, bool echo);
    QCString commaSeparatedList(QStringList);

    QCString m_Command, m_User;
    bool m_bOk, m_bTerminal;
    bool m_bWait, m_bErase;
};


/**
 * Execute a command as a normal user.
 */
class UserProcess
{
public:
    /**
     * Constructs a User_process object.
     * @param command The command to execute.
     */
    UserProcess(const char *command=0L);

    /**
     * Set the command to execute.
     */
    void setCommand(const char *command) { m_Command = command; }

    /**
     * Execute and wait for the command.
     */
    int exec();

private:
    QCString m_Command;
};

#endif
