TARGET = lca-tool
SOURCES += lca-tool.cpp
target.path = /usr/bin
INSTALLS += target

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 gio-unix-2.0
DEFINES += QT_NO_KEYWORDS # make glib happy

LIBS += -L../src
equals(QT_MAJOR_VERSION, 4): LIBS += -lcontentaction
equals(QT_MAJOR_VERSION, 5): LIBS += -lcontentaction5
INCLUDEPATH += ../src
