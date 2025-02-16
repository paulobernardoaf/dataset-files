#include "internal/compilers.h" 
#if MSC_VERSION_SINCE(1200)
#
#define COMPILER_WARNING_PUSH __pragma(warning(push))
#define COMPILER_WARNING_POP __pragma(warning(pop))
#define COMPILER_WARNING_ERROR(flag) __pragma(warning(error: flag))
#define COMPILER_WARNING_IGNORED(flag) __pragma(warning(disable: flag))
#elif defined(__clang__)
#
#define COMPILER_WARNING_PRAGMA0(x) _Pragma(#x)
#define COMPILER_WARNING_PRAGMA1(x) COMPILER_WARNING_PRAGMA0(clang diagnostic x)
#define COMPILER_WARNING_PRAGMA2(x, y) COMPILER_WARNING_PRAGMA1(x #y)
#define COMPILER_WARNING_PUSH COMPILER_WARNING_PRAGMA1(push)
#define COMPILER_WARNING_POP COMPILER_WARNING_PRAGMA1(pop)
#define COMPILER_WARNING_ERROR(flag) COMPILER_WARNING_PRAGMA2(error, flag)
#define COMPILER_WARNING_IGNORED(flag) COMPILER_WARNING_PRAGMA2(ignored, flag)
#elif GCC_VERSION_SINCE(4, 6, 0)
#
#define COMPILER_WARNING_PRAGMA0(x) _Pragma(#x)
#define COMPILER_WARNING_PRAGMA1(x) COMPILER_WARNING_PRAGMA0(GCC diagnostic x)
#define COMPILER_WARNING_PRAGMA2(x, y) COMPILER_WARNING_PRAGMA1(x #y)
#define COMPILER_WARNING_PUSH COMPILER_WARNING_PRAGMA1(push)
#define COMPILER_WARNING_POP COMPILER_WARNING_PRAGMA1(pop)
#define COMPILER_WARNING_ERROR(flag) COMPILER_WARNING_PRAGMA2(error, flag)
#define COMPILER_WARNING_IGNORED(flag) COMPILER_WARNING_PRAGMA2(ignored, flag)
#else
#
#define COMPILER_WARNING_PUSH 
#define COMPILER_WARNING_POP 
#define COMPILER_WARNING_ERROR(flag) 
#define COMPILER_WARNING_IGNORED(flag) 
#endif 
