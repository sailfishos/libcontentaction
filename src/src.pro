TEMPLATE = lib
TARGET = contentaction

# yes, 0.0.75. libcontentaction currently has a SONAME of 0, even though the
# version numbering is 0.1. god only knows why. fix this in the future, some
# day?
VERSION = 0.0.75

QT = core xml dbus
CONFIG += link_pkgconfig hide_symbols
PKGCONFIG += mlite gio-2.0 gio-unix-2.0
CONFIG += mobility
MOBILITY += systeminfo

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

DEFINES += DEFAULT_ACTIONS=\\\"\"/usr/share/contentaction\"\\\"
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
    tracker.cpp \
    highlighter.cpp \
    highlight.cpp \
    config.cpp \
    contentinfo.cpp

include.path = /usr/include/contentaction
include.files = contentaction.h \
                contentinfo.h
INSTALLS += include

PCFILE=contentaction-0.1.pc

# handle .pc file
system(cp $${PCFILE}.in $$PCFILE)
system(sed -i "s/\\@VERSION\\@/$$VERSION/g" $$PCFILE)

pcfiles.files = $$PCFILE
pcfiles.path = $$[QT_INSTALL_LIBS]/pkgconfig
INSTALLS += pcfiles
