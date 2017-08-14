#ifndef TEST_HH
#define TEST_HH

#include <string.h>
#include <assert.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

namespace libtest {

// -----------------------------------------------------------------------------
// ANSI color codes
extern const char *ansi_esc;
extern const char *ansi_red;
extern const char *ansi_bold;
extern const char *ansi_normal;

// -----------------------------------------------------------------------------
enum class DataType {
    Single        = 's',
    Double        = 'd',
    SingleComplex = 'c',
    DoubleComplex = 'z',
};

// Accepts: s (single/float), d (double), c (complex-single), z (complex-double)
inline DataType char2datatype( char ch )
{
    ch = tolower( ch );
    if (ch != 's' && ch != 'd' && ch != 'c' && ch != 'z')
        throw std::runtime_error( "invalid value for datatype" );
    return DataType( ch );
}

inline char datatype2char( DataType en ) { return char( en ); }

inline const char* datatype2str( DataType en )
{
    switch (en) {
        case DataType::Single:        return "s";
        case DataType::Double:        return "d";
        case DataType::SingleComplex: return "c";
        case DataType::DoubleComplex: return "z";
    }
    return "";
}

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

// -----------------------------------------------------------------------------
enum class ParamType
{
    Output,
    Value,
    List,
};

// =============================================================================
class ParamBase
{
public:
    friend class ParamsBase;

    ParamBase( const char* name, int width, ParamType type,
               const char* help ):
        m_name   ( name ),
        m_prefix ( std::string("--") + name ),
        m_help   ( help ),
        m_index  ( 0 ),
        m_width  ( width ),
        m_type   ( type ),
        m_is_default( true ),
        m_used   ( false )
    {
        s_params.push_back( this );
    }

    virtual ~ParamBase()
    {}

    virtual void parse( const char* str ) = 0;
    virtual void print() const = 0;
    virtual void header( int line ) const;
    virtual void help() const;
    virtual bool next();
    virtual size_t size() const = 0;

    void name( const char* in_name )
    {
        m_name = in_name;
    }

protected:
    // s_params is list of ParamBase objects, used by Params class
    static std::vector< ParamBase* > s_params;

    std::string m_name;
    std::string m_prefix;
    std::string m_help;
    size_t      m_index;
    int         m_width;
    ParamType   m_type;
    bool        m_is_default;
    int         m_used;
};

// =============================================================================
template< typename T >
class TParamBase : public ParamBase
{
public:
    TParamBase( const char* name, int width, ParamType type, T default_value,
                const char* help ):
        ParamBase( name, width, type, help ),
        m_default_value( default_value )
    {
        m_values.push_back( default_value );
    }

    virtual size_t size() const
    {
        return m_values.size();
    }

    T& value()
    {
        m_used = true;
        return m_values[ m_index ];
    }

    void push_back( T val );

protected:
    std::vector< T > m_values;
    T m_default_value;
};

// -----------------------------------------------------------------------------
template< typename T >
void TParamBase<T>::push_back( T val )
{
    if (m_type == ParamType::List) {
        if (m_is_default) {
            m_values.clear();
            m_is_default = false;
        }
        m_values.push_back( val );
    }
    else {
        m_values[0] = val;
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
        m_min_value( min_value ),
        m_max_value( max_value )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;
    void push_back( int64_t val );

protected:
    int64_t m_min_value;
    int64_t m_max_value;
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

    ParamInt3( const char* name, int width, ParamType type,
               int64_t min_value, int64_t max_value,
               const char* help ):
        TParamBase<int3_t>( name, width, type, int3_t(), help ),
        m_min_value( min_value ),
        m_max_value( max_value )
    {
        m_values.clear();
        for (int64_t i = 100; i <= 500; i += 100) {
            int3_t tmp = { i, i, i };
            m_values.push_back( tmp );
        }
    }

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void header( int line ) const;
    void push_back( int3_t val );

    int64_t& m()
    {
        m_used |= m_mask;
        return m_values[ m_index ].m;
    }

    int64_t& n()
    {
        m_used |= n_mask;
        return m_values[ m_index ].n;
    }

    int64_t& k()
    {
        m_used |= k_mask;
        return m_values[ m_index ].k;
    }

protected:
    int64_t m_min_value;
    int64_t m_max_value;
};

// =============================================================================
class ParamDouble : public TParamBase< double >
{
public:
    ParamDouble( const char* name, int width, int precision, ParamType type,
                 double default_value, double min_value, double max_value,
                 const char* help ):
        TParamBase( name, width, type, default_value, help ),
        m_precision( precision ),
        m_min_value( min_value ),
        m_max_value( max_value )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;
    void push_back( double val );

protected:
    int m_precision;
    double m_min_value;
    double m_max_value;
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
        m_valid( valid )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;
    void push_back( char val );

protected:
    std::string m_valid;
};

// =============================================================================
template< typename ENUM >
class ParamEnum : public TParamBase< ENUM >
{
public:
    typedef ENUM (*char2enum)( char ch );
    typedef char (*enum2char)( ENUM en );
    typedef const char* (*enum2str)( ENUM en );

    ParamEnum( const char* name, int width, ParamType type,
               ENUM default_value, char2enum in, enum2char out, enum2str str,
               const char* help ):
        TParamBase<ENUM>( name, width, type, default_value, help ),
        m_in( in ),
        m_out( out ),
        m_str( str )
    {}

    virtual void parse( const char* str );
    virtual void print() const;
    virtual void help() const;

protected:
    char2enum m_in;
    enum2char m_out;
    enum2str  m_str;
};

// -----------------------------------------------------------------------------
// virtual
template< typename ENUM >
void ParamEnum<ENUM>::parse( const char *str )
{
    char buf[81] = { 0 };
    while (true) {
        int len;
        int i = sscanf( str, " %80[a-zA-Z0-9_-] %n", buf, &len );
        str += len;
        if (i != 1) {
            throw std::runtime_error( "invalid option, expected char" );
        }
        // m_in throws errors
        ENUM val = m_in( buf[0] );
        this->push_back( val );
        if (*str == '\0') {
            break;
        }
        if (*str != ',') {
            throw std::runtime_error(
                "invalid argument, expected comma delimiter" );
        }
        str += 1;
    }
}

// -----------------------------------------------------------------------------
// virtual
template< typename ENUM >
void ParamEnum<ENUM>::print() const
{
    if (this->m_used && this->m_width > 0) {
        printf( "  %*s", this->m_width,
                this->m_str( this->m_values[ this->m_index ] ));
    }
}

// -----------------------------------------------------------------------------
// virtual
template< typename ENUM >
void ParamEnum<ENUM>::help() const
{
    if (this->m_type == ParamType::Value || this->m_type == ParamType::List) {
        printf( "    %-10s %s; default %c\n",
                this->m_prefix.c_str(), this->m_help.c_str(),
                this->m_out( this->m_default_value ));
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
    void help( const char* routine );
};

}  // namespace libtest

// =============================================================================
// main must define Params class (which is not in libtest namespace),
// as a subclass of ParamsBase
class Params;

namespace libtest {

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
    const char **section_names );

}  // namespace libtest

#endif        //  #ifndef TEST_HH
