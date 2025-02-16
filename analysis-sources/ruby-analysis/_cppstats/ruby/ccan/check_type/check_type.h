#if HAVE_TYPEOF
#define check_type(expr, type) ((typeof(expr) *)0 != (type *)0)
#define check_types_match(expr1, expr2) ((typeof(expr1) *)0 != (typeof(expr2) *)0)
#else
#include "ccan/build_assert/build_assert.h"
#define check_type(expr, type) BUILD_ASSERT_OR_ZERO(sizeof(expr) == sizeof(type))
#define check_types_match(expr1, expr2) BUILD_ASSERT_OR_ZERO(sizeof(expr1) == sizeof(expr2))
#endif 
