## Makefile.am for kdesu by Geert Jansen

INCLUDES= $(all_includes)

## kdesu
bin_PROGRAMS = 	kdesu
kdesu_SOURCES = kdesu.cpp sudlg.cpp
kdesu_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdesu_LDADD   = $(LIB_KDEUI) -lkdesu
kdesu_METASOURCES =  AUTO
noinst_HEADERS = sudlg.h

## Messages
messages:
	$(XGETTEXT) $(kdesu_SOURCES) -o $(podir)/kdesu.pot
