#if defined(JEMALLOC_H_TYPES)
#endif 
#if defined(JEMALLOC_H_STRUCTS)
#endif 
#if defined(JEMALLOC_H_EXTERNS)
#endif 
#if defined(JEMALLOC_H_INLINES)
#if !defined(JEMALLOC_ENABLE_INLINE)
void mb_write(void);
#endif
#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_MB_C_))
#if defined(__i386__)
JEMALLOC_INLINE void
mb_write(void)
{
#if 0
asm volatile ("pusha;"
"xor %%eax,%%eax;"
"cpuid;"
"popa;"
: 
: 
: "memory" 
);
#else
asm volatile ("nop;"
: 
: 
: "memory" 
);
#endif
}
#elif (defined(__amd64__) || defined(__x86_64__))
JEMALLOC_INLINE void
mb_write(void)
{
asm volatile ("sfence"
: 
: 
: "memory" 
);
}
#elif defined(__powerpc__)
JEMALLOC_INLINE void
mb_write(void)
{
asm volatile ("eieio"
: 
: 
: "memory" 
);
}
#elif defined(__sparc__) && defined(__arch64__)
JEMALLOC_INLINE void
mb_write(void)
{
asm volatile ("membar #StoreStore"
: 
: 
: "memory" 
);
}
#elif defined(__tile__)
JEMALLOC_INLINE void
mb_write(void)
{
__sync_synchronize();
}
#else
JEMALLOC_INLINE void
mb_write(void)
{
malloc_mutex_t mtx;
malloc_mutex_init(&mtx, "mb", WITNESS_RANK_OMIT);
malloc_mutex_lock(TSDN_NULL, &mtx);
malloc_mutex_unlock(TSDN_NULL, &mtx);
}
#endif
#endif
#endif 
