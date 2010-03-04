#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

setlocale() {
    export LANG="$1";
    gconftool-2 -t string -s /Dui/i18n/Language "$LANG";
}

export CONTENTACTION_ACTIONS=$srcdir/test-l10n-data
export CONTENTACTION_L10N_DIR=test-l10n-data

setlocale en_US
a=`l10ntest an.image`
b='lack of imagination
upload to album
show in gallery'

test "x$a" = "x$b" || exit 1;

setlocale hu_HU
a=`l10ntest an.image`
b='a kepzelet hianya
feltoltes az albumba
mutasd a galeriaban'

test "x$a" = "x$b" || exit 1;

setlocale xx_YY
a=`l10ntest an.image 2>/dev/null`
b='some.other.action
upload.to.album
com.nokia.imageviewer.show'

test "x$a" = "x$b" || exit 1;
