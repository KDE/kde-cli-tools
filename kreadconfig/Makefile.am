AM_CPPFLAGS = -DQT_NO_CAST_ASCII 

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =       $(LIB_KDECORE)
KDE_CXXFLAGS = -UQT3_SUPPORT

bin_PROGRAMS	= kreadconfig kwriteconfig
kreadconfig_SOURCES	= kreadconfig.cpp
kwriteconfig_SOURCES	= kwriteconfig.cpp

