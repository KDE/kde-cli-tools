/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * 
 * ssh.cpp: Execute a program on a remote machine using ssh.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <qglobal.h>
#include <qcstring.h>

#include <kdebug.h>
#include <kstddirs.h>

#include "ssh.h"
#include "kcookie.h"


#ifdef __GNUC__
#define ID __PRETTY_FUNCTION__
#else
#define ID __FILE__
#endif


SshProcess::SshProcess(QCString host, QCString user, QCString command)
{
    m_Host = host;
    m_User = user;
    m_Command = command;
    srand(time(0L));
}


SshProcess::~SshProcess()
{
}


QString SshProcess::checkNeedPassword()
{
    if (exec(0L, 2) == 1)
	return m_Prompt;
    return QString::null;
}


int SshProcess::checkInstall(const char *password)
{
    return exec((char *)password, 1);
}


int SshProcess::exec(char *password, int check)
{    
    if (PtyProcess::init() < 0)
	return -1;

    // Open the pty slave before forking. See PtyProcess::SetupTTY
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0) {
	kDebugError("%s: Could not open slave pty", ID);
	return -1;
    }

    // Get DCOP port forward.
    QString fwd = dcopForward();
    if (fwd.isEmpty()) {
	kDebugError("%s: Could not get DCOP forward", ID);
	return -1;
    }

    if ((m_Pid = fork()) == -1) {
	kDebugPError("%s: fork()", ID);
	return -1;
    }
    if (m_Pid) {
	close(slave); // See PtyProcess::SetupTTY
	int ret = ConverseSsh(password, check);
	if (ret < 0) {
	    kDebugError("%s: Conversation with ssh failed", ID);
	    return -1;
	} 
	if (m_bErase) {
	    int len = strlen(password);
	    for (int i=0; i<len; i++)
		password[i] = '\000';
	}
	setExitString("Waiting for forwarded connections to terminate");
	if (ret == 0) {
	    if (ConverseStub(check) < 0) {
		kDebugError("%s: Converstation with kdesu_stub failed", ID);
		kill(m_Pid, SIGTERM);
	    }
	    ret = waitForChild(check);
	} else {
	    kill(m_Pid, SIGTERM);
	    waitForChild(check);
	}
	return ret;
    } else {
	if (SetupTTY(slave) < 0)
	    _exit(1);
	if (m_bXOnly)
	    execl(__PATH_SSH, "ssh", "-l", m_User.data(), 
		"-o", "StrictHostKeyChecking no", m_Host.data(), 
		"kdesu_stub", 0L);
	else
	    execl(__PATH_SSH, "ssh", "-R", fwd.latin1(), "-l", m_User.data(), 
		"-o", "StrictHostKeyChecking no", m_Host.data(), 
		"kdesu_stub", 0L);
	kDebugPError("%s: execl(\"%s\")", ID, __PATH_SSH);
	_exit(1);
    }

    return 0;
}

/*
 * Create a port forwarding for DCOP. For the remote port, we take a pseudo
 * random number between 10k and 50k. This is not ok, of course, but I see
 * no other way. If the port happens to be occupied, we'll have no security 
 * concern because ssh will not start.
 */

QString SshProcess::dcopForward()
{
    bool ok;
    int i, j, port=0;
    QString host;
    QStringList srv = PtyProcess::dcopServer();
    QStringList::Iterator it;
    for (m_dcopSrv=0,it=srv.begin(); it!=srv.end(); m_dcopSrv++, it++) {
	i = (*it).find('/');
	if (i == -1)
	    continue;
	if ((*it).left(i) != "tcp")
	    continue;
	j = (*it).find(':', ++i);
	if (j == -1)
	    continue;
	host = (*it).mid(i, j-i);
	port = (*it).mid(++j).toInt(&ok);
	if (ok)
	    break;
    }
    if (it == srv.end())
	return QString::null;

    m_dcopPort = 10000 + (int) ((40000.0 * rand()) / (1.0 + RAND_MAX));
    return QString("%1:%2:%3").arg(m_dcopPort).arg(host).arg(port);
}

	
/*
 * Conversation with ssh. 
 * If check is 0, this waits for either a "Password: " prompt, 
 * or the header of the stub. If a prompt is received, the password is
 * written back. Used for running a command.
 * If check is 1, operation is the same as 0 except that if a stub header is
 * received, the stub is stopped with the "stop" command. This is used for
 * checking a password.
 * If check is 2, operation is the same as 1, except that no password is
 * written. The prompt is saved to m_Prompt. Used for checking the need for 
 * a password.
 */

int SshProcess::ConverseSsh(const char *password, int check)
{
    char buf[256]; 
    int nbytes;
    int i, j, colon, state;

    state = 0;
    while (state < 2) {
	nbytes = read(m_Fd, buf, 255);
	if (nbytes == -1) {
	    if (errno == EINTR) continue;
	    else {
		kDebugPError("%s: read()", ID);
		return -1;
	    }
	}
	if (nbytes == 0)
	    return -1;
	buf[nbytes] = '\000';

	switch (state) {
	case 0:
	    // Check for "kdesu_stub" header.
	    if (!strcmp(buf, "kdesu_stub\n")) {
		// We don't want this echoed back.
		disableLocalEcho();
		if (check > 0)
		    write(m_Fd, "stop\n", 5);
		else
		    write(m_Fd, "ok\n", 3);
		state += 2;
		break;
	    }

	    // Match "Password: " with the regex ^[^:]+:[\w]*$.
	    for (i=0,j=0,colon=0; i<nbytes; i++) {
		if (buf[i] == ':') {
		    j = i; colon++;
		    continue;
		}
		if (!isspace(buf[i]))
		    j++;
	    }
	    if ((colon == 1) && (buf[j] == ':')) {
		if ((check > 1) || (password == 0L)) {
		    m_Prompt = buf;
		    return 1;
		}
		WaitSlave();
		write(m_Fd, password, strlen(password));
		write(m_Fd, "\n", 1);
		state++; 
		break;
	    }
	    if (m_bTerminal)
		fwrite(buf, sizeof(char), nbytes, stdout);
	    break;

	case 1:
	    if (strchr(buf, '\n'))
		state++;
	    break;
	}
    }

    return 0;
}


// Display redirection is handled by ssh natively.
QString SshProcess::display()
{
    return QString("no");
}


QString SshProcess::displayAuth()
{
    return QString("no");
}


// Return the remote end of the forwarded connection.
QStringList SshProcess::dcopServer()
{
    QStringList lst;
    if (m_bXOnly)
	lst += QString("no");
    else
	lst += QString("tcp/localhost:%1").arg(m_dcopPort);
    return lst;
}


// Return the right cookie.
QStringList SshProcess::dcopAuth()
{
    QStringList lst;
    lst += PtyProcess::dcopAuth()[m_dcopSrv];
    return lst;
}


QStringList SshProcess::iceAuth()
{
    QStringList lst;
    lst += PtyProcess::iceAuth()[m_dcopSrv];
    return lst;
}

