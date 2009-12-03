#!/usr/bin/env python
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

class Actions(unittest.TestCase):
    def testActionsForImage(self):
        (status, output) = getstatusoutput("lca-tool --print an.image")
        self.assert_(status == 0)
        self.assert_(output.find("com.nokia.galleryserviceinterface.showImage\n") != -1)

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(Actions)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
