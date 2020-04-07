#include "nvim/macros.h"
#if defined(FUNC_ATTR_MALLOC)
#undef FUNC_ATTR_MALLOC
#endif
#if defined(FUNC_ATTR_ALLOC_SIZE)
#undef FUNC_ATTR_ALLOC_SIZE
#endif
#if defined(FUNC_ATTR_ALLOC_SIZE_PROD)
#undef FUNC_ATTR_ALLOC_SIZE_PROD
#endif
#if defined(FUNC_ATTR_ALLOC_ALIGN)
#undef FUNC_ATTR_ALLOC_ALIGN
#endif
#if defined(FUNC_ATTR_PURE)
#undef FUNC_ATTR_PURE
#endif
#if defined(FUNC_ATTR_CONST)
#undef FUNC_ATTR_CONST
#endif
#if defined(FUNC_ATTR_WARN_UNUSED_RESULT)
#undef FUNC_ATTR_WARN_UNUSED_RESULT
#endif
#if defined(FUNC_ATTR_ALWAYS_INLINE)
#undef FUNC_ATTR_ALWAYS_INLINE
#endif
#if defined(FUNC_ATTR_UNUSED)
#undef FUNC_ATTR_UNUSED
#endif
#if defined(FUNC_ATTR_NONNULL_ALL)
#undef FUNC_ATTR_NONNULL_ALL
#endif
#if defined(FUNC_ATTR_NONNULL_ARG)
#undef FUNC_ATTR_NONNULL_ARG
#endif
#if defined(FUNC_ATTR_NONNULL_RET)
#undef FUNC_ATTR_NONNULL_RET
#endif
#if defined(FUNC_ATTR_NORETURN)
#undef FUNC_ATTR_NORETURN
#endif
#if defined(FUNC_ATTR_NO_SANITIZE_UNDEFINED)
#undef FUNC_ATTR_NO_SANITIZE_UNDEFINED
#endif
#if defined(FUNC_ATTR_PRINTF)
#undef FUNC_ATTR_PRINTF
#endif
#if !defined(DID_REAL_ATTR)
#define DID_REAL_ATTR
#if defined(__GNUC__)
#define REAL_FATTR_MALLOC __attribute__((malloc))
#define REAL_FATTR_ALLOC_ALIGN(x) __attribute__((alloc_align(x)))
#define REAL_FATTR_PURE __attribute__ ((pure))
#define REAL_FATTR_CONST __attribute__((const))
#define REAL_FATTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define REAL_FATTR_ALWAYS_INLINE __attribute__((always_inline))
#define REAL_FATTR_UNUSED __attribute__((unused))
#define REAL_FATTR_NONNULL_ALL __attribute__((nonnull))
#define REAL_FATTR_NONNULL_ARG(...) __attribute__((nonnull(__VA_ARGS__)))
#define REAL_FATTR_NORETURN __attribute__((noreturn))
#define REAL_FATTR_PRINTF(x, y) __attribute__((format (printf, x, y)))
#if NVIM_HAS_ATTRIBUTE(returns_nonnull)
#define REAL_FATTR_NONNULL_RET __attribute__((returns_nonnull))
#endif
#if NVIM_HAS_ATTRIBUTE(alloc_size)
#define REAL_FATTR_ALLOC_SIZE(x) __attribute__((alloc_size(x)))
#define REAL_FATTR_ALLOC_SIZE_PROD(x, y) __attribute__((alloc_size(x, y)))
#endif
#if NVIM_HAS_ATTRIBUTE(no_sanitize_undefined)
#define REAL_FATTR_NO_SANITIZE_UNDEFINED __attribute__((no_sanitize_undefined))
#elif NVIM_HAS_ATTRIBUTE(no_sanitize)
#define REAL_FATTR_NO_SANITIZE_UNDEFINED __attribute__((no_sanitize("undefined")))
#endif
#endif
#if !defined(REAL_FATTR_MALLOC)
#define REAL_FATTR_MALLOC
#endif
#if !defined(REAL_FATTR_ALLOC_SIZE)
#define REAL_FATTR_ALLOC_SIZE(x)
#endif
#if !defined(REAL_FATTR_ALLOC_SIZE_PROD)
#define REAL_FATTR_ALLOC_SIZE_PROD(x, y)
#endif
#if !defined(REAL_FATTR_ALLOC_ALIGN)
#define REAL_FATTR_ALLOC_ALIGN(x)
#endif
#if !defined(REAL_FATTR_PURE)
#define REAL_FATTR_PURE
#endif
#if !defined(REAL_FATTR_CONST)
#define REAL_FATTR_CONST
#endif
#if !defined(REAL_FATTR_WARN_UNUSED_RESULT)
#define REAL_FATTR_WARN_UNUSED_RESULT
#endif
#if !defined(REAL_FATTR_ALWAYS_INLINE)
#define REAL_FATTR_ALWAYS_INLINE
#endif
#if !defined(REAL_FATTR_UNUSED)
#define REAL_FATTR_UNUSED
#endif
#if !defined(REAL_FATTR_NONNULL_ALL)
#define REAL_FATTR_NONNULL_ALL
#endif
#if !defined(REAL_FATTR_NONNULL_ARG)
#define REAL_FATTR_NONNULL_ARG(...)
#endif
#if !defined(REAL_FATTR_NONNULL_RET)
#define REAL_FATTR_NONNULL_RET
#endif
#if !defined(REAL_FATTR_NORETURN)
#define REAL_FATTR_NORETURN
#endif
#if !defined(REAL_FATTR_NO_SANITIZE_UNDEFINED)
#define REAL_FATTR_NO_SANITIZE_UNDEFINED
#endif
#if !defined(REAL_FATTR_PRINTF)
#define REAL_FATTR_PRINTF(x, y)
#endif
#endif
#if defined(DEFINE_FUNC_ATTRIBUTES)
#define FUNC_API_FAST
#define FUNC_API_NOEXPORT
#define FUNC_API_REMOTE_ONLY
#define FUNC_API_LUA_ONLY
#define FUNC_API_SINCE(X)
#define FUNC_API_DEPRECATED_SINCE(X)
#define FUNC_ATTR_MALLOC REAL_FATTR_MALLOC
#define FUNC_ATTR_ALLOC_SIZE(x) REAL_FATTR_ALLOC_SIZE(x)
#define FUNC_ATTR_ALLOC_SIZE_PROD(x, y) REAL_FATTR_ALLOC_SIZE_PROD(x, y)
#define FUNC_ATTR_ALLOC_ALIGN(x) REAL_FATTR_ALLOC_ALIGN(x)
#define FUNC_ATTR_PURE REAL_FATTR_PURE
#define FUNC_ATTR_CONST REAL_FATTR_CONST
#define FUNC_ATTR_WARN_UNUSED_RESULT REAL_FATTR_WARN_UNUSED_RESULT
#define FUNC_ATTR_ALWAYS_INLINE REAL_FATTR_ALWAYS_INLINE
#define FUNC_ATTR_UNUSED REAL_FATTR_UNUSED
#define FUNC_ATTR_NONNULL_ALL REAL_FATTR_NONNULL_ALL
#define FUNC_ATTR_NONNULL_ARG(...) REAL_FATTR_NONNULL_ARG(__VA_ARGS__)
#define FUNC_ATTR_NONNULL_RET REAL_FATTR_NONNULL_RET
#define FUNC_ATTR_NORETURN REAL_FATTR_NORETURN
#define FUNC_ATTR_NO_SANITIZE_UNDEFINED REAL_FATTR_NO_SANITIZE_UNDEFINED
#define FUNC_ATTR_PRINTF(x, y) REAL_FATTR_PRINTF(x, y)
#elif !defined(DO_NOT_DEFINE_EMPTY_ATTRIBUTES)
#define FUNC_ATTR_MALLOC
#define FUNC_ATTR_ALLOC_SIZE(x)
#define FUNC_ATTR_ALLOC_SIZE_PROD(x, y)
#define FUNC_ATTR_ALLOC_ALIGN(x)
#define FUNC_ATTR_PURE
#define FUNC_ATTR_CONST
#define FUNC_ATTR_WARN_UNUSED_RESULT
#define FUNC_ATTR_ALWAYS_INLINE
#define FUNC_ATTR_UNUSED
#define FUNC_ATTR_NONNULL_ALL
#define FUNC_ATTR_NONNULL_ARG(...)
#define FUNC_ATTR_NONNULL_RET
#define FUNC_ATTR_NORETURN
#define FUNC_ATTR_NO_SANITIZE_UNDEFINED
#define FUNC_ATTR_PRINTF(x, y)
#endif
