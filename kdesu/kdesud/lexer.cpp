/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * lexer.cpp: A lexer for the kdesud protocol. See kdesud.cpp for a
 *            description of the protocol.
 */

#include <ctype.h>

#include <string>

#include "lexer.h"
#include "kdesud.h"
#include "debug.h"


Lexer::Lexer(string &input)
{
    mInput = input; // copy
    in = mInput.begin();
}

Lexer::~Lexer()
{
    // Clear out buffers
    clear(mInput);
    clear(mOutput);
}

string &Lexer::lval()
{
    return mOutput;
}

/*
 * lex() is the lexer. There is no end-of-input check here so that has to be
 * done by the caller.
 */

int Lexer::lex()
{
    char c;

    c = *in++;
    clear(mOutput);
    mOutput.erase();

    while (1) {

	// newline? 
	if (c == '\n')
	    return '\n';

	// No control characters 
	if (iscntrl(c))
	    return Tok_none;

	if (isspace(c))
	    while (isspace(c = *in++));

	// number?
	if (isdigit(c)) {
	    mOutput += c;
	    while (isdigit(c = *in++))
		mOutput += c;
	    in--;
	    return Tok_num;
	}

	// quoted string?
	if (c == '"') {
	    c = *in++;
	    while ((c != '"') && !iscntrl(c)) {
		// handle escaped characters
		if (c == '\\')
		    mOutput += *in++;
		else
		    mOutput += c;
		c = *in++;
	    }
	    if (c == '"')
		return Tok_str;
	    return Tok_none;
	}

	// normal string
	while (!isspace(c) && !iscntrl(c)) {
	    mOutput += c;
	    c = *in++;
	}
	in--;

	// command? 
	if (mOutput.size() <= 4) {
	    if (mOutput == "EXEC")
		return Tok_exec;
	    if (mOutput == "PASS")
		return Tok_pass;
	    if (mOutput == "DEL")
		return Tok_del;
	    if (mOutput == "PING")
		return Tok_ping;
	    if (mOutput == "STOP")
		return Tok_stop;
	    if (mOutput == "USER")
		return Tok_user;
	}

	return Tok_str;
    }
}

