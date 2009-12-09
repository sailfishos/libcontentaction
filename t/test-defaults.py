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

try: import env
except: pass

import sys
import os
import unittest
from commands import getstatusoutput
from cltool import CLTool

class Defaults(unittest.TestCase):
    def testNoDefault(self):
        (status, output) = getstatusoutput("lca-tool --default http://fake.ontology/fke#NotAFile")
        self.assert_(status >>8 == 5)

    def testSetAndGetDefault(self):
        (status, output) = getstatusoutput("lca-tool --setdefault fake.action http://fake.ontology/fke#AudioFile")
        self.assert_(status == 0)

        (status, output) = getstatusoutput("lca-tool --default http://fake.ontology/fke#AudioFile")
        self.assert_(status == 0)
        self.assert_(output.find("fake.action\n") != -1)

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(Defaults)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
