EXTRA_DIST = \
	Exports

AM_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	$(QtXml_CFLAGS) \
	$(TRACKER_CFLAGS) \
	$(DUI_CFLAGS) \
	$(GCONF_CFLAGS) \
	-I/usr/include/gallery/service \
	-I/usr/include/music-suite/service \
	$(QtGui_CFLAGS) \
	'-DDEFAULT_ACTIONS="@sysconfdir@/contentaction/actions.d"'

LIBS += \
	$(QtCore_LIBS) \
	$(QtXml_LIBS) \
	$(TRACKER_LIBS) \
	$(DUI_LIBS) \
	$(GCONF_LIBS) \
	-lgalleryinterface \
	-lmusicsuitepublicinterface \
	$(QtGui_LIBS) # temporary; see configure.ac

incdir = $(includedir)/contentaction
inc_HEADERS = \
	contentaction.h

lib_LTLIBRARIES = libcontentaction.la

libcontentaction_la_DEPENDENCIES = \
	Exports

libcontentaction_la_SOURCES = \
	internal.h \
	contentaction.h \
	contentaction.cpp \
	config.cpp

libcontentaction_la_LDFLAGS = \
	-Wl,--version-script=$(srcdir)/Exports

bin_PROGRAMS = lca-tool

# Force different object file names by using per-target flags.
lca_tool_CPPFLAGS = \
	$(AM_CPPFLAGS)

lca_tool_SOURCES = \
	lca-tool.cpp \
	$(libcontentaction_la_SOURCES)
