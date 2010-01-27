#!/usr/bin/python2.5
##
## Copyright (C) 2008, 2009 Nokia. All rights reserved.
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

import sys
import os
# Otherwise env.py won't be found when running tests inside a VPATH build dir
sys.path.insert(0, os.getcwd())
from subprocess import Popen
from time import sleep

try: import env
except: pass

import unittest
from cltool import CLTool

class ServiceFWSignals(unittest.TestCase):
    def setUp(self):
        # start 2 fake gallery services
        self.gallery = CLTool("gallery.py", "just.a.gallery")
        self.assert_(self.gallery.expect("started"))
        self.gallery2 = CLTool("gallery.py", "just.another.gallery")
        self.assert_(self.gallery2.expect("started"))

        # reset the mapping inside the service mapper
        p = Popen('qdbus com.nokia.DuiServiceFw / org.maemo.contentaction.testing.changeMapping "just.a.gallery" "com.nokia.galleryserviceinterface"', shell=True)
        sts = os.waitpid(p.pid, 0)[1]
        self.assert_(sts == 0)

    def tearDown(self):
        self.gallery.kill()
        self.gallery2.kill()

    def testImplementorChanges(self):
        # start a program which repeatedly queries the default action and
        # invokes it
        progdir = "."
        if "builddir" in os.environ:
            progdir = os.environ["builddir"]
        invoker = CLTool(progdir + "/servicetest")

        # assert that the old implementor was invoked
        self.assert_(self.gallery.expect("showImage ; an.image"))

        # change the mapping inside our mock service mapper
        p = Popen('qdbus com.nokia.DuiServiceFw / org.maemo.contentaction.testing.changeMapping "just.another.gallery" "com.nokia.galleryserviceinterface"', shell=True)
        sts = os.waitpid(p.pid, 0)[1]
        self.assert_(sts == 0)

        # then command our mock service mapper to send a signal that the
        # implementor has changed
        p = Popen('qdbus com.nokia.DuiServiceFw / org.maemo.contentaction.testing.emitServiceAvailable "just.another.gallery" "com.nokia.galleryserviceinterface"', shell=True)
        sts = os.waitpid(p.pid, 0)[1]
        self.assert_(sts == 0)

        # and the new implementor as well
        self.assert_(self.gallery2.expect("showImage ; an.image", timeout=7))

    def testImplementorGone(self):
        # start a program which repeatedly queries the default action and
        # invokes it
        progdir = "."
        if "builddir" in os.environ:
            progdir = os.environ["builddir"]
        invoker = CLTool(progdir + "/servicetest")

        # assert that the old implementor was invoked
        self.assert_(self.gallery.expect("showImage ; an.image"))

        # change the mapping inside our mock service mapper
        p = Popen('qdbus com.nokia.DuiServiceFw / org.maemo.contentaction.testing.changeMapping "just.another.gallery" "com.nokia.galleryserviceinterface"', shell=True)
        sts = os.waitpid(p.pid, 0)[1]
        self.assert_(sts == 0)

        # then command our mock service mapper to send a signal that the
        # implementor is no longer available
        p = Popen('qdbus com.nokia.DuiServiceFw / org.maemo.contentaction.testing.emitServiceUnavailable "just.a.gallery"', shell=True)
        sts = os.waitpid(p.pid, 0)[1]
        self.assert_(sts == 0)

        # now, the implementor should be "just.another.gallery", because the
        # mapping was changed and the previous preferred implementor is no
        # longer available

        # and the new implementor as well
        self.assert_(self.gallery2.expect("showImage ; an.image", timeout=7))

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(ServiceFWSignals)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
