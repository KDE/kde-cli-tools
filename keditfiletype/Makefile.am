INCLUDES= $(all_includes)

libkcm_filetypes_la_LIBADD	= $(LIB_KSYCOCA) $(LIB_KDEUI)
libkcm_filetypes_la_LDFLAGS = $(all_libraries) -module -avoid-version
libkcm_filetypes_la_METASOURCES = AUTO

lib_LTLIBRARIES = libkcm_filetypes.la

libkcm_filetypes_la_SOURCES = \
	filetypesview.cpp typeslistitem.cpp \
	extensiondlg.cpp newtypedlg.cpp

noinst_HEADERS = filetypesview.h typeslistitem.h \
	extensiondlg.h newtypedlg.h

DISTCLEANFILES = $(libkcm_filetypes_la_METASOURCES)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/filetypes.pot
