#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

setlocale() {
    export LANG="$1";
    gconftool-2 -t string -s /meegotouch/i18n/language "$LANG";
}

#lca-tool adds these as translation paths
export CONTENTACTION_L10N_PATH=test-l10n-data

export XDG_DATA_HOME=$srcdir

# translation of tracker uri actions
setlocale en_US
a=$(lca-tool --l10n --tracker --print an.image 2>/dev/null)

strstr "$a" ".*lack of imagination" || exit 1;
strstr "$a" ".*upload to album" || exit 1;
strstr "$a" ".*show in gallery" || exit 1;

# translation for file actions
plaintext="$(abspath $srcdir)/plaintext"

a=$(lca-tool --l10n --file --print "file://$plaintext" 2>/dev/null)

strstr "$a" ".*englishexec" || exit 1;
strstr "$a" ".*englishmimeopen" || exit 1;
strstr "$a" ".*lack of imagination" || exit 1;

# translations for tracker uri actions
setlocale hu_HU
a=$(lca-tool --l10n --tracker --print an.image 2>/dev/null)

strstr "$a" ".*a kepzelet hianya" || exit 1;
strstr "$a" ".*feltoltes az albumba" || exit 1;
strstr "$a" ".*mutasd a galeriaban" || exit 1;

a=$(lca-tool --l10n --file --print "file://$plaintext" 2>/dev/null)

strstr "$a" ".*uberexec" || exit 1;
strstr "$a" ".*ubermimeopen" || exit 1;
strstr "$a" ".*a kepzelet hianya" || exit 1;

# a locale for which we don't have translations
setlocale xx_YY
a=$(lca-tool --l10n --tracker --print an.image 2>/dev/null)
b='some.other.action
upload.to.album
com.nokia.imageviewer.show'

strstr "$a" ".*!! othername" || exit 1;
strstr "$a" ".*!! uploadname" || exit 1;
strstr "$a" ".*!! showname" || exit 1;

a=$(lca-tool --l10n --file --print "file://$plaintext" 2>/dev/null)
strstr "$a" ".*uberexec" || exit 1;
strstr "$a" ".*ubermimeopen" || exit 1;
strstr "$a" ".*!! ubermeego" || exit 1;

exit 0;
