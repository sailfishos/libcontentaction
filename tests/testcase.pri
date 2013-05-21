QT += testlib
INCLUDEPATH += ../src
LIBS += -L../src
equals(QT_MAJOR_VERSION, 4): LIBS += -lcontentaction
equals(QT_MAJOR_VERSION, 5): LIBS += -lcontentaction5

include(../common.pri)

target.path = $$CONTENTACTION_TESTDIR
INSTALLS += target
