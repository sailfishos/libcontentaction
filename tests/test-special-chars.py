#!/usr/bin/python3
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

try: import env
except: pass

import unittest
from subprocess import getstatusoutput
from cltool import CLTool

class SpecialChars(unittest.TestCase):
    def setUp(self):
        # start a fake gallery service
        self.gallery = CLTool("gallery.py")
        self.assertTrue(self.gallery.expect("started"))
        # dummy content so it's not recognized as text/plain
        (status, output) = getstatusoutput("echo a > /tmp/some#file.mp3")
        (status, output) = getstatusoutput("ls /tmp/some#file.mp3")

    def tearDown(self):
        self.gallery.kill()
        (status, output) = getstatusoutput("rm /tmp/some#file.mp3")

    def testActionsForFileWithSpecialChars(self):
        filename = "file:///tmp/some%23file.mp3"
        (status, output) = getstatusoutput("lca-tool --file --print " + filename)
        self.assertTrue(status == 0)
        self.assertTrue(output.find("plainmusicplayer") != -1)

    def testActionsForFileWithSpecialChars2(self):
        filename = "/tmp/some#file.mp3"
        (status, output) = getstatusoutput("lca-tool --file --print " + filename)
        self.assertTrue(status == 0)
        self.assertTrue(output.find("plainmusicplayer") != -1)

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(SpecialChars)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
