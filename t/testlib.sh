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

trap do_cleanup EXIT;
