## Makefile.am for kdesud

INCLUDES = -I$(top_srcdir)/kdesu/common

bin_PROGRAMS = kdesud
kdesud_SOURCES = kdesud.cpp repo.cpp lexer.cpp handler.cpp secure.cpp 
kdesud_LDADD = $(top_srcdir)/kdesu/common/libkdesu.a
noinst_HEADERS = repo.h kdesud.h handler.h lexer.h secure.h

## kdesud needs to be suid or sgid something
INSTALL_PROGRAM = $(INSTALL) -m 2755 -g @grp_nogroup@
