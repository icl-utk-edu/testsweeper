#!/bin/bash -xe

maker=$1

hostname && pwd
export top=`pwd`

shopt -s expand_aliases

quiet() {
    { set +x; } 2> /dev/null;
    $@;
    set -x
}

print_section() {
    builtin echo "$*"
    date
    case "$save_flags" in
        (*x*)  set -x
    esac
}
alias section='{ save_flags="$-"; set +x; } 2> /dev/null; print_section'

section "======================================== Load compiler"
module load gcc@7.3.0

section "======================================== Verify environment"
module list
which g++
g++ --version

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
    module load cmake
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

