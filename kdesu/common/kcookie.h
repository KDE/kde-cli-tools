/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __KCookie_h_Included__
#define __KCookie_h_Included__

#include <qstring.h>
#include <qstringlist.h>

/**
 * Utility class to access the authentication tokens needed to run a KDE
 * program (X11 and DCOP cookies).
 */

class KCookie
{
public:
    KCookie();

    /** Returns the X11 display. */
    QString display() { return m_Display; }

    /** Returns the X11 magic cookie, if available. */
    QString displayAuth() { return m_DisplayAuth; }

    /** Returns a list of netid's where a dcopserver is running */
    QStringList dcopServer() { return m_DCOPSrv; }

    /** 
     * Returns a list of magic cookies for DCOP protocol authentication. 
     * The order is the same as in dcopServer().
     */
    QStringList dcopAuth() { return m_DCOPAuth; }

    /** Returns a list of magic cookies for the ICE protocol. */
    QStringList iceAuth() { return m_ICEAuth; }

private:
    void getXCookie();
    void getICECookie();

    QString m_Display, m_DisplayAuth;
    QStringList m_DCOPSrv, m_DCOPAuth, m_ICEAuth;
};


#endif // __KCookie_h_Included__
