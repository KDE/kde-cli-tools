/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * handler.cpp: A connection handler for kdesud.
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <qcstring.h>

#include <kdebug.h>
#include "handler.h"
#include "repo.h"
#include "lexer.h"
#include "secure.h"
#include "su.h"
#include "ssh.cpp"


// Global repository
extern Repository *repo;


ConnectionHandler::ConnectionHandler(int fd)
	: SocketSecurity(fd)
{
    m_Fd = fd;
    m_Priority = 50;
    m_Scheduler = SuProcess::SchedNormal;
}

ConnectionHandler::~ConnectionHandler()
{
    m_Buf.fill('x');
    m_Pass.fill('x');
}

void ConnectionHandler::respond(int ok, QCString s)
{
    QCString buf;

    switch (ok) {
    case Res_OK:
	buf = "OK";
	break;
    case Res_NO:
	buf = "NO";
	break;
    default:
	assert(1);
    }

    if (!s.isEmpty()) {
	buf += ' ';
	buf += s;
    }
    buf += '\n';

    send(m_Fd, buf.data(), buf.length(), 0);
}

QCString ConnectionHandler::makeKey(int namspace, QCString host, 
	QCString user, QCString command)
{
    QCString res;
    res.setNum(namspace);
    res += "*";
    res += host + "*" + user + "*" + command;
    return res;
}

/**
 * Parse and do one command. On a parse error, return -1. This will 
 * close the socket in the main accept loop.
 */

