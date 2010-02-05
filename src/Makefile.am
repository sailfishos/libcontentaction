BUILT_SOURCES=
CLEANFILES=

SUFFIXES = .cpp .h .moc.cpp

.cpp.moc:
	$(MOC) -o "$@" "$<"
.h.moc.cpp:
	$(MOC) -o "$@" "$<"

EXTRA_DIST = \
	Exports \
	Exports.dui

AM_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	$(QtXml_CFLAGS) \
	$(QtDBus_CFLAGS) \
	$(TRACKER_CFLAGS) \
	$(GCONF_CFLAGS) \
	$(GIO_CFLAGS) \
	-DDEFAULT_ACTIONS=\"@sysconfdir@/contentaction/actions.d\"

LIBS += \
	$(QtCore_LIBS) \
	$(QtXml_LIBS) \
	$(QtDBus_LIBS) \
	$(TRACKER_LIBS) \
	$(GCONF_LIBS) \
	$(GIO_LIBS)

incdir = $(includedir)/contentaction
inc_HEADERS = \
	contentaction.h

lib_LTLIBRARIES = libcontentaction.la

if WITH_DUI

lib_LTLIBRARIES += libcontentaction-dui.la

libcontentaction_dui_la_SOURCES = \
	dui-highlight.cpp

libcontentaction_dui_la_CPPFLAGS = \
	$(DUI_CFLAGS)

libcontentaction_dui_la_LDFLAGS = \
	-Wl,--version-script=$(srcdir)/Exports.dui

libcontentaction_dui_la_LIBADD = \
	$(DUI_LIBS) \
	libcontentaction.la

libcontentaction_dui_la_DEPENDENCIES = \
	Exports.dui

inc_HEADERS += \
	contentaction-dui.h

BUILT_SOURCES += dui-highlight.moc
CLEANFILES += dui-highlight.moc

endif

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

BUILT_SOURCES += service.moc.cpp
CLEANFILES += service.moc.cpp

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
