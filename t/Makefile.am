BUILT_SOURCES = \
	env.sh \
	env.py

citestdir = $(datarootdir)/libcontentaction-tests
citest_SCRIPTS = \
	sandbox.sh testlib.sh \
	cltool.py \
	gallery.py \
	servicemapper.py \
	$(TESTS)

citest_DATA = \
	tests.xml \
	service.map \
	data.contact \
	data.web \
	data.image

EXTRA_DIST = \
	$(citest_DATA) \
	$(citest_SCRIPTS)

# NOTE: we depend on automake executing $(TESTS) in order.  Should this
# change, we fail.
TESTS = \
	start-sandbox.sh \
	test-actions.sh \
	test-classes.sh \
	test-invoking.py \
	stop-sandbox.sh

TESTS_ENVIRONMENT = eval `cat env.sh`

clean-local:
	rm -f *.pyc;
