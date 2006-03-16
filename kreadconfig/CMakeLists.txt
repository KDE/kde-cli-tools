kde4_header()

project(kreadconfig)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kreadconfig_SRCS kreadconfig.cpp )

kde4_automoc(${kreadconfig_SRCS})

kde4_add_executable(kreadconfig ${kreadconfig_SRCS})

target_link_libraries(kreadconfig  ${LIB_KDEUI} )

install_targets(/bin kreadconfig )


########### next target ###############

set(kwriteconfig_SRCS kwriteconfig.cpp )

kde4_automoc(${kwriteconfig_SRCS})

kde4_add_executable(kwriteconfig ${kwriteconfig_SRCS})

target_link_libraries(kwriteconfig  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kwriteconfig )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -DQT_NO_CAST_ASCII 
#
#INCLUDES = $(all_includes)
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#LDADD   =       $(LIB_KDECORE)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#bin_PROGRAMS	= kreadconfig kwriteconfig
#kreadconfig_SOURCES	= kreadconfig.cpp
#kwriteconfig_SOURCES	= kwriteconfig.cpp
#
#### TODO: why do we need 2 POT files for one directory?
#messages:
#	$(XGETTEXT) kreadconfig.cpp -o $(podir)/kreadconfig.pot
#	$(XGETTEXT) kwriteconfig.cpp -o $(podir)/kwriteconfig.pot
