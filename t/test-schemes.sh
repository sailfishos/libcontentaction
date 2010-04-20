#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh
. $srcdir/testlib.sh

tstart $srcdir/gallery.py

a=$(lca-tool --scheme --print mailto:foo@bar)
strstr "$a" '.*emailer' || exit 1
# this tests the fallback to one of the existing actions if default not set
a=$(lca-tool --scheme --print mailto:foo@bar)
strstr "$a" '.*emailer' || exit 1
exit 0
