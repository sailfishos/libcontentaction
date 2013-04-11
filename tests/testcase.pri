QT += testlib
INCLUDEPATH += ../src
LIBS += -L../src
equals(QT_MAJOR_VERSION, 4): LIBS += -lcontentaction
equals(QT_MAJOR_VERSION, 5): LIBS += -lcontentaction5

include(../common.pri)

target.path = $$CONTENTACTION_TESTDIR
INSTALLS += target

DEFINES += srcdir=\\\"\"$$PWD\"\\\"
DEFINES += top_srcdir=\\\"\"$$PWD/..\"\\\"
DEFINES += CONTENTACTION_ACTIONS=\\\"\"$$PWD/../data\"\\\"
DEFINES += XDG_DATA_HOME=\\\"\"$$PWD\"\\\"
DEFINES += MIME_TEST_DIR=\\\"\"$$PWD\"\\\"

