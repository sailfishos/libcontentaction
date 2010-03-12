#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

strstr() {
    expr "$1" : "$2" >/dev/null;
}

res=$(./hl1 < $srcdir/hlinput.txt)

strstr "$res" ".*61 73 '+44 433 2236' caller" || exit 1
strstr "$res" ".*77 80 '911' caller" || exit 1 
strstr "$res" ".*15 33 'email@address.here' emailer" || exit 1 
strstr "$res" ".*81 104 'ooaoa+foo@motherland.ru' emailer" || exit 1
