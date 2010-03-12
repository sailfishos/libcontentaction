#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

tstart $srcdir/gallery.py

testOne() {
        a=`lca-tool --print an.image`;
        b='galleryserviceinterface';
        test "x$a" = "x$b";
        return $?
}

testTwo() {
        a=`lca-tool --print an.image b.image`;
        b='galleryserviceinterface';
        test "x$a" = "x$b";
        return $?
}

set -e;

testOne;
testTwo;

exit 0;
