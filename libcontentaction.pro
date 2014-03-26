TEMPLATE = subdirs
SUBDIRS += src \
           data \
           tests \
           tools

tests.depends = src
tools.depends = src

# TODO: fix tests, doc
