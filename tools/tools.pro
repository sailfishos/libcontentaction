TARGET = lca-tool
SOURCES += lca-tool.cpp
target.path = /usr/bin
INSTALLS += target

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 gio-unix-2.0
DEFINES += QT_NO_KEYWORDS # make glib happy

LIBS += -L../src
LIBS += -lcontentaction5
INCLUDEPATH += ../src
