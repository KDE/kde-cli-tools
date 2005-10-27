/*
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

#include <q3cstring.h>

#include <kdebug.h>
#include <kdesu/su.h>
#include <kdesu/ssh.h>

#include "handler.h"
#include "repo.h"
#include "lexer.h"
#include "secure.h"


// Global repository
extern Repository *repo;
void kdesud_cleanup();

ConnectionHandler::ConnectionHandler(int fd)
        : SocketSecurity(fd), m_exitCode(0), m_hasExitCode(false), m_needExitCode(false), m_pid(0)
{
    m_Fd = fd;
    m_Priority = 50;
    m_Scheduler = SuProcess::SchedNormal;
}

ConnectionHandler::~ConnectionHandler()
{
    m_Buf.fill('x');
    m_Pass.fill('x');
    close(m_Fd);
}

/*
 * Handle a connection: make sure we don't block
 */

int ConnectionHandler::handle()
{
    int ret, nbytes;

    // Add max 100 bytes to connection buffer

    char tmpbuf[100];
    nbytes = recv(m_Fd, tmpbuf, 99, 0);

    if (nbytes < 0)
    {
        if (errno == EINTR)
            return 0;
        // read error
        return -1;
    } else if (nbytes == 0)
    {
        // eof
        return -1;
    }
    tmpbuf[nbytes] = '\000';

    if (m_Buf.length()+nbytes > 1024)
    {
        kdWarning(1205) << "line too long";
        return -1;
    }

    m_Buf.append(tmpbuf);
    memset(tmpbuf, 'x', nbytes);

    // Do we have a complete command yet?
    int n;
    QByteArray newbuf;
    while ((n = m_Buf.find('\n')) != -1)
    {
        newbuf = m_Buf.left(n+1);
        m_Buf.fill('x', n+1);
        m_Buf.remove(0, n+1);
        ret = doCommand(newbuf);
        if (ret < 0)
            return ret;
    }

    return 0;
}

QByteArray ConnectionHandler::makeKey(int _namespace, QByteArray s1,
        QByteArray s2, QByteArray s3)
{
    QByteArray res;
    res.setNum(_namespace);
    res += "*";
    res += s1 + "*" + s2 + "*" + s3;
    return res;
}

void ConnectionHandler::sendExitCode()
{
    if (!m_needExitCode)
       return;
    QByteArray buf;
    buf.setNum(m_exitCode);
    buf.prepend("OK ");
    buf.append("\n");

    send(m_Fd, buf.data(), buf.length(), 0);
}

void ConnectionHandler::respond(int ok, QByteArray s)
{
    QByteArray buf;

    switch (ok) {
    case Res_OK:
        buf = "OK";
        break;
    case Res_NO:
    default:
        buf = "NO";
        break;
    }

    if (!s.isEmpty())
    {
        buf += ' ';
        buf += s;
    }

    buf += '\n';

    send(m_Fd, buf.data(), buf.length(), 0);
}

/*
 * Parse and do one command. On a parse error, return -1. This will
 * close the socket in the main accept loop.
 */

