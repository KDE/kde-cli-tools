/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __XCookie_h_included__
#define __XCookie_h_included__

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
    const char *cookie() { return mOk ? mCookie : 0L; }

    /**
     * Returns the cookie protocol.
     */
    const char *proto() { return mOk ? mProto : 0L; }

private:
    char *mProto;
    char *mCookie;
    bool mOk;
};

#endif
