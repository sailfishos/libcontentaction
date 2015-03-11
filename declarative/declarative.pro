TEMPLATE = lib
TARGET = contentaction
QT = core qml
CONFIG += plugin

TARGET = $$qtLibraryTarget($$TARGET)
uri = org.nemomobile.contentaction
installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)

INCLUDEPATH += ../src
LIBS += -L../src -lcontentaction5

SOURCES += \
    plugin.cpp \
    declarativecontentaction.cpp

HEADERS += \
    plugin.h \
    declarativecontentaction.h

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir

qmldir.path = $$installPath
target.path = $$installPath

INSTALLS += target qmldir
