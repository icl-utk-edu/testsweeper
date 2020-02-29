#!/usr/bin/env python

from __future__ import print_function

import sys
import re
import argparse
import subprocess
import xml.etree.ElementTree as ET
import io

# ------------------------------------------------------------------------------
# command line arguments
parser = argparse.ArgumentParser()
parser.add_argument( '--xml', help='generate report.xml for jenkins' )
parser.add_argument( 'tests', nargs=argparse.REMAINDER )
opts = parser.parse_args()

opts.tests = list( map( int, opts.tests ) )

#-------------------------------------------------------------------------------
# 4 tuple: [ index, command, strip_time=True, expected exit code=0 ]
cmds = [
    #----------
    # Basics
    #
    # help (no input)
    [ 0, '../example', False ],

    # help -h
    [ 1, '../example -h', False ],

    # help --help
    [ 2, '../example --help', False ],

    # routine help -h
    [ 3, '../example -h foo', False ],

    # routine help --help
    [ 4, '../example --help foo', False ],

    # Defaults (--type d --dim 100:500:100).
    [ 5, '../example foo' ],

    # Larger range; should elicit 2 failures (error = 1.23456e-17 * n).
    [ 6, '../example --dim 100:1000:100 foo', True, 2 ],

    #----------
    # Types (enum)
    #
    # Specify types.
    [ 100, '../example --type s,d foo' ],

    # Invalid type x; should return error.
    [ 101, '../example --type s,x,d foo', False, 255 ],

    #----------
    # Dimensions
    #
    # m == n == k
    [ 200, '../example --type s --dim 100:300:100 foo2' ],

    # m == n == k, descending
    [ 201, '../example --type s --dim 300:100:-100 foo2' ],

    # single dimension
    [ 202, '../example --type s --dim 1234 foo2' ],

    # multiple --dim
    [ 203, '../example --type s --dim 1234 --dim 100:300:100 foo2' ],

    #----------
    # Zip of dimensions
    #
    # m x n == k.
    [ 300, '../example --type s --dim 100:300:100x50:200:50 foo3' ],

    # m x n; k fixed.
    [ 301, '../example --type s --dim 100:300:100x50:200:50x50 foo3' ],

    # m; n, k fixed.
    [ 302, '../example --type s --dim 100:300:100x100x50 foo3' ],

    # m x n x k.
    [ 303, '../example --type s --dim 100:300:100x50:200:50x10:50:10 foo3' ],

    #----------
    # Cartesian product of dimensions
    #
    # m * n == k
    [ 400, '../example --type s --dim 100:300:100*50:200:50 foo4' ],

    # m * n * k
    [ 401, '../example --type s --dim 100:300:100*50:200:50*10:50:10 foo4' ],

    # m fixed * n * k
    [ 402, '../example --type s --dim 100*50:200:50*10:50:10 foo4' ],

    # m * n fixed * k
    [ 403, '../example --type s --dim 100:300:100*50*10:50:10 foo4' ],

    # m * n * k fixed
    [ 404, '../example --type s --dim 100:300:100*50:200:50*10 foo4' ],

    #----------
    # Check and ref
    #
    # check y
    [ 500, '../example --check y foo5' ],

    # check n
    [ 501, '../example --check n foo5' ],

    # ref y
    [ 502, '../example --ref y foo5' ],

    # ref n
    [ 503, '../example --ref n foo5' ],
]

# ------------------------------------------------------------------------------
# When output is redirected to file instead of TTY console,
# print extra messages to stderr on TTY console.
#
output_redirected = not sys.stdout.isatty()

# ------------------------------------------------------------------------------
# If output is redirected, prints to both stderr and stdout;
# otherwise prints to just stdout.
#
def print_tee( *args ):
    global output_redirected
    print( *args )
    if (output_redirected):
        print( *args, file=sys.stderr )
# end

