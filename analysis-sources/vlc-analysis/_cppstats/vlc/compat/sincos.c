#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <math.h>
void sincos(double r, double *restrict sr, double *restrict cr)
{
*sr = sin(r);
*cr = cos(r);
}
void sincosf(float r, float *restrict sr, float *restrict cr)
{
*sr = sinf(r);
*cr = cosf(r);
}
