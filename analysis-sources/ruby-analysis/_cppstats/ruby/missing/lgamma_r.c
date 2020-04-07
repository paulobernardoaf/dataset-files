#include "ruby/missing.h"
#include <math.h>
#include <errno.h>
#define PI 3.14159265358979324 
#define LOG_2PI 1.83787706640934548 
#define LOG_PI 1.14472988584940017 
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
if (x == 1.0 || x == 2.0) return 0.0;
v = 1;
while (x < N) { v *= x; x++; }
w = 1 / (x * x);
return ((((((((B16 / (16 * 15)) * w + (B14 / (14 * 13))) * w
+ (B12 / (12 * 11))) * w + (B10 / (10 * 9))) * w
+ (B8 / ( 8 * 7))) * w + (B6 / ( 6 * 5))) * w
+ (B4 / ( 4 * 3))) * w + (B2 / ( 2 * 1))) / x
+ 0.5 * LOG_2PI - log(v) - x + (x - 0.5) * log(x);
}
#if defined(__MINGW_ATTRIB_PURE)
#define modf(_X, _Y) __extension__ ({double intpart_modf_bug = intpart_modf_bug;double result_modf_bug = modf((_X), &intpart_modf_bug);*(_Y) = intpart_modf_bug;result_modf_bug;})
#endif
double
lgamma_r(double x, int *signp)
{
if (x <= 0) {
double i, f, s;
f = modf(-x, &i);
if (f == 0.0) { 
*signp = signbit(x) ? -1 : 1;
errno = ERANGE;
return HUGE_VAL;
}
*signp = (fmod(i, 2.0) != 0.0) ? 1 : -1;
s = sin(PI * f);
if (s < 0) s = -s;
return LOG_PI - log(s) - loggamma(1 - x);
}
*signp = 1;
return loggamma(x);
}
