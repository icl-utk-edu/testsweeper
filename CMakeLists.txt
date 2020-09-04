# CMake script for TestSweeper library
# repo: http://bitbucket.org/icl/testsweeper
# needed to build blaspp/test, lapackpp/test, slate/test

cmake_minimum_required( VERSION 3.8 )
# 3.1  target_compile_features
# 3.8  target_compile_features( cxx_std_11 )
# 3.14 install( LIBRARY DESTINATION lib ) default
# 3.15 $<$COMPILE_LANG_AND_ID  # optional

project(
    testsweeper
    VERSION 2020.06.00
    LANGUAGES CXX
)

#-------------------------------------------------------------------------------
# Options
option( BUILD_SHARED_LIBS "Build shared libraries" true )
option( build_tests "Build test suite" true )
option( color "Use ANSI color output" true )
option( use_openmp "Use OpenMP, if available" true )
option( testsweeper_install "Add install target" true )

# Default prefix=/opt/slate
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set( CMAKE_INSTALL_PREFIX "/opt/slate"
         CACHE PATH
         "Install path prefix, prepended onto install directories."
         FORCE
    )
    message( STATUS "Setting CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
    # Append the new CMAKE_INSTALL_PREFIX, since CMake appended the old value.
    list( APPEND CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_INSTALL_PREFIX} )
else()
    message( STATUS "Using CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
endif()

# Provide menu of options. (Why doesn't CMake do this?)
set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
              None Debug Release RelWithDebInfo MinSizeRel )

# When built as a sub-project, need to add a namespace to make common
# targets unique, e.g., `make lib` becomes `make testsweeper_lib`.
if (NOT "${CMAKE_PROJECT_NAME}" STREQUAL "testsweeper")
    set( testsweeper_ "testsweeper_" )
endif()

#-------------------------------------------------------------------------------
# Enforce out-of-source build
string( TOLOWER "${CMAKE_CURRENT_SOURCE_DIR}" source_dir )
string( TOLOWER "${CMAKE_CURRENT_BINARY_DIR}" binary_dir )
if ("${source_dir}" STREQUAL "${binary_dir}")
    message( FATAL_ERROR
    "Compiling TestSweeper with CMake requires an out-of-source build. To proceed:
    rm -rf CMakeCache.txt CMakeFiles/   # delete files in ${CMAKE_CURRENT_SOURCE_DIR}
    mkdir build
    cd build
    cmake ..
    make" )
endif()

#-------------------------------------------------------------------------------
# Build library.
add_library(
    testsweeper
    testsweeper.cc
    version.cc
)

# Include directory.
# During build it's {source}; after install it's {prefix}/include.
target_include_directories(
    testsweeper
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)

# OpenMP support.
if (NOT use_openmp)
    message( STATUS "User has requested to NOT use OpenMP" )
else()
    find_package( OpenMP )
    if (OpenMP_CXX_FOUND)
        target_link_libraries( testsweeper PUBLIC OpenMP::OpenMP_CXX )
    endif()
endif()

# Get git commit id.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process( COMMAND git rev-parse --short HEAD
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
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

if (NOT color)
    target_compile_definitions( testsweeper PUBLIC "NO_COLOR" )
endif()

#-------------------------------------------------------------------------------
# When TestSweeper is used as a sub-project,
# the parent (e.g., BLAS++) may not want to install it.
if (testsweeper_install)
    # GNU Filesystem Convensions
    include( GNUInstallDirs )
    set( INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/testsweeper )

    # Install library and add to <package>Targets.cmake
    install(
        TARGETS testsweeper
        EXPORT testsweeperTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}  # no default before 3.14
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # Install header files
    install(
        FILES "testsweeper.hh"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install <package>Targets.cmake
    install(
        EXPORT testsweeperTargets
        DESTINATION ${INSTALL_CONFIGDIR}
    )

    # Also export <package>Targets.cmake in build directory
    export(
        EXPORT testsweeperTargets
        FILE "testsweeperTargets.cmake"
    )

    # Install <package>Config.cmake and <package>ConfigVersion.cmake,
    # to enable find_package( <package> ).
    include( CMakePackageConfigHelpers )
    configure_package_config_file(
        "testsweeperConfig.cmake.in"
        "testsweeperConfig.cmake"
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
    )
    write_basic_package_version_file(
        "testsweeperConfigVersion.cmake"
        VERSION "${testsweeper_VERSION}"
        COMPATIBILITY AnyNewerVersion
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/testsweeperConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/testsweeperConfigVersion.cmake"
        DESTINATION ${INSTALL_CONFIGDIR}
    )
endif()

#-------------------------------------------------------------------------------
if (build_tests)
    # Compile example.
    set( example ${testsweeper_}example )
    add_executable( ${example} example.cc )
    target_link_libraries( ${example} testsweeper )
    set_target_properties( ${example} PROPERTIES CXX_EXTENSIONS false )

    enable_testing()

    add_test( NAME ${example} COMMAND ${example} )
    set_tests_properties(
        ${example} PROPERTIES PASS_REGULAR_EXPRESSION "Usage:" )

    add_test( NAME ${example}_foo COMMAND ${example} foo )
    set_tests_properties(
        ${example}_foo PROPERTIES PASS_REGULAR_EXPRESSION "All tests passed" )

    # Copy test script to build directory. Used in Jenkins.
    add_custom_command(
        TARGET testsweeper POST_BUILD
        COMMAND
        cp -a ${CMAKE_CURRENT_SOURCE_DIR}/test
              ${CMAKE_CURRENT_BINARY_DIR}/test
    )
endif()

#-------------------------------------------------------------------------------
# Add 'make lib' and 'make tester' targets.
add_custom_target( ${testsweeper_}lib DEPENDS testsweeper )
add_custom_target( ${testsweeper_}tester DEPENDS ${example} )
