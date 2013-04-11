TEMPLATE = aux

include(../../common.pri)

TS_FILES = \
    test_en.ts \
    test_hu.ts \
    test_fi.ts

updateqm.input = TS_FILES
updateqm.output = $$OUT_PWD/${QMAKE_FILE_BASE}.qm
updateqm.commands = lrelease ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE $QMAKE_FILE_IN
updateqm.config += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

l10n_tests.path = $$CONTENTACTION_TESTDIR/test-l10n-data
l10n_tests.files = \
    $$OUT_PWD/test_en.qm \
    $$OUT_PWD/test_hu.qm \
    $$OUT_PWD/test_fi.qm
l10n_tests.CONFIG += no_check_exist
INSTALLS += l10n_tests


