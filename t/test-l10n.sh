#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

setlocale() {
    export LANG="$1";
    gconftool-2 -t string -s /Dui/i18n/Language "$LANG";
}

abspath() {
    cd -P "$1" && pwd -P;
}

strstr() {
    expr "$1" : "$2" >/dev/null;
}

export CONTENTACTION_ACTIONS=$srcdir/test-l10n-data
export CONTENTACTION_L10N_PATH=test-l10n-data
export XDG_DATA_HOME=$srcdir

setlocale en_US
a=$(l10ntest an.image 2>/dev/null)
b='lack of imagination
upload to album
show in gallery'

test "x$a" = "x$b" || exit 1;

plaintext="$(abspath $srcdir)/plaintext"

a=$(lca-tool --l10n -f "file://$plaintext" 2>/dev/null)
strstr "$a" ".*englishexec" || exit 1;
strstr "$a" ".*englishmimeopen" || exit 1;
strstr "$a" ".*lack of imagination" || exit 1;

setlocale hu_HU
a=$(l10ntest an.image 2>/dev/null)
b='a kepzelet hianya
feltoltes az albumba
mutasd a galeriaban'

test "x$a" = "x$b" || exit 1;

a=$(lca-tool --l10n -f "file://$plaintext" 2>/dev/null)
strstr "$a" ".*uberexec" || exit 1;
strstr "$a" ".*ubermimeopen" || exit 1;
strstr "$a" ".*a kepzelet hianya" || exit 1;

setlocale xx_YY
a=$(l10ntest an.image 2>/dev/null)
b='some.other.action
upload.to.album
com.nokia.imageviewer.show'

test "x$a" = "x$b" || exit 1;

a=$(lca-tool --l10n -f "file://$plaintext" 2>/dev/null)
strstr "$a" ".*uberexec" || exit 1;
strstr "$a" ".*ubermimeopen" || exit 1;
strstr "$a" ".*!! uberdui" || exit 1;

exit 0;
