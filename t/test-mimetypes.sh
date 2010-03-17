#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

a=`lca-tool -m an.image`
strstr "$a" '.*x-maemo-nepomuk/image' || exit 1;
