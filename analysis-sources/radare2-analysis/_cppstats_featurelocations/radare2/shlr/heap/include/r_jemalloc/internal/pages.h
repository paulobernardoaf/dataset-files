
#if defined(JEMALLOC_H_TYPES)

#endif 

#if defined(JEMALLOC_H_STRUCTS)

#endif 

#if defined(JEMALLOC_H_EXTERNS)

void *pages_map(void *addr, size_t size, bool *commit);
void pages_unmap(void *addr, size_t size);
void *pages_trim(void *addr, size_t alloc_size, size_t leadsize,
size_t size, bool *commit);
bool pages_commit(void *addr, size_t size);
bool pages_decommit(void *addr, size_t size);
bool pages_purge(void *addr, size_t size);
bool pages_huge(void *addr, size_t size);
bool pages_nohuge(void *addr, size_t size);
void pages_boot(void);

#endif 

#if defined(JEMALLOC_H_INLINES)

#endif 


