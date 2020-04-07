#if !defined(INTERNAL_FIXNUM_H)
#define INTERNAL_FIXNUM_H









#include "ruby/config.h" 
#include <limits.h> 
#include "internal/compilers.h" 
#include "internal/stdbool.h" 
#include "ruby/intern.h" 
#include "ruby/ruby.h" 

#if HAVE_LONG_LONG && SIZEOF_LONG * 2 <= SIZEOF_LONG_LONG
#define DLONG LONG_LONG
#define DL2NUM(x) LL2NUM(x)
#elif defined(HAVE_INT128_T)
#define DLONG int128_t
#define DL2NUM(x) (RB_FIXABLE(x) ? LONG2FIX(x) : rb_int128t2big(x))
VALUE rb_int128t2big(int128_t n); 
#endif

static inline long rb_overflowed_fix_to_int(long x);
static inline VALUE rb_fix_plus_fix(VALUE x, VALUE y);
static inline VALUE rb_fix_minus_fix(VALUE x, VALUE y);
static inline VALUE rb_fix_mul_fix(VALUE x, VALUE y);
static inline void rb_fix_divmod_fix(VALUE x, VALUE y, VALUE *divp, VALUE *modp);
static inline VALUE rb_fix_div_fix(VALUE x, VALUE y);
static inline VALUE rb_fix_mod_fix(VALUE x, VALUE y);
static inline bool FIXNUM_POSITIVE_P(VALUE num);
static inline bool FIXNUM_NEGATIVE_P(VALUE num);
static inline bool FIXNUM_ZERO_P(VALUE num);

static inline long
rb_overflowed_fix_to_int(long x)
{
return (long)((unsigned long)(x >> 1) ^ (1LU << (SIZEOF_LONG * CHAR_BIT - 1)));
}

static inline VALUE
rb_fix_plus_fix(VALUE x, VALUE y)
{
#if !__has_builtin(__builtin_add_overflow)
long lz = FIX2LONG(x) + FIX2LONG(y);
return LONG2NUM(lz);
#else
long lz;




















if (__builtin_add_overflow((long)x, (long)y-1, &lz)) {
return rb_int2big(rb_overflowed_fix_to_int(lz));
}
else {
return (VALUE)lz;
}
#endif
}

static inline VALUE
rb_fix_minus_fix(VALUE x, VALUE y)
{
#if !__has_builtin(__builtin_sub_overflow)
long lz = FIX2LONG(x) - FIX2LONG(y);
return LONG2NUM(lz);
#else
long lz;
if (__builtin_sub_overflow((long)x, (long)y-1, &lz)) {
return rb_int2big(rb_overflowed_fix_to_int(lz));
}
else {
return (VALUE)lz;
}
#endif
}


static inline VALUE
rb_fix_mul_fix(VALUE x, VALUE y)
{
long lx = FIX2LONG(x);
long ly = FIX2LONG(y);
#if defined(DLONG)
return DL2NUM((DLONG)lx * (DLONG)ly);
#else
if (MUL_OVERFLOW_FIXNUM_P(lx, ly)) {
return rb_big_mul(rb_int2big(lx), rb_int2big(ly));
}
else {
return LONG2FIX(lx * ly);
}
#endif
}





static inline void
rb_fix_divmod_fix(VALUE a, VALUE b, VALUE *divp, VALUE *modp)
{




long x = FIX2LONG(a);
long y = FIX2LONG(b);
long div, mod;
if (x == FIXNUM_MIN && y == -1) {
if (divp) *divp = LONG2NUM(-FIXNUM_MIN);
if (modp) *modp = LONG2FIX(0);
return;
}
div = x / y;
mod = x % y;
if (y > 0 ? mod < 0 : mod > 0) {
mod += y;
div -= 1;
}
if (divp) *divp = LONG2FIX(div);
if (modp) *modp = LONG2FIX(mod);
}




static inline VALUE
rb_fix_div_fix(VALUE x, VALUE y)
{
VALUE div;
rb_fix_divmod_fix(x, y, &div, NULL);
return div;
}




static inline VALUE
rb_fix_mod_fix(VALUE x, VALUE y)
{
VALUE mod;
rb_fix_divmod_fix(x, y, NULL, &mod);
return mod;
}

static inline bool
FIXNUM_POSITIVE_P(VALUE num)
{
return (SIGNED_VALUE)num > (SIGNED_VALUE)INT2FIX(0);
}

static inline bool
FIXNUM_NEGATIVE_P(VALUE num)
{
return (SIGNED_VALUE)num < 0;
}

static inline bool
FIXNUM_ZERO_P(VALUE num)
{
return num == INT2FIX(0);
}
#endif 
