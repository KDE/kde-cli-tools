## Makefile.am for kdessh

INCLUDES= -I$(top_srcdir)/kdesu/common $(all_includes)

## kdessh
bin_PROGRAMS = 	kdessh
kdessh_SOURCES = kdessh.cpp sshdlg.cpp
kdessh_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdessh_LDADD   = $(LIB_KDEUI) $(top_builddir)/kdesu/common/libkdesu.la
kdessh_METASOURCES =  AUTO
noinst_HEADERS = sshdlg.h

## Messages
messages:
	$(XGETTEXT) -C -ki18n -kI18N_NOOP -x $(includedir)/kde.pot $(kdessh_SOURCES) -o $(podir)/kdessh.pot

