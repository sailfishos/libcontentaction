#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

a=`lca-tool -c an.image`
b='nfo#Image
nfo#Visual
nfo#Media
nie#InformationElement'

test "x$a" = "x$b" || exit 1;
