####### Fiddle here

AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_CLEAN_NAMESPACE -DQT_NO_COMPAT

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =       $(LIB_KDECORE)

#######	Files

bin_PROGRAMS	= kstart
kstart_SOURCES	= kstart.C
METASOURCES =	kstart.moc 

noinst_HEADERS = kstart.h version.h

messages:
	$(XGETTEXT) $(kstart_SOURCES) -o $(podir)/kstart.pot

EXTRA_DIST = CHANGES LICENSE.readme

