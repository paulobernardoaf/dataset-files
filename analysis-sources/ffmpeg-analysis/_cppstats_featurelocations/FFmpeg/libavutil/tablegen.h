






















#if !defined(AVUTIL_TABLEGEN_H)
#define AVUTIL_TABLEGEN_H

#include <math.h>



static inline double ff_cbrt(double x)
{
return x < 0 ? -pow(-x, 1.0 / 3.0) : pow(x, 1.0 / 3.0);
}
#define cbrt ff_cbrt

static inline double ff_rint(double x)
{
return x >= 0 ? floor(x + 0.5) : ceil(x - 0.5);
}
#define rint ff_rint

static inline long long ff_llrint(double x)
{
return rint(x);
}
#define llrint ff_llrint

static inline long ff_lrint(double x)
{
return rint(x);
}
#define lrint ff_lrint

#endif 
