/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * kdesu_stub.c: KDE su executes this stub, which in turn executes the
 *	         wanted program. Before that, startup parameters are sent
 *	         through the pty/tty channel.
 * 
 *
 * Available parameters:   
 *
 *   Parameter       Description         Format (csl = comma separated list)
 *
 * - display         X11 display         string
 * - display_auth    X11 authentication  "type cookie" pair
 * - dcopserver      KDE dcopserver      csl of netids
 * - dcop_auth       DCOP authentication csl of "type cookie" pairs for DCOP
 * - ice_auth        ICE authentication  csl of "type cookie" pairs for ICE
 * - command         Command to run      string
 * - path            PATH env. var       string
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


/**
 * Params sent by the peer.
 */

struct param_struct {
    char *name;
    char *value;
};

struct param_struct params[] = {
    {"display", 0L},
    {"display_auth", 0L},
    {"dcopserver", 0L},
    {"dcop_auth", 0L},
    {"ice_auth", 0L},
    {"command", 0L},
    {"path", 0L},
};

#define P_DISPLAY 0
#define P_DISPLAY_AUTH 1
#define P_DCOPSERVER 2
#define P_DCOP_AUTH 3
#define P_ICE_AUTH 4
#define P_COMMAND 5
#define P_PATH 6
#define P_LAST 7


/**
 * Safe malloc functions.
 */
char *xmalloc(int size)
{
    char *ptr = malloc(size);
    if (ptr) return ptr;
    perror("malloc()");
    exit(1);
}


char *xrealloc(char *ptr, int size)
{
    ptr = realloc(ptr, size);
    if (ptr) return ptr;
    perror("realloc()");
    exit(1);
}


/**
 * Solaris does not have a setenv()...
 */
int xsetenv(char *name, char *value)
{
    char *s = malloc(strlen(name)+strlen(value)+2);
    if (!s) return -1;
    strcpy(s, name);
    strcat(s, "=");
    strcat(s, value);
    return putenv(s); // yes: no free()!
}

/**
 * Safe strdup and strip newline
 */
char *xstrdup(char *src)
{
    int len = strlen(src);
    char *dst = xmalloc(len+1);
    strcpy(dst, src);
    if (dst[len-1] == '\n')
	dst[len-1] = '\000';
    return dst;
}

/**
 * Split comma separated list.
 */
char **xstrsep(char *str)
{
    int i = 0, size = 10;
    char **list = (char **) xmalloc(size * sizeof(char *));
    char *ptr = str, *nptr;
    while ((nptr = strchr(ptr, ',')) != 0L) {
	if (i > size-2)
	    list = realloc(list, (size *= 2) * sizeof(char *));
	*nptr = '\000';
	list[i++] = ptr;
	ptr = nptr+1;
    }
    if (*ptr != '\000')
	list[i++] = ptr;
    list[i] = 0L;
    return list;
}


/**
 * The main program
 */

int main()
{
    char buf[1024];
    char command[200], xauthority[200], iceauthority[200];
    char **host, **auth, *fname;
    int i, res;
    struct termios tio, tip;
    FILE *fout;

    /* Get startup parameters. Disable local echo: this this makes 
     * the parsing in process.cpp easier */

    if (tcgetattr(0, &tio) < 0) {
	printf("end\n"); perror("Fatal: tcgetattr()");
	exit(1);
    }
    tip = tio;
    tio.c_lflag &= ~ECHO;
    if (tcsetattr(0, TCSANOW, &tio) < 0) {
	printf("end\n"); perror("Fatal: tcsetattr()");
	exit(1);
    }
    for (i=0; i<P_LAST; i++) {
	printf("%s\n", params[i].name);
	if (fgets(buf, 1024, stdin) == 0L) {
	    printf("end\n"); perror("Fatal: fgets()");
	    exit(1);
	}
	params[i].value = xstrdup(buf);
    }
    printf("end\n");
    if (tcsetattr(0, TCSANOW, &tip) < 0)
	printf("Warning: cannot restore terminal mode");
    

    /* Handle display */

    xsetenv("DISPLAY", params[P_DISPLAY].value);
    if (params[P_DISPLAY_AUTH].value[0]) {
	fname = tmpnam(0L);
	fout = fopen(fname, "w");
	if (!fout) {
	    perror("Fatal: fopen()");
	    exit(1);
	}
	fprintf(fout, "add %s %s\n", params[P_DISPLAY].value, 
		params[P_DISPLAY_AUTH].value);
	fclose(fout);
	tmpnam(xauthority);
	xsetenv("XAUTHORITY", xauthority);
	sprintf(command, "xauth source %s >/dev/null 2>&1", fname);
	if (system(command))
	    printf("Warning: failed to add X authentication");
	unlink(fname);
    }
    

    /* Handle DCOP */

    xsetenv("DCOPSERVER", params[P_DCOPSERVER].value);
    host = xstrsep(params[P_DCOPSERVER].value);
    auth = xstrsep(params[P_ICE_AUTH].value);
    if (host[0]) {
	fname = tmpnam(0L);
	fout = fopen(fname, "w");
	if (!fout) {
	    perror("Fatal: fopen()");
	    exit(1);
	}
	for (i=0; host[i]; i++)
	    fprintf(fout, "add ICE \"\" %s %s\n", host[i], auth[i]);
	auth = xstrsep(params[P_DCOP_AUTH].value);
	for (i=0; host[i]; i++)
	    fprintf(fout, "add DCOP \"\" %s %s\n", host[i], auth[i]);
	fclose(fout);
	tmpnam(iceauthority);
	xsetenv("ICEAUTHORITY", iceauthority);
	sprintf(command, "iceauth source %s >/dev/null 2>&1", fname);
	if (system(command))
	    printf("Warning: failed to add DCOP authentication");
	unlink(fname);
    }
 

    /* Execute the command */

    xsetenv("PATH", params[P_PATH].value);
    i = system(params[P_COMMAND].value);
    unlink(xauthority);
    unlink(iceauthority);
    return i;
}
