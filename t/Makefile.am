BUILT_SOURCES = \
	env.sh \
	env.py

citestdir = $(datarootdir)/libcontentaction-tests
citest_SCRIPTS = \
	sandbox.sh testlib.sh \
	cltool.py \
	gallery.py \
	servicemapper.py \
	test-actions.sh \
	test-classes.sh \
	test-invoking.py \
	test-defaults.py \
	test-servicefw-signals.py

noinst_SCRIPTS = \
	start-sandbox.sh \
	stop-sandbox.sh

citest_DATA = \
	tests.xml \
	service.map \
	data.contact \
	data.web \
	data.image

check_PROGRAMS = \
	linktest
linktest_SOURCES = \
	linktest.cpp
linktest_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	-I$(top_srcdir)/src
linktest_LDADD = \
	../src/libcontentaction.la

noinst_PROGRAMS = servicetest

servicetest_SOURCES = \
	servicelistener.cpp \
	servicelistenerdummy.h
servicetest_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	-I$(top_srcdir)/src
servicetest_LDADD = \
	../src/libcontentaction.la

QT_TOMOC = servicelistenerdummy.h
nodist_servicetest_SOURCES = mocs.cpp

../src/libcontentaction.la: FORCE
	$(MAKE) -C ../src libcontentaction.la

.PHONY: FORCE

EXTRA_DIST = \
	$(citest_DATA) \
	$(citest_SCRIPTS) \
	$(noinst_SCRIPTS)

# NOTE: we depend on automake executing $(TESTS) in order.  Should this
# change, we fail.
TESTS = \
	start-sandbox.sh \
	linktest \
	test-actions.sh \
	test-classes.sh \
	test-invoking.py \
	test-defaults.py \
	test-servicefw-signals.py \
	stop-sandbox.sh

clean-local:
	rm -f *.pyc;

include $(top_srcdir)/am/qt.am