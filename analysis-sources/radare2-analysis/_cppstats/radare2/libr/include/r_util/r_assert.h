#include "r_log.h"
#define R_STATIC_ASSERT(x)switch (0) {case 0:case (x):;}
R_API void r_assert_log(RLogLevel level, const char *fmt, ...);
#if defined (__GNUC__) && defined (__cplusplus)
#define R_FUNCTION ((const char*) (__PRETTY_FUNCTION__))
#elif defined(__STDC__) && defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define R_FUNCTION ((const char*) (__func__))
#elif defined (__GNUC__) || (defined(_MSC_VER) && (_MSC_VER > 1300))
#define R_FUNCTION ((const char*) (__FUNCTION__))
#else
#warning Do not know how to get function name in this setup
#define R_FUNCTION ((const char*) ("???"))
#endif
#define r_warn_if_reached() do { r_assert_log (R_LOGLVL_WARN, "(%s:%d):%s%s code should not be reached\n", __FILE__, __LINE__, R_FUNCTION, R_FUNCTION[0] ? ":" : ""); } while (0)
#define r_warn_if_fail(expr) do { if (!(expr)) { r_assert_log (R_LOGLVL_WARN, "WARNING (%s:%d):%s%s runtime check failed: (%s)\n", __FILE__, __LINE__, R_FUNCTION, R_FUNCTION[0] ? ":" : "", #expr); } } while (0)
#if !defined(R_CHECKS_LEVEL)
#define R_CHECKS_LEVEL 2
#endif
#if R_CHECKS_LEVEL == 0
#define r_return_if_fail(expr) do { ; } while(0)
#define r_return_val_if_fail(expr, val) do { ; } while(0)
#define r_return_if_reached() do { ; } while(0)
#define r_return_val_if_reached(val) do { ; } while(0)
#elif R_CHECKS_LEVEL == 1 || R_CHECKS_LEVEL == 2 
#if R_CHECKS_LEVEL == 1
#define H_LOG_(loglevel, fmt, ...)
#else
#define H_LOG_(loglevel, fmt, ...) r_assert_log (loglevel, fmt, __VA_ARGS__)
#endif
#define r_return_if_fail(expr) do { if (!(expr)) { H_LOG_ (R_LOGLVL_WARN, "%s: assertion '%s' failed (line %d)\n", R_FUNCTION, #expr, __LINE__); return; } } while (0)
#define r_return_val_if_fail(expr, val) do { if (!(expr)) { H_LOG_ (R_LOGLVL_WARN, "%s: assertion '%s' failed (line %d)\n", R_FUNCTION, #expr, __LINE__); return (val); } } while (0)
#define r_return_if_reached() do { H_LOG_ (R_LOGLVL_ERROR, "file %s: line %d (%s): should not be reached\n", __FILE__, __LINE__, R_FUNCTION); return; } while (0)
#define r_return_val_if_reached(val) do { H_LOG_ (R_LOGLVL_ERROR, "file %s: line %d (%s): should not be reached\n", __FILE__, __LINE__, R_FUNCTION); return (val); } while (0)
#else 
#include <assert.h>
#define r_return_if_fail(expr) do { assert (expr); } while(0)
#define r_return_val_if_fail(expr, val) do { assert (expr); } while(0)
#define r_return_if_reached() do { assert (false); } while(0)
#define r_return_val_if_reached(val) do { assert (false); } while(0)
#endif 
