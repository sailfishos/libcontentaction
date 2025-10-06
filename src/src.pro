TEMPLATE = lib
TARGET = contentaction$${QT_MAJOR_VERSION}

include(../common.pri)

QT = core xml dbus
CONFIG += link_pkgconfig hide_symbols create_pc create_prl no_install_prl
CONFIG -= link_prl
PKGCONFIG += gio-2.0 gio-unix-2.0
PKGCONFIG += mlite$${QT_MAJOR_VERSION}

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

DEFINES += DEFAULT_ACTIONS=\\\"\"$$CONTENTACTION_DATADIR\"\\\"
DEFINES += LCA_BUILD
DEFINES += QT_NO_KEYWORDS # make glib happy

HEADERS += \
    internal.h \
    contentaction.h \
    service.h \
    contentinfo.h

SOURCES += \
    contentaction.cpp \
    service.cpp \
    dbus.cpp \
    exec.cpp \
    mime.cpp \
    highlighter.cpp \
    highlight.cpp \
    config.cpp \
    contentinfo.cpp

include.path = $$CONTENTACTION_INCLUDEDIR
include.files = contentaction.h \
                contentinfo.h
INSTALLS += include

QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = Library for associating content with actions
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$include.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_REQUIRES = Qt$${QT_MAJOR_VERSION}Core
QMAKE_PKGCONFIG_VERSION = $$VERSION

OTHER_FILES += doc.h
