#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

export CONTENTACTION_ACTIONS=$srcdir/test-l10n-data
export CONTENTACTION_L10N_DIR=test-l10n-data

export LANG=en_US
a=`l10ntest an.image`
b='lack of imagination
upload to album
show in gallery'

test "x$a" = "x$b" || exit 1;

export LANG=hu_HU
a=`l10ntest an.image`
b='a kepzelet hianya
feltoltes az albumba
mutasd a galeriaban'

test "x$a" = "x$b" || exit 1;

export LANG=xx_YY
a=`l10ntest an.image 2>/dev/null`
b='some.other.action
upload.to.album
com.nokia.imageviewer.show'

test "x$a" = "x$b" || exit 1;
