// Copyright (c) 2017-2023, University of Tennessee. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// This program is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef LIBTEST_HH
#define LIBTEST_HH

#include <string.h>
#include <assert.h>
#include <stdint.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <complex>
#include <type_traits>
#include <numeric>

// Version is updated by make_release.py; DO NOT EDIT.
// Version 2025.05.28
#define TESTSWEEPER_VERSION 20250528

namespace testsweeper {

int version();
const char* id();

extern double no_data_flag;

// -----------------------------------------------------------------------------
// ANSI color codes
extern const char *ansi_esc;
extern const char *ansi_red;
extern const char *ansi_green;
extern const char *ansi_blue;
extern const char *ansi_cyan;
extern const char *ansi_magenta;
extern const char *ansi_yellow;
extern const char *ansi_white;
extern const char *ansi_gray;
extern const char *ansi_bold;
extern const char *ansi_normal;

// -----------------------------------------------------------------------------
class QuitException: public std::exception
{
public:
    explicit QuitException()
        : std::exception()
    {}
};

// -----------------------------------------------------------------------------
void throw_error( const char* format, ... );


// =============================================================================
// Enums

// -----------------------------------------------------------------------------
enum class DataType {
    Integer       = 'i',
    Half          = 'h',
    Single        = 's',
    Double        = 'd',
    SingleComplex = 'c',
    DoubleComplex = 'z',
};

extern const char* DataType_help;

//----------------------------------------
/// Convert string to DataType enum. Accepts a variety of inputs:
///
/// { i,        int, integer            } => Integer
///
/// { h, r16,   half                    } => Half
/// { s, r32,   float, single           } => Float
/// { d, r64,   double                  } => Double
/// { c, c32,   complex<float>          } => FloatComplex
/// { z, c64,   complex<double>         } => DoubleComplex
///
/// Single letters come from traditional BLAS names (i, s, d, c, z)
/// and a few commonly accepted new ones (h, q).
/// The r32, c32, etc. come from XBLAS proposal,
/// https://bit.ly/blas-g2-proposal
///
/// @param[in] str
///     String value to convert.
///
/// @param[in] dummy
///     Dummy argument used to specify the return type for overloading.
///
inline void from_string( std::string const& str, DataType* val )
{
    std::string str_ = str;
    if      (str_ == "i"
          || str_ == "int"
          || str_ == "integer"       ) *val = DataType::Integer;
    else if (str_ == "h"
          || str_ == "r16"
          || str_ == "half"          ) *val = DataType::Half;
    else if (str_ == "s"
          || str_ == "r32"
          || str_ == "float"
          || str_ == "single"        ) *val = DataType::Single;
    else if (str_ == "d"
          || str_ == "r64"
          || str_ == "double"        ) *val = DataType::Double;
    else if (str_ == "c"
          || str_ == "c32"
          || str_ == "complex<float>"
          || str_ == "complex-float"
          || str_ == "complex-single") *val = DataType::SingleComplex;
    else if (str_ == "z"
          || str_ == "c64"
          || str_ == "complex<double>"
          || str_ == "complex-double") *val = DataType::DoubleComplex;
    else
        throw_error( "invalid datatype '%s'", str.c_str() );
}

//----------------------------------------
/// Convert DataType enum to C-style string representation.
/// Temporary common low-level implementation for to_string and datatype2str.
///
inline const char* to_c_string( DataType value )
{
    switch (value) {
        case DataType::Integer:       return "i";
        case DataType::Half:          return "h";
        case DataType::Single:        return "s";
        case DataType::Double:        return "d";
        case DataType::SingleComplex: return "c";
        case DataType::DoubleComplex: return "z";
    }
    throw_error( "invalid datatype" );
    return "?";
}

//--------------------
/// Convert DataType enum to C++ string representation.
///
inline std::string to_string( DataType value )
{
    return std::string( to_c_string( value ) );
}


//==============================================================================
// Utilities

//------------------------------------------------------------------------------
/// Use SFINAE to test for existence of from_string( string, T* ) function.
template <typename T>
class has_from_string
{
private:
    typedef T* T_ptr;

