/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __Handler_h_included__
#define __Handler_h_included__

#include "secure.h"

class QCString;

/**
 * A ConnectionHandler handles a client. It is called from the main program
 * loop whenever there is data to read from a corresponding socket.
 * It keeps reading data until a newline is read. Then, a command is parsed
 * and executed.
 */

class ConnectionHandler: public SocketSecurity 
{

public:
    ConnectionHandler(int fd);
    ~ConnectionHandler();

    /**
     * Call this when there is data to read from the socket.
     */
    int handle();

private:
    int doCommand();
    void respond(int ok, QCString s=0);
    QCString makeKey(int namspace, QCString host, QCString user, 
	    QCString command);

    enum Results { Res_OK, Res_NO };

    int m_Fd, m_Timeout;
    QCString m_Buf, m_Pass;
    QCString m_User, m_Host;
};

#endif
