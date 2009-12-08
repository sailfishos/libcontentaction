#!/bin/sh
[ -r ./env.sh ] && . ./env.sh;
exec $srcdir/sandbox.sh --stop