    /// Matches `from_string( string str, T* val )`; return type is void.
    template <typename T2>
    static auto test( std::string str, T2* val )
        -> decltype( from_string( str, val ) );

    /// Matches everything else; return type is int (something other than void).
    template <typename T2>
    static int test(...);

public:
    // True if `void from_string( string, T* )` exists.
    static constexpr bool value = std::is_same<
        void,
        decltype( test<T>( std::string(), T_ptr() ) )
    >::value;
};

//------------------------------------------------------------------------------
/// Use SFINAE to test for existence of to_string( T ) function.
/// This relies on ADL; it doesn't check the std namespace, so it won't
/// work for basic types (int, float, etc.) -- not sure how to do that.
template <typename T>
class has_to_string
{
private:
    /// Matches `to_string( T val )`; return type is string.
    template <typename T2>
    static auto test( T2 val )
        -> decltype( to_string( val ) );

    /// Matches everything else; return type is int (something other than string).
    template <typename T2>
    static int test(...);

public:
    // True if `std::string to_string( T val )` exists.
    static constexpr bool value = std::is_same<
        std::string,
        decltype( test<T>( T() ) )
    >::value;
};

// -----------------------------------------------------------------------------
int scan_range( const char **strp, int64_t *start, int64_t *end, int64_t *step );
int scan_range( const char **strp, double  *start, double  *end, double  *step );

void flush_cache( size_t cache_size );

// -----------------------------------------------------------------------------
/// For integers x >= 0, y > 0, returns ceil( x/y ).
/// For x == 0, this is 0.
/// @ingroup ceildiv
inline int64_t ceildiv( int64_t x, int64_t y )
{
    return (x + y - 1)/y;
}

/// For integers x >= 0, y > 0, returns x rounded up to multiple of y.
/// That is, ceil(x/y)*y.
/// For x == 0, this is 0.
/// This implementation does not assume y is a power of 2.
/// @ingroup ceildiv
inline int64_t roundup( int64_t x, int64_t y )
{
    return ceildiv( x, y ) * y;
}

//------------------------------------------------------------------------------
/// @return square of a number.
/// @ingroup sqr
///
template <typename T>
T sqr( T x )
{
    return x*x;
}

// -----------------------------------------------------------------------------
enum class ParamType
{
    Output,
    Value,
    List,
};

// =============================================================================
// class hierarchy
// ParamBase
//     TParamBase (template)
//         ParamInt
//             ParamOkay
//         ParamInt3
//         ParamDouble
//             ParamScientific
//         ParamChar
//         ParamEnum (template)

class ParamBase
{
public:
    friend class ParamsBase;

    //----------------------------------------
    /// Create parameter.
    ///
    /// @param[in] in_name
    ///     Parameter's name, used as header in output and command line option.
    ///     @see name() to override command line option.
    ///
    /// @param[in] width
    ///     Minimum width of column in output. If width=0, column is hidden.
    ///     Actual column width is max( width, length( name ) ).
    ///
    /// @param[in] type
    ///     Parameter type:
    ///     - Output: output-only parameter like time and gflops.
    ///     - Value:  single-valued parameter like check and ref.
    ///     - List:   parameter that can have multiple values for
    ///               TestSweeper to iterate over, like datatype.
    ///
    /// @param[in] help
    ///     Description of parameter to print with `tester -h routine`.
    ///
    ParamBase( const char* in_name, int width, ParamType type,
               const char* help ):
        help_   ( help ),
        index_  ( 0 ),
        width_  ( width ),
        type_   ( type ),
        is_default_( true ),
        used_   ( false )
    {
        name( in_name );
        s_params.push_back( this );
    }

    virtual ~ParamBase()
    {
        auto iter = std::find( s_params.begin(), s_params.end(), this );
        if (iter != s_params.end())
            s_params.erase( iter );
    }

    virtual void parse( const char* str ) = 0;
    virtual void print() const = 0;
    virtual void reset_output() = 0;
    virtual void header( int line ) const;
    virtual void help() const;
    virtual bool next();
    virtual size_t size() const = 0;

    bool used() const { return used_; }
    void used( bool in_used ) { used_ = in_used; }

