import sys
import subprocess
import re

rxt_regex = None
rxt_action = None

def rx_test_setup (rx, action):
    global rxt_regex
    global rxt_action
    rxt_regex = rx
    rxt_action = action

def rx_test_internal (spec):
    global rxt_regex
    parts = spec.split ("|")
    text = "".join (parts)
    expected = [ parts[i] for i in range (1, len (parts), 2) ]
    result = map (lambda m: m.group(0), re.finditer (rxt_regex, text))
    if result != expected:
        print >> sys.stderr, "Ouch: ", result, "!=", expected
        exit (1)

def rx_test_cita (spec):
    command = "echo '%s' | lca-tool --highlight |cat" % (spec)
    proc_stdout = subprocess.Popen ([command], stdout=subprocess.PIPE, shell=True).stdout

    parts = spec.split ("|")
    expected = [ parts[i].strip() for i in range (1, len (parts), 2) ]

    def position (string, char):
        i = 0
        for i in range (0, len (string)):
            if string[i] == char:
                yield i
        return

    separators = [sep for sep in position (spec, "|")]
    cut_points = [(separators[i], separators[i+1]) for i in range (0, len (separators), 2)]
    expected_results = [ cut_points[i] + ("'" + expected[i] + "'",) for i in range (0, len (expected))] 

    for line in proc_stdout.read().split ("\n"):

        if (len(line) == 0):
            continue

        (init, end, result) = line.strip().split (" ", 2)
        (expected_init, expected_end, expected_result) = expected_results.pop (0)

        if init != str(expected_init+1): #or end != expected_end:
            print >> sys.stderr,  "Ouch: cutting points for '%s' are (%s, %s) instead of (%s, %s)" % (spec, init, end, expected_init, expected_end)
            exit (1)
        
        if (result != expected_result):
            print >> sys.stderr, "Ouch: '%s' != '%s'" % (result, expected_result)
            exit (1)
            

if ("test" in sys.argv):
    rx_test = rx_test_cita
else:
    rx_test = rx_test_internal
