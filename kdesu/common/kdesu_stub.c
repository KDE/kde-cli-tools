/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * kdesu_stub.c: KDE su executes this stub, which in turn executes the
 *	         desired program.
 * 
 * We use a stub program because: In general, we need X authentication.
 * The X cookie is confidential and cannot be passed as a command line
 * argument. Because we want to be able to su to other users than root, 
 * we cannot use a private file. The solution used here is to use the 
 * pty/tty channel created by "class RootProcess" to pass the cookie. 
 * Now that we use this channel, we might as well pass all needed information
 * (nonconfidential -- like DISPLAY and PATH) through it.
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
 * Solaris does not have a setenv()...
 */
int xsetenv(char *name, char *value)
{
    char *s;

    s = malloc(strlen(name)+strlen(value)+2);
    if (!s)
	return -1;
    strcpy(s, name);
    strcat(s, "=");
    strcat(s, value);
    return putenv(s); // yes: no free(s)
}

/**
 * Safe strdup and strip newline
 */

char *xstrdup(char *src)
{
    char *dst = strdup(src);
    int i = strlen(src)-1;
    if (dst == 0L) {
	perror("malloc()");
	exit(1);
    }
    if (dst[i] == '\n')
	dst[i] = '\000';
    return dst;
}


/**
 * The main program
 */

int main()
{
    char buf[1024];
    int res, fd;
    pid_t pid;
    char *display, *cookie, *proto, *path, *command;
    char *fname=0L;
    struct termios tio, tip;

    /* Disable local echo: this makes the parsing in process.cpp easier */

    if (tcgetattr(0, &tio) < 0) {
	printf("end\n"); perror("tcgetattr()");
	exit(1);
    }
    tip = tio;
    tio.c_lflag &= ~ECHO;
    if (tcsetattr(0, TCSANOW, &tio) < 0) {
	printf("end\n"); perror("tcsetattr()");
	exit(1);
    }
	
    /* Get values */

    printf("display\n");
    if (fgets(buf, 1024, stdin) == 0L) {
	printf("end\n"); perror("fgets()");
	exit(1);
    }
    display = xstrdup(buf);

    printf("proto\n");
    if (fgets(buf, 1024, stdin) == 0L) {
	printf("end\n"); perror("fgets()");
	exit(1);
    }
    proto = xstrdup(buf);

    printf("cookie\n");
    if (fgets(buf, 1024, stdin) == 0L) {
	printf("end\n"); perror("fgets()");
	exit(1);
    }
    cookie = xstrdup(buf);

    printf("path\n");
    if (fgets(buf, 1024, stdin) == 0L) {
	printf("end\n"); perror("fgets()");
	exit(1);
    }
    path = xstrdup(buf);

    printf("command\n");
    if (fgets(buf, 1024, stdin) == 0L) {
	printf("end\n"); perror("fgets()");
	exit(1);
    }
    command = xstrdup(buf); 

    /* End conversation */

    printf("end\n");

    /* Restore echo mode */

    if (tcsetattr(0, TCSANOW, &tip) < 0)
	printf("warning: cannot restore terminal mode");
    
    /* Set environment */

    if (strlen(path))
	xsetenv("PATH", path);
    if (strlen(display))
	xsetenv("DISPLAY", display);
    
    /* If available, add an authentication cookie. */

    if (strlen(cookie)) {

	fname = tmpnam(NULL);
	fd = open(fname, O_CREAT|O_EXCL);
	if (fd < 0) {
	    perror("open()");
	    exit(1);
	}
	if (chmod(fname, 0600) < 0) {
	    perror("chmod()");
	    exit(1);
	}
	xsetenv("XAUTHORITY", fname);

	pid = fork();
	if (pid == -1) {
	    perror("fork()");
	    exit(1);
	}
	if (pid) {
	    pid = waitpid(pid, &res, 0);
	    if (pid < 0) {
		perror("waitpid()");
		exit(1);
	    }
	    if (!WIFEXITED(res) || WEXITSTATUS(res)) {
		printf("xauth retured error code %d\n", WEXITSTATUS(res));
		exit(1);
	    }
	} else {
	    execlp("xauth", "xauth", "add", display, proto, cookie, NULL);
	    perror("execlp()");
	    _exit(1);
	}
    }

    /* Execute the command */

    pid = fork();
    if (pid == -1) {
	perror("fork()");
	exit(1);
    }
    if (pid) {
	pid = waitpid(pid, &res, 0);
	if (pid < 0)
	    perror("waitpid()");
	if (fname)
	    unlink(fname);
    } else {
	execl("/bin/sh", "sh", "-c", command, NULL);
	perror("execl()");
	exit(1);
    }

    if (WIFEXITED(res))
	return WEXITSTATUS(res);
    return 1;
}

