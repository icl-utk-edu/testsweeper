#include <stdio.h>
#include <string.h>

#include <omp.h>

#include "libtest.hh"

namespace libtest {

// -----------------------------------------------------------------------------
// ANSI color codes
const char *ansi_esc    = "\x1b[";
const char *ansi_red    = "\x1b[31m";
const char *ansi_bold   = "\x1b[1m";
const char *ansi_normal = "\x1b[0m";

// -----------------------------------------------------------------------------
// static class variables
std::vector< ParamBase* > ParamBase::s_params;

// -----------------------------------------------------------------------------
// @brief Scans string for single integer or range of integers (start:end:step).
//        Advances the string to after the range or number.
//
// @param[in,out] strp - pointer to string containing an integer or range.
//                       On output, advanced to after the number or range.
// @param[out] start - start of range
// @param[out] end   - end of range
// @param[out] step  - step size; 0 if start = end
//
// @retval 1 - failure
// @retval 0 - success

int scan_range( const char **strp, int64_t *start, int64_t *end, int64_t *step )
{
    long long start_, end_, step_;
    int bytes1, bytes2, bytes3, cnt;
    cnt = sscanf(*strp, "%lld %n: %lld %n: %lld %n",
                 &start_, &bytes1, &end_, &bytes2, &step_, &bytes3);
    *start = start_;
    *end   = end_;
    *step  = step_;
    if (cnt == 3) {
        if (*start == *end)
            *step = 0;
        *strp += bytes3;
        return ! ((*step == 0 && *start == *end) ||
                  (*step >  0 && *start <  *end) ||
                  (*step <  0 && *start >  *end));
    }
    else if (cnt == 2) {
        *strp += bytes2;
        if (*start == *end)
            *step = 0;
        else
            *step = 1;
        return ! (*start <= *end);
    }
    else if (cnt == 1) {
        *strp += bytes1;
        *end  = *start;
        *step = 0;
        return 0;
    }
    else {
        return 1;
    }
}

// -----------------------------------------------------------------------------
// @brief Scans string for single double or range of doubles (start:end:step).
//        Advances the string to after the range or number.
//
// @param[in,out] strp - pointer to string containing a double or range.
//                       On output, advanced to after the number or range.
// @param[out] start - start of range
// @param[out] end   - end of range
// @param[out] step  - step size; 0 if start = end
//
// @retval 1 - failure
// @retval 0 - success

int scan_range( const char **strp, double *start, double *end, double *step )
{
    int bytes1, bytes2, bytes3, cnt;
    cnt = sscanf(*strp, "%lf %n: %lf %n: %lf %n",
                 start, &bytes1, end, &bytes2, step, &bytes3);
    if (cnt == 3) {
        if (*start == *end)
            *step = 0;
        *strp += bytes3;
        return ! ((*step == 0 && *start == *end) ||
                  (*step >  0 && *start <  *end) ||
                  (*step <  0 && *start >  *end));
    }
    else if (cnt == 2) {
        *strp += bytes2;
        if (*start == *end)
            *step = 0;
        else
            *step = 1;
        return ! (*start <= *end);
    }
    else if (cnt == 1) {
        *strp += bytes1;
        *end  = *start;
        *step = 0;
        return 0;
    }
    else {
        return 1;
    }
}

// -----------------------------------------------------------------------------
// Flushes cache by allocating buffer of 2*cache size (in MiB),
// and writing it in parallel.
void flush_cache( size_t cache_size )
{
    size_t len = 2 * cache_size * 1024 * 1024;
    unsigned char *buf = (unsigned char*) malloc( len );

    int nthread = 1;
    #pragma omp parallel
    #pragma omp master
    {
        #ifdef _OPENMP
        nthread = omp_get_num_threads();
        #endif
    }

    size_t per_core = len / nthread;

    #pragma omp parallel
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        for (size_t i = tid * per_core; i < (tid + 1) * per_core; ++i) {
            buf[i] = i % 256;
        }
    }

    free( buf );
}

// =============================================================================
// ParamBase class

