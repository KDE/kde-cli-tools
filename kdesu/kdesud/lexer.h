/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __Lexer_h_included__
#define __Lexer_h_included__

/**
 * This is a lexer for the kdesud protocol.
 */

class Lexer {
public:
    Lexer(string &input);
    ~Lexer();

    /**
     * Read next token.
     */
    int lex();

    /**
     * Return the token's semantic value.
     */
    string &lval();

    enum Tokens { 
	Tok_none, 
	Tok_exec=256, Tok_pass, 
	Tok_user, Tok_del, 
	Tok_ping, Tok_str, 
	Tok_num , Tok_stop
    };

private:
    string mInput;
    string mOutput;

    string::iterator in;
};

#endif
