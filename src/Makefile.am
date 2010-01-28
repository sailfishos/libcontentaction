EXTRA_DIST = \
	Exports

AM_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	$(QtXml_CFLAGS) \
	$(QtDBus_CFLAGS) \
	$(TRACKER_CFLAGS) \
	$(GCONF_CFLAGS) \
	-DDEFAULT_ACTIONS=\"@sysconfdir@/contentaction/actions.d\"

LIBS += \
	$(QtCore_LIBS) \
	$(QtXml_LIBS) \
	$(QtDBus_LIBS) \
	$(TRACKER_LIBS) \
	$(GCONF_LIBS)

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
	config.cpp \
	highlighter.cpp \
	service.h \
	service.cpp

moc_service_h.cpp: service.h
	$(MOC) -o "$@" "$<"

nodist_libcontentaction_la_SOURCES = moc_service_h.cpp

libcontentaction_la_LDFLAGS = \
	-Wl,--version-script=$(srcdir)/Exports

bin_PROGRAMS = lca-tool

# Force different object file names by using per-target flags.
lca_tool_CPPFLAGS = \
	$(AM_CPPFLAGS)

lca_tool_SOURCES = \
	lca-tool.cpp \
	$(libcontentaction_la_SOURCES)

nodist_lca_tool_SOURCES = moc_service_h.cpp

CLEANFILES = moc_service_h.cpp