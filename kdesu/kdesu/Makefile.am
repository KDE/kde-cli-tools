## Makefile.am for kdesu by Geert Jansen

INCLUDES= -I$(top_srcdir)/kdesu/common $(all_includes)

## kdesu
bin_PROGRAMS = 	kdesu
kdesu_SOURCES = kdesu.cpp sudlg.cpp
kdesu_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdesu_LDADD   = $(LIB_KDECORE) $(LIB_KDEUI) $(top_builddir)/kdesu/common/libkdesu.la
kdesu_METASOURCES =  AUTO
noinst_HEADERS = sudlg.h

## Messages
messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kdesu_SOURCES) && mv messages.po $(podir)/kdesu.pot