int ConnectionHandler::doCommand(QByteArray buf)
{
    if ((uid_t) peerUid() != getuid())
    {
        kdWarning(1205) << "Peer uid not equal to me\n";
        kdWarning(1205) << "Peer: " << peerUid() << " Me: " << getuid() << endl;
        return -1;
    }

    QByteArray key, command, pass, name, user, value, env_check;
    Data_entry data;

    Lexer *l = new Lexer(buf);
    int tok = l->lex();
    switch (tok)
    {
    case Lexer::Tok_pass:  // "PASS password:string timeout:int\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str)
            goto parse_error;
        m_Pass.fill('x');
        m_Pass = l->lval();
        tok = l->lex();
        if (tok != Lexer::Tok_num)
            goto parse_error;
        m_Timeout = l->lval().toInt();
        if (l->lex() != '\n')
            goto parse_error;
        if (m_Pass.isNull())
           m_Pass = "";
        kdDebug(1205) << "Password set!\n";
        respond(Res_OK);
        break;

    case Lexer::Tok_host:  // "HOST host:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str)
            goto parse_error;
        m_Host = l->lval();
        if (l->lex() != '\n')
            goto parse_error;
        kdDebug(1205) << "Host set to " << m_Host << endl;
        respond(Res_OK);
        break;

    case Lexer::Tok_prio:  // "PRIO priority:int\n"
        tok = l->lex();
        if (tok != Lexer::Tok_num)
            goto parse_error;
        m_Priority = l->lval().toInt();
        if (l->lex() != '\n')
            goto parse_error;
        kdDebug(1205) << "priority set to " << m_Priority << endl;
        respond(Res_OK);
        break;

    case Lexer::Tok_sched:  // "SCHD scheduler:int\n"
        tok = l->lex();
        if (tok != Lexer::Tok_num)
            goto parse_error;
        m_Scheduler = l->lval().toInt();
        if (l->lex() != '\n')
            goto parse_error;
        kdDebug(1205) << "Scheduler set to " << m_Scheduler << endl;
        respond(Res_OK);
        break;

    case Lexer::Tok_exec:  // "EXEC command:string user:string [options:string (env:string)*]\n"
    {
        QByteArray options;
        QList<QByteArray> env;
        tok = l->lex();
        if (tok != Lexer::Tok_str)
            goto parse_error;
        command = l->lval();
        tok = l->lex();
        if (tok != Lexer::Tok_str)
            goto parse_error;
        user = l->lval();
        tok = l->lex();
        if (tok != '\n')
        {
            if (tok != Lexer::Tok_str)
                goto parse_error;
            options = l->lval();
            tok = l->lex();
            while (tok != '\n')
            {
               if (tok != Lexer::Tok_str)
                   goto parse_error;
               QByteArray env_str = l->lval();
               env.append(env_str);
               if (strncmp(env_str, "DESKTOP_STARTUP_ID=", strlen("DESKTOP_STARTUP_ID=")) != 0)
                   env_check += "*"+env_str;
               tok = l->lex();
            }
        }

        QByteArray auth_user;
        if ((m_Scheduler != SuProcess::SchedNormal) || (m_Priority > 50))
            auth_user = "root";
        else
            auth_user = user;
        key = makeKey(2, m_Host, auth_user, command);
        // We only use the command if the environment is the same.
        if (repo->find(key) == env_check)
        {
           key = makeKey(0, m_Host, auth_user, command);
           pass = repo->find(key);
        }
        if (pass.isNull()) // isNull() means no password, isEmpty() can mean empty password
        {
            if (m_Pass.isNull())
            {
                respond(Res_NO);
                break;
            }
            data.value = env_check;
            data.timeout = m_Timeout;
            key = makeKey(2, m_Host, auth_user, command);
            repo->add(key, data);
            data.value = m_Pass;
            data.timeout = m_Timeout;
            key = makeKey(0, m_Host, auth_user, command);
            repo->add(key, data);
            pass = m_Pass;
        }

        // Execute the command asynchronously
        kdDebug(1205) << "Executing command: " << command << endl;
        pid_t pid = fork();
        if (pid < 0)
        {
            kdDebug(1205) << "fork(): " << strerror(errno) << endl;
            respond(Res_NO);
            break;
        } else if (pid > 0)
        {
            m_pid = pid;
            respond(Res_OK);
            break;
        }

        // Ignore SIGCHLD because "class SuProcess" needs waitpid()
        signal(SIGCHLD, SIG_DFL);

        int ret;
        if (m_Host.isEmpty())
        {
            SuProcess proc;
            proc.setCommand(command);
            proc.setUser(user);
            if (options.contains('x'))
               proc.setXOnly(true);
            if (options.contains('f'))
               proc.setDCOPForwarding(true);
            proc.setPriority(m_Priority);
            proc.setScheduler(m_Scheduler);
            proc.setEnvironment(env);
            ret = proc.exec(pass.data());
        } else
        {
            SshProcess proc;
            proc.setCommand(command);
            proc.setUser(user);
            proc.setHost(m_Host);
            ret = proc.exec(pass.data());
        }

        kdDebug(1205) << "Command completed: " << command << endl;
        _exit(ret);
    }

    case Lexer::Tok_delCmd:  // "DEL command:string user:string\n"
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    command = l->lval();
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    user = l->lval();
    if (l->lex() != '\n')
        goto parse_error;
    key = makeKey(0, m_Host, user, command);
    if (repo->remove(key) < 0) {
        kdDebug(1205) << "Unknown command: " << command << endl;
        respond(Res_NO);
    }
    else {
        kdDebug(1205) << "Deleted command: " << command << ", user = "
                      << user << endl;
        respond(Res_OK);
    }
    break;

    case Lexer::Tok_delVar:  // "DELV name:string \n"
    {
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    name = l->lval();
    tok = l->lex();
    if (tok != '\n')
        goto parse_error;
    key = makeKey(1, name);
    if (repo->remove(key) < 0)
    {
        kdDebug(1205) << "Unknown name: " << name << endl;
        respond(Res_NO);
    }
    else {
        kdDebug(1205) << "Deleted name: " << name << endl;
        respond(Res_OK);
    }
    break;
    }

    case Lexer::Tok_delGroup:  // "DELG group:string\n"
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    name = l->lval();
    if (repo->removeGroup(name) < 0)
    {
        kdDebug(1205) << "No keys found under group: " << name << endl;
        respond(Res_NO);
    }
    else
    {
        kdDebug(1205) << "Removed all keys under group: " << name << endl;
        respond(Res_OK);
    }
    break;

    case Lexer::Tok_delSpecialKey:  // "DELS special_key:string\n"
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    name = l->lval();
    if (repo->removeSpecialKey(name) < 0)
        respond(Res_NO);
    else
        respond(Res_OK);
    break;

    case Lexer::Tok_set:  // "SET name:string value:string group:string timeout:int\n"
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    name = l->lval();
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    data.value = l->lval();
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    data.group = l->lval();
    tok = l->lex();
    if (tok != Lexer::Tok_num)
        goto parse_error;
    data.timeout = l->lval().toInt();
    if (l->lex() != '\n')
        goto parse_error;
    key = makeKey(1, name);
    repo->add(key, data);
    kdDebug(1205) << "Stored key: " << key << endl;
    respond(Res_OK);
    break;

    case Lexer::Tok_get:  // "GET name:string\n"
    tok = l->lex();
    if (tok != Lexer::Tok_str)
        goto parse_error;
    name = l->lval();
    if (l->lex() != '\n')
        goto parse_error;
    key = makeKey(1, name);
    kdDebug(1205) << "Request for key: " << key << endl;
    value = repo->find(key);
    if (!value.isEmpty())
        respond(Res_OK, value);
    else
        respond(Res_NO);
    break;

    case Lexer::Tok_getKeys:  // "GETK groupname:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str)
            goto parse_error;
        name = l->lval();
        if (l->lex() != '\n')
            goto parse_error;
        kdDebug(1205) << "Request for group key: " << name << endl;
        value = repo->findKeys(name);
        if (!value.isEmpty())
            respond(Res_OK, value);
        else
            respond(Res_NO);
        break;

    case Lexer::Tok_chkGroup:  // "CHKG groupname:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str)
            goto parse_error;
        name = l->lval();
        if (l->lex() != '\n')
            goto parse_error;
        kdDebug(1205) << "Checking for group key: " << name << endl;
        if ( repo->hasGroup( name ) < 0 )
            respond(Res_NO);
        else
            respond(Res_OK);
        break;

    case Lexer::Tok_ping:  // "PING\n"
        tok = l->lex();
        if (tok != '\n')
            goto parse_error;
        respond(Res_OK);
        break;

    case Lexer::Tok_exit:  // "EXIT\n"
        tok = l->lex();
        if (tok != '\n')
            goto parse_error;
        m_needExitCode = true;
        if (m_hasExitCode)
           sendExitCode();
        break;

    case Lexer::Tok_stop:  // "STOP\n"
        tok = l->lex();
        if (tok != '\n')
            goto parse_error;
        kdDebug(1205) << "Stopping by command" << endl;
        respond(Res_OK);
        kdesud_cleanup();
        exit(0);

    default:
        kdWarning(1205) << "Unknown command: " << l->lval() << endl;
        respond(Res_NO);
        goto parse_error;
    }

    delete l;
    return 0;

parse_error:
    kdWarning(1205) << "Parse error" << endl;
    delete l;
    return -1;
}



