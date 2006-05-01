####### Fiddle here

AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_CLEAN_NAMESPACE -DQT_NO_COMPAT
KDE_CXXFLAGS = -UQT3_SUPPORT

INCLUDES = $(all_includes)

#######	Files

bin_PROGRAMS	= kstart
kstart_SOURCES	= kstart.cpp
kstart_LDFLAGS  = $(all_libraries) $(KDE_RPATH)
kstart_LDADD    = $(LIB_KDECORE)
METASOURCES =	kstart.moc 

noinst_HEADERS = kstart.h version.h


EXTRA_DIST = CHANGES LICENSE.readme

