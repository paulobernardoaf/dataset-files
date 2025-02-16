




extern JEMALLOC_EXPORT const char *jet_malloc_conf;
extern JEMALLOC_EXPORT void (*jet_malloc_message)(void *cbopaque,
const char *s);

JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *jet_malloc(size_t size)
JEMALLOC_CXX_THROW JEMALLOC_ATTR(malloc) JEMALLOC_ALLOC_SIZE(1);
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *jet_calloc(size_t num, size_t size)
JEMALLOC_CXX_THROW JEMALLOC_ATTR(malloc) JEMALLOC_ALLOC_SIZE2(1, 2);
JEMALLOC_EXPORT int JEMALLOC_NOTHROW jet_posix_memalign(void **memptr,
size_t alignment, size_t size) JEMALLOC_CXX_THROW JEMALLOC_ATTR(nonnull(1));
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *jet_aligned_alloc(size_t alignment,
size_t size) JEMALLOC_CXX_THROW JEMALLOC_ATTR(malloc)
JEMALLOC_ALLOC_SIZE(2);
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *jet_realloc(void *ptr, size_t size)
JEMALLOC_CXX_THROW JEMALLOC_ALLOC_SIZE(2);
JEMALLOC_EXPORT void JEMALLOC_NOTHROW jet_free(void *ptr)
JEMALLOC_CXX_THROW;

JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *jet_mallocx(size_t size, int flags)
JEMALLOC_ATTR(malloc) JEMALLOC_ALLOC_SIZE(1);
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *jet_rallocx(void *ptr, size_t size,
int flags) JEMALLOC_ALLOC_SIZE(2);
JEMALLOC_EXPORT size_t JEMALLOC_NOTHROW jet_xallocx(void *ptr, size_t size,
size_t extra, int flags);
JEMALLOC_EXPORT size_t JEMALLOC_NOTHROW jet_sallocx(const void *ptr,
int flags) JEMALLOC_ATTR(pure);
JEMALLOC_EXPORT void JEMALLOC_NOTHROW jet_dallocx(void *ptr, int flags);
JEMALLOC_EXPORT void JEMALLOC_NOTHROW jet_sdallocx(void *ptr, size_t size,
int flags);
JEMALLOC_EXPORT size_t JEMALLOC_NOTHROW jet_nallocx(size_t size, int flags)
JEMALLOC_ATTR(pure);

JEMALLOC_EXPORT int JEMALLOC_NOTHROW jet_mallctl(const char *name,
void *oldp, size_t *oldlenp, void *newp, size_t newlen);
JEMALLOC_EXPORT int JEMALLOC_NOTHROW jet_mallctlnametomib(const char *name,
size_t *mibp, size_t *miblenp);
JEMALLOC_EXPORT int JEMALLOC_NOTHROW jet_mallctlbymib(const size_t *mib,
size_t miblen, void *oldp, size_t *oldlenp, void *newp, size_t newlen);
JEMALLOC_EXPORT void JEMALLOC_NOTHROW jet_malloc_stats_print(
void (*write_cb)(void *, const char *), void *jet_cbopaque,
const char *opts);
JEMALLOC_EXPORT size_t JEMALLOC_NOTHROW jet_malloc_usable_size(
JEMALLOC_USABLE_SIZE_CONST void *ptr) JEMALLOC_CXX_THROW;

#if defined(JEMALLOC_OVERRIDE_MEMALIGN)
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *jet_memalign(size_t alignment, size_t size)
JEMALLOC_CXX_THROW JEMALLOC_ATTR(malloc);
#endif

#if defined(JEMALLOC_OVERRIDE_VALLOC)
JEMALLOC_EXPORT JEMALLOC_ALLOCATOR JEMALLOC_RESTRICT_RETURN
void JEMALLOC_NOTHROW *jet_valloc(size_t size) JEMALLOC_CXX_THROW
JEMALLOC_ATTR(malloc);
#endif
