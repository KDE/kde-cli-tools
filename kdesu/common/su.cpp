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
#define ID "SuProcess"
#endif


#ifndef __PATH_SU
#define __PATH_SU "false"
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
    return exec(password, true);
}

/*
 * Execute a command with su(1).
 */

int SuProcess::exec(const char *password, int check)
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
	    char *ptr = const_cast<char *>(password);
	    for (unsigned i=0; i<strlen(password); i++)
		ptr[i] = '\000';
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
    int state = 0;

    QCString line;
    while (state < 2) {
	line = readLine(); 
	if (line.isNull())
	    return -1;
	switch (state) {
	case 0:
	    // Write password
	    if (line.contains(':')) {
		WaitSlave();
		write(m_Fd, password, strlen(password));
		write(m_Fd, "\n", 1);
		state++;
	    } 
	    break;
	case 1:
	    if (line.isEmpty()) {
		state++;
		break;
	    }
	    return -1;
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

