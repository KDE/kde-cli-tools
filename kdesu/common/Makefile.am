## Makefile.am for kdesu/common

INCLUDES = -I$(qt_includes)
noinst_LTLIBRARIES = libkdesu.la
noinst_HEADERS = kdesu.h client.h process.h pty.h pathsearch.h xcookie.h
libkdesu_la_SOURCES = pty.cpp client.cpp process.cpp pathsearch.cpp xcookie.cpp
libkdesu_la_LDFLAGS = -static

bin_PROGRAMS = kdesu_stub
kdesu_stub_SOURCES = kdesu_stub.c

