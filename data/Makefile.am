actionsdir = $(datadir)/contentaction
dist_actions_DATA = \
	tracker1.xml \
	highlight1.xml

testdatadir = /opt/tests/libcontentaction/data
dist_testdata_DATA = \
	tracker1.xml \
	highlight1.xml \
	hl-examples.xml

applicationsdir = $(datadir)/applications
dist_applications_DATA = \
	defaults.list

backupconfdir = $(datadir)/backup-framework/applications
dist_backupconf_DATA = contentaction.conf

rfsscriptdir = $(sysconfdir)/osso-rfs-scripts
dist_rfsscript_SCRIPTS = contentaction-rfs.sh

regex.sed: gen-regexps
	./gen-regexps >$@ || exit -1

highlight1.xml: regex.sed highlight1.xml.in
	sed -f regex.sed <highlight1.xml.in >$@
