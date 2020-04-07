




















#if !defined(COMPAT_AIX_MATH_H)
#define COMPAT_AIX_MATH_H

#define class class_in_math_h_causes_problems

#include_next <math.h>

#undef class

#endif 
