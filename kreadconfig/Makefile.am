AM_CPPFLAGS = -DQT_NO_CAST_ASCII 

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =       $(LIB_KDECORE)

bin_PROGRAMS	= kreadconfig
kreadconfig_SOURCES	= kreadconfig.cpp

messages:
	$(XGETTEXT) $(kreadconfig_SOURCES) -o $(podir)/kreadconfig.pot
