## Makefile.am for kdesu/common

INCLUDES = $(all_includes)
noinst_LTLIBRARIES = libkdesu.la
noinst_HEADERS = kdesu.h client.h process.h pty.h kcookie.h su.h ssh.h stub.h
libkdesu_la_SOURCES = pty.cpp client.cpp process.cpp kcookie.cpp su.cpp ssh.cpp stub.cpp
libkdesu_la_LDFLAGS = -no-undefined $(all_libraries)
libkdesu_la_LIBADD = $(LIB_KDECORE)

bin_PROGRAMS = kdesu_stub
kdesu_stub_SOURCES = kdesu_stub.c

