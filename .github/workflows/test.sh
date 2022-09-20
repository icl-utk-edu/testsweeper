#!/bin/bash -xe

maker=$1
device=$2

mydir=`dirname $0`
source $mydir/setup_env.sh

section "======================================== Tests"
cd test
export OMP_NUM_THREADS=8
./run_tests.py --xml ${top}/report-${maker}.xml

# todo smoke tests

section "======================================== Finished"

