#!/usr/bin/python3
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
from subprocess import getstatusoutput
from cltool import CLTool

# this controls where the test files are
testfiles_dir = os.path.abspath('.')
if 'MIME_TEST_DIR' in os.environ:
    testfiles_dir = os.path.abspath(os.environ['MIME_TEST_DIR'])

class Mimes(unittest.TestCase):
    def testActionsForPlain(self):
        filename = "file://" + testfiles_dir + "/plaintext"
        (status, output) = getstatusoutput("lca-tool --file --print " + filename)
        self.assertTrue(status == 0)
        self.assertTrue(output.find("uberexec") != -1)
        self.assertTrue(output.find("ubermeego") != -1)
        self.assertTrue(output.find("ubermimeopen") != -1)

    def testInvokeMeego(self):
        program = CLTool("uberprogram.py")
        self.assertTrue(program.expect("started"))

        filename = "file://" + testfiles_dir + "/plaintext"
        (status, output) = getstatusoutput("lca-tool --file --trigger ubermeego " + filename)
        self.assertTrue(status == 0)

        self.assertTrue(program.expect("launch: "))
        program.kill()

    def testInvokeMimeOpen(self):
        program = CLTool("uberprogram.py")
        self.assertTrue(program.expect("started"))

        filename = "file://" + testfiles_dir + "/plaintext"
        (status, output) = getstatusoutput("lca-tool --file --trigger ubermimeopen " + filename)
        self.assertTrue(status == 0)

        self.assertTrue(program.expect("mime_open:"))
        program.kill()

    def testInvokeExec(self):
        filename = "file://" + testfiles_dir + "/plaintext"
        (status, output) = getstatusoutput("lca-tool --file --trigger uberexec " + filename)
        self.assertTrue(status == 0)
        f = open("/tmp/executedAction")
        content = f.read()
        f.close()
        os.remove("/tmp/executedAction")
        self.assertTrue(content.find("This is plain text") != -1)

    def testInvokeExecMulti(self):
        filename = testfiles_dir + "/plaintext"
        (status, output) = getstatusoutput("lca-tool --file --trigger unterexec " + filename + " " + filename + "2")
        self.assertTrue(status == 0)
        f = open("/tmp/executedParams")
        content = f.read()
        f.close()
        os.remove("/tmp/executedParams")
        self.assertTrue(content == filename + "\n" + filename + "2\n");

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(Mimes)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
