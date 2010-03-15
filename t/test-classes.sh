#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

a=`lca-tool -c an.image`
strstr "$a" '.*nfo-Image' || exit 1;
