kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kstart_SRCS kstart.cpp )

kde4_automoc(${kstart_SRCS})

kde4_add_executable(kstart ${kstart_SRCS})

target_link_libraries(kstart  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kstart )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

######## Fiddle here
#
#AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_CLEAN_NAMESPACE -DQT_NO_COMPAT
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#INCLUDES = $(all_includes)
#
########	Files
#
#bin_PROGRAMS	= kstart
#kstart_SOURCES	= kstart.cpp
#kstart_LDFLAGS  = $(all_libraries) $(KDE_RPATH)
#kstart_LDADD    = $(LIB_KDECORE)
#METASOURCES =	kstart.moc 
#
#noinst_HEADERS = kstart.h version.h
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kstart.pot
#
#EXTRA_DIST = CHANGES LICENSE.readme
#
