## Makefile.am for kcmkdesu

INCLUDES= $(all_includes) -I$(top_srcdir)/kdesu/common

bin_PROGRAMS = kcmkdesu
kcmkdesu_SOURCES = kcmkdesu.cpp 
kcmkdesu_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kcmkdesu_LDADD   = $(LIB_KDEUI) $(top_srcdir)/kdesu/common/libkdesu.a
kcmkdesu_METASOURCES = AUTO

noinst_HEADERS = kcmkdesu.h

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkdesu_SOURCES) && mv messages.po ../../po/kcmkdesu.pot
