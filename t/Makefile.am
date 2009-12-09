BUILT_SOURCES = \
	env.sh \
	env.py

citestdir = $(datarootdir)/libcontentaction-tests
citest_SCRIPTS = \
	sandbox.sh testlib.sh \
	cltool.py \
	gallery.py \
	servicemapper.py \
	start-sandbox.sh \
	test-actions.sh \
	test-classes.sh \
	test-invoking.py \
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

../src/libcontentaction.la: FORCE
	$(MAKE) -C ../src libcontentaction.la

.PHONY: FORCE

EXTRA_DIST = \
	$(citest_DATA) \
	$(citest_SCRIPTS)

# NOTE: we depend on automake executing $(TESTS) in order.  Should this
# change, we fail.
TESTS = \
	start-sandbox.sh \
	linktest \
	test-actions.sh \
	test-classes.sh \
	test-invoking.py \
	stop-sandbox.sh

clean-local:
	rm -f *.pyc;
