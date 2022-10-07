#!/bin/bash -xe

maker=$1

mydir=$(dirname $0)
source ${mydir}/setup_env.sh

print "======================================== Tests"
cd test
export OMP_NUM_THREADS=8
./run_tests.py --xml ${top}/report-${maker}.xml

# todo smoke tests

print "======================================== Finished"

