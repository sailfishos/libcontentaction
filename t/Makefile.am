SUBDIRS = applications
CLEANFILES =
AM_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	-I$(top_srcdir)/src
LIBS += \
	../src/libcontentaction.la

.cpp.moc:
	$(MOC) -o "$@" "$<"

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
	test-mimes.py \
	test-servicefw-signals.py

noinst_SCRIPTS = \
	start-sandbox.sh \
	stop-sandbox.sh \
	test-highlight.sh

citest_DATA = \
	tests.xml \
	service.map \
	data.contact \
	data.web \
	data.image \
	plaintext

check_PROGRAMS = \
	linktest \
	hl1

if WITH_DUI

noinst_PROGRAMS = hldemo
hldemo_SOURCES = hldemo.cpp
hldemo_CPPFLAGS = \
	$(DUI_CFLAGS) \
	-I$(top_srcdir)/src
hldemo_LDADD = \
	$(DUI_LIBS) \
	../src/libcontentaction.la \
	../src/libcontentaction-dui.la
BUILT_SOURCES += hldemo.moc
CLEANFILES += hldemo.moc

endif

hl1_SOURCES = \
	hl1.cpp
linktest_SOURCES = \
	linktest.cpp
linktest_LDADD = \
	../src/libcontentaction.la

testhelperdir = $(libdir)/libcontentaction-tests
testhelper_PROGRAMS = servicetest

servicetest_SOURCES = \
	servicelistener.cpp

BUILT_SOURCES += servicelistener.moc

servicetest_LDADD = \
	../src/libcontentaction.la

../src/libcontentaction.la: FORCE
	$(MAKE) -C ../src libcontentaction.la

.PHONY: FORCE

EXTRA_DIST = \
	$(citest_DATA) \
	$(citest_SCRIPTS) \
	$(noinst_SCRIPTS) \
	hlinput.txt \
	plaintext

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
	test-highlight.sh \
	test-mimes.py \
	stop-sandbox.sh

clean-local:
	rm -f *.pyc;

CLEANFILES += servicelistener.moc
