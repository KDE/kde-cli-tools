INCLUDES= $(all_includes)

libkcm_filetypes_la_LIBADD	= $(LIB_KIO) $(LIB_KDEUI)
libkcm_filetypes_la_LDFLAGS = $(all_libraries) -module -avoid-version
libkcm_filetypes_la_METASOURCES = AUTO

lib_LTLIBRARIES = libkcm_filetypes.la

libkcm_filetypes_la_SOURCES = \
	filetypesview.cpp typeslistitem.cpp \
	extensiondlg.cpp newtypedlg.cpp

noinst_HEADERS = filetypesview.h typeslistitem.h \
	extensiondlg.h newtypedlg.h

datadir   = $(kde_appsdir)/Settings/System
data_DATA = filetypes.desktop
#KDE_ICON = filetypes

DISTCLEANFILES = $(libkcm_filetypes_la_METASOURCES)

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot *.cpp && mv messages.po ../po/filetypes.pot
