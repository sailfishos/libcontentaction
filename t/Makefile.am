AM_CPPFLAGS = \
	$(TRACKER_CFLAGS) \
	$(QtCore_CFLAGS) \
	-I$(top_srcdir)/src

LIBS += \
	$(TRACKER_LIBS) \
	$(QtCore_LIBS) \
	$(top_builddir)/src/libcontentaction.la

EXTRA_DIST = \
	data.web

noinst_PROGRAMS = test-classesof
test_classesof_SOURCES = test-classesof.cpp
