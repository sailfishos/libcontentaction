#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh
. $srcdir/testlib.sh

tstart $srcdir/gallery.py

a=$(lca-tool --tracker --print an.image)
strstr "$a" '.*galleryserviceinterface' || exit 1
a=$(lca-tool --tracker --print an.image b.image)
strstr "$a" '.*galleryserviceinterface' || exit 1
echo test > test.html
atexit rm -f test.html
uri="file://$(abspath .)/test.html"
a=$(lca-tool --file --print $uri)
strstr "$a" '.*fixedparams' || exit 1
exit 0
