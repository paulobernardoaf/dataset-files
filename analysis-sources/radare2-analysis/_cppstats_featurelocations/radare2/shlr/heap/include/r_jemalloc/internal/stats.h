
#if defined(JEMALLOC_H_TYPES)

typedef struct tcache_bin_stats_s tcache_bin_stats_t;
typedef struct malloc_bin_stats_s malloc_bin_stats_t;
typedef struct malloc_large_stats_s malloc_large_stats_t;
typedef struct malloc_huge_stats_s malloc_huge_stats_t;
typedef struct arena_stats_s arena_stats_t;
typedef struct chunk_stats_s chunk_stats_t;

#endif 

#if defined(JEMALLOC_H_STRUCTS)

struct tcache_bin_stats_s {




uint64_t nrequests;
};

struct malloc_bin_stats_s {






uint64_t nmalloc;
uint64_t ndalloc;






uint64_t nrequests;





size_t curregs;


uint64_t nfills;


uint64_t nflushes;


uint64_t nruns;





uint64_t reruns;


size_t curruns;
};

struct malloc_large_stats_s {






uint64_t nmalloc;
uint64_t ndalloc;






uint64_t nrequests;





size_t curruns;
};

struct malloc_huge_stats_s {




uint64_t nmalloc;
uint64_t ndalloc;


size_t curhchunks;
};

struct arena_stats_s {

size_t mapped;







size_t retained;






uint64_t npurge;
uint64_t nmadvise;
uint64_t purged;





size_t metadata_mapped;
size_t metadata_allocated; 


size_t allocated_large;
uint64_t nmalloc_large;
uint64_t ndalloc_large;
uint64_t nrequests_large;

size_t allocated_huge;
uint64_t nmalloc_huge;
uint64_t ndalloc_huge;


malloc_large_stats_t *lstats;


malloc_huge_stats_t *hstats;
};

#endif 

#if defined(JEMALLOC_H_EXTERNS)

extern bool opt_stats_print;

extern size_t stats_cactive;

void stats_print(void (*write)(void *, const char *), void *cbopaque,
const char *opts);

#endif 

#if defined(JEMALLOC_H_INLINES)

#if !defined(JEMALLOC_ENABLE_INLINE)
size_t stats_cactive_get(void);
void stats_cactive_add(size_t size);
void stats_cactive_sub(size_t size);
#endif

#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_STATS_C_))
JEMALLOC_INLINE size_t
stats_cactive_get(void)
{

return (atomic_read_z(&stats_cactive));
}

JEMALLOC_INLINE void
stats_cactive_add(size_t size)
{

assert(size > 0);
assert((size & chunksize_mask) == 0);

atomic_add_z(&stats_cactive, size);
}

JEMALLOC_INLINE void
stats_cactive_sub(size_t size)
{

assert(size > 0);
assert((size & chunksize_mask) == 0);

atomic_sub_z(&stats_cactive, size);
}
#endif

#endif 

