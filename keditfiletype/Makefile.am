INCLUDES= $(all_includes)

kcm_filetypes_la_LIBADD	= libfiletypes.la
kcm_filetypes_la_LDFLAGS = -module -avoid-version -no-undefined
kcm_filetypes_la_METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_filetypes.la

kcm_filetypes_la_SOURCES = control.cpp

noinst_HEADERS = filetypesview.h filetypedetails.h filegroupdetails.h \
	kservicelistwidget.h typeslistitem.h newtypedlg.h \
	kserviceselectdlg.h

bin_PROGRAMS = keditfiletype

noinst_LTLIBRARIES = libfiletypes.la
libfiletypes_la_LDFLAGS = -no-undefined
libfiletypes_la_LIBADD = $(LIB_KFILE) $(all_libraries)
libfiletypes_la_SOURCES = filetypesview.cpp filetypedetails.cpp filegroupdetails.cpp \
		 kservicelistwidget.cpp typeslistitem.cpp newtypedlg.cpp \
		 kserviceselectdlg.cpp

keditfiletype_SOURCES = keditfiletype.cpp
keditfiletype_LDADD = libfiletypes.la
keditfiletype_LDFLAGS = $(KDE_RPATH)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/filetypes.pot

data_DATA = filetypes.desktop
datadir = $(kde_appsdir)/Settings/FileBrowsing
