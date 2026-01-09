#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

export CONTENTACTION_ACTIONS=/usr/share/contentaction

strstr() {
    expr "$1" : "$2" >/dev/null;
}

res=$(lca-tool --highlight < $srcdir/hlinput.txt)

strstr "$res" ".*61 73 '+44 433 2236' caller" || exit 10
strstr "$res" ".*77 80 '911' caller" || exit 11
strstr "$res" ".*15 33 'email@address.here' emailer" || exit 12
strstr "$res" ".*81 104 'ooaoa+foo@motherland.ru' emailer" || exit 13

# phone numbers

strstr "$res" ".*'1234567' caller" || exit 20
strstr "$res" ".*'(303)499-7111' caller" || exit 21
strstr "$res" ".*'+13034997111' caller" || exit 22
strstr "$res" ".*'303-499-7111' caller" || exit 23
strstr "$res" ".*'+1(303)499-7111' caller" || exit 24
strstr "$res" ".*'303.499.7111' caller" || exit 25
strstr "$res" ".*'3034997111' caller" || exit 26
strstr "$res" ".*'+48 123.12-3 123' caller" || exit 27
strstr "$res" ".*'+481234p12345' caller" || exit 28
strstr "$res" ".*'+481234#12345' caller" || exit 29
strstr "$res" ".*'+358 (9) 123 456' caller" || exit 30

# phone number uris
strstr "$res" ".*'callto:100000' caller" || exit 31
strstr "$res" ".*'tel:100000' caller" || exit 32
strstr "$res" ".*'sms:100000' caller" || exit 33

# suspicious phone uris with spaces:
strstr "$res" ".*'callto:+100 000 001' caller" || exit 34
strstr "$res" ".*'tel:+100 (000) 001' caller" || exit 35

# sip uris
strstr "$res" ".*'sip:thisissip@sip.com' caller" || exit 36
strstr "$res" ".*'sip:8071870pp20326#pp12397#' caller" || exit 37

# web addresses

strstr "$res" ".*'http://us.m.yahoo.com' browser" || exit 40
strstr "$res" ".*'http://www.google.com/m' browser" || exit 41
strstr "$res" ".*'www.google.com/m' browser" || exit 42
strstr "$res" ".*'http://us.m.yahoo.com' browser" || exit 43
strstr "$res" ".*'http://maps.google.com/maps?f=d&source=s_d&saddr=Nieznana+droga&daddr=krak%C3%B3w&hl&geocode=Fa5h9wIdlq87Aq%3BFQrt-wIdFFYwASnRGE41wEQWRzG_ikd2tbZrtA&mra=ls&sll=49.915862,20.323334&sspn=0.505808,1.234589&ie=UTF8&t=h&z=10' browser" || exit 44
strstr "$res" ".*'http://host-with-dash.com/path?%:@&=+\\$,-!~\\*'with(special)chars' browser" || exit 45

strstr "$res" ".*feed://browsefeed.com' browser" || exit 46
strstr "$res" ".*feed:browsefeed.com' browser" || exit 47
strstr "$res" ".*ftp://browseftp.com' browser" || exit 48

# parens and dots...

strstr "$res" ".*http://mydomain00.com' browser" || exit 50
strstr "$res" ".*http://mydomain01.com/' browser" || exit 51
strstr "$res" ".*http://mydomain02.com/file' browser" || exit 52
strstr "$res" ".*http://mydomain03.com/file/' browser" || exit 53
strstr "$res" ".*http://mydomain04.com' browser" || exit 54
strstr "$res" ".*http://mydomain05.com/' browser" || exit 55
strstr "$res" ".*http://mydomain06.com/file' browser" || exit 56
strstr "$res" ".*http://mydomain07.com/file/' browser" || exit 57

strstr "$res" ".*http://mydomain08.com' browser" || exit 58
strstr "$res" ".*http://mydomain09.com/' browser" || exit 59
strstr "$res" ".*http://mydomain10.com/file' browser" || exit 60
strstr "$res" ".*http://mydomain11.com/file/' browser" || exit 61

strstr "$res" ".*http://paren.com/something_(one_%26_two)' browser" || exit 62
strstr "$res" ".*http://paren.com/something_(three_%26_four)' browser" || exit 63
strstr "$res" ".*http://paren.com/something_(five_%26_six)' browser" || exit 64
strstr "$res" ".*www.foo.com/(bar)baz(bar)bar' browser" || exit 65

# allowing #
strstr "$res" ".*http://www.foo.com/page#fragment' browser" || exit 66
strstr "$res" ".*feed://browsefeed.com/page#fragment' browser" || exit 67
strstr "$res" ".*ftp://browseftp.com/page#fragment' browser" || exit 68

# email addresses
strstr "$res" ".*'john.doe@att.com' emailer" || exit 70
strstr "$res" ".*'mailto:john_doe2@att.com' emailer" || exit 71

# invalid and almost-invalid cases
strstr "$res" ".*'333222111444' caller" || exit 72 # changed to be valid
strstr "$res" ".*'+((((' caller" && exit 73
#+358 and +7777 will be recognized separately, though
strstr "$res" ".*'+358+7777' caller" && exit 74
#strstr "$res" ".*'+358' caller" && exit 75
#strstr "$res" ".*'+7777' caller" && exit 76
strstr "$res" ".*'13' caller" && exit 77
strstr "$res" ".*'14' caller" && exit 78
# a long phone number
strstr "$res" ".*'151515151515151515151' caller" && exit 79
# also: its prefix shouldn't be recognized
strstr "$res" ".*'15151515151515151515' caller" && exit 80
strstr "$res" ".*'141' caller" || exit 81
strstr "$res" ".*'14141414141414141414' caller" || exit 82

strstr "$res" ".*'double@atsign@bar.com' emailer" && exit 83
strstr "$res" ".*'onlyuser@andhostname' emailer" && exit 84
strstr "$res" ".*'onlyuser2@andhostname2.' emailer" && exit 85
strstr "$res" ".*'USERNAME@ALLUPPERCASE.COM' emailer" || exit 86

strstr "$res" ".*'http://http://double.http.com' browser" && exit 87
strstr "$res" ".*'http://double.http.com' browser" || exit 88

# previous bugs
strstr "$res" ".*'www.myurl.com/foo/bar' browser" || exit 90
strstr "$res" ".*'first@home.com' emailer" || exit 91
strstr "$res" ".*'1112223338' caller" || exit 92
strstr "$res" ".*'email@domain.org' emailer" || exit 93
strstr "$res" ".*'4445556668' caller" || exit 94
strstr "$res" ".*'email2@domain.org' emailer" || exit 95

strstr "$res" ".*'Http://www.capital.com' browser" || exit 96
strstr "$res" ".*'hTTp://www.capital.com' browser" || exit 97
strstr "$res" ".*'Www.capitalwww.com' browser" || exit 98

strstr "$res" ".*'http://ct.nokia.com?761435127&FK20R' browser" || exit 99
strstr "$res" ".*'http://maps.ovi.com/#|50.060185|19.9328789|11|0|0|normal.day' browser" || exit 100

# A URL immediately followed by non-ascii letters.
strstr "$res" ".*'www.10010.com' browser" || exit 101

strstr "$res" ".*'"

exit 0