    //----------------------------------------
    /// Set parameter's name.
    ///
    /// @param[in] in_name
    ///     Parameter's name, used as header in output.
    ///     Increases the column width to accomodate the new name as needed,
    ///     if the width is not zero.
    ///
    /// @param[in] in_option
    ///     If not null, used as command line option to set paramater.
    ///     If null, in_name is used as command line option.
    ///
    void name( const char* in_name, const char* in_option=nullptr )
    {
        name_ = in_name;
        if (width_ > 0) {
            int line1 = 0;
            while (in_name[ line1 ] != '\0' && in_name[ line1 ] != '\n') {
                ++line1;
            }
            int line2 = line1;
            while (in_name[ line2 ] != '\0') {
                ++line2;
            }
            line2 = line2 - line1 - 1;
            width_ = std::max( width_, std::max( line1, line2 ) );
        }
        if (in_option)
            option_ = "--" + std::string(in_option);
        else
            option_ = "--" + name_;
    }

    /// @return Parameter's name.
    std::string const& name() const
    {
        return name_;
    }

    int  width() const { return width_; }
    void width( int w ) { width_ = w; }

protected:
    // s_params is list of ParamBase objects, used by Params class
    static std::vector< ParamBase* > s_params;

    std::string name_;
    std::string option_;
    std::string help_;
    size_t      index_;
    int         width_;
    ParamType   type_;
    bool        is_default_;
    int         used_;
};

// =============================================================================
template <typename T>
class TParamBase : public ParamBase
{
public:
    TParamBase( const char* name, int width, ParamType type, T default_value,
                const char* help ):
        ParamBase( name, width, type, help ),
        default_value_( default_value )
    {
        values_.push_back( default_value );
    }

    virtual size_t size() const
    {
        return values_.size();
    }

    T& operator ()()
    {
        used_ = true;
        return values_[ index_ ];
    }

    T const& operator ()() const
    {
        return values_[ index_ ];
    }

    void operator ()( T const& value )
    {
        used_ = true;
        values_[ index_ ] = value;
    }

    void set_default( const T& default_value )
    {
        default_value_ = default_value;
        if (is_default_) {
            values_.clear();
            values_.push_back( default_value );
        }
    }

    virtual void reset_output()
    {
        if (type_ == ParamType::Output) {
            values_[0] = default_value_;
        }
    }

    void push_back( T val );

protected:
    std::vector< T > values_;
    T default_value_;
};

// -----------------------------------------------------------------------------
template <typename T>
void TParamBase<T>::push_back( T val )
{
    if (type_ == ParamType::List) {
        if (is_default_) {
            values_.clear();
            is_default_ = false;
        }
        values_.push_back( val );
    }
    else {
        values_[0] = val;
    }
}

// =============================================================================
class ParamInt : public TParamBase< int64_t >
{
public:
    friend class Params;

    ParamInt( const char* name, int width, ParamType type,
              int64_t default_value, int64_t min_value, int64_t max_value,
              const char* help ):
        TParamBase( name, width, type, default_value, help ),
        min_value_( min_value ),
        max_value_( max_value )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;
    void push_back( int64_t val );

protected:
    int64_t min_value_;
    int64_t max_value_;
};

// =============================================================================
// same as ParamInt, but prints ok (for non-zero) or FAILED (for zero).
class ParamOkay : public ParamInt
{
public:
    ParamOkay( const char* name, int width, ParamType type,
               int64_t default_value, int64_t min_value, int64_t max_value,
               const char* help ):
        ParamInt( name, width, type,
                  default_value, min_value, max_value, help )
    {}

    virtual void print() const;
};

const int no_check = -1;
const int skipped  = -2;

// =============================================================================
typedef struct { int64_t m, n, k; } int3_t;

class ParamInt3 : public TParamBase< int3_t >
{
public:
    friend class Params;

    enum {
        m_mask = 0x1,
        n_mask = 0x2,
        k_mask = 0x4,
    };

    /// default range 100 : 500 : 100
    ParamInt3( const char* name, int width, ParamType type,
               int64_t min_value, int64_t max_value,
               const char* help ):
        TParamBase<int3_t>( name, width, type, int3_t(), help ),
        min_value_( min_value ),
        max_value_( max_value ),
        m_name_("m"),
        n_name_("n"),
        k_name_("k")
    {
        values_.clear();
        for (int64_t i = 100; i <= 500; i += 100) {
            int3_t tmp = { i, i, i };
            values_.push_back( tmp );
        }
    }

