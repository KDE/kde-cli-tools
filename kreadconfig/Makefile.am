AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_CLEAN_NAMESPACE -DQT_NO_COMPAT

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =       $(LIB_KDECORE)

bin_PROGRAMS	= kreadconfig
kreadconfig_SOURCES	= kreadconfig.cpp

messages:
	$(XGETTEXT) $(kreadconfig_SOURCES) -o $(podir)/kreadconfig.pot
