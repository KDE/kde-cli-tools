/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * kcookie.cpp: KDE authentication cookies.
 */

#include <stdio.h>
#include <stdlib.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qglobal.h>

#include <kdebug.h>
#include "kcookie.h"

#ifdef __GNUC__
#define ID __PRETTY_FUNCTION__
#else
#define ID __FILE__
#endif

KCookie::KCookie()
{
    getXCookie();
    getICECookie();
}


void KCookie::getXCookie()
{    
    char buf[1024];
    FILE *f;

    m_Display = getenv("DISPLAY");
    if (m_Display.isEmpty()) {
	kDebugError("%s: $DISPLAY is not set", ID);
	return;
    }
    QString cmd = QString("xauth list %1").arg(m_Display);
    if (!(f = popen(cmd.latin1(), "r"))) {
	kDebugError("%s: popen(): %m", ID);
	return;
    }
    QString output = QString(fgets(buf, 1024, f)).simplifyWhiteSpace();
    if (pclose(f) < 0) {
	kDebugError("%s: Could not run xauth", ID);
	return;
    }
    QStringList lst = QStringList::split(' ', output);
    if (lst.count() != 3) {
	kDebugError("%s: parse error", ID);
	return;
    }
    m_DisplayAuth = (lst[1] + ' ' + lst[2]);
}


void KCookie::getICECookie()
{
    FILE *f;
    char buf[1024];

    QString dcopsrv(getenv("DCOPSERVER"));
    if (dcopsrv.isEmpty()) {
	QString home(getenv("HOME"));
	if (home.isEmpty()) {
	    kDebugError("%s: Cannot find DCOP server.", ID);
	    return;
	}
	if (!(f = fopen(home + "/.DCOPserver", "r"))) {
	    kDebugError("%s: Cannot open ~/.DCOPserver.", ID);
	    return;
	}
	dcopsrv = QString(fgets(buf, 1024, f)).stripWhiteSpace();
	fclose(f);
    }
    m_DCOPSrv = QStringList::split(',', dcopsrv);
    if (m_DCOPSrv.count() == 0)
	return;

    QStringList::Iterator it;
    for (it=m_DCOPSrv.begin(); it != m_DCOPSrv.end(); it++) {
	QString cmd = QString("iceauth list netid=%1").arg(*it);
	if (!(f = popen(cmd.latin1(), "r"))) {
	    kDebugError("%s: popen(): %m", ID);
	    break;
	}
	QStringList output;
	while (fgets(buf, 1024, f) > 0)
	    output += QString(buf);
	if (pclose(f) < 0) {
	    kDebugError("%s: Could not run iceauth.", ID);
	    break;
	}
	QStringList::Iterator it;
	for (it=output.begin(); it!=output.end(); it++) {
	    QStringList lst = QStringList::split(' ',
		    (*it).simplifyWhiteSpace());
	    if (lst.count() != 5) {
		kDebugError("%s: parse error", ID);
		break;
	    }
	    if (lst[0] == "DCOP")
		m_DCOPAuth += (lst[3] + ' ' + lst[4]);
	    else if (lst[0] == "ICE")
		m_ICEAuth += (lst[3] + ' ' + lst[4]);
	    else 
		kDebugError("%s: unknown protocol: %s", ID, lst[0].latin1());
	}
    }
}

