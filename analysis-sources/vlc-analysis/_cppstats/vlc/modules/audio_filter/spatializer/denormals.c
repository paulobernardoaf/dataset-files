#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <math.h>
#include "denormals.h"
float undenormalise( float f )
{
if( fpclassify( f ) == FP_SUBNORMAL )
return 0.0;
return f;
}
