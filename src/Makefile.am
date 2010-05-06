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
	$(GIO_CFLAGS) \
	$(MT_CFLAGS) \
	-DDEFAULT_ACTIONS=\"@datadir@/contentaction\" \
	-DDEFAULT_L10N_DIR=\"@datadir@/contentaction/l10n\"

LIBS += \
	$(QtCore_LIBS) \
	$(QtXml_LIBS) \
	$(QtDBus_LIBS) \
	$(GIO_LIBS) \
	$(MT_LIBS)

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
	service.h \
	service.cpp \
	dbus.cpp \
	exec.cpp \
	mime.cpp \
	tracker.cpp \
	highlighter.cpp \
	highlight.cpp \
	config.cpp

BUILT_SOURCES = \
	service.moc.cpp \
	highlight.moc
CLEANFILES = \
	service.moc.cpp \
	highlight.moc

nodist_libcontentaction_la_SOURCES = \
	service.moc.cpp

libcontentaction_la_LDFLAGS = \
	-Wl,--as-needed \
	-Wl,--version-script=$(srcdir)/Exports

bin_PROGRAMS = lca-tool

lca_tool_SOURCES = \
	lca-tool.cpp
lca_tool_LDADD = \
	libcontentaction.la

