SUFFIXES = .cpp .h .moc.cpp

.cpp.moc:
	$(MOC) -o "$@" "$<"
.h.moc.cpp:
	$(MOC) -o "$@" "$<"

EXTRA_DIST = \
	doc.h

# check for coverage
if COVERAGE
COV_CXXFLAGS=-ftest-coverage -fprofile-arcs -fno-elide-constructors
COV_LIBS=-lgcov
else
COV_CXXFLAGS=
COV_LIBS=
endif

AM_CPPFLAGS = \
	$(COV_CXXFLAGS) \
	$(QtCore_CFLAGS) \
	$(QtXml_CFLAGS) \
	$(QtDBus_CFLAGS) \
	$(QtSystemInfo_CFLAGS) \
	$(GIO_CFLAGS) \
	$(MT_CFLAGS) \
	-DDEFAULT_ACTIONS=\"@datadir@/contentaction\" \
	-DDEFAULT_L10N_DIR=\"@datadir@/contentaction/l10n\" \
	-DLCA_BUILD

LIBS += \
	$(COV_LIBS) \
	$(QtCore_LIBS) \
	$(QtXml_LIBS) \
	$(QtDBus_LIBS) \
	$(QtSystemInfo_LIBS) \
	$(GIO_LIBS) \
	$(MT_LIBS)

incdir = $(includedir)/contentaction
inc_HEADERS = \
	contentaction.h \
	contentinfo.h

lib_LTLIBRARIES = libcontentaction.la

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
	config.cpp \
	contentinfo.h \
	contentinfo.cpp

BUILT_SOURCES = \
	service.moc.cpp \
	highlight.moc
CLEANFILES = \
	service.moc.cpp \
	highlight.moc

nodist_libcontentaction_la_SOURCES = \
	service.moc.cpp

libcontentaction_la_CXXFLAGS = \
	-fvisibility=hidden

libcontentaction_la_LDFLAGS = \
	-Wl,--as-needed

bin_PROGRAMS = lca-tool

lca_tool_SOURCES = \
	lca-tool.cpp
lca_tool_LDADD = \
	libcontentaction.la

# coverage stuff
coverage:covoptioncheck
	mkdir -p ../coverage && \
	lcov -d . --capture --output-file ../coverage/all.cov -b . && \
	cd ../coverage && \
	lcov -e all.cov '*/src/*.cpp' -e all.cov '*/src/*.h' -o src.cov && \
	genhtml -o . src.cov	

include $(top_srcdir)/am/covoptioncheck.am
