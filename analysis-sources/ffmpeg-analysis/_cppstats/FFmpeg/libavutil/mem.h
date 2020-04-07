#include <limits.h>
#include <stdint.h>
#include "attributes.h"
#include "error.h"
#include "avutil.h"
#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1110 || defined(__SUNPRO_C)
#define DECLARE_ALIGNED(n,t,v) t __attribute__ ((aligned (n))) v
#define DECLARE_ASM_ALIGNED(n,t,v) t __attribute__ ((aligned (n))) v
#define DECLARE_ASM_CONST(n,t,v) const t __attribute__ ((aligned (n))) v
#elif defined(__DJGPP__)
#define DECLARE_ALIGNED(n,t,v) t __attribute__ ((aligned (FFMIN(n, 16)))) v
#define DECLARE_ASM_ALIGNED(n,t,v) t av_used __attribute__ ((aligned (FFMIN(n, 16)))) v
#define DECLARE_ASM_CONST(n,t,v) static const t av_used __attribute__ ((aligned (FFMIN(n, 16)))) v
#elif defined(__GNUC__) || defined(__clang__)
#define DECLARE_ALIGNED(n,t,v) t __attribute__ ((aligned (n))) v
#define DECLARE_ASM_ALIGNED(n,t,v) t av_used __attribute__ ((aligned (n))) v
#define DECLARE_ASM_CONST(n,t,v) static const t av_used __attribute__ ((aligned (n))) v
#elif defined(_MSC_VER)
#define DECLARE_ALIGNED(n,t,v) __declspec(align(n)) t v
#define DECLARE_ASM_ALIGNED(n,t,v) __declspec(align(n)) t v
#define DECLARE_ASM_CONST(n,t,v) __declspec(align(n)) static const t v
#else
#define DECLARE_ALIGNED(n,t,v) t v
#define DECLARE_ASM_ALIGNED(n,t,v) t v
#define DECLARE_ASM_CONST(n,t,v) static const t v
#endif
#if AV_GCC_VERSION_AT_LEAST(3,1)
#define av_malloc_attrib __attribute__((__malloc__))
#else
#define av_malloc_attrib
#endif
#if AV_GCC_VERSION_AT_LEAST(4,3)
#define av_alloc_size(...) __attribute__((alloc_size(__VA_ARGS__)))
#else
#define av_alloc_size(...)
#endif
void *av_malloc(size_t size) av_malloc_attrib av_alloc_size(1);
void *av_mallocz(size_t size) av_malloc_attrib av_alloc_size(1);
av_alloc_size(1, 2) void *av_malloc_array(size_t nmemb, size_t size);
av_alloc_size(1, 2) void *av_mallocz_array(size_t nmemb, size_t size);
void *av_calloc(size_t nmemb, size_t size) av_malloc_attrib;
void *av_realloc(void *ptr, size_t size) av_alloc_size(2);
av_warn_unused_result
int av_reallocp(void *ptr, size_t size);
void *av_realloc_f(void *ptr, size_t nelem, size_t elsize);
av_alloc_size(2, 3) void *av_realloc_array(void *ptr, size_t nmemb, size_t size);
int av_reallocp_array(void *ptr, size_t nmemb, size_t size);
void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size);
void av_fast_malloc(void *ptr, unsigned int *size, size_t min_size);
void av_fast_mallocz(void *ptr, unsigned int *size, size_t min_size);
void av_free(void *ptr);
void av_freep(void *ptr);
char *av_strdup(const char *s) av_malloc_attrib;
char *av_strndup(const char *s, size_t len) av_malloc_attrib;
void *av_memdup(const void *p, size_t size);
void av_memcpy_backptr(uint8_t *dst, int back, int cnt);
void av_dynarray_add(void *tab_ptr, int *nb_ptr, void *elem);
av_warn_unused_result
int av_dynarray_add_nofree(void *tab_ptr, int *nb_ptr, void *elem);
void *av_dynarray2_add(void **tab_ptr, int *nb_ptr, size_t elem_size,
const uint8_t *elem_data);
static inline int av_size_mult(size_t a, size_t b, size_t *r)
{
size_t t = a * b;
if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
return AVERROR(EINVAL);
*r = t;
return 0;
}
void av_max_alloc(size_t max);
