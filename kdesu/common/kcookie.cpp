/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
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

#include "kcookie.h"


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
	qWarning("KCookie::getXCookie(): DISPLAY is not set");
	return;
    }
    QString cmd = QString("xauth list %1").arg(m_Display);
    if (!(f = popen(cmd.latin1(), "r"))) {
	qWarning("KCookie::getXCookie(): popen() failed");
	return;
    }
    QString output = QString(fgets(buf, 1024, f)).simplifyWhiteSpace();
    if (pclose(f) < 0) {
	qWarning("KCookie::getXCookie(): could not run xauth");
	return;
    }
    QStringList lst = QStringList::split(' ', output);
    if (lst.count() != 3) {
	qWarning("KCookie::getXCookie(): parse error");
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
	    qWarning("KCookie::getICECookie(): cannot find DCOP server");
	    return;
	}
	if (!(f = fopen(home + "/.DCOPserver", "r"))) {
	    qWarning("KCookie::getICECookie(): cannot open ~/.DCOPserver");
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
	    qWarning("KCookie::getICECookie(): popen() failed");
	    break;
	}
	QStringList output;
	while (fgets(buf, 1024, f) > 0)
	    output += QString(buf);
	if (pclose(f) < 0) {
	    qWarning("KCookie::getICECookie(): could not run iceauth");
	    break;
	}
	QStringList::Iterator it;
	for (it=output.begin(); it!=output.end(); it++) {
	    QStringList lst = QStringList::split(' ',
		    (*it).simplifyWhiteSpace());
	    if (lst.count() != 5) {
		qWarning("KCookie::getICECookie(): parse error");
		break;
	    }
	    if (lst[0] == "DCOP")
		m_DCOPAuth += (lst[3] + ' ' + lst[4]);
	    else if (lst[0] == "ICE")
		m_ICEAuth += (lst[3] + ' ' + lst[4]);
	    else 
		qWarning("KCookie::getICECookie(): unknown protocol: %s",
			lst[0].latin1());
	}
    }
}

