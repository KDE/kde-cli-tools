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
 * PtyProcess: a base class for SuProcess and RshProcess, providing common
 * functionality related to PTY's.
 */

class PtyProcess
{
public:
    PtyProcess();
    virtual ~PtyProcess();

    /** (re)initialise  the pty. */
    int init();

    /**
     * Set the command
     */
    void setCommand(QCString command) { m_Command = command; }

    /**
     * Enable terminal output.
     * @param bool True if terminal output is wanted, false otherwise.
     */
    void setTerminal(bool terminal) { m_bTerminal = terminal; }

    /**
     * Set to "X only mode": DCOP is not forwarded and the sycoca is not
     * built.
     */
    void setXOnly(bool xonly) { m_bXOnly = xonly; }

    /**
     * If set to true, the password is overwritten as soon as it is used.
     */
    void setErase(bool erase) { m_bErase = erase; }

protected:
    /**
     * Set exit string. If a line of program output matches this,
     * waitForChild() will kill it.
     */
    void setExitString(QCString exit) { m_Exit = exit; }

    int exec(QCString command, QCStringList args);
    QCString readLine();

    int WaitSlave();
    int ConverseStub(bool check_only);
    int waitForChild(bool echo);
    int enableLocalEcho(bool enable=true);

    // These virtual functions can be overloaded when special behaviour is
    // needed (i.e. SshProcess).
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
    QCString commaSeparatedList(QCStringList);
    int SetupTTY(int fd);

    PTY *m_pPTY;
    KCookie *m_pCookie;
    QCString m_Inbuf, m_TTY;
};


#endif