int ConnectionHandler::doCommand()
{
    if ((uid_t) peerUid() != getuid()) {
	kDebugError("Peer uid not equal to me");
	kDebugError("Peer: %d, Me: %d", peerUid(), (int) getuid());
	return -1;
    }

    int n = m_Buf.find('\n');
    assert(n != -1);

    QCString newbuf = m_Buf.mid(0, n+1);
    for (int i=0; i<=n; i++)
	m_Buf[n] = 'x';
    m_Buf.remove(0, n+1);

    /* I'd like to use auto_ptr here, but egcs 1.1.2 has it
     * commented out.. */

    Lexer *l = new Lexer(newbuf);

    int tok = l->lex();

    QCString key, command, pass;
    Data_entry data;
    const Data_entry *pdata;

    switch (tok) {
    case Lexer::Tok_pass: 
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	m_Pass = l->lval();
	tok = l->lex();
	if (tok != Lexer::Tok_num)
	    goto parse_error;
	m_Timeout = l->lval().toInt();
	if (l->lex() != '\n')
	    goto parse_error;
	respond(Res_OK);
	kDebugInfo("kdesud: Password set!");
	break;

    case Lexer::Tok_user:
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	m_User = l->lval();
	if (l->lex() != '\n')
	    goto parse_error;
	respond(Res_OK);
	kDebugInfo("kdesud: User set to %s", m_User.data());
	break;

    case Lexer::Tok_host:
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	m_Host = l->lval();
	if (l->lex() != '\n')
	    goto parse_error;
	respond(Res_OK);
	kDebugInfo("kdesud: Host set to %s", m_Host.data());
	break;

    case Lexer::Tok_prio:
	tok = l->lex();
	if (tok != Lexer::Tok_num)
	    goto parse_error;
	m_Priority = l->lval().toInt();
	if (l->lex() != '\n')
	    goto parse_error;
	respond(Res_OK);
	kDebugInfo("kdesud: priority set to %d", m_Priority);
	break;

    case Lexer::Tok_sched:
	tok = l->lex();
	if (tok != Lexer::Tok_num)
	    goto parse_error;
	m_Scheduler = l->lval().toInt();
	if (l->lex() != '\n')
	    goto parse_error;
	respond(Res_OK);
	kDebugInfo("kdesud: Scheduler set to %d", m_Scheduler);
	break;

    case Lexer::Tok_exec:
    {
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	command = l->lval();
	if (l->lex() != '\n')
	    goto parse_error;

	if (m_User.isEmpty())
	    goto parse_error;

	QCString auth_user;
	if ((m_Scheduler != SuProcess::SchedNormal) || (m_Priority > 50))
	    auth_user = "root";
	else
	    auth_user = m_User;

	key = makeKey(0, m_Host, auth_user, command);
	pdata = repo->find(key);
	if (!pdata) {
	    if (m_Pass.isNull()) {
		respond(Res_NO);
		break;
	    }
	    data.value = m_Pass;
	    data.timeout = m_Timeout;
	    repo->add(key, data);
	    pass = m_Pass;
	} else
	    pass = pdata->value;

	// Execute the command asynchronously
	kDebugInfo("kdesud: Executing command: %s", command.data());
	pid_t pid = fork();
	if (pid < 0) {
	    kDebugPError("fork()");
	    respond(Res_NO);
	    break;
	} else if (pid > 0) {
	    respond(Res_OK);
	    break;
	}

	// Ignore SIGCHLD because "class SuProcess" needs waitpid()
	signal(SIGCHLD, SIG_DFL);

	int ret;
	if (m_Host.isEmpty()) {
	    SuProcess proc;
	    proc.setCommand(command);
	    proc.setUser(m_User);
	    proc.setPriority(m_Priority);
	    proc.setScheduler(m_Scheduler);
	    ret = proc.exec(pass.data());
	} else {
	    SshProcess proc;
	    proc.setCommand(command);
	    proc.setUser(m_User);
	    proc.setHost(m_Host);
	    ret = proc.exec(pass.data());
	}

	kDebugInfo("kdesud: Command completed");
	_exit(ret);
    }

    case Lexer::Tok_del:
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	command = l->lval();
	if (l->lex() != '\n')
	    goto parse_error;
	
	if (m_User.isEmpty())
	    goto parse_error;
	key = makeKey(0, m_Host, m_User, command);
	pdata = repo->find(key);
	if (!pdata) {
	    respond(Res_NO);
	    break;
	}
	repo->remove(key);
	respond(Res_OK);
	kDebugInfo("kdesud: Deleted key: %s", (const char *) key);
	break;

    case Lexer::Tok_set:
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	key = QCString("1*") + l->lval();
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	data.value = l->lval();
	if (l->lex() != '\n')
	    goto parse_error;
	data.timeout = 86400;
	repo->add(key, data);
	kDebugInfo("kdesud: stored key: %s", key.data());
	respond(Res_OK);
	break;

    case Lexer::Tok_get:
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	key = QCString("1*") + l->lval();
	if (l->lex() != '\n')
	    goto parse_error;
	kDebugInfo("kdesud: request for key: %s", key.data());
	pdata = repo->find(key);
	if (pdata)
	    respond(Res_OK, pdata->value);
	else
	    respond(Res_NO);
	break;
	
    case Lexer::Tok_ping:
	tok = l->lex();
	if (tok != '\n')
	    goto parse_error;
	respond(Res_OK);
	kDebugInfo("kdesud: PING");
	break;

    case Lexer::Tok_stop:
	tok = l->lex();
	if (tok != '\n')
	    goto parse_error;
	respond(Res_OK);
	kDebugInfo("kdesud: Stopping by command");
	exit(0);

    default:
	kDebugInfo("kdesud: Uknown command: %s", l->lval().data());
	goto parse_error;
    }

    delete l;
    return 0;

parse_error:
    kDebugInfo("kdesud: Parse error");
    delete l;
    return -1;
}


/**
 * Handle a connection: make sure we don't block
 */

int ConnectionHandler::handle()
{
    int ret, nbytes;

    /* Add max # bytes to connection buffer */

    char tmpbuf[100];
    nbytes = recv(m_Fd, tmpbuf, 99, 0);

    if (nbytes < 0) {
	if (errno == EINTR)
	    return 0;
	// read error
	return -1;
    } else if (nbytes == 0) {
	// eof
	kDebugInfo("kdesud: eof on fd %d", m_Fd);
	return -1;
    }
    tmpbuf[nbytes] = '\000';

    if (m_Buf.length()+nbytes > 500) {
	kDebugWarning("kdesud: line too long");
	return -1;
    }

    m_Buf.append(tmpbuf);
    memset(tmpbuf, 'x', nbytes);
    
    /* Do we have a complete command yet? */

    while (m_Buf.find('\n') != -1) {
	ret = doCommand();
	if (ret < 0)
	    return ret;
    }

    return 0;
}

