#include "hedley.h"
#include "check.h"
#include "simde-arch.h"
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
#define SIMDE_ALIGN(alignment) _Alignas(alignment)
#elif (defined(__cplusplus) && (__cplusplus >= 201103L))
#define SIMDE_ALIGN(alignment) alignas(alignment)
#elif HEDLEY_GCC_VERSION_CHECK(2, 95, 0) || HEDLEY_CRAY_VERSION_CHECK(8, 4, 0) || HEDLEY_IBM_VERSION_CHECK(11, 1, 0) || HEDLEY_INTEL_VERSION_CHECK(13, 0, 0) || HEDLEY_PGI_VERSION_CHECK(19, 4, 0) || HEDLEY_ARM_VERSION_CHECK(4, 1, 0) || HEDLEY_TINYC_VERSION_CHECK(0, 9, 24) || HEDLEY_TI_VERSION_CHECK(8, 1, 0)
#define SIMDE_ALIGN(alignment) __attribute__((aligned(alignment)))
#elif defined(_MSC_VER) && (!defined(_M_IX86) || defined(_M_AMD64))
#define SIMDE_ALIGN(alignment) __declspec(align(alignment))
#else
#define SIMDE_ALIGN(alignment)
#endif
#define simde_assert_aligned(alignment, val) simde_assert_int(((uintptr_t)(val)) % (alignment), ==, 0)
#if HEDLEY_GCC_HAS_ATTRIBUTE(vector_size, 4, 6, 0)
#define SIMDE__ENABLE_GCC_VEC_EXT
#endif
#if !defined(SIMDE_ENABLE_OPENMP) && ((defined(_OPENMP) && (_OPENMP >= 201307L)) || (defined(_OPENMP_SIMD) && (_OPENMP_SIMD >= 201307L)))
#define SIMDE_ENABLE_OPENMP
#endif
#if !defined(SIMDE_ENABLE_CILKPLUS) && defined(__cilk)
#define SIMDE_ENABLE_CILKPLUS
#endif
#if defined(SIMDE_ENABLE_OPENMP)
#define SIMDE__VECTORIZE _Pragma("omp simd")
#define SIMDE__VECTORIZE_SAFELEN(l) HEDLEY_PRAGMA(omp simd safelen(l))
#define SIMDE__VECTORIZE_REDUCTION(r) HEDLEY_PRAGMA(omp simd reduction(r))
#define SIMDE__VECTORIZE_ALIGNED(a) HEDLEY_PRAGMA(omp simd aligned(a))
#elif defined(SIMDE_ENABLE_CILKPLUS)
#define SIMDE__VECTORIZE _Pragma("simd")
#define SIMDE__VECTORIZE_SAFELEN(l) HEDLEY_PRAGMA(simd vectorlength(l))
#define SIMDE__VECTORIZE_REDUCTION(r) HEDLEY_PRAGMA(simd reduction(r))
#define SIMDE__VECTORIZE_ALIGNED(a) HEDLEY_PRAGMA(simd aligned(a))
#elif defined(__INTEL_COMPILER)
#define SIMDE__VECTORIZE _Pragma("simd")
#define SIMDE__VECTORIZE_SAFELEN(l) HEDLEY_PRAGMA(simd vectorlength(l))
#define SIMDE__VECTORIZE_REDUCTION(r) HEDLEY_PRAGMA(simd reduction(r))
#define SIMDE__VECTORIZE_ALIGNED(a)
#elif defined(__clang__)
#define SIMDE__VECTORIZE _Pragma("clang loop vectorize(enable)")
#define SIMDE__VECTORIZE_SAFELEN(l) HEDLEY_PRAGMA(clang loop vectorize_width(l))
#define SIMDE__VECTORIZE_REDUCTION(r) SIMDE__VECTORIZE
#define SIMDE__VECTORIZE_ALIGNED(a)
#elif HEDLEY_GCC_VERSION_CHECK(4, 9, 0)
#define SIMDE__VECTORIZE _Pragma("GCC ivdep")
#define SIMDE__VECTORIZE_SAFELEN(l) SIMDE__VECTORIZE
#define SIMDE__VECTORIZE_REDUCTION(r) SIMDE__VECTORIZE
#define SIMDE__VECTORIZE_ALIGNED(a)
#elif HEDLEY_CRAY_VERSION_CHECK(5, 0, 0)
#define SIMDE__VECTORIZE _Pragma("_CRI ivdep")
#define SIMDE__VECTORIZE_SAFELEN(l) SIMDE__VECTORIZE
#define SIMDE__VECTORIZE_REDUCTION(r) SIMDE__VECTORIZE
#define SIMDE__VECTORIZE_ALIGNED(a)
#else
#define SIMDE__VECTORIZE
#define SIMDE__VECTORIZE_SAFELEN(l)
#define SIMDE__VECTORIZE_REDUCTION(r)
#define SIMDE__VECTORIZE_ALIGNED(a)
#endif
#if HEDLEY_GCC_HAS_ATTRIBUTE(unused, 3, 1, 0)
#define SIMDE__UNUSED __attribute__((__unused__))
#else
#define SIMDE__UNUSED
#endif
#if HEDLEY_GCC_HAS_ATTRIBUTE(artificial, 4, 3, 0)
#define SIMDE__ARTIFICIAL __attribute__((__artificial__))
#else
#define SIMDE__ARTIFICIAL
#endif
#if defined(SIMDE_NO_INLINE)
#define SIMDE__FUNCTION_ATTRIBUTES HEDLEY_NEVER_INLINE SIMDE__UNUSED static
#else
#define SIMDE__FUNCTION_ATTRIBUTES HEDLEY_INLINE SIMDE__ARTIFICIAL static
#endif
#if defined(_MSC_VER)
#define SIMDE__BEGIN_DECLS HEDLEY_DIAGNOSTIC_PUSH __pragma(warning(disable : 4996 4204)) HEDLEY_BEGIN_C_DECLS
#define SIMDE__END_DECLS HEDLEY_DIAGNOSTIC_POP HEDLEY_END_C_DECLS
#else
#define SIMDE__BEGIN_DECLS HEDLEY_BEGIN_C_DECLS
#define SIMDE__END_DECLS HEDLEY_END_C_DECLS
#endif
#if defined(__SIZEOF_INT128__)
#define SIMDE__HAVE_INT128
typedef __int128 simde_int128;
typedef unsigned __int128 simde_uint128;
#endif
#if !defined(SIMDE_FLOAT32_TYPE)
#define SIMDE_FLOAT32_TYPE float
#define SIMDE_FLOAT32_C(value) value##f
#else
#define SIMDE_FLOAT32_C(value) ((SIMDE_FLOAT32_TYPE)value)
#endif
typedef SIMDE_FLOAT32_TYPE simde_float32;
HEDLEY_STATIC_ASSERT(sizeof(simde_float32) == 4,
"Unable to find 32-bit floating-point type.");
#if !defined(SIMDE_FLOAT64_TYPE)
#define SIMDE_FLOAT64_TYPE double
#define SIMDE_FLOAT64_C(value) value
#else
#define SIMDE_FLOAT32_C(value) ((SIMDE_FLOAT64_TYPE)value)
#endif
typedef SIMDE_FLOAT64_TYPE simde_float64;
HEDLEY_STATIC_ASSERT(sizeof(simde_float64) == 8,
"Unable to find 64-bit floating-point type.");
#if !defined(SIMDE_NO_ASSUME_VECTORIZATION) && !defined(SIMDE_ASSUME_VECTORIZATION)
#if defined(__SSE__) || defined(__ARM_NEON) || defined(__mips_msa) || defined(__ALTIVEC__)
#define SIMDE_ASSUME_VECTORIZATION
#endif
#endif
#if HEDLEY_CLANG_HAS_BUILTIN(__builtin_shufflevector)
#define SIMDE__SHUFFLE_VECTOR(elem_size, vec_size, a, b, ...) __builtin_shufflevector(a, b, __VA_ARGS__)
#elif HEDLEY_GCC_HAS_BUILTIN(__builtin_shuffle, 4, 7, 0) && !defined(__INTEL_COMPILER)
#define SIMDE__SHUFFLE_VECTOR(elem_size, vec_size, a, b, ...) __builtin_shuffle(a, b, (int##elem_size##_t __attribute__( (__vector_size__(vec_size)))){__VA_ARGS__})
#endif
#if !defined(SIMDE_ACCURACY_ITERS)
#define SIMDE_ACCURACY_ITERS 2
#endif
#if defined(SIMDE__REALLY_GCC)
#undef SIMDE__REALLY_GCC
#endif
#if !defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define SIMDE__REALLY_GCC 0
#else
#define SIMDE__REALLY_GCC 1
#endif
#if defined(SIMDE__ASSUME_ALIGNED)
#undef SIMDE__ASSUME_ALIGNED
#endif
#if HEDLEY_INTEL_VERSION_CHECK(9, 0, 0)
#define SIMDE__ASSUME_ALIGNED(ptr, align) __assume_aligned(ptr, align)
#elif HEDLEY_MSVC_VERSION_CHECK(13, 10, 0)
#define SIMDE__ASSUME_ALIGNED(ptr, align) __assume((((char *)ptr) - ((char *)0)) % (align) == 0)
#elif HEDLEY_GCC_HAS_BUILTIN(__builtin_assume_aligned, 4, 7, 0)
#define SIMDE__ASSUME_ALIGNED(ptr, align) (ptr = (__typeof__(ptr))__builtin_assume_aligned((ptr), align))
#elif HEDLEY_CLANG_HAS_BUILTIN(__builtin_assume)
#define SIMDE__ASSUME_ALIGNED(ptr, align) __builtin_assume((((char *)ptr) - ((char *)0)) % (align) == 0)
#elif HEDLEY_GCC_HAS_BUILTIN(__builtin_unreachable, 4, 5, 0)
#define SIMDE__ASSUME_ALIGNED(ptr, align) ((((char *)ptr) - ((char *)0)) % (align) == 0) ? (1) : (__builtin_unreachable(), 0)
#else
#define SIMDE__ASSUME_ALIGNED(ptr, align)
#endif
#if !defined(SIMDE_IGNORE_COMPILER_BUGS)
#if SIMDE__REALLY_GCC
#if !HEDLEY_GCC_VERSION_CHECK(4, 9, 0)
#define SIMDE_BUG_GCC_REV_208793
#endif
#if !HEDLEY_GCC_VERSION_CHECK(5, 0, 0)
#define SIMDE_BUG_GCC_BAD_MM_SRA_EPI32 
#endif
#if !HEDLEY_GCC_VERSION_CHECK(4, 6, 0)
#define SIMDE_BUG_GCC_BAD_MM_EXTRACT_EPI8 
#endif
#endif
#if defined(__EMSCRIPTEN__)
#define SIMDE_BUG_EMSCRIPTEN_MISSING_IMPL 
#define SIMDE_BUG_EMSCRIPTEN_5242
#endif
#endif
