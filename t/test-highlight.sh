#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

strstr() {
    expr "$1" : "$2" >/dev/null;
}

res=$(lca-tool --highlight < $srcdir/hlinput.txt)

strstr "$res" ".*61 73 '+44 433 2236' caller" || exit 1
strstr "$res" ".*77 80 '911' caller" || exit 1
strstr "$res" ".*15 33 'email@address.here' emailer" || exit 1 
strstr "$res" ".*81 104 'ooaoa+foo@motherland.ru' emailer" || exit 1

# phone numbers

strstr "$res" ".*'1234567' caller" || exit 2
strstr "$res" ".*'(303)499-7111' caller" || exit 2
strstr "$res" ".*'+13034997111' caller" || exit 2
strstr "$res" ".*'303-499-7111' caller" || exit 2
strstr "$res" ".*'+1(303)499-7111' caller" || exit 2
strstr "$res" ".*'303.499.7111' caller" || exit 2
strstr "$res" ".*'3034997111' caller" || exit 2
strstr "$res" ".*'+48 123.12-3 123' caller" || exit 2
strstr "$res" ".*'+481234p12345' caller" || exit 2
strstr "$res" ".*'+481234#12345' caller" || exit 2
strstr "$res" ".*'+358 (9) 123 456' caller" || exit 2

# web addresses

strstr "$res" ".*'http://us.m.yahoo.com' browser" || exit 3
strstr "$res" ".*'http://www.google.com/m' browser" || exit 3
strstr "$res" ".*'www.google.com/m' browser" || exit 3
strstr "$res" ".*'http://us.m.yahoo.com' browser" || exit 3
strstr "$res" ".*'http://maps.google.com/maps?f=d&source=s_d&saddr=Nieznana+droga&daddr=krak%C3%B3w&hl&geocode=Fa5h9wIdlq87Aq%3BFQrt-wIdFFYwASnRGE41wEQWRzG_ikd2tbZrtA&mra=ls&sll=49.915862,20.323334&sspn=0.505808,1.234589&ie=UTF8&t=h&z=10' browser" || exit 3
strstr "$res" ".*'http://host-with-dash.com/path?%:@&=+\\$,-!~\\*'with(special)chars' browser" || exit 3

strstr "$res" ".*feed://browsefeed.com' browser" || exit 3
strstr "$res" ".*feed:browsefeed.com' browser" || exit 3
strstr "$res" ".*ftp://browseftp.com' browser" || exit 3

# email addresses
strstr "$res" ".*'john.doe@att.com' emailer" || exit 4
strstr "$res" ".*'mailto:john_doe2@att.com' emailer" || exit 4

# invalid and almost-invalid cases
strstr "$res" ".*'333222111444' caller" && exit 5
strstr "$res" ".*'+((((' caller" && exit 5
#+358 and +7777 will be recognized separately, though
strstr "$res" ".*'+358+7777' caller" && exit 5
#strstr "$res" ".*'+358' caller" && exit 5
#strstr "$res" ".*'+7777' caller" && exit 5
strstr "$res" ".*'13' caller" && exit 5
strstr "$res" ".*'14' caller" && exit 5
# a long phone number
strstr "$res" ".*'151515151515151515151' caller" && exit 5
# also: it's prefix shouldn't be recognized
strstr "$res" ".*'15151515151515151515' caller" && exit 5
strstr "$res" ".*'141' caller" || exit 5
strstr "$res" ".*'14141414141414141414' caller" || exit 5

strstr "$res" ".*'double@atsign@bar.com' emailer" && exit 5
strstr "$res" ".*'onlyuser@andhostname' emailer" && exit 5
strstr "$res" ".*'onlyuser2@andhostname2.' emailer" && exit 5
strstr "$res" ".*'USERNAME@ALLUPPERCASE.COM' emailer" || exit 5

strstr "$res" ".*'http://http://double.http.com' browser" && exit 6
strstr "$res" ".*'http://double.http.com' browser" || exit 6

# previous bugs
strstr "$res" ".*'www.myurl.com/foo/bar' browser" || exit 7
strstr "$res" ".*'first@home.com' emailer" || exit 7
strstr "$res" ".*'1112223338' caller" || exit 7
strstr "$res" ".*'email@domain.org' emailer" || exit 7
strstr "$res" ".*'4445556668' caller" || exit 7
strstr "$res" ".*'email2@domain.org' emailer" || exit 7

strstr "$res" ".*'Http://www.capital.com' browser" || exit 8
strstr "$res" ".*'hTTp://www.capital.com' browser" || exit 8

exit 0
