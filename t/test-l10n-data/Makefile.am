cidir = /opt/tests/libcontentaction/test-l10n-data
ci_DATA = \
	test_en.qm \
	test_hu.qm \
	test_fi.qm

EXTRA_DIST = \
	test_en.ts \
	test_hu.ts \
	test_fi.ts

.ts.qm:
	lrelease $< -qm $@

clean-local:
	rm -f *.qm
