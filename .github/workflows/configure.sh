#!/bin/bash -x

maker=$1
compiler=$2

if [ "${maker}" = "cmake" ]; then
    rm -rf build
    mkdir -p build
fi

mydir=$(dirname $0)
source ${mydir}/setup_env.sh

print "======================================== Environment"
# Show environment variables, excluding functions.
(set -o posix; set)

print "======================================== Modules"
quiet module list -l

print "======================================== Setup build"
# Note: set all env variables in setup_env.sh,
# else build.sh and test.sh won't see them.

rm -rf ${top}/install
if [ "${maker}" = "make" ]; then
    make distclean
    make config prefix=${top}/install \
         || exit 10

elif [ "${maker}" = "cmake" ]; then
    cmake -Dcolor=no \
          -DCMAKE_INSTALL_PREFIX=${top}/install \
          .. \
          || exit 12
fi

print "======================================== Finished configure"
exit 0
