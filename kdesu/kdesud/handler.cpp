/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * handler.cpp: A connection handler for kdesud.
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>
#include <memory>

#include "kdesud.h"
#include "handler.h"
#include "repo.h"
#include "lexer.h"
#include "secure.h"
#include "process.h"
#include "debug.h"


ConnectionHandler::ConnectionHandler(int fd)
	: SocketSecurity(fd), mFd(fd)
{
    mbPass = false;
    mbUser = false;
}

ConnectionHandler::~ConnectionHandler()
{
    clear(mBuf);
}

void ConnectionHandler::respond(int ok, string s)
{
    string buf;

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

    if (s.size()) {
	buf += ' ';
	buf += s;
    }
    buf += '\n';

    send(mFd, buf.data(), buf.size(), 0);
}


/**
 * Parse and do one command. On a parse error, return -1. This will 
 * close the socket in the main accept loop.
 */

int ConnectionHandler::doCommand()
{
    if ((uid_t) peerUid() != getuid()) {
	error("Peer uid not equal to me");
	error("Peer: %d, Me: %d", peerUid(), (int) getuid());
	return -1;
    }

    string::size_type n;

    n = mBuf.find('\n');
    assert(n != string::npos);

    string newbuf(mBuf, 0, n+1);
    clear(mBuf, n+1);
    mBuf.erase(0, n+1);

    /* I'd like to use auto_ptr here, but egcs 1.1.2 has it
     * commented out.. */

    Lexer *l = new Lexer(newbuf);

    int tok = l->lex();

    string key, command;
    Data_entry data;
    const Data_entry *pdata;
    int ret;

    switch (tok) {
    case Lexer::Tok_pass: 
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	mPass = l->lval();
	mbPass = true;
	tok = l->lex();
	if (tok != Lexer::Tok_num)
	    goto parse_error;
	mTimeout = atoi(l->lval().c_str());
	tok = l->lex();
	if (tok != '\n')
	    goto parse_error;
	respond(Res_OK);
	debug("Password set!");
	break;

    case Lexer::Tok_user:
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	mUser = l->lval();
	mbUser = true;
	tok = l->lex();
	if (tok != '\n')
	    goto parse_error;
	respond(Res_OK);
	debug("User set to %s", mUser.c_str());
	break;

    case Lexer::Tok_exec:
    {
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	command = l->lval();
	tok = l->lex();
	if (tok != '\n')
	    goto parse_error;

	if (!mbUser)
	    goto parse_error;
	key = mUser;
	key += "*";
	key += command;
	
	pdata = repo->find(key);
	if (!pdata) {
	    if (!mbPass) {
		respond(Res_NO);
		break;
	    }
	    data.value = mPass;
	    data.timeout = mTimeout;
	    repo->add(key, &data);
	    pdata = &data;
	}

	// Execute the command asynchronously
	debug("Executing command: %s", command.c_str());
	pid_t pid = fork();
	if (pid < 0) {
	    xerror("fork(): %s");
	    respond(Res_NO);
	    break;
	} else if (pid > 0) {
	    respond(Res_OK);
	    break;
	}

	// Ignore SIGCHLD because "class SuProcess" needs waitpid()
	signal(SIGCHLD, SIG_DFL);

	SuProcess proc;
	proc.setCommand(command.c_str());
	proc.setUser(mUser.c_str());
	ret = proc.exec((char *)pdata->value.c_str());
	debug("Command completed");
	_exit(ret);
    }

    case Lexer::Tok_del:
	tok = l->lex();
	if (tok != Lexer::Tok_str)
	    goto parse_error;
	command = l->lval();
	tok = l->lex();
	if (tok != '\n')
	    goto parse_error;
	
	if (!mbUser)
	    goto parse_error;
	key = mUser;
	key += "*";
	key += command;

	pdata = repo->find(key);
	if (!pdata) {
	    respond(Res_NO);
	    break;
	}
	repo->erase(key);
	respond(Res_OK);
	debug("Deleted key: %s", key.c_str());
	break;

    case Lexer::Tok_ping:
	tok = l->lex();
	if (tok != '\n')
	    goto parse_error;
	respond(Res_OK);
	debug("PING");
	break;

    case Lexer::Tok_stop:
	tok = l->lex();
	if (tok != '\n')
	    goto parse_error;
	respond(Res_OK);
	debug("Stopping by command");
	exit(0);

    default:
	debug("Uknown command");
	goto parse_error;
    }

    delete l;
    return 0;

parse_error:
    debug("Parse error");
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
    nbytes = recv(mFd, tmpbuf, 99, 0);

    if (nbytes < 0) {
	if (errno == EINTR)
	    return 0;
	// read error
	return -1;
    } else if (nbytes == 0) {
	// eof
	debug("eof on fd %d", mFd);
	return -1;
    }
    tmpbuf[nbytes] = '\000';

    if (mBuf.size()+nbytes > 500) {
	warning("line too long");
	return -1;
    }

    mBuf += tmpbuf;
    memset(tmpbuf, 'x', nbytes);

    
    /* Do we have a complete command yet? */

    while (mBuf.find('\n') != string::npos) {
	ret = doCommand();
	if (ret < 0)
	    return ret;
    }

    return 0;
}

