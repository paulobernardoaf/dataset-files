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
