TEMPLATE = subdirs
SUBDIRS += src \
           data \
           tools \
           tests

tests.depends = src
tools.depends = src

# TODO: fix tests, doc
