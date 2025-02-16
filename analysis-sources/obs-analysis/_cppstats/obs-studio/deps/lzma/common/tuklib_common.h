#include "tuklib_config.h"
#if !defined(TUKLIB_SYMBOL_PREFIX)
#define TUKLIB_SYMBOL_PREFIX
#endif
#define TUKLIB_CAT_X(a, b) a ##b
#define TUKLIB_CAT(a, b) TUKLIB_CAT_X(a, b)
#if !defined(TUKLIB_SYMBOL)
#define TUKLIB_SYMBOL(sym) TUKLIB_CAT(TUKLIB_SYMBOL_PREFIX, sym)
#endif
#if !defined(TUKLIB_DECLS_BEGIN)
#if defined(__cplusplus)
#define TUKLIB_DECLS_BEGIN extern "C" {
#else
#define TUKLIB_DECLS_BEGIN
#endif
#endif
#if !defined(TUKLIB_DECLS_END)
#if defined(__cplusplus)
#define TUKLIB_DECLS_END }
#else
#define TUKLIB_DECLS_END
#endif
#endif
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define TUKLIB_GNUC_REQ(major, minor) ((__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)) || __GNUC__ > (major))
#else
#define TUKLIB_GNUC_REQ(major, minor) 0
#endif
#if TUKLIB_GNUC_REQ(2, 5)
#define tuklib_attr_noreturn __attribute__((__noreturn__))
#else
#define tuklib_attr_noreturn
#endif
#if (defined(_WIN32) && !defined(__CYGWIN__)) || defined(__OS2__) || defined(__MSDOS__)
#define TUKLIB_DOSLIKE 1
#endif
