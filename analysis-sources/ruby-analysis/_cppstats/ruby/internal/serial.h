#include "ruby/config.h" 
#include "ruby/defines.h" 
#if !defined(HAVE_LONG_LONG)
#error need C99+
#endif
typedef unsigned LONG_LONG rb_serial_t;
#define SERIALT2NUM ULL2NUM
#define PRI_SERIALT_PREFIX PRI_LL_PREFIX
#define SIZEOF_SERIAL_T SIZEOF_LONG_LONG
