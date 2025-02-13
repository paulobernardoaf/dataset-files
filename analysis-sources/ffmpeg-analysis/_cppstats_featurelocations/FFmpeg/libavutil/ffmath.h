
























#if !defined(AVUTIL_FFMATH_H)
#define AVUTIL_FFMATH_H

#include "attributes.h"
#include "libm.h"











static av_always_inline double ff_exp10(double x)
{
return exp2(M_LOG2_10 * x);
}

static av_always_inline float ff_exp10f(float x)
{
return exp2f(M_LOG2_10 * x);
}











static av_always_inline float ff_fast_powf(float x, float y)
{
return expf(logf(x) * y);
}

#endif 
