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

#include "pty.h"
#include "kcookie.h"

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
     * Enable/disable build of syscoca for the target.
     */
    void setBuildSycoca(bool build) { m_bSycoca = build; }

    /**
     * Enable terminal output.
     * @param bool True if terminal output is wanted, false otherwise.
     */
    void setTerminal(bool terminal) { m_bTerminal = terminal; }

    /**
     * Set to "x only" mode: DCOP is not forwarded and the sycoca is not
     * built.
     */
    void setXOnly(bool xonly) { m_bXOnly = xonly; }

    /**
     * If set to true, the password is overwritten as soon as it is used.
     */
    void setErase(bool erase) { m_bErase = erase; }

    /**
     * Set exit string. If a line of program output matches this,
     * waitForChild() will kill it.
     */
    void setExitString(QCString exit) { m_Exit = exit; }

protected:
    // These virtual functions can be overloaded when special behaviour is
    // needed (i.e. SshProcess).
    virtual QString display() { return m_pCookie->display(); }
    virtual QString displayAuth() { return m_pCookie->displayAuth(); }
    virtual QStringList dcopServer() { return m_pCookie->dcopServer(); }
    virtual QStringList dcopAuth() { return m_pCookie->dcopAuth(); }
    virtual QStringList iceAuth() { return m_pCookie->iceAuth(); }

    bool m_bTerminal, m_bSycoca;
    bool m_bErase, m_bXOnly;
    int m_Pid, m_Fd;
    QCString m_TTY, m_Command, m_Exit;

    int SetupTTY(int fd);
    int WaitSlave();
    int ConverseStub(bool check_only);
    int waitForChild(bool echo);
    int disableLocalEcho();

private:
    PTY *m_pPTY;
    KCookie *m_pCookie;
    QCString commaSeparatedList(QStringList);
};


#endif