    /// application gives default range as string
    ParamInt3( const char* name, int width, ParamType type,
               const char* default_value,
               int64_t min_value, int64_t max_value,
               const char* help ):
        TParamBase<int3_t>( name, width, type, int3_t(), help ),
        min_value_( min_value ),
        max_value_( max_value ),
        m_name_("m"),
        n_name_("n"),
        k_name_("k")
    {
        values_.clear();
        parse( default_value );
        is_default_ = true;
    }

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void header( int line ) const;
    void push_back( int3_t val );

    int64_t& m()
    {
        used_ |= m_mask;
        return values_[ index_ ].m;
    }

    int64_t& n()
    {
        used_ |= n_mask;
        return values_[ index_ ].n;
    }

    int64_t& k()
    {
        used_ |= k_mask;
        return values_[ index_ ].k;
    }

    /// Sets the names of the header values.
    void names(const char* m, const char* n, const char* k="")
    {
        m_name_ = m;
        n_name_ = n;
        k_name_ = k;
    }

protected:
    int64_t min_value_;
    int64_t max_value_;
    std::string m_name_;
    std::string n_name_;
    std::string k_name_;
};

//------------------------------------------------------------------------------
// Derived from BLAS++.

/// For real scalar types.
template <typename real_t>
struct MakeScalarTraits {
    static real_t make( real_t re, real_t im )
        { return re; }
};

/// For complex scalar types.
template <typename real_t>
struct MakeScalarTraits< std::complex<real_t> > {
    static std::complex<real_t> make( real_t re, real_t im )
        { return std::complex<real_t>( re, im ); }
};

/// Converts complex value into scalar_t,
/// discarding imaginary part if scalar_t is real.
template <typename scalar_t>
scalar_t make_scalar( std::complex<double> val )
{
    return MakeScalarTraits<scalar_t>::make( std::real(val), std::imag(val) );
}

// =============================================================================
class ParamComplex : public TParamBase< std::complex<double> >
{
public:
   friend class Params;
   ParamComplex( const char* name, int width, int precision, ParamType type,
               const char* default_value,
               double min_value, double max_value,
               const char* help ):
        // Compute width to account for
        // initial -, [+-] between real & complex parts, and i at end.
        TParamBase< std::complex<double> >(
            name, 2*width + 3, type, std::complex<double>(), help ),
        display_width_( width ),
        precision_( precision ),
        min_value_( min_value ),
        max_value_( max_value )
    {
        values_.clear();
        parse( default_value );
        default_value_ = values_[ 0 ];
        is_default_ = true;
    }

    template <typename T>
    T get() {
        used_ = true;
        return make_scalar<T>( values_[ index_ ] );
    }
    std::complex<double> scan_complex( const char** str );
    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;

protected:
    int display_width_;
    int precision_;
    double min_value_;
    double max_value_;
};


// =============================================================================
class ParamDouble : public TParamBase< double >
{
public:
    ParamDouble( const char* name, int width, int precision, ParamType type,
                 double default_value, double min_value, double max_value,
                 const char* help ):
        TParamBase( name, width, type, default_value, help ),
        precision_( precision ),
        min_value_( min_value ),
        max_value_( max_value )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;
    void push_back( double val );

protected:
    int precision_;
    double min_value_;
    double max_value_;
};

// =============================================================================
// same as ParamDouble, but prints using scientific notation (%e)
class ParamScientific : public ParamDouble
{
public:
    ParamScientific( const char* name, int width, int precision, ParamType type,
                     double default_value, double min_value, double max_value,
                     const char* help ):
        ParamDouble( name, width, precision, type,
                     default_value, min_value, max_value, help )
    {}

