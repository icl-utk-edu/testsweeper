#ifndef EXAMPLE_HH
#define EXAMPLE_HH

#include "libtest.hh"

// -----------------------------------------------------------------------------
class Params: public libtest::ParamsBase
{
public:
    const double nan = std::numeric_limits<double>::quiet_NaN();

    Params();

    // Field members are explicitly public.
    // Order here determines output order.
    // ----- test framework parameters
    libtest::ParamChar   check;
    libtest::ParamChar   ref;
    libtest::ParamDouble tol;
    libtest::ParamInt    repeat;
    libtest::ParamInt    verbose;
    libtest::ParamInt    cache;

    // ----- routine parameters
    libtest::ParamEnum< libtest::DataType > datatype;
    libtest::ParamInt3   dim;

    // ----- output parameters
    libtest::ParamScientific error;
    libtest::ParamScientific ortho;
    libtest::ParamDouble     time;
    libtest::ParamDouble     gflops;

    libtest::ParamScientific ref_error;
    libtest::ParamScientific ref_ortho;
    libtest::ParamDouble     ref_time;
    libtest::ParamDouble     ref_gflops;

    libtest::ParamOkay       okay;
};

// -----------------------------------------------------------------------------
// Level 1
void test_foo( Params& params, bool run );

// -----------------------------------------------------------------------------
// Level 2
void test_bar( Params& params, bool run );

// -----------------------------------------------------------------------------
// Level 3
void test_baz( Params& params, bool run );

#endif  //  #ifndef EXAMPLE_HH
