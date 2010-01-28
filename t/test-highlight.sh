#!/bin/sh

srcdir=.
[ -r ./env.sh ] && . ./env.sh;
. $srcdir/testlib.sh

./hl1 < $srcdir/hlinput.txt |
{
        read start end x;
        test "$start" -eq 15 -a "$end" -eq 33 || exit 1;
        read start end x;
        test "$start" -eq 81 -a "$end" -eq 104 || exit 1;
        read start end x;
        test "$start" -eq 61 -a "$end" -eq 73 || exit 1;
        read start end x;
        test "$start" -eq 77 -a "$end" -eq 80 || exit 1;
}
