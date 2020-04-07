#if !defined(RB_INTEGER_TYPE_P)
#define RB_INTEGER_TYPE_P(obj) (RB_FIXNUM_P(obj) || RB_TYPE_P(obj, T_BIGNUM))
#endif
#if !defined(ST2FIX)
#define RB_ST2FIX(h) LONG2FIX((long)(h))
#define ST2FIX(h) RB_ST2FIX(h)
#endif
