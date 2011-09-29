import unittest
import sys, re, os
import subprocess
import functools

# 
# Note: unittest in python < 2.7 doesn't support expectedFailures
#

def escape_for_shell (str):
    "'" + str.replace("'", "'\\''") + "'"

class LocalRegexTest (unittest.TestCase):
    
    def __init__ (self, regex, name, spec, delimiter, counter):
        unittest.TestCase.__init__ (self, methodName="regex_test")
        self.__regex = regex
        self.__name = name
        self.__spec = spec
        self.__delimiter = delimiter

        methodName = "test_%s_%d" % (self.__name, int(counter))
        # 
        # Expected failures run the same test function "decorated" with expectedFailure
        #
        setattr (self,
                 methodName,
                 self.regex_test)
        self._testMethodName = methodName

    def regex_test (self):
        parts = self.__spec.split (self.__delimiter)
        text = "".join (parts)
        expected = [ parts[i] for i in range (1, len (parts), 2) ]
        result = map (lambda m: m.group(0), re.finditer (self.__regex, text))
        self.assertEquals (result, expected)


class SystemRegexTest (unittest.TestCase):

    def __init__ (self, regex, name, spec, delimiter, counter):
        unittest.TestCase.__init__ (self, methodName="regex_test")
        self.__regex = regex
        self.__name = name
        self.__spec = spec
        self.__delimiter = delimiter

        methodName = "test_%s_cita_%d" % (self.__name, int(counter))
        # 
        # Expected failures run the same test function "decorated" with expectedFailure
        #
        setattr (self,
                 methodName,
                 self.regex_test)
        self._testMethodName = methodName

    def __check_tools (self):
        lca_in_path = False
        for d in os.environ["PATH"].split (":"):
            if os.path.exists (os.path.join (d, "lca-tool")):
                return

        raise Exception ("lca-tool is not in the $PATH")


    def regex_test (self):
        self.__check_tools ()
        command = ("echo %s | lca-tool --highlight |cat"
                   % (escape_for_shell (self.__spec.replace (self.__delimiter, ""))))
        proc_stdout = subprocess.Popen ([command], stdout=subprocess.PIPE, shell=True).stdout

        parts = self.__spec.split (self.__delimiter)
        expected = [parts[i].strip() for i in range (1, len (parts), 2) ]

        for line in proc_stdout.read().split ("\n"):

            if (len(line) == 0):
                continue

            # In real environments, usually there is an action after the match
            (init, end, raw_result) = line.strip().split (" ", 2)
            result = raw_result if raw_result[-1] == "'" else raw_result[:raw_result.rindex (" ")]
            
            expected_result = expected.pop (0)

            self.assertEquals (result[1:-1], expected_result, 
                               "'%s' is not recognized properly " % (self.__spec)
                               + "(matched '%s' instead of '%s')" % (result,
                                                                     expected_result)
                               + "\nCommand line:\n%s" % (command))


        
def static_counter_gen ():
    k = 0
    while True:
        k += 1
        yield k
static_counter = static_counter_gen ().next


rxt_regex = None
rxt_action = None

regexTestSuite = unittest.TestSuite ()

def rx_test_setup (rx, action):
    global rxt_regex
    global rxt_action
    rxt_regex = rx
    rxt_action = action

def rx_test (spec, delimiter = "|"):
    if ("test" in sys.argv):
        t = SystemRegexTest (rxt_regex, rxt_action, spec, delimiter, static_counter ())
    else:
        t = LocalRegexTest (rxt_regex, rxt_action, spec, delimiter, static_counter())
    regexTestSuite.addTest(t)


def rx_run ():
    result = unittest.TextTestRunner (verbosity=1).run (regexTestSuite)
    sys.exit(not result.wasSuccessful())
