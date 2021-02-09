TestSweeper Installation Notes
================================================================================

[TOC]

Synopsis
--------------------------------------------------------------------------------

Configure and compile the TestSweeper library and its example,
then install the headers and library.

Option 1: Makefile

    make && make install

Option 2: CMake

    mkdir build && cd build
    cmake ..
    make && make test && make install


Environment variables (Makefile and CMake)
--------------------------------------------------------------------------------

Standard environment variables affect both Makefile (configure.py) and CMake.
These include:

    CXX                 C++ compiler
    CXXFLAGS            C++ compiler flags
    LDFLAGS             linker flags
    CPATH               compiler include search path
    LIBRARY_PATH        compile-time library search path
    LD_LIBRARY_PATH     runtime library search path
    DYLD_LIBRARY_PATH   runtime library search path on macOS

TestSweeper does not rely on any libraries, other than optionally OpenMP,
so setting LDFLAGS, CPATH, LIBRARY_PATH, etc. is not generally needed.


Makefile Installation
--------------------------------------------------------------------------------

Available targets:

    make           - configures (if make.inc is missing),
                     then compiles the library and tester
    make config    - configures TestSweeper, creating a make.inc file
    make lib       - compiles the library (lib/libtestsweeper.so)
    make tester    - compiles test/tester
    make docs      - todo: generates documentation in docs/html/index.html
    make install   - installs the library and headers to ${prefix}
    make uninstall - remove installed library and headers from ${prefix}
    make clean     - deletes object (*.o) and library (*.a, *.so) files
    make distclean - also deletes make.inc and dependency files (*.d)


### Options

    make config [options]

Runs the `configure.py` script to detect your compiler and library properties,
then creates a make.inc configuration file. You can also manually edit the
make.inc file. Options are name=value pairs to set variables. The configure.py
script can be invoked directly:

    python configure.py [options]

Running `configure.py -h` will print a help message with the current options.
In addition to those listed in the Environment variables section above,
options include:

    color={auto,yes,no} use ANSI colors in TestSweeper output
    static={0,1}        build as shared (default) or static library
    prefix              where to install, default /opt/slate.
                        headers go   in ${prefix}/include,
                        library goes in ${prefix}/lib${LIB_SUFFIX}

These can be set in your environment or on the command line, e.g.,

    python configure.py CXX=g++ prefix=/usr/local


### Manual configuration

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

The CMake script enforces an out-of-source build. Create a build
directory under the TestSweeper root directory:

    cd /path/to/testsweeper
    mkdir build && cd build
    cmake [options] ..
    make
    make test
    make install


### Options

CMake uses the settings in the Environment variables section above.
Standard CMake options include:

    BUILD_SHARED_LIBS={ON,off}  build as shared (default) or static library
    CMAKE_INSTALL_PREFIX        where to install, default /opt/slate

TestSweeper specific options include (all values case insensitive):

    color={ON,off}                use ANSI colors in output
    use_openmp={ON,off}           use OpenMP, if available
    build_tests={ON,off}          build test suite (test/tester)

These options are defined on the command line using `-D`, e.g.,

    # in build directory
    cmake -Dbuild_tests=off -DCMAKE_INSTALL_PREFIX=/usr/local ..

Alternatively, use the `ccmake` text-based interface or the CMake app GUI.

    # in build directory
    ccmake ..
    Type 'c' to configure, then 'g' to generate Makefile

To re-configure CMake, you may need to delete CMake's cache:

    # in build directory
    rm CMakeCache.txt
    # or
    rm -rf *

To debug the build, set `VERBOSE`:

    # in build directory, after running cmake
    make VERBOSE=1
