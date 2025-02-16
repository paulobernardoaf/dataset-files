#if defined(__GNUC__)
#define AV_GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#define AV_GCC_VERSION_AT_MOST(x,y) (__GNUC__ < (x) || __GNUC__ == (x) && __GNUC_MINOR__ <= (y))
#else
#define AV_GCC_VERSION_AT_LEAST(x,y) 0
#define AV_GCC_VERSION_AT_MOST(x,y) 0
#endif
#if !defined(av_always_inline)
#if AV_GCC_VERSION_AT_LEAST(3,1)
#define av_always_inline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define av_always_inline __forceinline
#else
#define av_always_inline inline
#endif
#endif
#if !defined(av_extern_inline)
#if defined(__ICL) && __ICL >= 1210 || defined(__GNUC_STDC_INLINE__)
#define av_extern_inline extern inline
#else
#define av_extern_inline inline
#endif
#endif
#if AV_GCC_VERSION_AT_LEAST(3,4)
#define av_warn_unused_result __attribute__((warn_unused_result))
#else
#define av_warn_unused_result
#endif
#if AV_GCC_VERSION_AT_LEAST(3,1)
#define av_noinline __attribute__((noinline))
#elif defined(_MSC_VER)
#define av_noinline __declspec(noinline)
#else
#define av_noinline
#endif
#if AV_GCC_VERSION_AT_LEAST(3,1) || defined(__clang__)
#define av_pure __attribute__((pure))
#else
#define av_pure
#endif
#if AV_GCC_VERSION_AT_LEAST(2,6) || defined(__clang__)
#define av_const __attribute__((const))
#else
#define av_const
#endif
#if AV_GCC_VERSION_AT_LEAST(4,3) || defined(__clang__)
#define av_cold __attribute__((cold))
#else
#define av_cold
#endif
#if AV_GCC_VERSION_AT_LEAST(4,1) && !defined(__llvm__)
#define av_flatten __attribute__((flatten))
#else
#define av_flatten
#endif
#if AV_GCC_VERSION_AT_LEAST(3,1)
#define attribute_deprecated __attribute__((deprecated))
#elif defined(_MSC_VER)
#define attribute_deprecated __declspec(deprecated)
#else
#define attribute_deprecated
#endif
#if !defined(AV_NOWARN_DEPRECATED)
#if AV_GCC_VERSION_AT_LEAST(4,6)
#define AV_NOWARN_DEPRECATED(code) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") code _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define AV_NOWARN_DEPRECATED(code) __pragma(warning(push)) __pragma(warning(disable : 4996)) code; __pragma(warning(pop))
#else
#define AV_NOWARN_DEPRECATED(code) code
#endif
#endif
#if defined(__GNUC__) || defined(__clang__)
#define av_unused __attribute__((unused))
#else
#define av_unused
#endif
#if AV_GCC_VERSION_AT_LEAST(3,1) || defined(__clang__)
#define av_used __attribute__((used))
#else
#define av_used
#endif
#if AV_GCC_VERSION_AT_LEAST(3,3) || defined(__clang__)
#define av_alias __attribute__((may_alias))
#else
#define av_alias
#endif
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__INTEL_COMPILER)
#define av_uninit(x) x=x
#else
#define av_uninit(x) x
#endif
#if defined(__GNUC__) || defined(__clang__)
#define av_builtin_constant_p __builtin_constant_p
#define av_printf_format(fmtpos, attrpos) __attribute__((__format__(__printf__, fmtpos, attrpos)))
#else
#define av_builtin_constant_p(x) 0
#define av_printf_format(fmtpos, attrpos)
#endif
#if AV_GCC_VERSION_AT_LEAST(2,5) || defined(__clang__)
#define av_noreturn __attribute__((noreturn))
#else
#define av_noreturn
#endif
