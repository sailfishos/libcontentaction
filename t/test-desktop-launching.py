#!/usr/bin/python2.5
##
## Copyright (C) 2010 Nokia. All rights reserved.
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
from commands import getstatusoutput
from cltool import CLTool
import time

# this controls where the test files are
testfiles_dir = os.path.abspath('.')
if 'MIME_TEST_DIR' in os.environ:
    testfiles_dir = os.path.abspath(os.environ['MIME_TEST_DIR'])

class Launching(unittest.TestCase):
    def testActionsForDesktop(self):
        filename = "file://" + testfiles_dir + "/launchme.desktop"
        (status, output) = getstatusoutput("lca-tool --file --print " + filename)
        self.assert_(status == 0)
        self.assert_(output.find("launchme") != -1)

    def testLaunch(self):
        filename = "file://" + testfiles_dir + "/launchme.desktop"
        (status, output) = getstatusoutput("lca-tool --file --triggerdefault " + filename)
        self.assert_(status == 0)

        time.sleep(3)
        f = open('./launchedAction')
        content = f.read()
        f.close()
        os.remove("./launchedAction")
        self.assert_(content.find("I was launched") != -1)

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(Launching)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
