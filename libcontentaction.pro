TEMPLATE = subdirs
SUBDIRS += src \
           data \
           tests \
           tools \
           declarative

tests.depends = src
tools.depends = src
declarative.depends = src

# TODO: fix tests, doc
