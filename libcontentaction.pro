TEMPLATE = subdirs
SUBDIRS += src \
           data \
           tests

equals(QT_MAJOR_VERSION, 5): SUBDIRS += tools

tests.depends = src
tools.depends = src

# TODO: fix tests, doc
