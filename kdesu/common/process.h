/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 */

#ifndef __Process_h_Included__
#define __Process_h_Included__


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
     * Enable/disable build of syscoca for the target.
     */
    void setBuildSycoca(bool build) { m_bSycoca = build; }

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
    int checkPass(const char *pass) { return exec((char *) pass, true); }

private:
    int SetupTTY();
    int disableLocalEcho();
    int WaitSlave();
    int ConverseSU(const char *password);
    int ConverseStub();
    int waitForChild(bool echo);
    QCString commaSeparatedList(QStringList);

    int m_Pid, m_Fd;
    QCString m_TTY, m_Command, m_User;
    bool m_bOk, m_bTerminal, m_bSycoca;
    bool m_bWait, m_bErase;
};

#endif
