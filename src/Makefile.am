AM_CXXFLAGS = \
	$(QtCore_CFLAGS) \
	$(TRACKER_CFLAGS) \
	$(DUI_CFLAGS) \
	-I/usr/include/gallery/service

LIBS += \
	$(QtCore_LIBS) \
	$(TRACKER_LIBS) \
	$(DUI_LIBS) \
	-lgalleryinterface

incdir = $(includedir)/contentaction
inc_HEADERS = \
	contentaction.h

lib_LTLIBRARIES = libcontentaction.la

libcontentaction_la_SOURCES = \
	contentaction.h contentaction.cpp \
	internal.h

bin_PROGRAMS = lca-tool

lca_tool_CXXFLAGS = \
	$(AM_CXXFLAGS)

lca_tool_SOURCES = \
	lca-tool.cpp \
	contentaction.h contentaction.cpp \
	internal.h
