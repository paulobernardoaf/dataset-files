#if defined(JEMALLOC_H_TYPES)
#endif 
#if defined(JEMALLOC_H_STRUCTS)
#endif 
#if defined(JEMALLOC_H_EXTERNS)
void *chunk_alloc_mmap(void *new_addr, size_t size, size_t alignment,
bool *zero, bool *commit);
bool chunk_dalloc_mmap(void *chunk, size_t size);
#endif 
#if defined(JEMALLOC_H_INLINES)
#endif 
