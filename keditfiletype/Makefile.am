INCLUDES= $(all_includes)

libkcm_filetypes_la_LIBADD	= $(LIB_KSYCOCA) $(LIB_KDEUI) $(LIB_KFILE)
libkcm_filetypes_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkcm_filetypes_la_METASOURCES = AUTO

kde_module_LTLIBRARIES = libkcm_filetypes.la

libkcm_filetypes_la_SOURCES = \
	filetypesview.cpp filetypedetails.cpp filegroupdetails.cpp \
	kservicelistwidget.cpp typeslistitem.cpp newtypedlg.cpp \
	kserviceselectdlg.cpp

noinst_HEADERS = filetypesview.h filetypedetails.h filegroupdetails.h \
	kservicelistwidget.h typeslistitem.h newtypedlg.h \
	kserviceselectdlg.h

bin_PROGRAMS = keditfiletype

keditfiletype_SOURCES = keditfiletype.cpp
keditfiletype_LDADD = libkcm_filetypes.la
keditfiletype_LDFLAGS = $(all_libraries) $(KDE_RPATH)

DISTCLEANFILES = $(libkcm_filetypes_la_METASOURCES)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/filetypes.pot

data_DATA = filetypes.desktop
datadir = $(kde_appsdir)/Settings/FileBrowsing
