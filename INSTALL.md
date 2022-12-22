TestSweeper Installation Notes
================================================================================

[TOC]

Synopsis
--------------------------------------------------------------------------------

Configure and compile the TestSweeper library and its tester,
then install the headers and library.

Option 1: Makefile

    make && make install

Option 2: CMake

    mkdir build && cd build
    cmake ..
    make && make install


Environment variables (Makefile and CMake)
--------------------------------------------------------------------------------

Standard environment variables affect both Makefile (configure.py) and CMake.
These include:

    LD                  Linker; defaults to CXX
    CXX                 C++ compiler
    CXXFLAGS            C++ compiler flags
    LDFLAGS             linker flags
    CPATH               compiler include search path
    LIBRARY_PATH        compile-time library search path
    LD_LIBRARY_PATH     runtime library search path
    DYLD_LIBRARY_PATH   runtime library search path on macOS

TestSweeper does not rely on any libraries, other than optionally OpenMP,
so setting LDFLAGS, CPATH, LIBRARY_PATH, etc. is not generally needed.


Options (Makefile and CMake)
--------------------------------------------------------------------------------

TestSweeper specific options include (all values are case insensitive):

    color
        Whether to use ANSI colors in output. One of:
        auto            uses color if output is a TTY
                        (default with Makefile; not support with CMake)
        yes             (default with CMake)
        no

With Makefile, options are specified as environment variables or on the
command line using `option=value` syntax, such as:

    python3 configure.py color=no

With CMake, options are specified on the command line using
`-Doption=value` syntax (not as environment variables), such as:

    cmake -Dcolor=no ..


Makefile Installation
--------------------------------------------------------------------------------

Available targets:

    make           - configures (if make.inc is missing),
                     then compiles the library and tester
    make config    - configures TestSweeper, creating a make.inc file
    make lib       - compiles the library (lib/libtestsweeper.so)
    make tester    - compiles test/tester
    make check     - run basic checks using tester
    make docs      - todo: generates documentation in docs/html/index.html
    make install   - installs the library and headers to ${prefix}
    make uninstall - remove installed library and headers from ${prefix}
    make clean     - deletes object (*.o) and library (*.a, *.so) files
    make distclean - also deletes make.inc and dependency files (*.d)


### Options

    make config [options]
    or
    python3 configure.py [options]

Runs the `configure.py` script to detect your compiler and library properties,
then creates a make.inc configuration file. You can also manually edit the
make.inc file. Options are name=value pairs to set variables.

Besides the Environment variables and Options listed above, additional
options include:

    static
        Whether to build as a static or shared library.
        0               shared library (default)
        1               static library

    prefix
        Where to install, default /opt/slate.
        Headers go   in ${prefix}/include,
        library goes in ${prefix}/lib${LIB_SUFFIX}

These can be set in your environment or on the command line, e.g.,

    python3 configure.py CXX=g++ prefix=/usr/local


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
    cmake [-DCMAKE_INSTALL_PREFIX=/path/to/install] [options] ..
    make
    make install


### Options

Besides the Environment variables and Options listed above, additional
options include:

    use_openmp
        Whether to use OpenMP, if available. One of:
        yes (default)
        no

    build_tests
        Whether to build test suite (test/tester).
        Requires TestSweeper, CBLAS, and LAPACK. One of:
        yes (default)
        no

Standard CMake options include:

    BUILD_SHARED_LIBS
        Whether to build as a static or shared library. One of:
        yes             shared library (default)
        no              static library

    CMAKE_INSTALL_PREFIX (alias prefix)
        Where to install, default /opt/slate.
        Headers go   in ${prefix}/include,
        library goes in ${prefix}/lib

    CMAKE_PREFIX_PATH
        Where to look for CMake packages such as BLAS++ and TestSweeper.

    CMAKE_BUILD_TYPE
        Type of build. One of:
        [empty]         default compiler optimization          (no flags)
        Debug           no optimization, with asserts          (-O0 -g)
        Release         optimized, no asserts, no debug info   (-O3 -DNDEBUG)
        RelWithDebInfo  optimized, no asserts, with debug info (-O2 -DNDEBUG -g)
        MinSizeRel      Release, but optimized for size        (-Os -DNDEBUG)

    CMAKE_MESSAGE_LOG_LEVEL (alias log)
        Level of messages to report. In ascending order:
        FATAL_ERROR, SEND_ERROR, WARNING, AUTHOR_WARNING, DEPRECATION,
        NOTICE, STATUS, VERBOSE, DEBUG, TRACE.
        Particularly, DEBUG or TRACE gives useful information.

With CMake, options are specified on the command line using
`-Doption=value` syntax (not as environment variables), such as:

    # in build directory
    cmake -Dbuild_tests=no -DCMAKE_INSTALL_PREFIX=/usr/local ..

Alternatively, use the `ccmake` text-based interface or the CMake app GUI.

    # in build directory
    ccmake ..
    # Type 'c' to configure, then 'g' to generate Makefile

To re-configure CMake, you may need to delete CMake's cache:

    # in build directory
    rm CMakeCache.txt
    # or
    rm -rf *
    cmake [options] ..

To debug the build, set `VERBOSE`:

    # in build directory, after running cmake
    make VERBOSE=1
