SUBDIRS = \
	applications \
	test-l10n-data
CLEANFILES =
AM_CPPFLAGS = \
	$(QtCore_CFLAGS) \
	$(QtTest_CFLAGS) \
	-I$(top_srcdir)/src
LIBS += \
	../src/libcontentaction.la \
	$(QtTest_LIBS) \
	-lstdc++

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
	uberprogram.py \
	../data/regextest2.py \
	../data/gen-regexps \
	test-actions.sh \
	test-mimetypes.sh \
	test-invoking.py \
	test-defaults.py \
	test-mimes.py \
	test-servicefw-signals.py \
	test-desktop-launching.py \
	test-l10n.sh \
	test-fixed-params.py \
	test-schemes.sh \
	test-special-chars.py \
	test-invalid-defaults.py \
	test-regexps.py \
	test-highlight.sh

citest_PROGRAMS = \
	test-info \
	test-findhighlights \
	test-action \
	test-mimedefaults

bin_SCRIPTS = \
	lca-cita-test

noinst_SCRIPTS = \
	start-sandbox.sh \
	stop-sandbox.sh \
	test-highlight.sh

citest_DATA = \
	tests.xml \
	service.map \
	testdata.ttl \
	plaintext \
	empty.pdf \
	launchme.desktop \
	test-image.png \
	hlinput.txt

check_PROGRAMS = \
	linktest \
	test-info \
	test-findhighlights \
	test-action \
	test-mimedefaults

BUILT_SOURCES += test-action.moc test-info.moc

testhelperdir = $(libdir)/libcontentaction-tests
testhelper_PROGRAMS = \
	servicetest

linktest_SOURCES = linktest.cpp
linktest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(GIO_CFLAGS) \
	$(MT_CFLAGS)
linktest_LDADD = \
	$(MT_LIBS)

servicetest_SOURCES = servicelistener.cpp

test_findhighlights_SOURCES = \
	test-findhighlights.cpp \
	test-findhighlights.moc

test_findhighlights_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(QtTest_CFLAGS)

test_findhighlights_LDADD = \
	$(QtTest_LIBS)

test_mimedefaults_SOURCES = \
	test-mimedefaults.cpp \
	test-mimedefaults.moc

test_mimedefaults_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(QtTest_CFLAGS)

test_mimedefaults_LDADD = \
	$(QtTest_LIBS)

test_info_SOURCES = \
	test-info.cpp \
	test-info.moc

test_action_SOURCES = \
	test-action.cpp \
	test-action.moc

BUILT_SOURCES += servicelistener.moc \
	test-findhighlights.moc \
	test-mimedefaults.moc

../src/libcontentaction.la: FORCE
	$(MAKE) -C ../src libcontentaction.la

.PHONY: FORCE

EXTRA_DIST = \
	$(citest_DATA) \
	$(citest_SCRIPTS) \
	$(noinst_SCRIPTS) \
	hlinput.txt \
	plaintext \
	empty.pdf \
	launchme.desktop \
	lca-cita-test

# NOTE: we depend on automake executing $(TESTS) in order.  Should this
# change, we fail.
TESTS = \
	start-sandbox.sh \
	linktest \
	test-info \
	test-findhighlights \
	test-action \
	test-mimedefaults \
	test-actions.sh \
	test-mimetypes.sh \
	test-invoking.py \
	test-defaults.py \
	test-servicefw-signals.py \
	test-highlight.sh \
	test-mimes.py \
	test-desktop-launching.py \
	test-l10n.sh \
	test-fixed-params.py \
	test-schemes.sh \
	test-special-chars.py \
	test-invalid-defaults.py \
	test-regexps.py \
	stop-sandbox.sh \
	../data/gen-regexps

clean-local:
	rm -f *.py?;

CLEANFILES += servicelistener.moc
