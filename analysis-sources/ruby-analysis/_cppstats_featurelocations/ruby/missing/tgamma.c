










#include "ruby/config.h"
#include "ruby/missing.h"
#include <math.h>
#include <errno.h>

#if defined(_WIN32)
#include <float.h>
#if !defined __MINGW32__ || defined __NO_ISOCEXT
#if !defined(isnan)
#define isnan(x) _isnan(x)
#endif
#if !defined(isinf)
#define isinf(x) (!_finite(x) && !_isnan(x))
#endif
#if !defined(finite)
#define finite(x) _finite(x)
#endif
#endif
#endif

#if !defined(HAVE_LGAMMA_R)

#include <errno.h>
#define PI 3.14159265358979324 
#define LOG_2PI 1.83787706640934548 
#define N 8

#define B0 1 
#define B1 (-1.0 / 2.0)
#define B2 ( 1.0 / 6.0)
#define B4 (-1.0 / 30.0)
#define B6 ( 1.0 / 42.0)
#define B8 (-1.0 / 30.0)
#define B10 ( 5.0 / 66.0)
#define B12 (-691.0 / 2730.0)
#define B14 ( 7.0 / 6.0)
#define B16 (-3617.0 / 510.0)

static double
loggamma(double x) 
{
double v, w;

v = 1;
while (x < N) { v *= x; x++; }
w = 1 / (x * x);
return ((((((((B16 / (16 * 15)) * w + (B14 / (14 * 13))) * w
+ (B12 / (12 * 11))) * w + (B10 / (10 * 9))) * w
+ (B8 / ( 8 * 7))) * w + (B6 / ( 6 * 5))) * w
+ (B4 / ( 4 * 3))) * w + (B2 / ( 2 * 1))) / x
+ 0.5 * LOG_2PI - log(v) - x + (x - 0.5) * log(x);
}
#endif

double tgamma(double x) 
{
int sign;
if (x == 0.0) { 
errno = ERANGE;
return 1/x < 0 ? -HUGE_VAL : HUGE_VAL;
}
if (isinf(x)) {
if (x < 0) goto domain_error;
return x;
}
if (x < 0) {
static double zero = 0.0;
double i, f;
f = modf(-x, &i);
if (f == 0.0) { 
domain_error:
errno = EDOM;
return zero/zero;
}
#if !defined(HAVE_LGAMMA_R)
sign = (fmod(i, 2.0) != 0.0) ? 1 : -1;
return sign * PI / (sin(PI * f) * exp(loggamma(1 - x)));
#endif
}
#if !defined(HAVE_LGAMMA_R)
return exp(loggamma(x));
#else
x = lgamma_r(x, &sign);
return sign * exp(x);
#endif
}
