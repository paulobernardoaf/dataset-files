



















#include_next <float.h>

#if defined(FLT_MAX)
#undef FLT_MAX
#define FLT_MAX 3.40282346638528859812e+38F

#undef FLT_MIN
#define FLT_MIN 1.17549435082228750797e-38F

#undef DBL_MAX
#define DBL_MAX ((double)1.79769313486231570815e+308L)

#undef DBL_MIN
#define DBL_MIN ((double)2.22507385850720138309e-308L)
#endif
