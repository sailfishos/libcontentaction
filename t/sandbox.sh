#!/bin/sh
#
# SYNOPSIS
#
# ./sandbox.sh [--start|--stop]
#
# Starts or stops the sandbox environment for running the tests.

set -e;

srcdir=.
[ -r ./env.sh ] && . ./env.sh;

case "$1" in
        --start)
                exec >/tmp/lca-sandbox-start.log 2>&1
                echo "Starting"
                echo "Importing data to tracker"
                # reset tracker
                tracker-control -r;
                # try to wait until tracker has started
                for i in `seq 60`; do
                        if tracker-stats; then
			        break;
		        fi
			sleep 1;
                done
                # import our test data to tracker
                tracker-import "$srcdir/testdata.ttl"
                echo "Killing servicemapper"
                # replace servicemapper with ours
                kill -9 `ps axw | awk '/mservicemapper/ { print $1; }'` || true;
                $srcdir/servicemapper.py &
                echo "$!" > /tmp/servicemapper.py.pid;
                # wait until our service mapper has started
                for i in `seq 10`; do
                        if qdbus | grep "com.nokia.MServiceFw"; then
			        break;
		        fi
			sleep 1;
                done

                echo "Launching gconfd-2"
		if  ps axw | grep -v grep | grep /usr/lib/gconf2/gconfd-2; then
			/usr/lib/gconf2/gconfd-2 &
			echo "$!" > /tmp/gconfd-2.pid;
		fi
                ;;
        --stop)
                exec >/tmp/lca-sandbox-stop.log 2>&1
                # kill the gconfd if we started it
                kill -9 `cat /tmp/gconfd-2.pid` || true;
                rm -f /tmp/gconfd-2.pid || true;
                # kill our servicemapper
                kill -9 `cat /tmp/servicemapper.py.pid` || true;
                rm -f /tmp/servicemapper.py.pid || true;
                tracker-control -t;
                ;;
        *|--help)
                echo >&2 "./sandbox.sh [--start|--stop]";
                echo >&2 "starts or stops the sandbox for testing";
                exit 1;;
esac
