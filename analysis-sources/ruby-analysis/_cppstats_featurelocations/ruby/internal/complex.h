#if !defined(INTERNAL_COMPLEX_H)
#define INTERNAL_COMPLEX_H









#include "ruby/ruby.h" 

struct RComplex {
struct RBasic basic;
VALUE real;
VALUE imag;
};

#define RCOMPLEX(obj) (R_CAST(RComplex)(obj))


#define RCOMPLEX_SET_REAL(cmp, r) RB_OBJ_WRITE((cmp), &RCOMPLEX(cmp)->real, (r))
#define RCOMPLEX_SET_IMAG(cmp, i) RB_OBJ_WRITE((cmp), &RCOMPLEX(cmp)->imag, (i))


VALUE rb_dbl_complex_new_polar_pi(double abs, double ang);

#endif 
