# CMake script for testsweeper framework
# needed to build BLASPP/test, LAPACK/test, SLATE/test
# repo: http://bitbucket.org/icl/testsweeper

cmake_minimum_required (VERSION 3.2)

project (TESTSWEEPER
    LANGUAGES CXX
)

option(BUILD_TESTS "build and run tests" ON)

string (TOLOWER "${CMAKE_CURRENT_SOURCE_DIR}" SOURCE_DIR_LOWER)
string (TOLOWER "${CMAKE_CURRENT_BINARY_DIR}" BINARY_DIR_LOWER )
if (SOURCE_DIR_LOWER STREQUAL BINARY_DIR_LOWER)
    message (FATAL_ERROR "Compiling TESTSWEEPER with CMake requires an out-of-source build. To proceed:
    rm -rf CMakeCache.txt CMakeFiles/   # delete files in ${CMAKE_CURRENT_SOURCE_DIR}
    mkdir build
    cd build
    cmake ..
    make")
endif ()

if (CMAKE_HOST_APPLE)
    set (CMAKE_MACOSX_RPATH 1)
endif()

add_library (testsweeper SHARED
    testsweeper.cc
    version.cc
)

target_include_directories (testsweeper PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

execute_process (COMMAND hg id -i OUTPUT_VARIABLE testsweeper_id)
string (STRIP ${testsweeper_id} testsweeper_id)

target_compile_definitions(testsweeper PUBLIC
    TESTSWEEPER_ID="${testsweeper_id}"
)

set_target_properties (testsweeper PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

if(NO_COLOR)
    target_compile_definitions(testsweeper PUBLIC
        NO_COLOR
    )
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/opt/slate"
        CACHE PATH
        "Install path prefix, prepended onto install directories."
        FORCE
        )
endif()

#message("cmake install prefix: " ${CMAKE_INSTALL_PREFIX})
install (TARGETS testsweeper
    EXPORT testsweeperTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

install (
    EXPORT testsweeperTargets
    DESTINATION lib/testsweeper
)

install(
    FILES testsweeper.hh
    DESTINATION include
)

configure_file(testsweeperConfig.cmake.in
    testsweeperConfig.cmake
    COPYONLY
)

install (
    FILES ${CMAKE_CURRENT_BINARY_DIR}/testsweeperConfig.cmake
    DESTINATION lib/testsweeper
)

export (
    EXPORT testsweeperTargets
    FILE testsweeperTargets.cmake
)

if(BUILD_TESTS)
    add_executable(example example.cc)
    set_target_properties (example PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
    target_include_directories(example PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(example testsweeper)

    enable_testing()

    add_test(NAME run_test
        COMMAND "example"
    )
    set_tests_properties(run_test
        PROPERTIES
            PASS_REGULAR_EXPRESSION
                "(Usage:)"
    )
    add_test(NAME run_test_foo
        COMMAND "example" "foo"
    )
    set_tests_properties(run_test_foo
        PROPERTIES
            PASS_REGULAR_EXPRESSION
                "(All tests passed)"
        )
endif(BUILD_TESTS)
