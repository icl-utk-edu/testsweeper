#!/bin/bash -x

maker=$1

mydir=$(dirname $0)
source ${mydir}/setup_env.sh

print "======================================== Tests"

# Instead of exiting on the first failed test (bash -e),
# run all the tests and accumulate failures into $err.
err=0

cd test
export OMP_NUM_THREADS=8
./run_tests.py --xml ${top}/report-${maker}.xml
(( err += $? ))

# todo smoke tests

print "======================================== Finished test"
exit ${err}
