/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
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

    enum Results { Res_OK, Res_NO };

    bool mbPass;
    bool mbUser;
    int mFd;
    int mTimeout;
    QCString mBuf;
    QCString mPass;
    QCString mUser;
};

#endif
