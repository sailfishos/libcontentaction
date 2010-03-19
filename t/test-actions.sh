#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

tstart $srcdir/gallery.py

a=`lca-tool --tracker --print an.image`;
strstr "$a" '.*galleryserviceinterface' || exit 1;
a=`lca-tool --tracker --print an.image b.image`;
strstr "$a" '.*galleryserviceinterface' || exit 1;

exit 0;
