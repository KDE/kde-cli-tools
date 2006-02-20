AM_CPPFLAGS = -DQT_NO_CAST_ASCII 

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =       $(LIB_KDECORE)
KDE_CXXFLAGS = -UQT3_SUPPORT

bin_PROGRAMS	= kreadconfig kwriteconfig
kreadconfig_SOURCES	= kreadconfig.cpp
kwriteconfig_SOURCES	= kwriteconfig.cpp

### TODO: why do we need 2 POT files for one directory?
messages:
	$(XGETTEXT) kreadconfig.cpp -o $(podir)/kreadconfig.pot
	$(XGETTEXT) kwriteconfig.cpp -o $(podir)/kwriteconfig.pot
