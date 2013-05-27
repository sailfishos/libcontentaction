#!/bin/sh

# Source this in tests.

CLEANUP=':'

# Called on exit.  Executes everything registered with atexit.
do_cleanup() {
        eval $CLEANUP;
}

# tstart BINARY ARGS...
# Starts BINARY in the background and kills it when the script exits.
tstart() {
        "$@" &
        atexit "kill -9 $!";
}

# atexit COMMAND
# Registers COMMAND to be run upon exit.
atexit() {
        CLEANUP="$CLEANUP ; $@";
}

abspath() {
    cd -P "$1" && pwd -P;
}

# strstr HAYSTACK NEEDLE
strstr() {
        if ! expr "$1" : "$2" >/dev/null;
        then
                echo "FAILED: Tested string does not contain expected value" >&2;
                echo "    HAYSTACK: '$1'" >&2;
                echo "    NEEDLE: '$2'" >&2;
                return 1;
        fi
}

trap 'test ${?} -eq 0 || echo "${0}:${LINENO}: FAILED"; do_cleanup' EXIT;
