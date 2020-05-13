# CMake script for TestSweeper framework
# needed to build BLASPP/test, LAPACKPP/test, SLATE/test
# repo: http://bitbucket.org/icl/testsweeper

cmake_minimum_required( VERSION 3.8 )
# 3.1  target_compile_features
# 3.8  target_compile_features( cxx_std_11 )
# 3.14 install( LIBRARY DESTINATION lib ) default
# 3.15 $<$COMPILE_LANG_AND_ID  # optional

project(
    testsweeper
    VERSION 2020.06.0
    LANGUAGES CXX
)

#-------------------------------------------------------------------------------
# Options
option( BUILD_SHARED_LIBS "Build shared libraries" true )
option( BUILD_TESTS "Build and run tests" true )
option( COLOR "Use ANSI color output" true )

# Default prefix=/opt/slate
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set( CMAKE_INSTALL_PREFIX "/opt/slate"
         CACHE PATH
         "Install path prefix, prepended onto install directories."
         FORCE
    )
    message( STATUS "Setting CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
endif()

# Provide menu of options. (Why doesn't CMake do this?)
set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
              None Debug Release RelWithDebInfo MinSizeRel )

#-------------------------------------------------------------------------------
# Require build directory.
string( TOLOWER "${CMAKE_CURRENT_SOURCE_DIR}" source_dir_lower )
string( TOLOWER "${CMAKE_CURRENT_BINARY_DIR}" binary_dir_lower )
if (source_dir_lower STREQUAL binary_dir_lower)
    message( FATAL_ERROR
    "Compiling TestSweeper with CMake requires an out-of-source build. To proceed:
    rm -rf CMakeCache.txt CMakeFiles/   # delete files in ${CMAKE_CURRENT_SOURCE_DIR}
    mkdir build
    cd build
    cmake ..
    make"
    )
endif()

#-------------------------------------------------------------------------------
# Build library.
add_library(
    testsweeper
    testsweeper.cc
    version.cc
)

# Include directory.
# During build it's source dir; after install it's {prefix}/include.
target_include_directories(
    testsweeper
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)

# Get hg id.
# todo: replace with `git rev-parse --short HEAD`.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.hg")
    execute_process( COMMAND hg id -i ${CMAKE_CURRENT_SOURCE_DIR}
                     OUTPUT_VARIABLE testsweeper_id )
    string( STRIP "${testsweeper_id}" testsweeper_id )
    message( STATUS "testsweeper_id = ${testsweeper_id}" )
    target_compile_definitions(
        testsweeper PRIVATE TESTSWEEPER_ID="${testsweeper_id}" )
endif()

# Use and export -std=c++11; don't allow -std=gnu++11 extensions.
target_compile_features( testsweeper PUBLIC cxx_std_11 )
set_target_properties( testsweeper PROPERTIES CXX_EXTENSIONS false )

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.15)
    # Conditionally add -Wall. See CMake tutorial.
    set( gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>" )
    target_compile_options(
        testsweeper PRIVATE "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall>>" )
endif()

if (NOT COLOR)
    target_compile_definitions( testsweeper PUBLIC "NO_COLOR" )
endif()

#-------------------------------------------------------------------------------
# Install libtestsweeper, and add to testsweeperTargets.cmake
install(
    TARGETS testsweeper
    EXPORT testsweeperTargets
    LIBRARY DESTINATION "lib"  # no default before 3.14
)

# Install header file
install(
    FILES "testsweeper.hh"
    DESTINATION "include"
)

# Install <package>Targets.cmake
install(
    EXPORT testsweeperTargets
    DESTINATION "lib/testsweeper"
)

# Also export <package>Targets.cmake in build directory
export (
    EXPORT testsweeperTargets
    FILE "testsweeperTargets.cmake"
)

# Install <package>Config.cmake and <package>ConfigVersion.cmake,
# to enable find_package( <package> ).
include( CMakePackageConfigHelpers )
configure_package_config_file(
    "testsweeperConfig.cmake.in"
    "testsweeperConfig.cmake"
    INSTALL_DESTINATION "lib/testsweeper"
)
write_basic_package_version_file(
    "testsweeperConfigVersion.cmake"
    VERSION "${testsweeper_VERSION}"
    COMPATIBILITY AnyNewerVersion
)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/testsweeperConfig.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/testsweeperConfigVersion.cmake"
    DESTINATION "lib/testsweeper"
)

#-------------------------------------------------------------------------------
if (BUILD_TESTS)
    # Compile example.
    add_executable( example example.cc )
    target_link_libraries( example testsweeper )
    set_target_properties( example PROPERTIES CXX_EXTENSIONS false )

    enable_testing()

    add_test( NAME example COMMAND example )
    set_tests_properties(
        example PROPERTIES PASS_REGULAR_EXPRESSION "Usage:" )

    add_test( NAME example_foo COMMAND example foo )
    set_tests_properties(
        example_foo PROPERTIES PASS_REGULAR_EXPRESSION "All tests passed" )

    # Copy test script to build directory. Used in Jenkins.
    add_custom_command(
        TARGET testsweeper POST_BUILD
        COMMAND
        cp -a ${CMAKE_CURRENT_SOURCE_DIR}/test
              ${CMAKE_CURRENT_BINARY_DIR}/test
    )
endif()

#-------------------------------------------------------------------------------
# To make this more user friendly, add 'make lib' and 'make tester' targets.
add_custom_target( lib DEPENDS testsweeper )
add_custom_target( tester DEPENDS example )
