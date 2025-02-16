#if !defined(NVIM_ASSERT_H)
#define NVIM_ASSERT_H

#include "auto/config.h"







#if !defined(__has_feature)
#define __has_feature(x) 0
#endif

#if !defined(__has_extension)
#define __has_extension __has_feature
#endif






























#define STATIC_ASSERT_PRAGMA_START
#define STATIC_ASSERT_PRAGMA_END
#define STATIC_ASSERT(cond, msg) do { STATIC_ASSERT_PRAGMA_START STATIC_ASSERT_STATEMENT(cond, msg); STATIC_ASSERT_PRAGMA_END } while (0)








#if __STDC_VERSION__ >= 201112L || __has_feature(c_static_assert)
#define STATIC_ASSERT_STATEMENT(cond, msg) _Static_assert(cond, msg)


#elif (!defined(__clang__) && !defined(__INTEL_COMPILER)) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))


#define STATIC_ASSERT_STATEMENT(cond, msg) _Static_assert(cond, msg)

#undef STATIC_ASSERT_PRAGMA_START

#if __GNUC__ >= 6
#define STATIC_ASSERT_PRAGMA_START _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wpedantic\"")


#else
#define STATIC_ASSERT_PRAGMA_START _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-pedantic\"")


#endif

#undef STATIC_ASSERT_PRAGMA_END
#define STATIC_ASSERT_PRAGMA_END _Pragma("GCC diagnostic pop")



#elif defined(__clang__) && __has_extension(c_static_assert)

#define STATIC_ASSERT_STATEMENT(cond, msg) _Static_assert(cond, msg)

#undef STATIC_ASSERT_PRAGMA_START
#define STATIC_ASSERT_PRAGMA_START _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wc11-extensions\"")



#undef STATIC_ASSERT_PRAGMA_END
#define STATIC_ASSERT_PRAGMA_END _Pragma("clang diagnostic pop")



#elif _MSC_VER >= 1600

#define STATIC_ASSERT_STATEMENT(cond, msg) static_assert(cond, msg)




#else
#define STATIC_ASSERT_STATEMENT STATIC_ASSERT_EXPR
#endif

#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)

#if defined(__COUNTER__)
#define STATIC_ASSERT_EXPR(e, m) ((enum { ASSERT_CONCAT(static_assert_, __COUNTER__) = 1/(!!(e)) }) 0)

#else




#define STATIC_ASSERT_EXPR(e, m) ((enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }) 0)

#endif















#if defined(HAVE_BUILTIN_ADD_OVERFLOW)
#define STRICT_ADD(a, b, c, t) do { if (__builtin_add_overflow(a, b, c)) { ELOG("STRICT_ADD overflow"); abort(); } } while (0)






#else
#define STRICT_ADD(a, b, c, t) do { *(c) = (t)((a) + (b)); } while (0)

#endif




#if defined(HAVE_BUILTIN_ADD_OVERFLOW)
#define STRICT_SUB(a, b, c, t) do { if (__builtin_sub_overflow(a, b, c)) { ELOG("STRICT_SUB overflow"); abort(); } } while (0)






#else
#define STRICT_SUB(a, b, c, t) do { *(c) = (t)((a) - (b)); } while (0)

#endif

#endif 
