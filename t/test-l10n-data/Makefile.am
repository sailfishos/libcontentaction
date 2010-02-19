cidir = $(datarootdir)/libcontentaction-tests/test-l10n-data
ci_DATA = \
	test_en_US.qm \
	test_hu_HU.qm

dist_ci_DATA = \
	test-l10n-actions.xml

EXTRA_DIST = \
	test_en_US.ts \
	test_hu_HU.ts

.ts.qm:
	lrelease $< -qm $@

clean-local:
	rm -f *.qm
