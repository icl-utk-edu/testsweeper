// Copyright (c) 2017-2023, University of Tennessee. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// This program is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#include <complex>
#include <cstdint>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "test.hh"

// -----------------------------------------------------------------------------
using testsweeper::ParamType;
using testsweeper::DataType;
using testsweeper::DataType_help;

#ifdef DEPRECATED
using testsweeper::char2datatype;
using testsweeper::datatype2char;
using testsweeper::str2datatype;
using testsweeper::datatype2str;
#endif

using testsweeper::ansi_bold;
using testsweeper::ansi_red;
using testsweeper::ansi_normal;

const ParamType PT_Value = ParamType::Value;
const ParamType PT_List  = ParamType::List;
const ParamType PT_Out   = ParamType::Output;

const double no_data = testsweeper::no_data_flag;
const char*  pi_rt2i = "3.141592653589793 + 1.414213562373095i";
const char*  e_rt3i  = "2.718281828459045 + 1.732050807568877i";
const double pi      = 3.141592653589793;
const double e       = 2.718281828459045;

// -----------------------------------------------------------------------------
// each section must have a corresponding entry in section_names
enum Section {
    newline = 0,  // zero flag forces newline
    level1,
    level2,
    level3,
    num_sections,  // last
};

const char* section_names[] = {
   "",  // none
   "Level 1",
   "Level 2",
   "Level 3",
};

// { "", nullptr, Section::newline } entries force newline in help
std::vector< testsweeper::routines_t > routines = {
    // Level 1
    { "sort",   test_sort,      Section::level1 },
    { "sort2",  test_sort,      Section::level1 },
    { "sort3",  test_sort,      Section::level1 },
    { "sort4",  test_sort,      Section::level1 },
    { "sort5",  test_sort,      Section::level1 },
    { "sort6",  test_sort,      Section::level1 },
    { "sort7",  test_sort,      Section::level1 },
    { "sort8",  test_sort,      Section::level1 },

    // Level 2
    { "bar",    test_bar,       Section::level2 },
    { "bar2",   test_bar,       Section::level2 },
    { "bar3",   test_bar,       Section::level2 },
    { "",       nullptr,        Section::newline },

    { "bar4",   test_bar,       Section::level2 },
    { "bar5",   test_bar,       Section::level2 },
    { "bar6",   test_bar,       Section::level2 },

    // Level 3
    { "baz",    test_baz,       Section::level3 },
    { "baz2",   test_baz,       Section::level3 },
    { "baz3",   test_baz,       Section::level3 },
    { "",       nullptr,        Section::newline },

    { "baz4",   test_baz,       Section::level3 },
    { "baz5",   test_baz,       Section::level3 },
};

// -----------------------------------------------------------------------------
// Params class
// List of parameters

Params::Params():
    ParamsBase(),

    // w = width
    // p = precision
    //----- test framework parameters
    //          name,         w, type, default, valid, help
    check     ( "check",      0, PT_Value, 'y', "ny", "check the results" ),
    ref       ( "ref",        0, PT_Value, 'n', "ny", "run reference; sometimes check implies ref" ),

    //          name,         w, p, type, default,  min,  max, help
    tol       ( "tol",        0, 0, PT_Value,  50,    1, 1000, "tolerance (e.g., error < tol*epsilon to pass)" ),
    repeat    ( "repeat",     0,    PT_Value,   1,    1, 1000, "times to repeat each test" ),
    verbose   ( "verbose",    0,    PT_Value,   0,    0,   10, "verbose level" ),
    cache     ( "cache",      0,    PT_Value,  20,    1, 1024, "total cache size, in MiB" ),

    //----- routine parameters, enums
    #ifdef DEPRECATED
    //      name,             w, type, default; char2enum, enum2char, enum2str, help
    datatype_old
              ( "type-old",       4, PT_List,   DataType::Double,
                char2datatype, datatype2char, datatype2str, DataType_help ),

    //      name,             w, type, default; str2enum, enum2str, help
    datatype_old2
              ( "type-old2",      4, PT_List,   DataType::Double,
                str2datatype, datatype2str, DataType_help ),
    #endif

    //          name,         w, type,    default, help
    datatype  ( "type",       4, PT_List, DataType::Double, DataType_help ),

    //----- routine parameters, numeric
    //          name,         w, p, type,    default,  min,  max, help
    dim       ( "dim",        6,    PT_List,             0, 1e10, "m by n by k dimensions" ),
    nb        ( "nb",         4,    PT_List,     384,    0,  1e6, "block size" ),
    alpha     ( "alpha",      3, 1, PT_List, pi_rt2i, -inf,  inf, "scalar alpha" ),
    beta      ( "beta",       3, 1, PT_List,       e, -inf,  inf, "scalar beta" ),
    grid      ( "grid",       3,    PT_List,   "1x1",    0,  1e6, "MPI grid p by q dimensions" ),

    //----- output parameters
    // min, max are ignored
    // error:   %8.2e allows 9.99e-99
    // time:    %9.3f allows 99999.999 s = 2.9 days
    // gflops: %12.3f allows 99999999.999 Gflop/s = 100 Pflop/s
    //          name,         w, p, type,   default, min, max, help
    error     ( "error",      8, 2, PT_Out, no_data, 0, 0, "numerical error" ),
    ortho     ( "orth.",      8, 2, PT_Out, no_data, 0, 0, "orthogonality error" ),
    time      ( "time (s)",   9, 3, PT_Out, no_data, 0, 0, "time to solution" ),
    gflops    ( "Gflop/s",   12, 3, PT_Out, no_data, 0, 0, "Gflop/s rate" ),

    ref_time  ( "ref time (s)",  9, 3, PT_Out, no_data, 0, 0, "reference time to solution" ),
    ref_gflops( "ref Gflop/s",  12, 3, PT_Out, no_data, 0, 0, "reference Gflop/s rate" ),

    // default -1 means "no check"
    //          name,         w, type, default, min, max, help
    okay      ( "status",     6, PT_Out,    -1, 0, 0, "success indicator" ),
    msg       ( "",           1, PT_Out,    "",       "error message" )
{
    // mark standard set of output fields as used
    okay();
    error();
    time();

    // mark framework parameters as used, so they will be accepted on the command line
    check();
    tol();
    repeat();
    verbose();
    cache();

    // routine's parameters are marked by the test routine; see main
}

