AM_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	-I$(top_srcdir)/src
LIBS += \
	../src/libcontentaction.la

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
	test-defaults.py

noinst_SCRIPTS = \
	start-sandbox.sh \
	stop-sandbox.sh \
	test-highlight.sh

citest_DATA = \
	tests.xml \
	service.map \
	data.contact \
	data.web \
	data.image

check_PROGRAMS = \
	linktest \
	hl1

hl1_SOURCES = \
	hl1.cpp
linktest_SOURCES = \
	linktest.cpp
linktest_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	-I$(top_srcdir)/src
linktest_LDADD = \
	../src/libcontentaction.la

../src/libcontentaction.la: FORCE
	$(MAKE) -C ../src libcontentaction.la

.PHONY: FORCE

EXTRA_DIST = \
	$(citest_DATA) \
	$(citest_SCRIPTS) \
	$(noinst_SCRIPTS) \
	hlinput.txt

# NOTE: we depend on automake executing $(TESTS) in order.  Should this
# change, we fail.
TESTS = \
	start-sandbox.sh \
	linktest \
	test-actions.sh \
	test-classes.sh \
	test-invoking.py \
	test-defaults.py \
	test-highlight.sh \
	stop-sandbox.sh

clean-local:
	rm -f *.pyc;