#-------------------------------------------------------------------------------
# Runs cmd. Returns exit code and output (stdout and stderr merged).
#
def run_test( num, cmd, strip_time, expected_err=0 ):
    print_tee( str(num) + ': ' + cmd )
    output = ''
    p = subprocess.Popen( cmd.split(), stdout=subprocess.PIPE,
                                       stderr=subprocess.STDOUT )
    p_out = p.stdout
    if (sys.version_info.major >= 3):
        p_out = io.TextIOWrapper(p.stdout, encoding='utf-8')
    # Read unbuffered ("for line in p.stdout" will buffer).
    for line in iter(p_out.readline, ''):
        print( line, end='' )
        output += line
    err = p.wait()
    if (err == expected_err):
        if (err == 0):
            print_tee( 'pass' )
        else:
            print_tee( 'pass: got expected exit code =', err )
    else:
        print_tee( 'FAILED: got exit code = %d, expected exit code = %d'
                   % (err, expected_err) )
    # end

    # Save output.
    outfile = 'tst-%03d.txt' % (num)
    print( 'Saving to', outfile )
    try:
        # Always strip out ANSI codes and version numbers.
        output2 = re.sub( r'\x1B\[\d+m', r'', output )
        output2 = re.sub( r'version \d+, id \w+',
                          r'version NA, id NA', output2 )
        if (strip_time):
            # Usually, strip out 4 time and gflops columns.
            # This messes up some output like `example -h foo`.
            output2 = re.sub( r'^((?: +\S+){5})((?: +\S+){4})(.*)',
                              r'\1  -----------  -----------  -----------  -----------\3',
                              output2, 0, re.M )
        out = open( outfile, 'w' )
        out.write( output2 )
        out.close()
    except Exception as ex:
        print_tee( 'FAILED: ' + outfile + ': ' + str(ex) )
        err = -2

    # Compare with reference.
    reffile = 'ref-%03d.txt' % (num)
    print( 'Comparing to', reffile )
    try:
        file = open( reffile )
        ref = file.read()
        if (ref != output2):
            print_tee( 'FAILED: diff', outfile, reffile )
            err = -1
    except Exception as ex:
        print_tee( 'FAILED: ' + reffile + ': ' + str(ex) )
        err = -2

    return (err, output)
# end

# ------------------------------------------------------------------------------
# Utility to pretty print XML.
# See https://stackoverflow.com/a/33956544/1655607
#
def indent_xml( elem, level=0 ):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent_xml( elem, level+1 )
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i
# end

# ------------------------------------------------------------------------------
# run each test
failed_tests = []
passed_tests = []
ntests = len(opts.tests)
run_all = (ntests == 0)

seen = set()
for tst in cmds:
    num = tst[0]
    cmd = tst[1]

    strip_time = True
    if (len( tst ) > 2):
        strip_time = tst[2]

    expected_err = 0
    if (len( tst ) > 3):
        expected_err = tst[3]

    if (run_all or tst[0] in opts.tests):
        seen.add( tst[0] )
        (err, output) = run_test( num, cmd, strip_time, expected_err )
        if (err != expected_err):
            failed_tests.append( (cmd, err, output) )
        else:
            passed_tests.append( cmd )
not_seen = list( filter( lambda x: x not in seen, opts.tests ))

if (not_seen):
    print_tee( 'Warning: unknown tests:', ' '.join( map( str, not_seen )))

# print summary of failures
nfailed = len( failed_tests )
if (nfailed > 0):
    print_tee( '\n' + str(nfailed) + ' tests FAILED:\n' +
               '\n'.join( [x[0] for x in failed_tests] ) )

# generate jUnit compatible test report
if opts.xml:
    print( 'writing XML file', opts.xml )
    root = ET.Element("testsuites")
    doc = ET.SubElement(root, "testsuite",
                        name="TestSweeper_suite",
                        tests=str(ntests),
                        errors="0",
                        failures=str(nfailed))

    for (test, err, output) in failed_tests:
        testcase = ET.SubElement(doc, "testcase", name=test)

        failure = ET.SubElement(testcase, "failure")
        if (err < 0):
            failure.text = "exit with signal " + str(-err)
        else:
            failure.text = str(err) + " tests failed"

        system_out = ET.SubElement(testcase, "system-out")
        system_out.text = output
    # end

    for test in passed_tests:
        testcase = ET.SubElement(doc, 'testcase', name=test)
        testcase.text = 'PASSED'

    tree = ET.ElementTree(root)
    indent_xml( root )
    tree.write( opts.xml )
# end

exit( nfailed )
