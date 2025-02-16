#include "jemalloc_internal_defs.h"
#include "jemalloc_internal_decls.h"
#if defined(JEMALLOC_UTRACE)
#include <sys/ktrace.h>
#endif
#define JEMALLOC_NO_DEMANGLE
#if defined(JEMALLOC_JET)
#define JEMALLOC_N(n) jet_##n
#include "public_namespace.h"
#define JEMALLOC_NO_RENAME
#include "../jemalloc.h"
#undef JEMALLOC_NO_RENAME
#else
#define JEMALLOC_N(n) je_##n
#include "../jemalloc.h"
#endif
#include "private_namespace.h"
static const bool config_debug =
#if defined(JEMALLOC_DEBUG)
true
#else
false
#endif
;
static const bool have_dss =
#if defined(JEMALLOC_DSS)
true
#else
false
#endif
;
static const bool config_fill =
#if defined(JEMALLOC_FILL)
true
#else
false
#endif
;
static const bool config_lazy_lock =
#if defined(JEMALLOC_LAZY_LOCK)
true
#else
false
#endif
;
static const char * const config_malloc_conf = JEMALLOC_CONFIG_MALLOC_CONF;
static const bool config_prof =
#if defined(JEMALLOC_PROF)
true
#else
false
#endif
;
static const bool config_prof_libgcc =
#if defined(JEMALLOC_PROF_LIBGCC)
true
#else
false
#endif
;
static const bool config_prof_libunwind =
#if defined(JEMALLOC_PROF_LIBUNWIND)
true
#else
false
#endif
;
static const bool maps_coalesce =
#if defined(JEMALLOC_MAPS_COALESCE)
true
#else
false
#endif
;
static const bool config_munmap =
#if defined(JEMALLOC_MUNMAP)
true
#else
false
#endif
;
static const bool config_stats =
#if defined(JEMALLOC_STATS)
true
#else
false
#endif
;
static const bool config_tcache =
#if defined(JEMALLOC_TCACHE)
true
#else
false
#endif
;
static const bool config_thp =
#if defined(JEMALLOC_THP)
true
#else
false
#endif
;
static const bool config_tls =
#if defined(JEMALLOC_TLS)
true
#else
false
#endif
;
static const bool config_utrace =
#if defined(JEMALLOC_UTRACE)
true
#else
false
#endif
;
static const bool config_valgrind =
#if defined(JEMALLOC_VALGRIND)
true
#else
false
#endif
;
static const bool config_xmalloc =
#if defined(JEMALLOC_XMALLOC)
true
#else
false
#endif
;
static const bool config_ivsalloc =
#if defined(JEMALLOC_IVSALLOC)
true
#else
false
#endif
;
static const bool config_cache_oblivious =
#if defined(JEMALLOC_CACHE_OBLIVIOUS)
true
#else
false
#endif
;
#if defined(JEMALLOC_ATOMIC9)
#include <machine/atomic.h>
#endif
#if (defined(JEMALLOC_OSATOMIC) || defined(JEMALLOC_OSSPIN))
#include <libkern/OSAtomic.h>
#endif
#if defined(JEMALLOC_ZONE)
#include <mach/mach_error.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#endif
#include "ph.h"
#if !defined(__PGI)
#define RB_COMPACT
#endif
#include "rb.h"
#include "qr.h"
#include "ql.h"
#define JEMALLOC_H_TYPES
#include "jemalloc_internal_macros.h"
typedef unsigned pszind_t;
typedef unsigned szind_t;
#define MALLOCX_ARENA_MASK ((int)~0xfffff)
#define MALLOCX_ARENA_MAX 0xffe
#define MALLOCX_TCACHE_MASK ((int)~0xfff000ffU)
#define MALLOCX_TCACHE_MAX 0xffd
#define MALLOCX_LG_ALIGN_MASK ((int)0x3f)
#define MALLOCX_ALIGN_GET_SPECIFIED(flags) (ZU(1) << (flags & MALLOCX_LG_ALIGN_MASK))
#define MALLOCX_ALIGN_GET(flags) (MALLOCX_ALIGN_GET_SPECIFIED(flags) & (SIZE_T_MAX-1))
#define MALLOCX_ZERO_GET(flags) ((bool)(flags & MALLOCX_ZERO))
#define MALLOCX_TCACHE_GET(flags) (((unsigned)((flags & MALLOCX_TCACHE_MASK) >> 8)) - 2)
#define MALLOCX_ARENA_GET(flags) (((unsigned)(((unsigned)flags) >> 20)) - 1)
#define TINY_MIN (1U << LG_TINY_MIN)
#if !defined(LG_QUANTUM)
#if (defined(__i386__) || defined(_M_IX86))
#define LG_QUANTUM 4
#endif
#if defined(__ia64__)
#define LG_QUANTUM 4
#endif
#if defined(__alpha__)
#define LG_QUANTUM 4
#endif
#if (defined(__sparc64__) || defined(__sparcv9) || defined(__sparc_v9__))
#define LG_QUANTUM 4
#endif
#if (defined(__amd64__) || defined(__x86_64__) || defined(_M_X64))
#define LG_QUANTUM 4
#endif
#if defined(__arm__)
#define LG_QUANTUM 3
#endif
#if defined(__aarch64__)
#define LG_QUANTUM 4
#endif
#if defined(__hppa__)
#define LG_QUANTUM 4
#endif
#if defined(__m68k__)
#define LG_QUANTUM 3
#endif
#if defined(__mips__)
#define LG_QUANTUM 3
#endif
#if defined(__or1k__)
#define LG_QUANTUM 3
#endif
#if defined(__powerpc__)
#define LG_QUANTUM 4
#endif
#if defined(__riscv) || defined(__riscv__)
#define LG_QUANTUM 4
#endif
#if defined(__s390__)
#define LG_QUANTUM 4
#endif
#if defined(__SH4__)
#define LG_QUANTUM 4
#endif
#if defined(__tile__)
#define LG_QUANTUM 4
#endif
#if defined(__le32__)
#define LG_QUANTUM 4
#endif
#if !defined(LG_QUANTUM)
#error "Unknown minimum alignment for architecture; specify via "
"--with-lg-quantum"
#endif
#endif
#define QUANTUM ((size_t)(1U << LG_QUANTUM))
#define QUANTUM_MASK (QUANTUM - 1)
#define QUANTUM_CEILING(a) (((a) + QUANTUM_MASK) & ~QUANTUM_MASK)
#define LONG ((size_t)(1U << LG_SIZEOF_LONG))
#define LONG_MASK (LONG - 1)
#define LONG_CEILING(a) (((a) + LONG_MASK) & ~LONG_MASK)
#define SIZEOF_PTR (1U << LG_SIZEOF_PTR)
#define PTR_MASK (SIZEOF_PTR - 1)
#define PTR_CEILING(a) (((a) + PTR_MASK) & ~PTR_MASK)
#define LG_CACHELINE 6
#define CACHELINE 64
#define CACHELINE_MASK (CACHELINE - 1)
#define CACHELINE_CEILING(s) (((s) + CACHELINE_MASK) & ~CACHELINE_MASK)
#if defined(PAGE_MASK)
#undef PAGE_MASK
#endif
#define PAGE ((size_t)(1U << LG_PAGE))
#define PAGE_MASK ((size_t)(PAGE - 1))
#define PAGE_ADDR2BASE(a) ((void *)((uintptr_t)(a) & ~PAGE_MASK))
#define PAGE_CEILING(s) (((s) + PAGE_MASK) & ~PAGE_MASK)
#define ALIGNMENT_ADDR2BASE(a, alignment) ((void *)((uintptr_t)(a) & ((~(alignment)) + 1)))
#define ALIGNMENT_ADDR2OFFSET(a, alignment) ((size_t)((uintptr_t)(a) & (alignment - 1)))
#define ALIGNMENT_CEILING(s, alignment) (((s) + (alignment - 1)) & ((~(alignment)) + 1))
#if __STDC_VERSION__ < 199901L
#if defined(_MSC_VER)
#include <malloc.h>
#define alloca _alloca
#else
#include <stdlib.h>
#endif
#define VARIABLE_ARRAY(type, name, count) type *name = alloca(sizeof(type) * (count))
#else
#define VARIABLE_ARRAY(type, name, count) type name[(count)]
#endif
#include "nstime.h"
#include "valgrind.h"
#include "util.h"
#include "atomic.h"
#include "spin.h"
#include "prng.h"
#include "ticker.h"
#include "ckh.h"
#include "size_classes.h"
#include "smoothstep.h"
#include "stats.h"
#include "ctl.h"
#include "witness.h"
#include "mutex.h"
#include "tsd.h"
#include "mb.h"
#include "extent.h"
#include "arena.h"
#include "bitmap.h"
#include "base.h"
#include "rtree.h"
#include "pages.h"
#include "chunk.h"
#include "huge.h"
#include "tcache.h"
#include "hash.h"
#include "quarantine.h"
#include "prof.h"
#undef JEMALLOC_H_TYPES
#define JEMALLOC_H_STRUCTS
#include "nstime.h"
#include "valgrind.h"
#include "util.h"
#include "atomic.h"
#include "spin.h"
#include "prng.h"
#include "ticker.h"
#include "ckh.h"
#include "size_classes.h"
#include "smoothstep.h"
#include "stats.h"
#include "ctl.h"
#include "witness.h"
#include "mutex.h"
#include "mb.h"
#include "bitmap.h"
#define JEMALLOC_ARENA_STRUCTS_A
#include "arena.h"
#undef JEMALLOC_ARENA_STRUCTS_A
#include "extent.h"
#define JEMALLOC_ARENA_STRUCTS_B
#include "arena.h"
#undef JEMALLOC_ARENA_STRUCTS_B
#include "base.h"
#include "rtree.h"
#include "pages.h"
#include "chunk.h"
#include "huge.h"
#include "tcache.h"
#include "hash.h"
#include "quarantine.h"
#include "prof.h"
#include "tsd.h"
#undef JEMALLOC_H_STRUCTS
#define JEMALLOC_H_EXTERNS
extern bool opt_abort;
extern const char *opt_junk;
extern bool opt_junk_alloc;
extern bool opt_junk_free;
extern size_t opt_quarantine;
extern bool opt_redzone;
extern bool opt_utrace;
extern bool opt_xmalloc;
extern bool opt_zero;
extern unsigned opt_narenas;
extern bool in_valgrind;
extern unsigned ncpus;
extern unsigned narenas_auto;
extern arena_t **arenas;
extern size_t const pind2sz_tab[NPSIZES];
extern size_t const index2size_tab[NSIZES];
extern uint8_t const size2index_tab[];
arena_t *a0get(void);
void *a0malloc(size_t size);
void a0dalloc(void *ptr);
void *bootstrap_malloc(size_t size);
void *bootstrap_calloc(size_t num, size_t size);
void bootstrap_free(void *ptr);
unsigned narenas_total_get(void);
arena_t *arena_init(tsdn_t *tsdn, unsigned ind);
arena_tdata_t *arena_tdata_get_hard(tsd_t *tsd, unsigned ind);
arena_t *arena_choose_hard(tsd_t *tsd, bool internal);
void arena_migrate(tsd_t *tsd, unsigned oldind, unsigned newind);
void thread_allocated_cleanup(tsd_t *tsd);
void thread_deallocated_cleanup(tsd_t *tsd);
void iarena_cleanup(tsd_t *tsd);
void arena_cleanup(tsd_t *tsd);
void arenas_tdata_cleanup(tsd_t *tsd);
void narenas_tdata_cleanup(tsd_t *tsd);
void arenas_tdata_bypass_cleanup(tsd_t *tsd);
void jemalloc_prefork(void);
void jemalloc_postfork_parent(void);
void jemalloc_postfork_child(void);
#include "nstime.h"
#include "valgrind.h"
#include "util.h"
#include "atomic.h"
#include "spin.h"
#include "prng.h"
#include "ticker.h"
#include "ckh.h"
#include "size_classes.h"
#include "smoothstep.h"
#include "stats.h"
#include "ctl.h"
#include "witness.h"
#include "mutex.h"
#include "mb.h"
#include "bitmap.h"
#include "extent.h"
#include "arena.h"
#include "base.h"
#include "rtree.h"
#include "pages.h"
#include "chunk.h"
#include "huge.h"
#include "tcache.h"
#include "hash.h"
#include "quarantine.h"
#include "prof.h"
#include "tsd.h"
#undef JEMALLOC_H_EXTERNS
#define JEMALLOC_H_INLINES
#include "nstime.h"
#include "valgrind.h"
#include "util.h"
#include "atomic.h"
#include "spin.h"
#include "prng.h"
#include "ticker.h"
#include "ckh.h"
#include "size_classes.h"
#include "smoothstep.h"
#include "stats.h"
#include "ctl.h"
#include "tsd.h"
#include "witness.h"
#include "mutex.h"
#include "mb.h"
#include "extent.h"
#include "base.h"
#include "rtree.h"
#include "pages.h"
#include "chunk.h"
#include "huge.h"
#if !defined(JEMALLOC_ENABLE_INLINE)
pszind_t psz2ind(size_t psz);
size_t pind2sz_compute(pszind_t pind);
size_t pind2sz_lookup(pszind_t pind);
size_t pind2sz(pszind_t pind);
size_t psz2u(size_t psz);
szind_t size2index_compute(size_t size);
szind_t size2index_lookup(size_t size);
szind_t size2index(size_t size);
size_t index2size_compute(szind_t index);
size_t index2size_lookup(szind_t index);
size_t index2size(szind_t index);
size_t s2u_compute(size_t size);
size_t s2u_lookup(size_t size);
size_t s2u(size_t size);
size_t sa2u(size_t size, size_t alignment);
arena_t *arena_choose_impl(tsd_t *tsd, arena_t *arena, bool internal);
arena_t *arena_choose(tsd_t *tsd, arena_t *arena);
arena_t *arena_ichoose(tsd_t *tsd, arena_t *arena);
arena_tdata_t *arena_tdata_get(tsd_t *tsd, unsigned ind,
bool refresh_if_missing);
arena_t *arena_get(tsdn_t *tsdn, unsigned ind, bool init_if_missing);
ticker_t *decay_ticker_get(tsd_t *tsd, unsigned ind);
#endif
#include "bitmap.h"
#define JEMALLOC_ARENA_INLINE_A
#include "arena.h"
#undef JEMALLOC_ARENA_INLINE_A
#include "tcache.h"
#define JEMALLOC_ARENA_INLINE_B
#include "arena.h"
#undef JEMALLOC_ARENA_INLINE_B
#include "hash.h"
#include "quarantine.h"
