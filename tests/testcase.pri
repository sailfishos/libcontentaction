QT = core testlib
INCLUDEPATH += ../src

LIBS += -L../src
LIBS += -lcontentaction$${QT_MAJOR_VERSION}

include(../common.pri)

target.path = $$CONTENTACTION_TESTDIR
INSTALLS += target
