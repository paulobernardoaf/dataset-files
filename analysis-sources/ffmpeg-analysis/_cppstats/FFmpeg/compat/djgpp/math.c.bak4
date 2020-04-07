

















#include <math.h>

#define FUN(name, type, op) type name(type x, type y) { if (fpclassify(x) == FP_NAN) return y; if (fpclassify(y) == FP_NAN) return x; return x op y ? x : y; }







FUN(fmin, double, <)
FUN(fmax, double, >)
FUN(fminf, float, <)
FUN(fmaxf, float, >)

long double fmodl(long double x, long double y)
{
return fmod(x, y);
}

long double scalbnl(long double x, int exp)
{
return scalbn(x, exp);
}

long double copysignl(long double x, long double y)
{
return copysign(x, y);
}
