#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

a=`lca-tool -c an.image`
b='http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image
http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Visual
http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Media
http://www.semanticdesktop.org/ontologies/2007/01/19/nie#InformationElement
http://www.w3.org/2000/01/rdf-schema#Resource'

test "x$a" = "x$b" || exit 1;
