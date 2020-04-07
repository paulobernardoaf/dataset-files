#include "ruby/config.h" 
#if defined(HAVE_STDBOOL_H)
#include <stdbool.h>
#endif
#if defined(__bool_true_false_are_defined)
#undef bool
#undef true
#undef false
#undef __bool_true_false_are_defined
#else
typedef unsigned char _Bool;
#endif
#define bool _Bool
#define true ((_Bool)+1)
#define false ((_Bool)+0)
#define __bool_true_false_are_defined
