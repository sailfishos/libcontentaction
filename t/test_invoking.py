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
import unittest
from commands import getstatusoutput
from cltool import CLTool

class Actions(unittest.TestCase):
    def setUp(self):
        # start a fake gallery service
        self.gallery = CLTool("./gallery.py")
        self.assert_(self.gallery.expect("started"))

    def tearDown(self):
        self.gallery.kill()

    def testInvokeForImage(self):
        (status, output) = getstatusoutput("lca-tool --invokedefault an.image")
        self.assert_(status == 0)

        # assert that the gallery was invoked
        self.assert_(self.gallery.expect("showImage ;  ; an.image"))

    def testInvokeForTwoImages(self):
        (status, output) = getstatusoutput("lca-tool --invokedefault an.image b.image")
        self.assert_(status == 0)

        # assert that the gallery was invoked
        self.assert_(self.gallery.expect("showImage ;  ; an.image,b.image"))

    def testInvokeForInvalid(self):
        lcatool = CLTool("lca-tool", "--invokedefault", "invalid.uri")
        self.assert_(lcatool.expect("triggered an invalid action, not doing anything."))

    def testInvokeForDifferentClasses(self):
        lcatool = CLTool("lca-tool", "--invokedefault", "an.image", "a.contact")
        self.assert_(lcatool.expect("triggered an invalid action, not doing anything."))

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(Actions)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
