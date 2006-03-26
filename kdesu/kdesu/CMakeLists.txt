kde4_header()

include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(kdesu_SRCS kdesu.cpp sudlg.cpp )

kde4_automoc(${kdesu_SRCS})

kde4_add_executable(kdesu ${kdesu_SRCS})

target_link_libraries(kdesu  ${KDE4_KIO_LIBS} kdesu )

install_targets(/bin kdesu )


########### install files ###############


kde4_footer()



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
