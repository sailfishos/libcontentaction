#!/bin/sh -e

cd $(dirname $0)
autoreconf -v -f -i

if patch -s -t -p0 --dry-run < ltmain-asneeded.patch; then
    patch -t -p0 < ltmain-asneeded.patch
fi
