/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 */

#ifndef __Process_h_Included__
#define __Process_h_Included__

#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "pty.h"
#include "kcookie.h"

typedef QValueList<QCString> QCStringList;

/**
 * PtyProcess: Provides functionality for creating a GUI front end to
 * password requiring terminal programs (i.e. su, passwd, ...).
 *
 * It can be used "an sich", but you probably want to derive from it.
 */

class PtyProcess
{
public:
    PtyProcess();
    virtual ~PtyProcess();

    /**
     * Fork off and execute a command. The command's standard in/output is
     * connected to a pty. This pty can be accessed by #ref readLine and
     * #ref writeLine.
     * @param command The command to execute.
     * @param args The arguments to the command.
     */
    int exec(QCString command, QCStringList args);

    /**
     * Read a line from the pty. This call blocks until a single, full line
     * is read. This does not return with EINTR when the read() system call
     * is interrupted by a signal.
     */
    QCString readLine(bool block=true);

    /**
     * Write a line of text to the pty.
     * @param line The text to write.
     * @param addNewline Adds a '\n' to the line.
     */
    void writeLine(QCString line, bool addNewline=true);

    /** Enable/disable terminal output.  */
    void setTerminal(bool terminal) { m_bTerminal = terminal; }

    /** Overwritte the password as soon as it is used. */
    void setErase(bool erase) { m_bErase = erase; }

    /**
     * Set exit string. If a line of program output matches this,
     * @ref #waitForChild() will kill it.
     */
    void setExitString(QCString exit) { m_Exit = exit; }

    /**
     * Wait for the child to exit. If a line of output matches the exit
     * string set by @ref #setExitString, the child is terminated.
     */
    int waitForChild();

    /**
     * Wait until the terminal has cleared the ECHO flag. This is usefull 
     * when programs write a password prompt before they disable ECHO,
     * because disabling it might flush the terminal I/O queues.
     */
    int WaitSlave();

    /** Enables/disables the ECHO flag.  */
    int enableLocalEcho(bool enable=true);

    /**
     * Have a conversation with kdesu_stub. If you execute a different 
     * program, this method is not applicable.
     */
    int ConverseStub(bool check_only);

    /**
     * Set the command. Relevant only when executing kdesu_stub.
     */
    void setCommand(QCString command) { m_Command = command; }

    /**
     * Set to "X only mode": DCOP is not forwarded and the sycoca is not
     * built. Relevant only when executing kdesu_stub.
     */
    void setXOnly(bool xonly) { m_bXOnly = xonly; }


protected:
    /** 
     * These virtual functions can be overloaded when special behaviour is
     * desired.
     */
    virtual QCString display() { return m_pCookie->display(); }
    virtual QCString displayAuth() { return m_pCookie->displayAuth(); }
    virtual QCStringList dcopServer() { return m_pCookie->dcopServer(); }
    virtual QCStringList dcopAuth() { return m_pCookie->dcopAuth(); }
    virtual QCStringList iceAuth() { return m_pCookie->iceAuth(); }

    bool m_bTerminal;
    bool m_bErase, m_bXOnly;
    int m_Pid, m_Fd;
    QCString m_Command, m_Exit;

private:
    int init();
    int SetupTTY(int fd);
    QCString commaSeparatedList(QCStringList);

    PTY *m_pPTY;
    KCookie *m_pCookie;
    QCString m_Inbuf, m_TTY;
};


#endif
