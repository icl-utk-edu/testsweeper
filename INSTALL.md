TestSweeper Installation Notes
================================================================================

[TOC]

Synopsis
--------------------------------------------------------------------------------

    make && make install

will configure and compile the TestSweeper library and its example,
then install the headers and library.

There is a CMake option as well.  See below.

Overview
--------------------------------------------------------------------------------

    make           - configures (if make.inc is missing),
                     then compiles the library and tester.
    make config    - configures TestSweeper, creating a make.inc file.
    make lib       - compiles the library (lib/libtestsweeper.so).
    make tester    - compiles the tester (example).
    make docs      - todo: generates documentation in docs/html/index.html
    make install   - installs the library and headers to ${prefix}.
    make uninstall - remove installed library and headers from ${prefix}.
    make clean     - deletes object (*.o) and library (*.a, *.so) files.
    make distclean - also deletes make.inc and dependency files (*.d).
    If static=1, makes .a instead of .so library.

Details
--------------------------------------------------------------------------------

    make config [options]

Runs the `configure.py` script to detect your compiler and library properties,
then creates a make.inc configuration file. You can also manually edit the
make.inc file. Options are name=value pairs to set variables. The configure.py
script can be invoked directly:

    python configure.py [options]

Running `configure.py -h` will print a help message with the current options.
Variables that affect configure.py include:

    CXX                C++ compiler
    CXXFLAGS           C++ compiler flags
    LDFLAGS            linker flags
    CPATH              compiler include search path
    LIBRARY_PATH       compile time library search path
    LD_LIBRARY_PATH    runtime library search path
    DYLD_LIBRARY_PATH  runtime library search path on macOS
    prefix             where to install:
                       headers go   in ${prefix}/include,
                       library goes in ${prefix}/lib${LIB_SUFFIX}

These can be set in your environment or on the command line, e.g.,

    python configure.py CXX=g++ prefix=/usr/local

TestSweeper does not rely on any libraries, other than optionally OpenMP,
so setting LDFLAGS, CPATH, LIBRARY_PATH, etc. is not generally needed.

Manual configuration
--------------------------------------------------------------------------------

If you have a specific configuration that you want, set CXX, CXXFLAGS, LDFLAGS,
and LIBS, e.g.:

    export CXX="g++"
    export CXXFLAGS="-fopenmp"
    export LDFLAGS="-fopenmp"

These can also be set when running configure:

    make config CXX=g++ \
                CXXFLAGS="-fopenmp" \
                LDFLAGS="-fopenmp"

Note that all test programs are compiled with those options, so errors may cause
configure to fail.

If you experience unexpected problems, please see config/log.txt to diagnose the
issue. The log shows the option being tested, the exact command run, the
command's standard output (stdout), error output (stderr), and exit status. All
test files are in the config directory.

CMake Installation
--------------------------------------------------------------------------------

The CMake script enforces an out of source build.  The simplest way to accomplish
this is to create a build directory off the Testsweeper root directory:

    cd /my/testsweeper/dir
    mkdir build && cd build

By default Testsweeper is set to install into `/opt/slate/`.  If you wish to change this, CMake needs to be told where to install the Testsweeper library.  You can do this
by defining CMAKE_INSTALL_PREFIX variable in via the CMake command line:

    # Assuming the working dir is still /my/testsweeper/dir/build
    cmake -DCMAKE_INSTALL_PREFIX=/path/to/my/dir ..

This generates the required makefiles and can build built and installed as normal:

    # Assuming the working dir is still /my/testsweeper/dir/build
    make
    make install

