#!/bin/bash -xe

maker=$1

mydir=$(dirname $0)
source ${mydir}/setup_env.sh

print "======================================== Build"
make -j8

print "======================================== Install"
make -j8 install
ls -R ${top}/install

print "======================================== Verify build"
ldd_result=$(ldd test/tester)
echo "${ldd_result}"
