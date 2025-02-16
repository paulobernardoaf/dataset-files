#if defined(JEMALLOC_DEBUG) || defined(JEMALLOC_CODE_COVERAGE)
#define JEMALLOC_ALWAYS_INLINE
#define JEMALLOC_ALWAYS_INLINE_C static
#undef JEMALLOC_INLINE
#define JEMALLOC_INLINE_C static
#define inline
#else
#if defined(JEMALLOC_HAVE_ATTR)
#define JEMALLOC_ALWAYS_INLINE static inline JEMALLOC_ATTR(unused) JEMALLOC_ATTR(always_inline)
#define JEMALLOC_ALWAYS_INLINE_C static inline JEMALLOC_ATTR(always_inline)
#else
#define JEMALLOC_ALWAYS_INLINE static inline
#define JEMALLOC_ALWAYS_INLINE_C static inline
#endif
#define JEMALLOC_INLINE static inline
#define JEMALLOC_INLINE_C static inline
#if defined(_MSC_VER)
#define inline _inline
#endif
#endif
#define ZU(z) ((size_t)z)
#define ZI(z) ((ssize_t)z)
#define QU(q) ((uint64_t)q)
#define QI(q) ((int64_t)q)
#define KZU(z) ZU(z##ULL)
#define KZI(z) ZI(z##LL)
#define KQU(q) QU(q##ULL)
#define KQI(q) QI(q##LL)
#if !defined(__DECONST)
#define __DECONST(type, var) ((type)(uintptr_t)(const void *)(var))
#endif
#if !defined(JEMALLOC_HAS_RESTRICT)
#define restrict
#endif
