TEMPLATE = aux

include(../common.pri)

actions.path = $$CONTENTACTION_DATADIR
actions.files = \
    tracker1.xml \
    highlight1.xml
INSTALLS += actions

testdata.path = $$CONTENTACTION_TESTDIR/data
testdata.files = \
    tracker1.xml \
    highlight1.xml \
    hl-examples.xml
INSTALLS += testdata

applications.path = /usr/share/applications
applications.files = defaults.list
INSTALLS += applications

backupconf.path = /usr/share/backup-framework/applications
backupconf.files = contentaction.conf
INSTALLS += backupconf

rfs.path = /etc/osso-rfs-scripts
rfs.files = contentaction-rfs.sh
INSTALLS += rfs


XML_FILES = highlight1.xml.in

genreg.input = XML_FILES
genreg.output = highlight1.xml
genreg.commands = $$PWD/gen-regexps > regex.sed && sed -f regex.sed < ${QMAKE_FILE_IN} > ${QMAKE_FILE_OUT}
genreg.name = genreg
genreg.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += genreg

highlight1.path = $$CONTENTACTION_DATADIR
highlight1.files = highlight1.xml
highlight1.CONFIG += no_check_exist
INSTALLS += highlight1
