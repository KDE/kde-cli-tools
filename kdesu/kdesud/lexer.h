/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __Lexer_h_included__
#define __Lexer_h_included__

#include <QByteArray>

/**
 * This is a lexer for the kdesud protocol.
 */

class Lexer {
public:
    Lexer(const QByteArray &input);
    ~Lexer();

    /** Read next token. */
    int lex();

    /** Return the token's value. */
    QByteArray &lval();

    enum Tokens { 
    Tok_none, Tok_exec=256, Tok_pass, Tok_delCmd,
    Tok_ping, Tok_str, Tok_num , Tok_stop,
    Tok_set, Tok_get, Tok_delVar, Tok_delGroup,
    Tok_host, Tok_prio, Tok_sched, Tok_getKeys,
    Tok_chkGroup, Tok_delSpecialKey, Tok_exit
    };

private:
    QByteArray m_Input;
    QByteArray m_Output;

    int in;
};

#endif
