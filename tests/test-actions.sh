#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh
. $srcdir/testlib.sh

tstart $srcdir/gallery.py

a=$(lca-tool --tracker --print an.image)
strstr "$a" '.*galleryserviceinterface' || exit 1
strstr "$a" '.*plainimageviewer' || exit 1

a=$(lca-tool --tracker --print an.image b.image)
strstr "$a" '.*galleryserviceinterface' || exit 1
strstr "$a" '.*plainimageviewer' || exit 1

a=$(lca-tool --tracker --print a.music)
strstr "$a" '.*plainmusicplayer' || exit 1

a=$(lca-tool --tracker --print a.music b.music)
strstr "$a" '.*plainmusicplayer' || exit 1

echo test > test.html
atexit rm -f test.html
uri="file://$(abspath .)/test.html"
a=$(lca-tool --file --print $uri)
strstr "$a" '.*fixedparams' || exit 1

appuri="file://$(abspath $srcdir)/applications/emailer.desktop"
tracker-sparql -u -q "INSERT { <a.softwareapp> nie:url \"$appuri\" . }"
a=$(lca-tool --tracker --print a.softwareapp)
strstr "$a" '.*emailer' || exit 1
a=$(lca-tool --tracker --printdefault a.softwareapp)
test "$a" = "emailer" || exit 1

exit 0
