#!/bin/bash -xe

#-------------------------------------------------------------------------------
# Functions

# Suppress echo (-x) output of commands executed with `quiet`.
# Useful for sourcing files, loading modules, spack, etc.
# set +x, set -x are not echo'd.
quiet() {
    { set +x; } 2> /dev/null;
    $@;
    set -x
}

# `section` is like `echo`, but suppresses output of the command itself.
# https://superuser.com/a/1141026
print_section() {
    builtin echo "$*"
    date
    case "$save_flags" in
        (*x*)  set -x
    esac
}
alias section='{ save_flags="$-"; set +x; } 2> /dev/null; print_section'


#-------------------------------------------------------------------------------
maker=$1

quiet source /etc/profile

hostname && pwd
export top=`pwd`

shopt -s expand_aliases


section "======================================== Load compiler"
quiet module load gcc@7.3.0

section "======================================== Verify environment"
quiet module list
quiet which g++
quiet g++ --version

section "======================================== Environment"
env

section "======================================== Setup"
export color=no
rm -rf ${top}/install
if [ "${maker}" = "make" ]; then
    make distclean
    make config CXXFLAGS="-Werror" prefix=${top}/install
fi
if [ "${maker}" = "cmake" ]; then
    quiet module load cmake
    which cmake
    cmake --version

    rm -rf build && mkdir build && cd build
    cmake -Dcolor=no -DCMAKE_CXX_FLAGS="-Werror" \
          -DCMAKE_INSTALL_PREFIX=${top}/install ..
fi

section "======================================== Build"
make -j8

section "======================================== Install"
make -j8 install
ls -R ${top}/install

section "======================================== Verify build"
ldd test/tester

section "======================================== Test"
cd test
./run_tests.py --xml ${top}/report-${maker}.xml

# todo smoke tests

section "======================================== Finished"

