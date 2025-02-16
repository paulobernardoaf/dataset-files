



























#if !defined(NEDMALLOC_H)
#define NEDMALLOC_H



















#include <stddef.h> 

#if !defined(EXTSPEC)
#define EXTSPEC extern
#endif

#if defined(_MSC_VER) && _MSC_VER>=1400
#define MALLOCATTR __declspec(restrict)
#endif
#if defined(__GNUC__)
#define MALLOCATTR __attribute__ ((malloc))
#endif
#if !defined(MALLOCATTR)
#define MALLOCATTR
#endif

#if defined(REPLACE_SYSTEM_ALLOCATOR)
#define nedmalloc malloc
#define nedcalloc calloc
#define nedrealloc realloc
#define nedfree free
#define nedmemalign memalign
#define nedmallinfo mallinfo
#define nedmallopt mallopt
#define nedmalloc_trim malloc_trim
#define nedmalloc_stats malloc_stats
#define nedmalloc_footprint malloc_footprint
#define nedindependent_calloc independent_calloc
#define nedindependent_comalloc independent_comalloc
#if defined(_MSC_VER)
#define nedblksize _msize
#endif
#endif

#if !defined(NO_MALLINFO)
#define NO_MALLINFO 0
#endif

#if !NO_MALLINFO
struct mallinfo;
#endif

#if defined(__cplusplus)
#if !defined(NO_NED_NAMESPACE)
namespace nedalloc {
#else
extern "C" {
#endif
#define THROWSPEC throw()
#else
#define THROWSPEC
#endif






EXTSPEC size_t nedblksize(void *mem) THROWSPEC;

EXTSPEC void nedsetvalue(void *v) THROWSPEC;

EXTSPEC MALLOCATTR void * nedmalloc(size_t size) THROWSPEC;
EXTSPEC MALLOCATTR void * nedcalloc(size_t no, size_t size) THROWSPEC;
EXTSPEC MALLOCATTR void * nedrealloc(void *mem, size_t size) THROWSPEC;
EXTSPEC void nedfree(void *mem) THROWSPEC;
EXTSPEC MALLOCATTR void * nedmemalign(size_t alignment, size_t bytes) THROWSPEC;
#if !NO_MALLINFO
EXTSPEC struct mallinfo nedmallinfo(void) THROWSPEC;
#endif
EXTSPEC int nedmallopt(int parno, int value) THROWSPEC;
EXTSPEC int nedmalloc_trim(size_t pad) THROWSPEC;
EXTSPEC void nedmalloc_stats(void) THROWSPEC;
EXTSPEC size_t nedmalloc_footprint(void) THROWSPEC;
EXTSPEC MALLOCATTR void **nedindependent_calloc(size_t elemsno, size_t elemsize, void **chunks) THROWSPEC;
EXTSPEC MALLOCATTR void **nedindependent_comalloc(size_t elems, size_t *sizes, void **chunks) THROWSPEC;


struct nedpool_t;
typedef struct nedpool_t nedpool;








EXTSPEC MALLOCATTR nedpool *nedcreatepool(size_t capacity, int threads) THROWSPEC;



EXTSPEC void neddestroypool(nedpool *p) THROWSPEC;




EXTSPEC void nedpsetvalue(nedpool *p, void *v) THROWSPEC;



EXTSPEC void *nedgetvalue(nedpool **p, void *mem) THROWSPEC;




EXTSPEC void neddisablethreadcache(nedpool *p) THROWSPEC;

EXTSPEC MALLOCATTR void * nedpmalloc(nedpool *p, size_t size) THROWSPEC;
EXTSPEC MALLOCATTR void * nedpcalloc(nedpool *p, size_t no, size_t size) THROWSPEC;
EXTSPEC MALLOCATTR void * nedprealloc(nedpool *p, void *mem, size_t size) THROWSPEC;
EXTSPEC void nedpfree(nedpool *p, void *mem) THROWSPEC;
EXTSPEC MALLOCATTR void * nedpmemalign(nedpool *p, size_t alignment, size_t bytes) THROWSPEC;
#if !NO_MALLINFO
EXTSPEC struct mallinfo nedpmallinfo(nedpool *p) THROWSPEC;
#endif
EXTSPEC int nedpmallopt(nedpool *p, int parno, int value) THROWSPEC;
EXTSPEC int nedpmalloc_trim(nedpool *p, size_t pad) THROWSPEC;
EXTSPEC void nedpmalloc_stats(nedpool *p) THROWSPEC;
EXTSPEC size_t nedpmalloc_footprint(nedpool *p) THROWSPEC;
EXTSPEC MALLOCATTR void **nedpindependent_calloc(nedpool *p, size_t elemsno, size_t elemsize, void **chunks) THROWSPEC;
EXTSPEC MALLOCATTR void **nedpindependent_comalloc(nedpool *p, size_t elems, size_t *sizes, void **chunks) THROWSPEC;

#if defined(__cplusplus)
}
#endif

#undef MALLOCATTR
#undef EXTSPEC

#endif
