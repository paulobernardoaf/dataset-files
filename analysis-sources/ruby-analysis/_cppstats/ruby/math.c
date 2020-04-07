#include "ruby/config.h"
#if defined(_MSC_VER)
#define _USE_MATH_DEFINES 1
#endif
#include <errno.h>
#include <float.h>
#include <math.h>
#include "internal.h"
#include "internal/bignum.h"
#include "internal/complex.h"
#include "internal/math.h"
#include "internal/object.h"
#include "internal/vm.h"
#if defined(HAVE_SIGNBIT) && defined(__GNUC__) && defined(__sun) && !defined(signbit)
extern int signbit(double);
#endif
#define RB_BIGNUM_TYPE_P(x) RB_TYPE_P((x), T_BIGNUM)
VALUE rb_mMath;
VALUE rb_eMathDomainError;
#define Get_Double(x) rb_num_to_dbl(x)
#define domain_error(msg) rb_raise(rb_eMathDomainError, "Numerical argument is out of domain - " #msg)
static VALUE
math_atan2(VALUE unused_obj, VALUE y, VALUE x)
{
double dx, dy;
dx = Get_Double(x);
dy = Get_Double(y);
if (dx == 0.0 && dy == 0.0) {
if (!signbit(dx))
return DBL2NUM(dy);
if (!signbit(dy))
return DBL2NUM(M_PI);
return DBL2NUM(-M_PI);
}
#if !defined(ATAN2_INF_C99)
if (isinf(dx) && isinf(dy)) {
if (dx < 0.0) {
const double dz = (3.0 * M_PI / 4.0);
return (dy < 0.0) ? DBL2NUM(-dz) : DBL2NUM(dz);
}
else {
const double dz = (M_PI / 4.0);
return (dy < 0.0) ? DBL2NUM(-dz) : DBL2NUM(dz);
}
}
#endif
return DBL2NUM(atan2(dy, dx));
}
static VALUE
math_cos(VALUE unused_obj, VALUE x)
{
return DBL2NUM(cos(Get_Double(x)));
}
static VALUE
math_sin(VALUE unused_obj, VALUE x)
{
return DBL2NUM(sin(Get_Double(x)));
}
static VALUE
math_tan(VALUE unused_obj, VALUE x)
{
return DBL2NUM(tan(Get_Double(x)));
}
static VALUE
math_acos(VALUE unused_obj, VALUE x)
{
double d;
d = Get_Double(x);
if (d < -1.0 || 1.0 < d) domain_error("acos");
return DBL2NUM(acos(d));
}
static VALUE
math_asin(VALUE unused_obj, VALUE x)
{
double d;
d = Get_Double(x);
if (d < -1.0 || 1.0 < d) domain_error("asin");
return DBL2NUM(asin(d));
}
static VALUE
math_atan(VALUE unused_obj, VALUE x)
{
return DBL2NUM(atan(Get_Double(x)));
}
#if !defined(HAVE_COSH)
double
cosh(double x)
{
return (exp(x) + exp(-x)) / 2;
}
#endif
static VALUE
math_cosh(VALUE unused_obj, VALUE x)
{
return DBL2NUM(cosh(Get_Double(x)));
}
#if !defined(HAVE_SINH)
double
sinh(double x)
{
return (exp(x) - exp(-x)) / 2;
}
#endif
static VALUE
math_sinh(VALUE unused_obj, VALUE x)
{
return DBL2NUM(sinh(Get_Double(x)));
}
#if !defined(HAVE_TANH)
double
tanh(double x)
{
#if defined(HAVE_SINH) && defined(HAVE_COSH)
const double c = cosh(x);
if (!isinf(c)) return sinh(x) / c;
#else
const double e = exp(x+x);
if (!isinf(e)) return (e - 1) / (e + 1);
#endif
return x > 0 ? 1.0 : -1.0;
}
#endif
static VALUE
math_tanh(VALUE unused_obj, VALUE x)
{
return DBL2NUM(tanh(Get_Double(x)));
}
static VALUE
math_acosh(VALUE unused_obj, VALUE x)
{
double d;
d = Get_Double(x);
if (d < 1.0) domain_error("acosh");
return DBL2NUM(acosh(d));
}
static VALUE
math_asinh(VALUE unused_obj, VALUE x)
{
return DBL2NUM(asinh(Get_Double(x)));
}
static VALUE
math_atanh(VALUE unused_obj, VALUE x)
{
double d;
d = Get_Double(x);
if (d < -1.0 || +1.0 < d) domain_error("atanh");
if (d == -1.0) return DBL2NUM(-HUGE_VAL);
if (d == +1.0) return DBL2NUM(+HUGE_VAL);
return DBL2NUM(atanh(d));
}
static VALUE
math_exp(VALUE unused_obj, VALUE x)
{
return DBL2NUM(exp(Get_Double(x)));
}
#if defined __CYGWIN__
#include <cygwin/version.h>
#if CYGWIN_VERSION_DLL_MAJOR < 1005
#define nan(x) nan()
#endif
#define log(x) ((x) < 0.0 ? nan("") : log(x))
#define log10(x) ((x) < 0.0 ? nan("") : log10(x))
#endif
#if !defined(M_LN2)
#define M_LN2 0.693147180559945309417232121458176568
#endif
#if !defined(M_LN10)
#define M_LN10 2.30258509299404568401799145468436421
#endif
static double math_log1(VALUE x);
FUNC_MINIMIZED(static VALUE math_log(int, const VALUE *, VALUE));
static VALUE
math_log(int argc, const VALUE *argv, VALUE unused_obj)
{
return rb_math_log(argc, argv);
}
VALUE
rb_math_log(int argc, const VALUE *argv)
{
VALUE x, base;
double d;
rb_scan_args(argc, argv, "11", &x, &base);
d = math_log1(x);
if (argc == 2) {
d /= math_log1(base);
}
return DBL2NUM(d);
}
static double
get_double_rshift(VALUE x, size_t *pnumbits)
{
size_t numbits;
if (RB_BIGNUM_TYPE_P(x) && BIGNUM_POSITIVE_P(x) &&
DBL_MAX_EXP <= (numbits = rb_absint_numwords(x, 1, NULL))) {
numbits -= DBL_MANT_DIG;
x = rb_big_rshift(x, SIZET2NUM(numbits));
}
else {
numbits = 0;
}
*pnumbits = numbits;
return Get_Double(x);
}
static double
math_log1(VALUE x)
{
size_t numbits;
double d = get_double_rshift(x, &numbits);
if (d < 0.0) domain_error("log");
if (d == 0.0) return -HUGE_VAL;
return log(d) + numbits * M_LN2; 
}
#if !defined(log2)
#if !defined(HAVE_LOG2)
double
log2(double x)
{
return log10(x)/log10(2.0);
}
#else
extern double log2(double);
#endif
#endif
static VALUE
math_log2(VALUE unused_obj, VALUE x)
{
size_t numbits;
double d = get_double_rshift(x, &numbits);
if (d < 0.0) domain_error("log2");
if (d == 0.0) return DBL2NUM(-HUGE_VAL);
return DBL2NUM(log2(d) + numbits); 
}
static VALUE
math_log10(VALUE unused_obj, VALUE x)
{
size_t numbits;
double d = get_double_rshift(x, &numbits);
if (d < 0.0) domain_error("log10");
if (d == 0.0) return DBL2NUM(-HUGE_VAL);
return DBL2NUM(log10(d) + numbits * log10(2)); 
}
static VALUE rb_math_sqrt(VALUE x);
static VALUE
math_sqrt(VALUE unused_obj, VALUE x)
{
return rb_math_sqrt(x);
}
#define f_boolcast(x) ((x) ? Qtrue : Qfalse)
inline static VALUE
f_negative_p(VALUE x)
{
if (FIXNUM_P(x))
return f_boolcast(FIX2LONG(x) < 0);
return rb_funcall(x, '<', 1, INT2FIX(0));
}
inline static VALUE
f_signbit(VALUE x)
{
if (RB_TYPE_P(x, T_FLOAT)) {
double f = RFLOAT_VALUE(x);
return f_boolcast(!isnan(f) && signbit(f));
}
return f_negative_p(x);
}
static VALUE
rb_math_sqrt(VALUE x)
{
double d;
if (RB_TYPE_P(x, T_COMPLEX)) {
VALUE neg = f_signbit(RCOMPLEX(x)->imag);
double re = Get_Double(RCOMPLEX(x)->real), im;
d = Get_Double(rb_complex_abs(x));
im = sqrt((d - re) / 2.0);
re = sqrt((d + re) / 2.0);
if (neg) im = -im;
return rb_complex_new(DBL2NUM(re), DBL2NUM(im));
}
d = Get_Double(x);
if (d < 0.0) domain_error("sqrt");
if (d == 0.0) return DBL2NUM(0.0);
return DBL2NUM(sqrt(d));
}
static VALUE
math_cbrt(VALUE unused_obj, VALUE x)
{
double f = Get_Double(x);
double r = cbrt(f);
#if defined __GLIBC__
if (isfinite(r)) {
r = (2.0 * r + (f / r / r)) / 3.0;
}
#endif
return DBL2NUM(r);
}
static VALUE
math_frexp(VALUE unused_obj, VALUE x)
{
double d;
int exp;
d = frexp(Get_Double(x), &exp);
return rb_assoc_new(DBL2NUM(d), INT2NUM(exp));
}
static VALUE
math_ldexp(VALUE unused_obj, VALUE x, VALUE n)
{
return DBL2NUM(ldexp(Get_Double(x), NUM2INT(n)));
}
static VALUE
math_hypot(VALUE unused_obj, VALUE x, VALUE y)
{
return DBL2NUM(hypot(Get_Double(x), Get_Double(y)));
}
static VALUE
math_erf(VALUE unused_obj, VALUE x)
{
return DBL2NUM(erf(Get_Double(x)));
}
static VALUE
math_erfc(VALUE unused_obj, VALUE x)
{
return DBL2NUM(erfc(Get_Double(x)));
}
static VALUE
math_gamma(VALUE unused_obj, VALUE x)
{
static const double fact_table[] = {
1.0,
1.0,
2.0,
6.0,
24.0,
120.0,
720.0,
5040.0,
40320.0,
362880.0,
3628800.0,
39916800.0,
479001600.0,
6227020800.0,
87178291200.0,
1307674368000.0,
20922789888000.0,
355687428096000.0,
6402373705728000.0,
121645100408832000.0,
2432902008176640000.0,
51090942171709440000.0,
1124000727777607680000.0,
};
enum {NFACT_TABLE = numberof(fact_table)};
double d;
d = Get_Double(x);
if (isinf(d)) {
if (signbit(d)) domain_error("gamma");
return DBL2NUM(HUGE_VAL);
}
if (d == 0.0) {
return signbit(d) ? DBL2NUM(-HUGE_VAL) : DBL2NUM(HUGE_VAL);
}
if (d == floor(d)) {
if (d < 0.0) domain_error("gamma");
if (1.0 <= d && d <= (double)NFACT_TABLE) {
return DBL2NUM(fact_table[(int)d - 1]);
}
}
return DBL2NUM(tgamma(d));
}
static VALUE
math_lgamma(VALUE unused_obj, VALUE x)
{
double d;
int sign=1;
VALUE v;
d = Get_Double(x);
if (isinf(d)) {
if (signbit(d)) domain_error("lgamma");
return rb_assoc_new(DBL2NUM(HUGE_VAL), INT2FIX(1));
}
if (d == 0.0) {
VALUE vsign = signbit(d) ? INT2FIX(-1) : INT2FIX(+1);
return rb_assoc_new(DBL2NUM(HUGE_VAL), vsign);
}
v = DBL2NUM(lgamma_r(d, &sign));
return rb_assoc_new(v, INT2FIX(sign));
}
#define exp1(n) VALUE rb_math_##n(VALUE x){return math_##n(0, x);}
#define exp2(n) VALUE rb_math_##n(VALUE x, VALUE y){return math_##n(0, x, y);}
exp2(atan2)
exp1(cos)
exp1(cosh)
exp1(exp)
exp2(hypot)
exp1(sin)
exp1(sinh)
#if 0
exp1(sqrt)
#endif
void
InitVM_Math(void)
{
rb_mMath = rb_define_module("Math");
rb_eMathDomainError = rb_define_class_under(rb_mMath, "DomainError", rb_eStandardError);
rb_define_const(rb_mMath, "PI", DBL2NUM(M_PI));
#if defined(M_E)
rb_define_const(rb_mMath, "E", DBL2NUM(M_E));
#else
rb_define_const(rb_mMath, "E", DBL2NUM(exp(1.0)));
#endif
rb_define_module_function(rb_mMath, "atan2", math_atan2, 2);
rb_define_module_function(rb_mMath, "cos", math_cos, 1);
rb_define_module_function(rb_mMath, "sin", math_sin, 1);
rb_define_module_function(rb_mMath, "tan", math_tan, 1);
rb_define_module_function(rb_mMath, "acos", math_acos, 1);
rb_define_module_function(rb_mMath, "asin", math_asin, 1);
rb_define_module_function(rb_mMath, "atan", math_atan, 1);
rb_define_module_function(rb_mMath, "cosh", math_cosh, 1);
rb_define_module_function(rb_mMath, "sinh", math_sinh, 1);
rb_define_module_function(rb_mMath, "tanh", math_tanh, 1);
rb_define_module_function(rb_mMath, "acosh", math_acosh, 1);
rb_define_module_function(rb_mMath, "asinh", math_asinh, 1);
rb_define_module_function(rb_mMath, "atanh", math_atanh, 1);
rb_define_module_function(rb_mMath, "exp", math_exp, 1);
rb_define_module_function(rb_mMath, "log", math_log, -1);
rb_define_module_function(rb_mMath, "log2", math_log2, 1);
rb_define_module_function(rb_mMath, "log10", math_log10, 1);
rb_define_module_function(rb_mMath, "sqrt", math_sqrt, 1);
rb_define_module_function(rb_mMath, "cbrt", math_cbrt, 1);
rb_define_module_function(rb_mMath, "frexp", math_frexp, 1);
rb_define_module_function(rb_mMath, "ldexp", math_ldexp, 2);
rb_define_module_function(rb_mMath, "hypot", math_hypot, 2);
rb_define_module_function(rb_mMath, "erf", math_erf, 1);
rb_define_module_function(rb_mMath, "erfc", math_erfc, 1);
rb_define_module_function(rb_mMath, "gamma", math_gamma, 1);
rb_define_module_function(rb_mMath, "lgamma", math_lgamma, 1);
}
void
Init_Math(void)
{
InitVM(Math);
}
