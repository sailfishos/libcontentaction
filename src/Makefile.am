SUFFIXES = .cpp .h .moc.cpp

.cpp.moc:
	$(MOC) -o "$@" "$<"
.h.moc.cpp:
	$(MOC) -o "$@" "$<"

EXTRA_DIST = \
	Exports \
	doc.h

AM_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	$(QtXml_CFLAGS) \
	$(QtDBus_CFLAGS) \
	$(TRACKER_CFLAGS) \
	$(GCONF_CFLAGS) \
	$(GIO_CFLAGS) \
	$(DUI_CFLAGS) \
	-DDEFAULT_ACTIONS=\"@sysconfdir@/contentaction/actions.d\" \
	-DDEFAULT_L10N_DIR=\"@datadir@/contentaction/l10n\"

LIBS += \
	$(QtCore_LIBS) \
	$(QtXml_LIBS) \
	$(QtDBus_LIBS) \
	$(TRACKER_LIBS) \
	$(GCONF_LIBS) \
	$(GIO_LIBS) \
	$(DUI_LIBS)

incdir = $(includedir)/contentaction
inc_HEADERS = \
	contentaction.h \
	contentaction-dui.h

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
	service.cpp \
	mime.cpp \
	tracker.cpp \
	dui-highlight.cpp

BUILT_SOURCES = \
	service.moc.cpp \
	dui-highlight.moc
CLEANFILES = \
	service.moc.cpp \
	dui-highlight.moc

nodist_libcontentaction_la_SOURCES = \
	service.moc.cpp

libcontentaction_la_LDFLAGS = \
	-Wl,--version-script=$(srcdir)/Exports

bin_PROGRAMS = lca-tool

# Force different object file names by using per-target flags.
lca_tool_CPPFLAGS = \
	$(AM_CPPFLAGS)

lca_tool_SOURCES = \
	lca-tool.cpp \
	$(libcontentaction_la_SOURCES)

nodist_lca_tool_SOURCES = \
	service.moc.cpp
