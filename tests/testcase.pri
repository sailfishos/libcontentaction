QT += testlib
INCLUDEPATH += ../src
LIBS += -L../src
LIBS += -lcontentaction5

include(../common.pri)

target.path = $$CONTENTACTION_TESTDIR
INSTALLS += target
