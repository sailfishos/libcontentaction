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

strstr "$res" ".*'1234567' caller" || exit 2
strstr "$res" ".*'(303)499-7111' caller" || exit 2
strstr "$res" ".*'+13034997111' caller" || exit 2
strstr "$res" ".*'303-499-7111' caller" || exit 2
strstr "$res" ".*'+1(303)499-7111' caller" || exit 2
strstr "$res" ".*'303.499.7111' caller" || exit 2
strstr "$res" ".*'(3034997111)' caller" || exit 2
strstr "$res" ".*'+48 123.12-3 123' caller" || exit 2
strstr "$res" ".*'+481234p12345' caller" || exit 2
strstr "$res" ".*'+481234#12345' caller" || exit 2

strstr "$res" ".*'http://us.m.yahoo.com' browser" || exit 3
strstr "$res" ".*'http://www.google.com/m' browser" || exit 3
strstr "$res" ".*'www.google.com/m' browser" || exit 3
strstr "$res" ".*'http://us.m.yahoo.com' browser" || exit 3

strstr "$res" ".*'john.doe@att.com' emailer" || exit 4
strstr "$res" ".*'<mailto:john_doe2@att.com>' emailer" || exit 4

# invalid and almost-invalid cases
#strstr "$res" ".*'333222111444' caller" && exit 5
strstr "$res" ".*'+((((' caller" && exit 5
strstr "$res" ".*'+358+7777' caller" && exit 5

strstr "$res" ".*'http://http://double.http.com' browser" && exit 6
strstr "$res" ".*'http://double.http.com' browser" || exit 6

exit 0
