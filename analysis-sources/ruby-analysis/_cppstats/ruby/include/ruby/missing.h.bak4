









#if !defined(RUBY_MISSING_H)
#define RUBY_MISSING_H 1

#if defined(__cplusplus)
extern "C" {
#if 0
} 
#endif
#endif

#include "ruby/config.h"
#include <stddef.h>
#include <math.h> 
#if defined(RUBY_ALTERNATIVE_MALLOC_HEADER)
#include RUBY_ALTERNATIVE_MALLOC_HEADER
#endif
#if defined(RUBY_EXTCONF_H)
#include RUBY_EXTCONF_H
#endif

#if !defined(HAVE_STRUCT_TIMEVAL) || !defined(HAVE_STRUCT_TIMESPEC)
#if defined(HAVE_TIME_H)
#include <time.h>
#endif
#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#endif

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif
#if !defined(M_PI_2)
#define M_PI_2 (M_PI/2)
#endif

#if !defined(RUBY_SYMBOL_EXPORT_BEGIN)
#define RUBY_SYMBOL_EXPORT_BEGIN 
#define RUBY_SYMBOL_EXPORT_END 
#endif

#if !defined(HAVE_STRUCT_TIMEVAL)
struct timeval {
time_t tv_sec; 
long tv_usec; 
};
#endif 

#if !defined(HAVE_STRUCT_TIMESPEC)
struct timespec {
time_t tv_sec; 
long tv_nsec; 
};
#endif

#if !defined(HAVE_STRUCT_TIMEZONE)
struct timezone {
int tz_minuteswest;
int tz_dsttime;
};
#endif

#if defined(RUBY_EXPORT)
#undef RUBY_EXTERN
#endif
#if !defined(RUBY_EXTERN)
#define RUBY_EXTERN extern
#endif

RUBY_SYMBOL_EXPORT_BEGIN

#if !defined(HAVE_ACOSH)
RUBY_EXTERN double acosh(double);
RUBY_EXTERN double asinh(double);
RUBY_EXTERN double atanh(double);
#endif

#if !defined(HAVE_CRYPT)
RUBY_EXTERN char *crypt(const char *, const char *);
#endif

#if !defined(HAVE_DUP2)
RUBY_EXTERN int dup2(int, int);
#endif

#if !defined(HAVE_EACCESS)
RUBY_EXTERN int eaccess(const char*, int);
#endif

#if !defined(HAVE_ROUND)
RUBY_EXTERN double round(double); 
#endif

#if !defined(HAVE_FINITE)
RUBY_EXTERN int finite(double);
#endif

#if !defined(HAVE_FLOCK)
RUBY_EXTERN int flock(int, int);
#endif







#if !defined(HAVE_HYPOT)
RUBY_EXTERN double hypot(double, double);
#endif

#if !defined(HAVE_ERF)
RUBY_EXTERN double erf(double);
RUBY_EXTERN double erfc(double);
#endif

#if !defined(HAVE_TGAMMA)
RUBY_EXTERN double tgamma(double);
#endif

#if !defined(HAVE_LGAMMA_R)
RUBY_EXTERN double lgamma_r(double, int *);
#endif

#if !defined(HAVE_CBRT)
RUBY_EXTERN double cbrt(double);
#endif

#if !defined(INFINITY) || !defined(NAN)
union bytesequence4_or_float {
unsigned char bytesequence[4];
float float_value;
};
#endif

#if !defined(INFINITY)

RUBY_EXTERN const union bytesequence4_or_float rb_infinity;
#define INFINITY (rb_infinity.float_value)
#define USE_RB_INFINITY 1
#endif

#if !defined(NAN)

RUBY_EXTERN const union bytesequence4_or_float rb_nan;
#define NAN (rb_nan.float_value)
#define USE_RB_NAN 1
#endif

#if !defined(HUGE_VAL)
#define HUGE_VAL ((double)INFINITY)
#endif

#if !defined(isinf)
#if !defined(HAVE_ISINF)
#if defined(HAVE_FINITE) && defined(HAVE_ISNAN)
#if defined(HAVE_IEEEFP_H)
#include <ieeefp.h>
#endif
#define isinf(x) (!finite(x) && !isnan(x))
#elif defined(__cplusplus) && __cplusplus >= 201103L
#include <cmath> 
#else
RUBY_EXTERN int isinf(double);
#endif
#endif
#endif

#if !defined(isnan)
#if !defined(HAVE_ISNAN)
#if defined(__cplusplus) && __cplusplus >= 201103L
#include <cmath> 
#else
RUBY_EXTERN int isnan(double);
#endif
#endif
#endif

#if !defined(isfinite)
#if !defined(HAVE_ISFINITE)
#define HAVE_ISFINITE 1
#define isfinite(x) finite(x)
#endif
#endif

#if !defined(HAVE_NAN)
RUBY_EXTERN double nan(const char *);
#endif

#if !defined(HAVE_NEXTAFTER)
RUBY_EXTERN double nextafter(double x, double y);
#endif







#if !defined(HAVE_MEMMOVE)
RUBY_EXTERN void *memmove(void *, const void *, size_t);
#endif







#if !defined(HAVE_STRCHR)
RUBY_EXTERN char *strchr(const char *, int);
RUBY_EXTERN char *strrchr(const char *, int);
#endif

#if !defined(HAVE_STRERROR)
RUBY_EXTERN char *strerror(int);
#endif

#if !defined(HAVE_STRSTR)
RUBY_EXTERN char *strstr(const char *, const char *);
#endif

#if !defined(HAVE_STRLCPY)
RUBY_EXTERN size_t strlcpy(char *, const char*, size_t);
#endif

#if !defined(HAVE_STRLCAT)
RUBY_EXTERN size_t strlcat(char *, const char*, size_t);
#endif

#if !defined(HAVE_SIGNBIT)
RUBY_EXTERN int signbit(double x);
#endif

#if !defined(HAVE_FFS)
RUBY_EXTERN int ffs(int);
#endif

#if defined(BROKEN_CLOSE)
#include <sys/types.h>
#include <sys/socket.h>
RUBY_EXTERN int ruby_getpeername(int, struct sockaddr *, socklen_t *);
RUBY_EXTERN int ruby_getsockname(int, struct sockaddr *, socklen_t *);
RUBY_EXTERN int ruby_shutdown(int, int);
RUBY_EXTERN int ruby_close(int);
#endif

#if !defined(HAVE_SETPROCTITLE)
RUBY_EXTERN void setproctitle(const char *fmt, ...);
#endif

#if !defined(HAVE_EXPLICIT_BZERO)
RUBY_EXTERN void explicit_bzero(void *b, size_t len);
#if defined SecureZeroMemory
#define explicit_bzero(b, len) SecureZeroMemory(b, len)
#endif
#endif

RUBY_SYMBOL_EXPORT_END

#if defined(__cplusplus)
#if 0
{ 
#endif
} 
#endif

#endif 