    virtual void print() const;
    virtual void help() const;
};

// =============================================================================
class ParamChar : public TParamBase< char >
{
public:
    ParamChar( const char* name, int width, ParamType type,
               char default_value, const char* valid,
               const char* help ):
        TParamBase( name, width, type, default_value, help ),
        valid_( valid )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;
    void push_back( char val );

protected:
    std::string valid_;
};

// =============================================================================
class ParamString : public TParamBase< std::string >
{
public:
    ParamString( const char* name, int width, ParamType type,
                 const char* default_value,
                 const char* help ):
        TParamBase( name, width, type, default_value, help )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void header( int line ) const;
    virtual void help() const;
    void push_back( const char* str );
    void add_valid( const char* str );
    bool is_valid( const std::string& str );

protected:
    std::vector< std::string > valid_;
};

// =============================================================================
template <typename ENUM>
class ParamEnum : public TParamBase< ENUM >
{
public:
    /// Constructor for enums that have to_string and from_string.
    ParamEnum( const char* name, int width, ParamType type,
               ENUM default_value,
               const char* help ):
        TParamBase<ENUM>( name, width, type, default_value, help )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;
};

// -----------------------------------------------------------------------------
// virtual
template <typename ENUM>
void ParamEnum<ENUM>::parse( const char *str )
{
    char buf[81] = { 0 };
    while (true) {
        // Read next word, up to 80 chars.
        int len;
        int i = sscanf( str, " %80[a-zA-Z0-9_<>-] %n", buf, &len );
        if (i != 1) {
            throw_error( "invalid argument at '%s'", str );
        }
        str += len;
        // Parse word into enum.
        ENUM val{};
        from_string( buf, &val );
        this->push_back( val );
        if (*str == '\0') {
            break;
        }
        if (*str != ',' && *str != ';') {
            throw_error( "invalid argument at '%s', expected comma", str );
        }
        str += 1;
    }
}

// -----------------------------------------------------------------------------
// virtual
template <typename ENUM>
void ParamEnum<ENUM>::print() const
{
    if (this->used_ && this->width_ > 0) {
        printf( "%*s  ", this->width_,
                to_string( this->values_[ this->index_ ] ).c_str() );
    }
}

// -----------------------------------------------------------------------------
// virtual
template <typename ENUM>
void ParamEnum<ENUM>::help() const
{
    if (this->type_ == ParamType::Value || this->type_ == ParamType::List) {
        printf( "    %-16s %s; default %s\n",
                this->option_.c_str(), this->help_.c_str(),
                to_string( this->default_value_ ).c_str() );
    }
}

// =============================================================================
class ParamsBase
{
public:
    ParamsBase() {}

    void parse( const char* routine, int n, char** args );
    bool next();
    void header();
    void print();
    void reset_output();
    void help( const char* routine );
};

//------------------------------------------------------------------------------
/// If paramater is used, print min, max, avg, stddev of data.
/// If data has NaN, whether min and max are NaN is implementation defined.
///
/// @param[in] param  Paramater to summarize.
/// @param[in] data   Data to summarize.
///
template <typename T>
void print_stats( ParamBase const& param, std::vector<T> const& data )
{
    if (param.used()) {
        T min = *std::min_element( data.begin(), data.end() );
        T max = *std::max_element( data.begin(), data.end() );
        T avg = std::accumulate( data.begin(), data.end(), 0.0 ) / data.size();

        T ssq = 0;
        for (double x : data) {
            ssq += sqr( x - avg );
        }
        T stddev = sqrt( ssq / (data.size() - 1) );

        printf( "%-16s min %#9.4g, max %#9.4g, avg %#9.4g, stddev %#9.4g\n",
                param.name().c_str(), min, max, avg, stddev );
    }
}

}  // namespace testsweeper

// =============================================================================
// main must define Params class (which is not in testsweeper namespace),
// as a subclass of ParamsBase
class Params;

namespace testsweeper {

typedef void (*test_func_ptr)( Params& params, bool run );

typedef struct {
    const char* name;
    test_func_ptr func;
    int section;
} routines_t;

test_func_ptr find_tester(
    const char *name,
    std::vector< routines_t >& routines );

void usage(
    int argc, char **argv,
    std::vector< routines_t >& routines,
    const char **section_names,
    int col_width=18, int ncols=4 );

double get_wtime();

}  // namespace testweeper

#endif        //  #ifndef LIBTEST_HH
