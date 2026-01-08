TEMPLATE = aux

include(../common.pri)

testdata.files = \
    service.map \
    testdata.ttl \
    plaintext \
    empty.pdf \
    launchme.desktop \
    test-image.png \
    hlinput.txt
testdata.path = $$CONTENTACTION_TESTDIR
INSTALLS += testdata

testscripts.files = \
    testlib.sh \
    cltool.py \
    gallery.py \
    servicemapper.py \
    uberprogram.py \
    ../data/regextest2.py \
    ../data/gen-regexps \
    test-mimetypes.sh \
    test-defaults.py \
    test-mimes.py \
    test-desktop-launching.py \
    test-l10n.sh \
    test-fixed-params.py \
    test-schemes.sh \
    test-special-chars.py \
    test-regexps.py \
    test-highlight.sh
testscripts.path = $$CONTENTACTION_TESTDIR
INSTALLS += testscripts

unix{
    bin.target = lca-cita-test
    bin.files = lca-cita-test
    bin.path = $$CONTENTACTION_TESTDIR/bin
    bin.CONFIG = no_check_exist executable
    bin.commands = sed \'s%@PATH@%$${CONTENTACTION_TESTDIR}%\' $$PWD/lca-cita-test.in > $$PWD/lca-cita-test
    QMAKE_DISTCLEAN += lca-cita-test
    INSTALLS += bin

    tests_xml.target = tests.xml
    tests_xml.files = tests.xml
    tests_xml.path = $${CONTENTACTION_TESTDIR}
    tests_xml.CONFIG = no_check_exist
    tests_xml.commands = sed \'s%@PATH@%$${CONTENTACTION_TESTDIR}%\' $$PWD/tests.xml.in > $$PWD/tests.xml
    QMAKE_DISTCLEAN += tests.xml
    INSTALLS += tests_xml
}

