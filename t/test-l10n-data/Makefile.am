cidir = $(datarootdir)/libcontentaction-tests/test-l10n-data
ci_DATA = \
	test_en_US.qm \
	test_hu_HU.qm \
	test_fi_FI.qm

dist_ci_DATA = \
	applications/

EXTRA_DIST = \
	test_en_US.ts \
	test_hu_HU.ts \
	test_fi_FI.ts

.ts.qm:
	lrelease $< -qm $@

clean-local:
	rm -f *.qm
