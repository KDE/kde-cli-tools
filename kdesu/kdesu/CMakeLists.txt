



########### next target ###############

set(kdesu_SRCS kdesu.cpp sudlg.cpp )

kde4_automoc(${kdesu_SRCS})

kde4_add_executable(kdesu_executable ${kdesu_SRCS})
# in KDELibsDependencies.cmake installed by kdelibs there is a dependency to "kdesu"
# which is then recognized here as the target name for this executable
# so give the target here a different name and use the OUTPUT_NAME property to 
# give it the name kdesu
set_target_properties(kdesu_executable PROPERTIES OUTPUT_NAME kdesu)

target_link_libraries(kdesu_executable  ${KDE4_KIO_LIBS} ${KDE4_KDESU_LIBS} )

install(TARGETS kdesu_executable DESTINATION ${BIN_INSTALL_DIR})





#original Makefile.am contents follow:

### Makefile.am for kdesu by Geert Jansen
#
#INCLUDES= $(all_includes)
#
### kdesu
#bin_PROGRAMS = 	kdesu
#kdesu_SOURCES = kdesu.cpp sudlg.cpp
#kdesu_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kdesu_LDADD   = $(LIB_KIO) -lkdesu
#kdesu_METASOURCES =  AUTO
#noinst_HEADERS = sudlg.h
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kdesu.pot
