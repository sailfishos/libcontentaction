#!/usr/bin/python
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

# this controls where the test files are
testfiles_dir = os.path.abspath('.')
if 'MIME_TEST_DIR' in os.environ:
    testfiles_dir = os.path.abspath(os.environ['MIME_TEST_DIR'])

class FixedParams(unittest.TestCase):
    def setUp(self):
        # start a fake gallery service
        self.gallery = CLTool("gallery.py")
        self.assert_(self.gallery.expect("started"))

    def tearDown(self):
        self.gallery.kill()

    def testInvokeWithFixedParams(self):
        filename = "file://" + testfiles_dir + "/plaintext"
        (status, output) = getstatusoutput("lca-tool --file --trigger fixedparams " + filename)
        self.assert_(status == 0)
        # assert that the gallery was invoked and the fixed params got prepended
        self.assert_(self.gallery.expect("showImage ; these,are,fixed," + filename))

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(FixedParams)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
