## Makefile.am for kcmkdesu

INCLUDES= $(all_includes) -I$(top_srcdir)/kdesu/common

lib_LTLIBRARIES = libkcm_kdesu.la
libkcm_kdesu_la_SOURCES = kcmkdesu.cpp
libkcm_kdesu_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_kdesu_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/kdesu/common/libkdesu.la

METASOURCES = AUTO
noinst_HEADERS = kcmkdesu.h

messages:
	$(XGETTEXT) $(libkcm_kdesu_la_SOURCES) -o $(podir)/kcmkdesu.pot
