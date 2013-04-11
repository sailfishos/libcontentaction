equals(QT_MAJOR_VERSION, 4): TARGET = lca-tool
equals(QT_MAJOR_VERSION, 5): TARGET = lca-tool5
SOURCES += lca-tool.cpp
target.path = /usr/bin
INSTALLS += target

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 gio-unix-2.0
equals(QT_MAJOR_VERSION, 4): PKGCONFIG += mlocale
equals(QT_MAJOR_VERSION, 5): PKGCONFIG += mlocale5
DEFINES += QT_NO_KEYWORDS # make glib happy

LIBS += -L../src -lcontentaction
INCLUDEPATH += ../src
