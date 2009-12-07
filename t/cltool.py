#!/usr/bin/env python
##
## Copyright (C) 2009 Nokia. All rights reserved.
##
## Contact: Marius Vollmer <marius.vollmer@nokia.com>
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public License
## version 2.1 as published by the Free Software Foundation.
##
## This library is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public
## License along with this library; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
## 02110-1301 USA

from __future__ import with_statement

import os
import re
import sys
import time
import math
import atexit
import signal
from threading import Thread, Lock
from subprocess import Popen, PIPE, STDOUT

class Reader(Thread):
    def __init__(self, cltool):
        Thread.__init__(self)
        self.cltool = cltool
        self.running = True

    def run(self):
        while True:
            l = self.cltool.process.stdout.readline()
            if l:
                l = l.rstrip() + '\n'  # delete trailing whitespace
                event = (time.time(), CLTool.STDOUT, l)
            else:
                event = (time.time(), CLTool.COMMENT, "EOF ON STDOUT")
                self.running = False
            with self.cltool.iolock:
                self.cltool.io.append(event)
            # not equivalent with a while self.running, this is repeat ... until
            if not self.running:
                break
        rc = self.cltool.process.wait()
        if rc < 0 and not self.cltool.killed:
            self.cltool.comment("TERMINATED WITH SIGNAL %d" % -rc)
            self.cltool.printio()
        elif rc > 0:
            self.cltool.comment("EXIT CODE: %d" % rc)
        self.cltool = None

class CLTool:

    COMMENT = -1
    STDIN = 0
    STDOUT = 1

    def __init__(self, *cline):
        self.process = Popen(cline, stdin=PIPE, stdout=PIPE, stderr=STDOUT,
                             preexec_fn=self._preexec, close_fds=True)
        self.io = []
        self.iolock = Lock()
        self.reader = Reader(self)
        self.last_expect = 0
        self.reader.start()
        self.last_output = "" # can be used externally to get the last input read during expect()
        self.killed = False
        atexit.register(self.atexit)

    def atexit(self):
        self.close()
        self.reader.running = False
        self.reader.join(3)

    def _preexec(self):
        """Enable core dumps and setup to be killed when the parent exits."""
        # see prctl(2) -> / PDEATHSIG
        # 0 -> RTLD_DEFAULT in dlopen(3), prctl is in libc.so.6
        # 1 -> PDEATHSIG
        # 9 -> SIGKILL
        # 0 0 0 -> unused arguments
        import ctypes
        ctypes.CDLL("", handle=0).prctl(1, 9, 0, 0, 0)

        import resource
        resource.setrlimit(resource.RLIMIT_CORE, (-1, -1))

    def send(self, string):
        """Writes STRING to the standard input of the child."""
        with self.iolock:
            self.io.append((time.time(), CLTool.STDIN, string))
        try:
            print >>self.process.stdin, string
            self.process.stdin.flush()
        except:
            self.printio()
            raise

    def _return_event(self, wantdump):
        with self.iolock:
            self.last_expect = len(self.io)
        if wantdump:
            self.printio()

    def _last_output(self):
        """Compute the output read since the last match."""
        r = []
        with self.iolock:
            for pos in xrange(self.last_expect, len(self.io)):
                ts, fno, l = self.io[pos]
                if fno == CLTool.STDOUT:
                    r.append(l)
        return ''.join(r)

    # exp: is either a single or a list of regexes and all of them has to match
    # timeout: the match has to be occur in this many seconds
    # wantdump: in case of error should we print a dump or not
    def expect(self, exp, timeout=5, wantdump=True):
        """Expect one more more regexps on the output.

        EXP is either a regular expression or a list of them
        `timeout'  -- specifies how many seconds to wait for the match [5]
        `wantdump' -- causes the io to be dumped if the expectation fails [True]

        """
        if not isinstance(exp, (list, tuple)):
            exp = [exp]
        rexp = [re.compile(s, re.MULTILINE) for s in exp]
        abs_timeout = time.time() + timeout
        while True:
            # check if we are matching
            # enumerate all of the patterns and remove the matching ones
            i = 0
            now = time.time()
            self.last_output = self._last_output()
            while i < len(rexp):
                if re.search(rexp[i], self.last_output):
                    del rexp[i]
                    del exp[i]
                    i -= 1
                i += 1
            if not rexp:
                self.comment('EXPECT OK')
                self._return_event(False)
                return True
            # timed out
            if now > abs_timeout:
                self.comment('TIMEOUT ' + str(exp))
                self._return_event(wantdump)
                return False
            # stream is closed
            if not self.reader.running:
                self._return_event(wantdump)
                return False
            time.sleep(0.1)

    def comment(self, st):
        """Append a comment to the io log."""
        with self.iolock:
            self.io.append((time.time(), CLTool.COMMENT, st))

    def suspend(self):
        """Suspend the child with a SIGSTOP signal."""
        os.kill(self.process.pid, signal.SIGSTOP)

    def resume(self):
        """Resume the child with SIGCONT."""
        os.kill(self.process.pid, signal.SIGCONT)

    def close(self):
        """Close the standard input of the child."""
        self.comment("EOF ON STDIN")
        self.process.stdin.close()

    def kill(self):
        os.kill(self.process.pid, signal.SIGTERM)
        self.killed = True

    def wait(self):
        """Close standard input and wait for the reader"""
        self.close()
        self.reader.join()
        return self.process.returncode

    def printio(self):
        """Dump the io log to the standard output."""
        print
        print '-' * 72
        with self.iolock:
            for ts, fno, line in self.io:
                line = line.rstrip("\r\n")
                t = "%s.%03d" % (time.strftime("%H:%M:%S", time.localtime(ts)),
                                 math.modf(ts)[0] * 1000)
                if fno == CLTool.COMMENT: info = "###";
                if fno == CLTool.STDIN:   info = "<--";
                if fno == CLTool.STDOUT:  info = "-->";
                print "%s %s %s" % (t, info, line)
        print '-' * 72
        sys.stdout.flush()

def wanted(name, type, value):
    """Construct an expect() regexp expecting a context property with VALUE."""
    # XXX: rewrite tests then remove this below.
    if type == "int": type = "qu?longlong"
    return "^%s = %s:%s$" % (name, type, value)

def wantedUnknown(name):
    """Construct an expect() regexp expecting an unset context property."""
    return "^%s = Unknown$" % (name)

__all__ = ['wanted', 'wantedUnknown', 'CLTool']
