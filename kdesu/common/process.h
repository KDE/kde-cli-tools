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


#include <string>


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
    SuProcess(const char *command="");

    /**
     * Set the command
     */
    void setCommand(const char *command) { mCommand = command; }

    /**
     * Enable terminal output.
     * @param bool True if terminal output is wanted, false otherwise.
     */
    void setTerminal(bool terminal) { mTerminal = terminal; }

    /**
     * If set to true, the password is overwritten as soon as it is used.
     */
    void setErase(bool erase) { mErase = erase; }

    /**
     * Set the user to su to.
     */
    void setUser(const char *user) { mUser = user; }

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

    string mCommand;
    string mUser;
    bool mOk;
    bool mTerminal;
    bool mWait;
    bool mErase;
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
    UserProcess(const char *command="");

    /**
     * Set the command to execute.
     */
    void setCommand(const char *command) { mCommand = command; }

    /**
     * Execute and wait for the command.
     */
    int exec();

private:
    string mCommand;
};

#endif
