#if !defined(JEMALLOC_H_)
#define JEMALLOC_H_
#if defined(__cplusplus)
extern "C" {
#endif


#define JEMALLOC_HAVE_ATTR


#define JEMALLOC_HAVE_ATTR_ALLOC_SIZE


#define JEMALLOC_HAVE_ATTR_FORMAT_GNU_PRINTF


#define JEMALLOC_HAVE_ATTR_FORMAT_PRINTF





#define JEMALLOC_OVERRIDE_MEMALIGN
#define JEMALLOC_OVERRIDE_VALLOC








#define JEMALLOC_USABLE_SIZE_CONST






#define JEMALLOC_USE_CXX_THROW

#if defined(_MSC_VER)
#if defined(_WIN64)
#define LG_SIZEOF_PTR_WIN 3
#else
#define LG_SIZEOF_PTR_WIN 2
#endif
#endif


#define LG_SIZEOF_PTR 3






#if !defined(JEMALLOC_NO_RENAME)
#define je_malloc_conf malloc_conf
#define je_malloc_message malloc_message
#define je_malloc malloc
#define je_calloc calloc
#define je_posix_memalign posix_memalign
#define je_aligned_alloc aligned_alloc
#define je_realloc realloc
#define je_free free
#define je_mallocx mallocx
#define je_rallocx rallocx
#define je_xallocx xallocx
#define je_sallocx sallocx
#define je_dallocx dallocx
#define je_sdallocx sdallocx
#define je_nallocx nallocx
#define je_mallctl mallctl
#define je_mallctlnametomib mallctlnametomib
#define je_mallctlbymib mallctlbymib
#define je_malloc_stats_print malloc_stats_print

#define je_memalign memalign
#define je_valloc valloc
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <strings.h>

#define JEMALLOC_VERSION "4.5.0-0-g04380e79f1e2428bd0ad000bbc6e3d2dfc6b66a5"
#define JEMALLOC_VERSION_MAJOR 4
#define JEMALLOC_VERSION_MINOR 5
#define JEMALLOC_VERSION_BUGFIX 0
#define JEMALLOC_VERSION_NREV 0
#define JEMALLOC_VERSION_GID "04380e79f1e2428bd0ad000bbc6e3d2dfc6b66a5"

#define MALLOCX_LG_ALIGN(la) ((int)(la))
#if LG_SIZEOF_PTR == 2
#define MALLOCX_ALIGN(a) ((int)(ffs((int)(a))-1))
#else
#define MALLOCX_ALIGN(a) ((int)(((size_t)(a) < (size_t)INT_MAX) ? ffs((int)(a))-1 : ffs((int)(((size_t)(a))>>32))+31))


#endif
#define MALLOCX_ZERO ((int)0x40)




#define MALLOCX_TCACHE(tc) ((int)(((tc)+2) << 8))
#define MALLOCX_TCACHE_NONE MALLOCX_TCACHE(-1)



#define MALLOCX_ARENA(a) ((((int)(a))+1) << 20)

#if defined(__cplusplus) && defined(JEMALLOC_USE_CXX_THROW)
#define JEMALLOC_CXX_THROW throw()
#else
#define JEMALLOC_CXX_THROW
#endif

#if _MSC_VER
#define JEMALLOC_ATTR(s)
#define JEMALLOC_ALIGNED(s) __declspec(align(s))
#define JEMALLOC_ALLOC_SIZE(s)
#define JEMALLOC_ALLOC_SIZE2(s1, s2)
#if !defined(JEMALLOC_EXPORT)
#if defined(DLLEXPORT)
#define JEMALLOC_EXPORT __declspec(dllexport)
#else
#define JEMALLOC_EXPORT __declspec(dllimport)
#endif
#endif
#define JEMALLOC_FORMAT_PRINTF(s, i)
#define JEMALLOC_NOINLINE __declspec(noinline)
#if defined(__cplusplus)
#define JEMALLOC_NOTHROW __declspec(nothrow)
#else
#define JEMALLOC_NOTHROW
#endif
#define JEMALLOC_SECTION(s) __declspec(allocate(s))
#define JEMALLOC_RESTRICT_RETURN __declspec(restrict)
#if _MSC_VER >= 1900 && !defined(__EDG__)
#define JEMALLOC_ALLOCATOR __declspec(allocator)
#else
#define JEMALLOC_ALLOCATOR
#endif
#elif defined(JEMALLOC_HAVE_ATTR)
#define JEMALLOC_ATTR(s) __attribute__((s))
#define JEMALLOC_ALIGNED(s) JEMALLOC_ATTR(aligned(s))

#if 0
#define JEMALLOC_ALLOC_SIZE(s) JEMALLOC_ATTR(alloc_size(s))
#define JEMALLOC_ALLOC_SIZE2(s1, s2) JEMALLOC_ATTR(alloc_size(s1, s2))
#else
#define JEMALLOC_ALLOC_SIZE(s)
#define JEMALLOC_ALLOC_SIZE2(s1, s2)
#endif
#if !defined(JEMALLOC_EXPORT)
#define JEMALLOC_EXPORT JEMALLOC_ATTR(visibility("default"))
#endif
#if defined(JEMALLOC_HAVE_ATTR_FORMAT_GNU_PRINTF)
#define JEMALLOC_FORMAT_PRINTF(s, i) JEMALLOC_ATTR(format(gnu_printf, s, i))
#elif defined(JEMALLOC_HAVE_ATTR_FORMAT_PRINTF)
#define JEMALLOC_FORMAT_PRINTF(s, i) JEMALLOC_ATTR(format(printf, s, i))
#else
#define JEMALLOC_FORMAT_PRINTF(s, i)
#endif
#define JEMALLOC_NOINLINE JEMALLOC_ATTR(noinline)
#define JEMALLOC_NOTHROW JEMALLOC_ATTR(nothrow)
#define JEMALLOC_SECTION(s) JEMALLOC_ATTR(section(s))
#define JEMALLOC_RESTRICT_RETURN
#define JEMALLOC_ALLOCATOR
#else
#define JEMALLOC_ATTR(s)
#define JEMALLOC_ALIGNED(s)
#define JEMALLOC_ALLOC_SIZE(s)
#define JEMALLOC_ALLOC_SIZE2(s1, s2)
#define JEMALLOC_EXPORT
#define JEMALLOC_FORMAT_PRINTF(s, i)
#define JEMALLOC_NOINLINE
#define JEMALLOC_NOTHROW
#define JEMALLOC_SECTION(s)
#define JEMALLOC_RESTRICT_RETURN
#define JEMALLOC_ALLOCATOR
#endif






extern JEMALLOC_EXPORT const char *je_malloc_conf;
extern JEMALLOC_EXPORT void (*je_malloc_message)(void *cbopaque,
const char *s);

JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *je_malloc(size_t size)
JEMALLOC_CXX_THROW JEMALLOC_ATTR(malloc) JEMALLOC_ALLOC_SIZE(1);
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *je_calloc(size_t num, size_t size)
JEMALLOC_CXX_THROW JEMALLOC_ATTR(malloc) JEMALLOC_ALLOC_SIZE2(1, 2);
JEMALLOC_EXPORT int JEMALLOC_NOTHROW je_posix_memalign(void **memptr,
size_t alignment, size_t size) JEMALLOC_CXX_THROW JEMALLOC_ATTR(nonnull(1));
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *je_aligned_alloc(size_t alignment,
size_t size) JEMALLOC_CXX_THROW JEMALLOC_ATTR(malloc)
JEMALLOC_ALLOC_SIZE(2);
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *je_realloc(void *ptr, size_t size)
JEMALLOC_CXX_THROW JEMALLOC_ALLOC_SIZE(2);
JEMALLOC_EXPORT void JEMALLOC_NOTHROW je_free(void *ptr)
JEMALLOC_CXX_THROW;

JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *je_mallocx(size_t size, int flags)
JEMALLOC_ATTR(malloc) JEMALLOC_ALLOC_SIZE(1);
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *je_rallocx(void *ptr, size_t size,
int flags) JEMALLOC_ALLOC_SIZE(2);
JEMALLOC_EXPORT size_t JEMALLOC_NOTHROW je_xallocx(void *ptr, size_t size,
size_t extra, int flags);
JEMALLOC_EXPORT size_t JEMALLOC_NOTHROW je_sallocx(const void *ptr,
int flags) JEMALLOC_ATTR(pure);
JEMALLOC_EXPORT void JEMALLOC_NOTHROW je_dallocx(void *ptr, int flags);
JEMALLOC_EXPORT void JEMALLOC_NOTHROW je_sdallocx(void *ptr, size_t size,
int flags);
JEMALLOC_EXPORT size_t JEMALLOC_NOTHROW je_nallocx(size_t size, int flags)
JEMALLOC_ATTR(pure);

JEMALLOC_EXPORT int JEMALLOC_NOTHROW je_mallctl(const char *name,
void *oldp, size_t *oldlenp, void *newp, size_t newlen);
JEMALLOC_EXPORT int JEMALLOC_NOTHROW je_mallctlnametomib(const char *name,
size_t *mibp, size_t *miblenp);
JEMALLOC_EXPORT int JEMALLOC_NOTHROW je_mallctlbymib(const size_t *mib,
size_t miblen, void *oldp, size_t *oldlenp, void *newp, size_t newlen);
JEMALLOC_EXPORT void JEMALLOC_NOTHROW je_malloc_stats_print(
void (*write_cb)(void *, const char *), void *je_cbopaque,
const char *opts);
JEMALLOC_EXPORT size_t JEMALLOC_NOTHROW je_malloc_usable_size(
JEMALLOC_USABLE_SIZE_CONST void *ptr) JEMALLOC_CXX_THROW;

#if defined(JEMALLOC_OVERRIDE_MEMALIGN)
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *je_memalign(size_t alignment, size_t size)
JEMALLOC_CXX_THROW JEMALLOC_ATTR(malloc);
#endif

#if defined(JEMALLOC_OVERRIDE_VALLOC)
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *je_valloc(size_t size) JEMALLOC_CXX_THROW
JEMALLOC_ATTR(malloc);
#endif






typedef void *(chunk_alloc_t)(void *, size_t, size_t, bool *, bool *, unsigned);





typedef bool (chunk_dalloc_t)(void *, size_t, bool, unsigned);






typedef bool (chunk_commit_t)(void *, size_t, size_t, size_t, unsigned);






typedef bool (chunk_decommit_t)(void *, size_t, size_t, size_t, unsigned);






typedef bool (chunk_purge_t)(void *, size_t, size_t, size_t, unsigned);






typedef bool (chunk_split_t)(void *, size_t, size_t, size_t, bool, unsigned);






typedef bool (chunk_merge_t)(void *, size_t, void *, size_t, bool, unsigned);

typedef struct {
chunk_alloc_t *alloc;
chunk_dalloc_t *dalloc;
chunk_commit_t *commit;
chunk_decommit_t *decommit;
chunk_purge_t *purge;
chunk_split_t *split;
chunk_merge_t *merge;
} chunk_hooks_t;








#if defined(JEMALLOC_MANGLE)
#if !defined(JEMALLOC_NO_DEMANGLE)
#define JEMALLOC_NO_DEMANGLE
#endif
#define malloc_conf je_malloc_conf
#define malloc_message je_malloc_message
#define malloc je_malloc
#define calloc je_calloc
#define posix_memalign je_posix_memalign
#define aligned_alloc je_aligned_alloc
#define realloc je_realloc
#define free je_free
#define mallocx je_mallocx
#define rallocx je_rallocx
#define xallocx je_xallocx
#define sallocx je_sallocx
#define dallocx je_dallocx
#define sdallocx je_sdallocx
#define nallocx je_nallocx
#define mallctl je_mallctl
#define mallctlnametomib je_mallctlnametomib
#define mallctlbymib je_mallctlbymib
#define malloc_stats_print je_malloc_stats_print
#define malloc_usable_size je_malloc_usable_size
#define memalign je_memalign
#define valloc je_valloc
#endif








#if !defined(JEMALLOC_NO_DEMANGLE)
#undef je_malloc_conf
#undef je_malloc_message
#undef je_malloc
#undef je_calloc
#undef je_posix_memalign
#undef je_aligned_alloc
#undef je_realloc
#undef je_free
#undef je_mallocx
#undef je_rallocx
#undef je_xallocx
#undef je_sallocx
#undef je_dallocx
#undef je_sdallocx
#undef je_nallocx
#undef je_mallctl
#undef je_mallctlnametomib
#undef je_mallctlbymib
#undef je_malloc_stats_print
#undef je_malloc_usable_size
#undef je_memalign
#undef je_valloc
#endif

#if defined(__cplusplus)
}
#endif
#endif 
