#include "ruby/defines.h" 
#if defined(_MSC_VER)
#define MSC_VERSION_SINCE(_) (_MSC_VER >= _)
#define MSC_VERSION_BEFORE(_) (_MSC_VER < _)
#else
#define MSC_VERSION_SINCE(_) 0
#define MSC_VERSION_BEFORE(_) 0
#endif
#if !defined(__has_attribute)
#define __has_attribute(...) __has_attribute_##__VA_ARGS__
#
#define __has_attribute_aligned GCC_VERSION_SINCE(0, 0, 0)
#define __has_attribute_alloc_size GCC_VERSION_SINCE(4, 3, 0)
#define __has_attribute_artificial GCC_VERSION_SINCE(4, 3, 0)
#define __has_attribute_always_inline GCC_VERSION_SINCE(3, 1, 0)
#define __has_attribute_cdecl GCC_VERSION_SINCE(0, 0, 0)
#define __has_attribute_cold GCC_VERSION_SINCE(4, 3, 0)
#define __has_attribute_const GCC_VERSION_SINCE(2, 6, 0)
#define __has_attribute_deprecated GCC_VERSION_SINCE(3, 1, 0)
#define __has_attribute_dllexport GCC_VERSION_SINCE(0, 0, 0)
#define __has_attribute_dllimport GCC_VERSION_SINCE(0, 0, 0)
#define __has_attribute_error GCC_VERSION_SINCE(4, 3, 0)
#define __has_attribute_format GCC_VERSION_SINCE(0, 0, 0)
#define __has_attribute_hot GCC_VERSION_SINCE(4, 3, 0)
#define __has_attribute_leaf GCC_VERSION_SINCE(4, 6, 0)
#define __has_attribute_malloc GCC_VERSION_SINCE(3, 0, 0)
#define __has_attribute_no_address_safety_analysis GCC_VERSION_SINCE(4, 8, 0)
#define __has_attribute_no_sanitize_address GCC_VERSION_SINCE(4, 8, 0)
#define __has_attribute_no_sanitize_undefined GCC_VERSION_SINCE(4, 9, 0)
#define __has_attribute_noinline GCC_VERSION_SINCE(3, 1, 0)
#define __has_attribute_nonnull GCC_VERSION_SINCE(3, 3, 0)
#define __has_attribute_noreturn GCC_VERSION_SINCE(2, 5, 0)
#define __has_attribute_nothrow GCC_VERSION_SINCE(3, 3, 0)
#define __has_attribute_pure GCC_VERSION_SINCE(2, 96, 0)
#define __has_attribute_returns_nonnull GCC_VERSION_SINCE(4, 9, 0)
#define __has_attribute_returns_twice GCC_VERSION_SINCE(4, 1, 0)
#define __has_attribute_stdcall GCC_VERSION_SINCE(0, 0, 0)
#define __has_attribute_unused GCC_VERSION_SINCE(0, 0, 0)
#define __has_attribute_visibility GCC_VERSION_SINCE(3, 3, 0)
#define __has_attribute_visibility GCC_VERSION_SINCE(3, 3, 0)
#define __has_attribute_warn_unused_result GCC_VERSION_SINCE(3, 4, 0)
#define __has_attribute_warning GCC_VERSION_SINCE(4, 3, 0)
#define __has_attribute_weak GCC_VERSION_SINCE(0, 0, 0)
#
#endif
#if !defined(__has_c_attribute)
#
#define __has_c_attribute(...) 0
#endif
#if !defined(__has_declspec_attribute)
#define __has_declspec_attribute(...) __has_declspec_attribute_##__VA_ARGS__
#define __has_declspec_attribute_align MSC_VERSION_SINCE( 800)
#define __has_declspec_attribute_deprecated MSC_VERSION_SINCE(1300)
#define __has_declspec_attribute_dllexport MSC_VERSION_SINCE( 800)
#define __has_declspec_attribute_dllimport MSC_VERSION_SINCE( 800)
#define __has_declspec_attribute_noalias MSC_VERSION_SINCE( 800)
#define __has_declspec_attribute_noinline MSC_VERSION_SINCE(1300)
#define __has_declspec_attribute_noreturn MSC_VERSION_SINCE(1100)
#define __has_declspec_attribute_nothrow MSC_VERSION_SINCE( 800)
#define __has_declspec_attribute_restrict MSC_VERSION_SINCE( 800)
#
#endif
#if !defined(__has_builtin)
#
#
#define __has_builtin(...) __has_builtin_##__VA_ARGS__
#define __has_builtin____builtin_add_overflow GCC_VERSION_SINCE(5, 1, 0)
#define __has_builtin____builtin_bswap16 GCC_VERSION_SINCE(4, 8, 0) 
#define __has_builtin____builtin_bswap32 GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_bswap64 GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_clz GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_clzl GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_clzll GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_constant_p GCC_VERSION_SINCE(2,95, 3)
#define __has_builtin____builtin_ctz GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_ctzl GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_ctzll GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_mul_overflow GCC_VERSION_SINCE(5, 1, 0)
#define __has_builtin____builtin_mul_overflow_p GCC_VERSION_SINCE(7, 0, 0)
#define __has_builtin____builtin_popcount GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_popcountl GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_popcountll GCC_VERSION_SINCE(3, 6, 0)
#define __has_builtin____builtin_sub_overflow GCC_VERSION_SINCE(5, 1, 0)
#
#if defined(HAVE_BUILTIN____BUILTIN_ADD_OVERFLOW)
#undef __has_builtin____builtin_add_overflow
#define __has_builtin____builtin_add_overflow HAVE_BUILTIN____BUILTIN_ADD_OVERFLOW
#endif
#if defined(HAVE_BUILTIN____BUILTIN_BSWAP16)
#undef __has_builtin____builtin_bswap16
#define __has_builtin____builtin_bswap16 HAVE_BUILTIN____BUILTIN_BSWAP16
#endif
#if defined(HAVE_BUILTIN____BUILTIN_BSWAP32)
#undef __has_builtin____builtin_bswap32
#define __has_builtin____builtin_bswap16 HAVE_BUILTIN____BUILTIN_BSWAP32
#endif
#if defined(HAVE_BUILTIN____BUILTIN_BSWAP64)
#undef __has_builtin____builtin_bswap64
#define __has_builtin____builtin_bswap64 HAVE_BUILTIN____BUILTIN_BSWAP64
#endif
#if defined(HAVE_BUILTIN____BUILTIN_CLZ)
#undef __has_builtin____builtin_clz
#define __has_builtin____builtin_clz HAVE_BUILTIN____BUILTIN_CLZ
#endif
#if defined(HAVE_BUILTIN____BUILTIN_CLZL)
#undef __has_builtin____builtin_clzl
#define __has_builtin____builtin_clzl HAVE_BUILTIN____BUILTIN_CLZL
#endif
#if defined(HAVE_BUILTIN____BUILTIN_CLZLL)
#undef __has_builtin____builtin_clzll
#define __has_builtin____builtin_clzll HAVE_BUILTIN____BUILTIN_CLZLL
#endif
#if defined(HAVE_BUILTIN____BUILTIN_CONSTANT_P)
#undef __has_builtin____builtin_constant_p
#define __has_builtin____builtin_constant_p HAVE_BUILTIN____BUILTIN_CONSTANT_P
#endif
#if defined(HAVE_BUILTIN____BUILTIN_CTZ)
#undef __has_builtin____builtin_ctz
#define __has_builtin____builtin_ctz HAVE_BUILTIN____BUILTIN_CTZ
#endif
#if defined(HAVE_BUILTIN____BUILTIN_CTZL)
#undef __has_builtin____builtin_ctzl
#define __has_builtin____builtin_ctzl HAVE_BUILTIN____BUILTIN_CTZL
#endif
#if defined(HAVE_BUILTIN____BUILTIN_CTZLL)
#undef __has_builtin____builtin_ctzll
#define __has_builtin____builtin_ctzll HAVE_BUILTIN____BUILTIN_CTZLL
#endif
#if defined(HAVE_BUILTIN____BUILTIN_MUL_OVERFLOW)
#undef __has_builtin____builtin_mul_overflow
#define __has_builtin____builtin_mul_overflow HAVE_BUILTIN____BUILTIN_MUL_OVERFLOW
#endif
#if defined(HAVE_BUILTIN____BUILTIN_MUL_OVERFLOW_P)
#undef __has_builtin____builtin_mul_overflow_p
#define __has_builtin____builtin_mul_overflow_p HAVE_BUILTIN____BUILTIN_MUL_OVERFLOW_P
#endif
#if defined(HAVE_BUILTIN____BUILTIN_POPCOUNT)
#undef __has_builtin____builtin_popcount
#define __has_builtin____builtin_popcount HAVE_BUILTIN____BUILTIN_POPCOUNT
#endif
#if defined(HAVE_BUILTIN____BUILTIN_POPCOUNTL)
#undef __has_builtin____builtin_popcountl
#define __has_builtin____builtin_popcountl HAVE_BUILTIN____BUILTIN_POPCOUNTL
#endif
#if defined(HAVE_BUILTIN____BUILTIN_POPCOUNTLL)
#undef __has_builtin____builtin_popcountll
#define __has_builtin____builtin_popcountll HAVE_BUILTIN____BUILTIN_POPCOUNTLL
#endif
#if defined(HAVE_BUILTIN____BUILTIN_SUB_OVERFLOW)
#undef __has_builtin____builtin_SUB_overflow
#define __has_builtin____builtin_sub_overflow HAVE_BUILTIN____BUILTIN_SUB_OVERFLOW
#endif
#endif
#if !defined(__has_feature)
#define __has_feature(...) 0
#endif
#if !defined(__has_extension)
#
#define __has_extension __has_feature
#endif
#if !defined(__has_warning)
#
#define __has_warning(...) 0
#endif
#if !defined(__GNUC__)
#define __extension__ 
#endif
#if !defined(MAYBE_UNUSED)
#define MAYBE_UNUSED(x) x
#endif
#if !defined(WARN_UNUSED_RESULT)
#define WARN_UNUSED_RESULT(x) x
#endif
#define RB_OBJ_BUILTIN_TYPE(obj) rb_obj_builtin_type(obj)
#define OBJ_BUILTIN_TYPE(obj) RB_OBJ_BUILTIN_TYPE(obj)
#if defined(__GNUC__)
#define rb_obj_builtin_type(obj) __extension__({ VALUE arg_obj = (obj); RB_SPECIAL_CONST_P(arg_obj) ? -1 : RB_BUILTIN_TYPE(arg_obj); })
#else
#include "ruby/ruby.h"
static inline int
rb_obj_builtin_type(VALUE obj)
{
return RB_SPECIAL_CONST_P(obj) ? -1 :
RB_BUILTIN_TYPE(obj);
}
#endif
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define FLEX_ARY_LEN 
#elif defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define FLEX_ARY_LEN 0 
#else
#define FLEX_ARY_LEN 1 
#endif
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define BITFIELD(type, name, size) type name : size
#else
#define BITFIELD(type, name, size) unsigned int name : size
#endif