// -----------------------------------------------------------------------------
int main( int argc, char** argv )
{
    using testsweeper::QuitException;

    // These may or may not be used; mark unused to silence warnings.
    #define unused( var ) ((void)var)
    unused( pi_rt2i );
    unused( e_rt3i  );
    unused( pi      );
    unused( e       );

    // check that all sections have names
    assert( sizeof(section_names)/sizeof(*section_names) == Section::num_sections );

    int status = 0;
    try {
        int version = testsweeper::version();
        printf( "TestSweeper version %d.%02d.%02d, id %s\n",
                version / 10000, (version % 10000) / 100, version % 100,
                testsweeper::id() );

        // print input so running `test [input] > out.txt` documents input
        printf( "input: %s", argv[0] );
        for (int i = 1; i < argc; ++i) {
            // quote arg if necessary
            std::string arg( argv[i] );
            const char* wordchars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-=";
            if (arg.find_first_not_of( wordchars ) != std::string::npos)
                printf( " '%s'", argv[i] );
            else
                printf( " %s", argv[i] );
        }
        printf( "\n" );

        // Usage: test [params] routine
        if (argc < 2
            || strcmp( argv[argc-1], "-h" ) == 0
            || strcmp( argv[argc-1], "--help" ) == 0)
        {
            usage( argc, argv, routines, section_names );
            throw QuitException();
        }

        // find routine to test
        const char* routine = argv[ argc-1 ];
        testsweeper::test_func_ptr test_routine = find_tester( routine, routines );
        if (test_routine == nullptr) {
            usage( argc, argv, routines, section_names );
            throw std::runtime_error(
                std::string("routine ") + routine + " not found" );
        }

        // mark fields that are used (run=false)
        Params params;
        test_routine( params, false );

        // parse parameters up to routine name
        try {
            params.parse( routine, argc-2, argv+1 );
        }
        catch (const std::exception& ex) {
            params.help( routine );
            throw;
        }

        // run tests
        int repeat = params.repeat();
        std::vector<double> times( repeat ), ref_times( repeat ),
                            gflops( repeat ), ref_gflops( repeat );
        testsweeper::DataType last = params.datatype();
        params.header();
        do {
            if (params.datatype() != last) {
                last = params.datatype();
                printf( "\n" );
            }
            for (int iter = 0; iter < repeat; ++iter) {
                try {
                    test_routine( params, true );
                }
                catch (const std::exception& ex) {
                    fprintf( stderr, "%s%sError: %s%s\n",
                             ansi_bold, ansi_red, ex.what(), ansi_normal );
                    params.okay() = false;
                }

                // Collect stats.
                times     [ iter ] = params.time();
                gflops    [ iter ] = params.gflops();
                ref_times [ iter ] = params.ref_time();
                ref_gflops[ iter ] = params.ref_gflops();

                params.print();
                status += ! params.okay();
                params.reset_output();
            }
            if (repeat > 1) {
                testsweeper::print_stats( params.time,       times      );
                testsweeper::print_stats( params.ref_time,   ref_times  );
                testsweeper::print_stats( params.gflops,     gflops     );
                testsweeper::print_stats( params.ref_gflops, ref_gflops );
                printf( "\n" );
            }
        } while( params.next() );

        if (status) {
            printf( "%d tests FAILED.\n", status );
        }
        else {
            printf( "All tests passed.\n" );
        }
    }
    catch (const QuitException& ex) {
        // pass: no error to print
    }
    catch (const std::exception& ex) {
        fprintf( stderr, "\n%s%sError: %s%s\n",
                 ansi_bold, ansi_red, ex.what(), ansi_normal );
        status = -1;
    }

    return status;
}
