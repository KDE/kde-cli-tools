/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * debug.h: Debugging macros used all over kdesu. They are macros instead of
 *          functions so that I can use the __FUNCTION__ stuff.
 */

#ifndef __Debug_h_included__
#define __Debug_h_included__

#include <stdio.h>
#include <errno.h>
#include <string.h>

// Globals: define once in each program.
extern int _show_dbg;
extern int _show_wrn;

#define debug(fmt,arg...) \
    do { \
	if (!_show_dbg) \
	    break; \
	fprintf(stderr, "DEBUG: %s() at %s:%d: ", __FUNCTION__,  __FILE__, __LINE__); \
	fprintf(stderr, fmt, ##arg); \
	fprintf(stderr, "\n"); \
    } while (0)

#define warning(fmt,arg...) \
    do { \
	if (!_show_wrn) \
	    break; \
	fprintf(stderr, "Warning: "); \
	fprintf(stderr, fmt, ##arg); \
	fprintf(stderr, "\n"); \
    } while (0)

#define error(fmt,arg...) \
    do { \
	fprintf(stderr, "Error: "); \
	fprintf(stderr, fmt, ##arg); \
	fprintf(stderr, "\n"); \
    } while (0)

#define xerror(fmt,arg...) \
    do { \
	fprintf(stderr, "Error: "); \
	fprintf(stderr, fmt, ##arg, strerror(errno)); \
	fprintf(stderr, "\n"); \
    } while (0)

#endif // __Debug_h_included__
