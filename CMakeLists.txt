# CMake script for libtest framerwork
# needed to build BLASPP/test, LAPACK/test, SLATE/test
# repo: http://bitbucket.org/icl/libtest

cmake_minimum_required (VERSION 3.2)

project (LIBTEST
    LANGUAGES CXX
)

option(BUILD_TESTS "build and run tests" ON)

string (TOLOWER "${CMAKE_CURRENT_SOURCE_DIR}" SOURCE_DIR_LOWER)
string (TOLOWER "${CMAKE_CURRENT_BINARY_DIR}" BINARY_DIR_LOWER )
if (SOURCE_DIR_LOWER STREQUAL BINARY_DIR_LOWER)
    message (FATAL_ERROR "Compiling LIBTEST with CMake requires an out-of-source build. To proceed:
    rm -rf CMakeCache.txt CMakeFiles/   # delete files in ${CMAKE_CURRENT_SOURCE_DIR}
    mkdir build
    cd build
    cmake ..
    make")
endif ()

if (CMAKE_HOST_APPLE)
    set (CMAKE_MACOSX_RPATH 1)
endif()

add_library (libtest
    libtest.cc
)

target_include_directories (libtest PUBLIC
    #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${CMAKE_CURRENT_SOURCE_DIR}
    #$<INSTALL_INTERFACE:include>
)

set_target_properties (libtest PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories (libtest PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/opt/slate"
        CACHE PATH
        "Install path prefix, prepended onto install directories."
        FORCE
        )
endif()
message("cmake install prefix: " ${CMAKE_INSTALL_PREFIX})
install (TARGETS libtest
    EXPORT libtestTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/test/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/test/lib
    INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}/test/include
)

install(
    FILES libtest.hh
    DESTINATION ${CMAKE_INSTALL_PREFIX}/test/include
)

export(EXPORT libtestTargets
    FILE ${CMAKE_INSTALL_PREFIX}/test/libtestTargets.cmake
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libtestConfig.cmake.in
    ${CMAKE_INSTALL_PREFIX}/test/libtestConfig.cmake
    COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libtestConfig.cmake.in
    ${CMAKE_INSTALL_PREFIX}/test/libtestConfig.cmake
    COPYONLY
    )

if(BUILD_TESTS)
    add_executable(testExample example.cc)
    set_target_properties (testExample PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
    target_include_directories(testExample PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(testExample libtest)

    enable_testing()

    add_test(NAME run_test
        COMMAND "testExample"
    )
    set_tests_properties(run_test
        PROPERTIES
            PASS_REGULAR_EXPRESSION
                "(Usage:)")
    add_test(NAME run_test_foo
        COMMAND "testExample" "foo"
    )
    set_tests_properties(run_test_foo
        PROPERTIES
            PASS_REGULAR_EXPRESSION
                "(All tests passed)"
        )
endif(BUILD_TESTS)
