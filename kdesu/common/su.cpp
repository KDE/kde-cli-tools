/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * su.cpp: Execute a program as another user with "class SuProcess".
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <qglobal.h>
#include <qcstring.h>

#include <kdebug.h>
#include <kstddirs.h>

#include "su.h"
#include "kcookie.h"

#ifdef __GNUC__
#define ID __PRETTY_FUNCTION__
#else
#define ID __FILE__
#endif


SuProcess::SuProcess(QCString user, QCString command)
{
    m_User = user;
    m_Command = command;
}


SuProcess::~SuProcess()
{
}


int SuProcess::checkInstall(const char *password)
{
    return exec((char *)password, true);
}

/*
 * Execute a command with su(1).
 */

int SuProcess::exec(char *password, int check)
{
    if (PtyProcess::init() < 0)
	return -1;

    // Open slave before forking: see PtyProcess::SetupTTY.
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0) {
	kDebugError("%s: Could not open slave pty", ID);
	return -1;
    }
    if ((m_Pid = fork()) == -1) {
	kDebugWarning("%s: fork(): %m", ID);
	return -1;
    }
    if (m_Pid) {
	close(slave);
	if (ConverseSU(password) < 0) {
	    kDebugError("%s: Conversation with su failed", ID);
	    return -1;
	} 
	if (m_bErase) {
	    int len = strlen(password);
	    for (int i=0; i<len; i++)
		password[i] = '\000';
	}
	if (ConverseStub(check) < 0) {
	    kDebugError("%s: Converstation with kdesu_stub failed", ID);
	    return -1;
	}
	int ret = waitForChild(check);
	return ret;

    } else {

	QString stub = KStandardDirs::findExe("kdesu_stub");
	if (stub.isEmpty()) {
	    kDebugError("%s: kdesu_stub not found", ID);
	    _exit(1);
	}

	if (SetupTTY(slave) < 0)
	    _exit(1);

	// Terminal output will go through the tty from now on. It is visible
	// only if the user uses the '-t' switch.

	execl(__PATH_SU, "su", m_User.data(), "-c", stub.latin1(), 0L);
	kDebugWarning("%s: execl(\"%s\"): %m", ID, __PATH_SU);
	_exit(1);
    }

    return 0;
}

/*
 * Conversation with su: feed the password.
 */

int SuProcess::ConverseSU(const char *password)
{	
    char buf[256]; 
    int nbytes, i;
    int state = 0;

    while (state < 2) {
	nbytes = read(m_Fd, buf, 255);
	if (nbytes == -1) {
	    if (errno == EINTR) continue;
	    else {
		kDebugError("%s: read(): %m", ID);
		return -1;
	    }
	}
	if (nbytes == 0)
	    return -1;
	buf[nbytes] = '\000';
	switch (state) {
	case 0:
	    // Write password
	    if (strchr(buf, ':')) {
		WaitSlave();
		write(m_Fd, password, strlen(password));
		write(m_Fd, "\n", 1);
		state++;
	    } 
	    break;
	case 1:
	    // Wait for newline
	    for (i=0; (i<nbytes) && (buf[i] != '\n'); i++);
	    if (i < nbytes)
		state++;
	    break;
	}
    }
    return 0;
}


QStringList SuProcess::dcopServer()
{
    if (!m_bXOnly) 
	return PtyProcess::dcopServer();

    QStringList lst;
    lst += QString("no");
    return lst;
}

