/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __XCookie_h_included__
#define __XCookie_h_included__

#include <qcstring.h>

/**
 * Extract cookies from the user's .Xauthority file.
 */
class Cookie
{
public:
    /**
     * The constructor. This reads the cookie from the .Xauthority file. Use
     * cookie() to access it.
     */
    Cookie();

    /**
     * Returns the cookie.
     */
    QCString cookie() { return m_bOk ? m_Cookie : QCString(); }

    /**
     * Returns the cookie protocol.
     */
    QCString proto() { return m_bOk ? m_Proto : QCString(); }

private:
    bool m_bOk;
    QCString m_Proto, m_Cookie;
};

#endif
