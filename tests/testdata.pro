TEMPLATE = aux

testdata.files = \
    tests.xml \
    service.map \
    testdata.ttl \
    plaintext \
    empty.pdf \
    launchme.desktop \
    test-image.png \
    hlinput.txt
testdata.path = /opt/tests/libcontentaction
INSTALLS += testdata

testscripts.files = \
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
testscripts.path = /opt/tests/libcontentaction
INSTALLS += testscripts

bin.files = lca-cita-test
bin.path = /opt/tests/libcontentaction/bin
INSTALLS += bin


