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

try: import env
except: pass

import unittest
from commands import getstatusoutput
from cltool import CLTool

class Defaults(unittest.TestCase):
    def DONOTtearDown(self):
        getstatusoutput("gconftool-2 --recursive-unset /Dui/contentaction")

    def DONOTtestNoDefaultForClass(self):
        (status, output) = getstatusoutput("lca-tool --classdefault http://fake.ontology/fke#NotAFile")
        self.assert_(status >>8 == 5)

    def DONOTtestSetAndGetDefaultForClass(self):
        (status, output) = getstatusoutput("lca-tool --setclassdefault fake.action http://fake.ontology/fke#AudioFile")
        self.assert_(status == 0)

        (status, output) = getstatusoutput("lca-tool --classdefault http://fake.ontology/fke#AudioFile")
        self.assert_(status == 0)
        self.assert_(output.find("fake.action") != -1)

    def DONOTtestNonImmediateDefault(self):
        # set a default for a non-immediate superclass
        (status, output) = getstatusoutput("lca-tool --setclassdefault nonimmediate.default http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Visual")
        (status, output) = getstatusoutput("lca-tool --default an.image")
        self.assert_(status == 0)
        self.assert_(output.find("nonimmediate.default") != -1)

    def DONOTtestImmediateDefault(self):
        # set a default for a non-immediate superclass
        (status, output) = getstatusoutput("lca-tool --setclassdefault nonimmediate.default http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Visual")
        # and also for an immediate superclass
        (status, output) = getstatusoutput("lca-tool --setclassdefault better.default http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image")

        (status, output) = getstatusoutput("lca-tool --default an.image")
        self.assert_(status == 0)
        self.assert_(output.find("better.default") != -1)

    def DONOTtestDefaultForManyUris(self):
        (status, output) = getstatusoutput("lca-tool --setclassdefault better.default http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image")

        (status, output) = getstatusoutput("lca-tool --default an.image b.image")
        self.assert_(status == 0)
        self.assert_(output.find("better.default") != -1)

    def DONOTtestSettingDefault(self):
        # only an applicable action can be set as default, so set the only applicable action
        (status, output) = getstatusoutput("lca-tool --setdefault com.nokia.galleryserviceinterface.showImage an.image")
        self.assert_(status == 0)

        # the default action should be set for the most specific subclass
        (status, output) = getstatusoutput("lca-tool --classdefault http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image")
        self.assert_(status == 0)
        self.assert_(output.find("com.nokia.galleryserviceinterface.showImage") != -1)

    def DONOTtestSettingDefaultForManyUris(self):
        # only an applicable action can be set as default, so set the only applicable action
        (status, output) = getstatusoutput("lca-tool --setdefault com.nokia.galleryserviceinterface.showImage an.image b.image")
        self.assert_(status == 0)

        # the default action should be set for the most specific subclass
        (status, output) = getstatusoutput("lca-tool --classdefault http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image")
        self.assert_(status == 0)
        self.assert_(output.find("com.nokia.galleryserviceinterface.showImage") != -1)

    def DONOTtestSettingDefaultForIncompatibleUris(self):
        # only an applicable action can be set as default, so set the only applicable action
        (status, output) = getstatusoutput("lca-tool --setdefault com.nokia.galleryserviceinterface.showImage an.image a.contact")
        self.assert_(status >> 8 == 7)

    def testQueryDefault(self):
        (status, output) = getstatusoutput("lca-tool --default an.image")
        self.assert_(status == 0);
        self.assert_(output.find("galleryserviceinterface.desktop"))

def runTests():
    suite = unittest.TestLoader().loadTestsFromTestCase(Defaults)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    sys.exit(runTests())
