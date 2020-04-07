#pragma once
#include "../util/c99defs.h"
#include <math.h>
#if defined(__cplusplus)
extern "C" {
#endif
#if !defined(M_PI)
#define M_PI 3.1415926535897932384626433832795f
#endif
#define RAD(val) ((val)*0.0174532925199432957692369076848f)
#define DEG(val) ((val)*57.295779513082320876798154814105f)
#define LARGE_EPSILON 1e-2f
#define EPSILON 1e-4f
#define TINY_EPSILON 1e-5f
#define M_INFINITE 3.4e38f
static inline bool close_float(float f1, float f2, float precision)
{
return fabsf(f1 - f2) <= precision;
}
#if defined(__cplusplus)
}
#endif
