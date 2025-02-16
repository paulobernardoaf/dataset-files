


#include <math.h>

#include "nvim/math.h"

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "math.c.generated.h"
#endif

#if defined(__clang__) && __clang__ == 1 && __clang_major__ >= 6


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#endif
int xfpclassify(double d)
{
#if defined(__MINGW32__)

return __fpclassify(d);
#else
return fpclassify(d);
#endif
}
int xisinf(double d)
{
return isinf(d);
}
int xisnan(double d)
{
#if defined(__MINGW32__)

return _isnan(d);
#else
return isnan(d);
#endif
}
#if defined(__clang__) && __clang__ == 1 && __clang_major__ >= 6
#pragma clang diagnostic pop
#endif