// -----------------------------------------------------------------------------
// if name has \n, for line=0, print 1st line; for line=1, print 2nd line.
// otherwise,      for line=0, print blank;    for line=1, print name.
// virtual
void ParamBase::header( int line ) const
{
    if (m_used && m_width > 0) {
        size_t i = m_name.find( '\n' );
        const char *str = "";
        if (i != std::string::npos) {
            str = (line == 0
                ? m_name.substr( 0, i ).c_str()
                : m_name.substr( i+1 ).c_str() );
        }
        else {
            str = (line == 0
                ? ""
                : m_name.c_str() );
        }
        printf( "  %*s", m_width, str );
    }
}

// -----------------------------------------------------------------------------
// virtual
void ParamBase::help() const
{
    if (m_type == ParamType::Value || m_type == ParamType::List) {
        printf( "    %-16s %s\n", m_prefix.c_str(), m_help.c_str() );
    }
}

// -----------------------------------------------------------------------------
// virtual
bool ParamBase::next()
{
    assert( m_index >= 0 && m_index < size() );
    if (m_index == size() - 1) {
        m_index = 0;
        return false;
    }
    else {
        m_index += 1;
        return true;
    }
}

// =============================================================================
// ParamInt class
// Integer parameters

// -----------------------------------------------------------------------------
// virtual
void ParamInt::parse( const char *str )
{
    while( true ) {
        int64_t start, end, step;
        if (scan_range( &str, &start, &end, &step ) != 0) {
            throw std::runtime_error(
                "invalid argument, expected integer or range start:end:step" );
        }
        if (start == end) {
            push_back( start );
        }
        else {
            for (int64_t val = start; val <= end; val += step) {
                push_back( val );
            }
        }
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
void ParamInt::push_back( int64_t val )
{
    if (val < m_min_value || val > m_max_value) {
        char msg[1000];
        snprintf( msg, sizeof(msg),
                  "invalid argument, %lld outside [%lld, %lld]\n",
                  (long long) val,
                  (long long) m_min_value,
                  (long long) m_max_value );
        throw std::runtime_error( msg );
    }
    TParamBase<int64_t>::push_back( val );
}

// -----------------------------------------------------------------------------
// virtual
void ParamInt::print() const
{
    if (m_used && m_width > 0) {
        printf( "  %*lld", m_width, (long long) m_values[ m_index ] );
    }
}

// -----------------------------------------------------------------------------
// virtual
void ParamInt::help() const
{
    if (m_type == ParamType::Value || m_type == ParamType::List) {
        printf( "    %-16s %s; default %lld\n",
                m_prefix.c_str(), m_help.c_str(), (long long) m_default_value );
    }
}

// =============================================================================
// ParamOkay class
// same as ParamInt, but prints pass (for non-zero) or FAILED (for zero).

// -----------------------------------------------------------------------------
// virtual
void ParamOkay::print() const
{
    if (m_used && m_width > 0) {
        const char *msg;
        switch (m_values[ m_index ]) {
            case  0: msg = "FAILED";   break;
            case  1: msg = "pass";     break;
            case -1: msg = "no check"; break;
        }
        printf( "  %-*s", m_width, msg );
    }
}

// =============================================================================
// ParamInt3 class
// Integer 3-tuple parameters for M x N x K dimensions

// -----------------------------------------------------------------------------
// virtual
void ParamInt3::parse( const char *str )
{
    bool outer = false;  // todo: add --dim-outer and pass it in somehow
    int64_t m_start, m_end, m_step;
    int64_t n_start, n_end, n_step;
    int64_t k_start, k_end, k_step;
    int len;
    while (true) {
        // scan M
        if (scan_range( &str, &m_start, &m_end, &m_step ) != 0) {
            throw std::runtime_error(
                "invalid m argument, "
                "expected integer or range start:end:step" );
        }
        // if "x", scan N; else K = N = M
        len = 0;
        sscanf( str, " x %n", &len );
        if (len > 0) {
            str += len;
            if (scan_range( &str, &n_start, &n_end, &n_step ) != 0) {
                throw std::runtime_error(
                    "invalid n argument, "
                    "expected integer or range start:end:step" );
            }
            // if "x", scan K; else K = N
            len = 0;
            sscanf( str, " x %n", &len );
            if (len > 0) {
                str += len;
                if (scan_range( &str, &k_start, &k_end, &k_step ) != 0) {
                    throw std::runtime_error(
                        "invalid k argument, "
                        "expected integer or range start:end:step" );
                }
            }
            else {
                k_start = n_start;
                k_end   = n_end;
                k_step  = n_step;
            }
        }
        else {
            k_start = n_start = m_start;
            k_end   = n_end   = m_end;
            k_step  = n_step  = m_step;
        }

        if (m_start == m_end && n_start == n_end && k_start == k_end) {
            // single size
            int3_t dim = { m_start, n_start, k_start };
            push_back( dim );
        }
        else if (outer) {
            // outer product of M x N x K
            // require non-zero step
            if (m_step == 0) m_step = 1;
            if (n_step == 0) n_step = 1;
            if (k_step == 0) k_step = 1;
            for (int64_t m = m_start;
                 (m_step >= 0 ? m <= m_end : m >= m_end);
                 m += m_step) {

                for (int64_t n = n_start;
                     (n_step >= 0 ? n <= n_end : n >= n_end);
                     n += n_step) {

                    for (int64_t k = k_start;
                         (k_step >= 0 ? k <= k_end : k >= k_end);
                         k += k_step)
                    {
                        int3_t dim = { m, n, k };
                        push_back( dim );
                    }
                }
            }
        }
        else {
            // inner product of M x N x K
            // at least one of the variables must advance
            assert( m_step != 0 || n_step != 0 || k_step != 0 );
            for (int64_t m = m_start,
                         n = n_start,
                         k = k_start;

                 (m_step >= 0 ? m <= m_end : m >= m_end) &&
                 (n_step >= 0 ? n <= n_end : n >= n_end) &&
                 (k_step >= 0 ? k <= k_end : k >= k_end);

                 m += m_step,
                 n += n_step,
                 k += k_step)
            {
                int3_t dim = { m, n, k };
                push_back( dim );
            }
        }
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
void ParamInt3::push_back( int3_t val )
{
    if (val.m < m_min_value || val.m > m_max_value ||
        val.n < m_min_value || val.n > m_max_value ||
        val.k < m_min_value || val.k > m_max_value)
    {
        char msg[1000];
        snprintf( msg, sizeof(msg),
                  "invalid value, %lld x %lld x %lld outside [%lld, %lld]\n",
                  (long long) val.m,
                  (long long) val.n,
                  (long long) val.k,
                  (long long) m_min_value,
                  (long long) m_max_value );
        throw std::runtime_error( msg );
    }
    TParamBase<int3_t>::push_back( val );
}

// -----------------------------------------------------------------------------
// virtual
void ParamInt3::print() const
{
    if (m_width > 0) {
        if (m_used & m_mask) {
            printf( "  %*lld", m_width, (long long) m_values[ m_index ].m );
        }
        if (m_used & n_mask) {
            printf( "  %*lld", m_width, (long long) m_values[ m_index ].n );
        }
        if (m_used & k_mask) {
            printf( "  %*lld", m_width, (long long) m_values[ m_index ].k );
        }
    }
}

// -----------------------------------------------------------------------------
// for line=0, print blanks
// for line=1, print whichever of m, n, k are used
// virtual
void ParamInt3::header( int line ) const
{
    if (m_width > 0) {
        if (m_used & m_mask) {
            printf( "  %*s", m_width, (line == 0 ? "" : "m") );
        }
        if (m_used & n_mask) {
            printf( "  %*s", m_width, (line == 0 ? "" : "n") );
        }
        if (m_used & k_mask) {
            printf( "  %*s", m_width, (line == 0 ? "" : "k") );
        }
    }
}

// =============================================================================
// ParamDouble class
// Double precision parameter

// -----------------------------------------------------------------------------
// virtual
void ParamDouble::parse( const char *str )
{
    double start, end, step;
    while (true) {
        if (scan_range( &str, &start, &end, &step ) != 0) {
            throw std::runtime_error(
                "invalid argument, expected float or range start:end:step" );
        }
        if (start == end) {
            push_back( start );
        }
        else {
            end += step / 10.;  // avoid rounding issues
            for (double val = start; val <= end; val += step) {
                push_back( val );
            }
        }
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
void ParamDouble::push_back( double val )
{
    if (val < m_min_value || val > m_max_value) {
        char msg[1000];
        snprintf( msg, sizeof(msg),
                  "invalid argument, %.*f outside [%.*f, %.*f]\n",
                  m_precision, val,
                  m_precision, m_min_value,
                  m_precision, m_max_value );
        throw std::runtime_error( msg );
    }
    TParamBase<double>::push_back( val );
}

// -----------------------------------------------------------------------------
// virtual
void ParamDouble::print() const
{
    if (m_used && m_width > 0) {
        printf( "  %*.*f", m_width, m_precision, m_values[ m_index ] );
    }
}

// -----------------------------------------------------------------------------
// virtual
void ParamDouble::help() const
{
    if (m_type == ParamType::Value || m_type == ParamType::List) {
        printf( "    %-16s %s; default %.*f\n",
                m_prefix.c_str(), m_help.c_str(),
                m_precision, m_default_value );
    }
}

// =============================================================================
// ParamScientific class
// same as ParamDouble, but prints using scientific notation (%e)

// -----------------------------------------------------------------------------
// virtual
void ParamScientific::print() const
{
    if (m_used && m_width > 0) {
        printf( "  %*.*e", m_width, m_precision, m_values[ m_index ] );
    }
}

// -----------------------------------------------------------------------------
// virtual
void ParamScientific::help() const
{
    if (m_type == ParamType::Value || m_type == ParamType::List) {
        printf( "    %-16s %s; default %.*e\n",
                m_prefix.c_str(), m_help.c_str(),
                m_precision, m_default_value );
    }
}

// =============================================================================
// ParamChar class
// Character parameters

// -----------------------------------------------------------------------------
// virtual
void ParamChar::parse( const char *str )
{
    while (true) {
        char val;
        int len;
        int i = sscanf( str, "%c %n", &val, &len );
        str += len;
        if (i != 1) {
            throw std::runtime_error( "invalid option, expect char" );
        }
        push_back( val );
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
void ParamChar::push_back( char val )
{
    if (m_valid.find( val ) == std::string::npos) {  // not found
        char msg[1000];
        snprintf( msg, sizeof(msg), "invalid option, %c not in [%s]\n",
                  val, m_valid.c_str() );
        throw std::runtime_error( msg );
    }
    TParamBase<char>::push_back( val );
}

// -----------------------------------------------------------------------------
// virtual
void ParamChar::print() const
{
    if (m_used && m_width > 0) {
        printf( "  %*c", m_width, m_values[ m_index ] );
    }
}

// -----------------------------------------------------------------------------
// virtual
void ParamChar::help() const
{
    if (m_type == ParamType::Value || m_type == ParamType::List) {
        printf( "    %-16s %s; default %c; valid: [%s]\n",
                m_prefix.c_str(), m_help.c_str(),
                m_default_value, m_valid.c_str() );
    }
}

// =============================================================================
// ParamsBase class
// List of parameters

// -----------------------------------------------------------------------------
void ParamsBase::parse( const char *routine, int n, char **args )
{
    // Usage: test command [params]
    for (int i = 0; i < n; ++i) {
        const char *arg = args[i];
        size_t len = strlen( arg );
        try {
            bool found = false;
            if (strncmp( arg, "-h", 2 ) == 0 ||
                strncmp( arg, "--help", 6 ) == 0)
            {
                help( routine );
                exit(0);
            }
            for (auto param = ParamBase::s_params.begin();
                 param != ParamBase::s_params.end(); ++param)
            {
                // handles both "--arg value" (two arg)
                // and          "--arg=value" (one arg)
                size_t plen = (*param)->m_prefix.size();
                if (strncmp( arg, (*param)->m_prefix.c_str(), plen ) == 0
                    && (len == plen || (len > plen && arg[plen] == '=')))
                {
                    if ( ! (*param)->m_used) {
                        char msg[1000];
                        snprintf( msg, sizeof(msg),
                                  "invalid parameter for routine '%s'",
                                  routine );
                        throw std::runtime_error( msg );
                    }
                    const char *value;
                    if (len == plen && i+1 < n) {
                        // --arg value (two arguments)
                        i += 1;
                        value = args[i];
                    }
                    else {
                        // --arg=value (one argument)
                        value = arg + (*param)->m_prefix.size() + 1;
                    }
                    (*param)->parse( value );
                    found = true;
                    break;
                }
            }
            if ( ! found) {
                throw std::runtime_error( "invalid parameter" );
            }
        }
        catch( const std::exception& e ) {
            fprintf( stderr, "%s%sError: %s: %s%s\n\n",
                     ansi_bold, ansi_red, e.what(), arg, ansi_normal );
            help( routine );
            exit(1);
        }
    }
}

// -----------------------------------------------------------------------------
bool ParamsBase::next()
{
    // "outer product" iteration
    // uses reverse order so in output, parameters on right cycle fastest
    for (auto param = ParamBase::s_params.rbegin();
         param != ParamBase::s_params.rend(); ++param)
    {
        if ((*param)->next()) {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
void ParamsBase::header()
{
    for (int line = 0; line < 2; ++line) {
        for (auto param = ParamBase::s_params.begin();
             param != ParamBase::s_params.end(); ++param)
        {
            (*param)->header( line );
        }
        printf( "\n" );
    }
}

// -----------------------------------------------------------------------------
void ParamsBase::print()
{
    for (auto param = ParamBase::s_params.begin();
         param != ParamBase::s_params.end(); ++param)
    {
        (*param)->print();
    }
    printf( "\n" );
}

// -----------------------------------------------------------------------------
void ParamsBase::help( const char *routine )
{
    printf( "%sUsage:%s test [-h|--help]\n"
            "       test routine [-h|--help]\n"
            "       test routine [parameters]\n\n"
            "%sParameters for %s:%s\n",
            ansi_bold, ansi_normal,
            ansi_bold, routine, ansi_normal );
    for (auto param = ParamBase::s_params.begin();
         param != ParamBase::s_params.end(); ++param)
    {
        if ((*param)->m_used && (*param)->m_type == ParamType::Value)
            (*param)->help();
    }
    printf( "\n%sParameters that take comma-separated list of values and may be repeated:%s\n",
            ansi_bold, ansi_normal );
    for (auto param = ParamBase::s_params.begin();
         param != ParamBase::s_params.end(); ++param)
    {
        if ((*param)->m_used && (*param)->m_type == ParamType::List)
            (*param)->help();
    }
}

// -----------------------------------------------------------------------------
test_func_ptr find_tester(
    const char *name,
    std::vector< routines_t >& routines )
{
    for (size_t i = 0; i < routines.size(); ++i) {
        if (strcmp( name, routines[i].name ) == 0) {
            return routines[i].func;
        }
    }
    return nullptr;
}

// -----------------------------------------------------------------------------
void usage(
    int argc, char **argv,
    std::vector< routines_t >& routines,
    const char **section_names )
{
    using namespace libtest;
    int ncols = 4;
    printf( "%sUsage:%s %s [-h|--help]\n"
            "       %s routine [-h|--help]\n"
            "       %s routine [parameters]\n\n"
            "%sAvailable routines:%s",
            ansi_bold, ansi_normal,
            argv[0], argv[0], argv[0],
            ansi_bold, ansi_normal );
    int last_section = 0;
    int cnt = 0;
    for (size_t i = 0; i < routines.size(); ++i) {
        if (routines[i].section == 0) {
            cnt = 0;  // force newline before next routine
            continue;
        }
        if (routines[i].section != last_section) {
            last_section = routines[i].section;
            cnt = 0;
            printf( "\n\n%s%s%s\n",
                    ansi_bold, section_names[last_section], ansi_normal );
        }
        else if (cnt % ncols == 0) {
            printf( "\n" );
        }
        printf( "  %-18s", routines[i].name );
        cnt += 1;
    }
    printf( "\n" );
}

} // namespace libtest
