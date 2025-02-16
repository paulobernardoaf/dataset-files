
#if defined(JEMALLOC_H_TYPES)


#define MALLOC_TSD_CLEANUPS_MAX 2

typedef bool (*malloc_tsd_cleanup_t)(void);

#if (!defined(JEMALLOC_MALLOC_THREAD_CLEANUP) && !defined(JEMALLOC_TLS) && !defined(_WIN32))

typedef struct tsd_init_block_s tsd_init_block_t;
typedef struct tsd_init_head_s tsd_init_head_t;
#endif

typedef struct tsd_s tsd_t;
typedef struct tsdn_s tsdn_t;

#define TSDN_NULL ((tsdn_t *)0)

typedef enum {
tsd_state_uninitialized,
tsd_state_nominal,
tsd_state_purgatory,
tsd_state_reincarnated
} tsd_state_t;





















































#if defined(JEMALLOC_MALLOC_THREAD_CLEANUP)
#define malloc_tsd_types(a_name, a_type)
#elif (defined(JEMALLOC_TLS))
#define malloc_tsd_types(a_name, a_type)
#elif (defined(_WIN32))
#define malloc_tsd_types(a_name, a_type) typedef struct { bool initialized; a_type val; } a_name##tsd_wrapper_t;




#else
#define malloc_tsd_types(a_name, a_type) typedef struct { bool initialized; a_type val; } a_name##tsd_wrapper_t;




#endif


#define malloc_tsd_protos(a_attr, a_name, a_type) a_attr bool a_name##tsd_boot0(void); a_attr void a_name##tsd_boot1(void); a_attr bool a_name##tsd_boot(void); a_attr bool a_name##tsd_booted_get(void); a_attr a_type * a_name##tsd_get(bool init); a_attr void a_name##tsd_set(a_type *val);














#if defined(JEMALLOC_MALLOC_THREAD_CLEANUP)
#define malloc_tsd_externs(a_name, a_type) extern __thread a_type a_name##tsd_tls; extern __thread bool a_name##tsd_initialized; extern bool a_name##tsd_booted;



#elif (defined(JEMALLOC_TLS))
#define malloc_tsd_externs(a_name, a_type) extern __thread a_type a_name##tsd_tls; extern pthread_key_t a_name##tsd_tsd; extern bool a_name##tsd_booted;



#elif (defined(_WIN32))
#define malloc_tsd_externs(a_name, a_type) extern DWORD a_name##tsd_tsd; extern a_name##tsd_wrapper_t a_name##tsd_boot_wrapper; extern bool a_name##tsd_booted;



#else
#define malloc_tsd_externs(a_name, a_type) extern pthread_key_t a_name##tsd_tsd; extern tsd_init_head_t a_name##tsd_init_head; extern a_name##tsd_wrapper_t a_name##tsd_boot_wrapper; extern bool a_name##tsd_booted;




#endif


#if defined(JEMALLOC_MALLOC_THREAD_CLEANUP)
#define malloc_tsd_data(a_attr, a_name, a_type, a_initializer) a_attr __thread a_type JEMALLOC_TLS_MODEL a_name##tsd_tls = a_initializer; a_attr __thread bool JEMALLOC_TLS_MODEL a_name##tsd_initialized = false; a_attr bool a_name##tsd_booted = false;





#elif (defined(JEMALLOC_TLS))
#define malloc_tsd_data(a_attr, a_name, a_type, a_initializer) a_attr __thread a_type JEMALLOC_TLS_MODEL a_name##tsd_tls = a_initializer; a_attr pthread_key_t a_name##tsd_tsd; a_attr bool a_name##tsd_booted = false;




#elif (defined(_WIN32))
#define malloc_tsd_data(a_attr, a_name, a_type, a_initializer) a_attr DWORD a_name##tsd_tsd; a_attr a_name##tsd_wrapper_t a_name##tsd_boot_wrapper = { false, a_initializer }; a_attr bool a_name##tsd_booted = false;






#else
#define malloc_tsd_data(a_attr, a_name, a_type, a_initializer) a_attr pthread_key_t a_name##tsd_tsd; a_attr tsd_init_head_t a_name##tsd_init_head = { ql_head_initializer(blocks), MALLOC_MUTEX_INITIALIZER }; a_attr a_name##tsd_wrapper_t a_name##tsd_boot_wrapper = { false, a_initializer }; a_attr bool a_name##tsd_booted = false;










#endif


#if defined(JEMALLOC_MALLOC_THREAD_CLEANUP)
#define malloc_tsd_funcs(a_attr, a_name, a_type, a_initializer, a_cleanup) a_attr bool a_name##tsd_cleanup_wrapper(void) { if (a_name##tsd_initialized) { a_name##tsd_initialized = false; a_cleanup(&a_name##tsd_tls); } return (a_name##tsd_initialized); } a_attr bool a_name##tsd_boot0(void) { if (a_cleanup != malloc_tsd_no_cleanup) { malloc_tsd_cleanup_register( &a_name##tsd_cleanup_wrapper); } a_name##tsd_booted = true; return (false); } a_attr void a_name##tsd_boot1(void) { } a_attr bool a_name##tsd_boot(void) { return (a_name##tsd_boot0()); } a_attr bool a_name##tsd_booted_get(void) { return (a_name##tsd_booted); } a_attr bool a_name##tsd_get_allocates(void) { return (false); } a_attr a_type * a_name##tsd_get(bool init) { assert(a_name##tsd_booted); return (&a_name##tsd_tls); } a_attr void a_name##tsd_set(a_type *val) { assert(a_name##tsd_booted); a_name##tsd_tls = (*val); if (a_cleanup != malloc_tsd_no_cleanup) a_name##tsd_initialized = true; }
































































#elif (defined(JEMALLOC_TLS))
#define malloc_tsd_funcs(a_attr, a_name, a_type, a_initializer, a_cleanup) a_attr bool a_name##tsd_boot0(void) { if (a_cleanup != malloc_tsd_no_cleanup) { if (pthread_key_create(&a_name##tsd_tsd, a_cleanup) != 0) return (true); } a_name##tsd_booted = true; return (false); } a_attr void a_name##tsd_boot1(void) { } a_attr bool a_name##tsd_boot(void) { return (a_name##tsd_boot0()); } a_attr bool a_name##tsd_booted_get(void) { return (a_name##tsd_booted); } a_attr bool a_name##tsd_get_allocates(void) { return (false); } a_attr a_type * a_name##tsd_get(bool init) { assert(a_name##tsd_booted); return (&a_name##tsd_tls); } a_attr void a_name##tsd_set(a_type *val) { assert(a_name##tsd_booted); a_name##tsd_tls = (*val); if (a_cleanup != malloc_tsd_no_cleanup) { if (pthread_setspecific(a_name##tsd_tsd, (void *)(&a_name##tsd_tls))) { malloc_write("<jemalloc>: Error" " setting TSD for "#a_name"\n"); if (opt_abort) abort(); } } }






























































#elif (defined(_WIN32))
#define malloc_tsd_funcs(a_attr, a_name, a_type, a_initializer, a_cleanup) a_attr bool a_name##tsd_cleanup_wrapper(void) { DWORD error = GetLastError(); a_name##tsd_wrapper_t *wrapper = (a_name##tsd_wrapper_t *) TlsGetValue(a_name##tsd_tsd); SetLastError(error); if (wrapper == NULL) return (false); if (a_cleanup != malloc_tsd_no_cleanup && wrapper->initialized) { wrapper->initialized = false; a_cleanup(&wrapper->val); if (wrapper->initialized) { return (true); } } malloc_tsd_dalloc(wrapper); return (false); } a_attr void a_name##tsd_wrapper_set(a_name##tsd_wrapper_t *wrapper) { if (!TlsSetValue(a_name##tsd_tsd, (void *)wrapper)) { malloc_write("<jemalloc>: Error setting" " TSD for "#a_name"\n"); abort(); } } a_attr a_name##tsd_wrapper_t * a_name##tsd_wrapper_get(bool init) { DWORD error = GetLastError(); a_name##tsd_wrapper_t *wrapper = (a_name##tsd_wrapper_t *) TlsGetValue(a_name##tsd_tsd); SetLastError(error); if (init && unlikely(wrapper == NULL)) { wrapper = (a_name##tsd_wrapper_t *) malloc_tsd_malloc(sizeof(a_name##tsd_wrapper_t)); if (wrapper == NULL) { malloc_write("<jemalloc>: Error allocating" " TSD for "#a_name"\n"); abort(); } else { wrapper->initialized = false; wrapper->val = a_initializer; } a_name##tsd_wrapper_set(wrapper); } return (wrapper); } a_attr bool a_name##tsd_boot0(void) { a_name##tsd_tsd = TlsAlloc(); if (a_name##tsd_tsd == TLS_OUT_OF_INDEXES) return (true); if (a_cleanup != malloc_tsd_no_cleanup) { malloc_tsd_cleanup_register( &a_name##tsd_cleanup_wrapper); } a_name##tsd_wrapper_set(&a_name##tsd_boot_wrapper); a_name##tsd_booted = true; return (false); } a_attr void a_name##tsd_boot1(void) { a_name##tsd_wrapper_t *wrapper; wrapper = (a_name##tsd_wrapper_t *) malloc_tsd_malloc(sizeof(a_name##tsd_wrapper_t)); if (wrapper == NULL) { malloc_write("<jemalloc>: Error allocating" " TSD for "#a_name"\n"); abort(); } memcpy(wrapper, &a_name##tsd_boot_wrapper, sizeof(a_name##tsd_wrapper_t)); a_name##tsd_wrapper_set(wrapper); } a_attr bool a_name##tsd_boot(void) { if (a_name##tsd_boot0()) return (true); a_name##tsd_boot1(); return (false); } a_attr bool a_name##tsd_booted_get(void) { return (a_name##tsd_booted); } a_attr bool a_name##tsd_get_allocates(void) { return (true); } a_attr a_type * a_name##tsd_get(bool init) { a_name##tsd_wrapper_t *wrapper; assert(a_name##tsd_booted); wrapper = a_name##tsd_wrapper_get(init); if (a_name##tsd_get_allocates() && !init && wrapper == NULL) return (NULL); return (&wrapper->val); } a_attr void a_name##tsd_set(a_type *val) { a_name##tsd_wrapper_t *wrapper; assert(a_name##tsd_booted); wrapper = a_name##tsd_wrapper_get(true); wrapper->val = *(val); if (a_cleanup != malloc_tsd_no_cleanup) wrapper->initialized = true; }



































































































































#else
#define malloc_tsd_funcs(a_attr, a_name, a_type, a_initializer, a_cleanup) a_attr void a_name##tsd_cleanup_wrapper(void *arg) { a_name##tsd_wrapper_t *wrapper = (a_name##tsd_wrapper_t *)arg; if (a_cleanup != malloc_tsd_no_cleanup && wrapper->initialized) { wrapper->initialized = false; a_cleanup(&wrapper->val); if (wrapper->initialized) { if (pthread_setspecific(a_name##tsd_tsd, (void *)wrapper)) { malloc_write("<jemalloc>: Error" " setting TSD for "#a_name"\n"); if (opt_abort) abort(); } return; } } malloc_tsd_dalloc(wrapper); } a_attr void a_name##tsd_wrapper_set(a_name##tsd_wrapper_t *wrapper) { if (pthread_setspecific(a_name##tsd_tsd, (void *)wrapper)) { malloc_write("<jemalloc>: Error setting" " TSD for "#a_name"\n"); abort(); } } a_attr a_name##tsd_wrapper_t * a_name##tsd_wrapper_get(bool init) { a_name##tsd_wrapper_t *wrapper = (a_name##tsd_wrapper_t *) pthread_getspecific(a_name##tsd_tsd); if (init && unlikely(wrapper == NULL)) { tsd_init_block_t block; wrapper = (a_name##tsd_wrapper_t *) tsd_init_check_recursion(&a_name##tsd_init_head, &block); if (wrapper) return (wrapper); wrapper = (a_name##tsd_wrapper_t *) malloc_tsd_malloc(sizeof(a_name##tsd_wrapper_t)); block.data = (void *)wrapper; if (wrapper == NULL) { malloc_write("<jemalloc>: Error allocating" " TSD for "#a_name"\n"); abort(); } else { wrapper->initialized = false; wrapper->val = a_initializer; } a_name##tsd_wrapper_set(wrapper); tsd_init_finish(&a_name##tsd_init_head, &block); } return (wrapper); } a_attr bool a_name##tsd_boot0(void) { if (pthread_key_create(&a_name##tsd_tsd, a_name##tsd_cleanup_wrapper) != 0) return (true); a_name##tsd_wrapper_set(&a_name##tsd_boot_wrapper); a_name##tsd_booted = true; return (false); } a_attr void a_name##tsd_boot1(void) { a_name##tsd_wrapper_t *wrapper; wrapper = (a_name##tsd_wrapper_t *) malloc_tsd_malloc(sizeof(a_name##tsd_wrapper_t)); if (wrapper == NULL) { malloc_write("<jemalloc>: Error allocating" " TSD for "#a_name"\n"); abort(); } memcpy(wrapper, &a_name##tsd_boot_wrapper, sizeof(a_name##tsd_wrapper_t)); a_name##tsd_wrapper_set(wrapper); } a_attr bool a_name##tsd_boot(void) { if (a_name##tsd_boot0()) return (true); a_name##tsd_boot1(); return (false); } a_attr bool a_name##tsd_booted_get(void) { return (a_name##tsd_booted); } a_attr bool a_name##tsd_get_allocates(void) { return (true); } a_attr a_type * a_name##tsd_get(bool init) { a_name##tsd_wrapper_t *wrapper; assert(a_name##tsd_booted); wrapper = a_name##tsd_wrapper_get(init); if (a_name##tsd_get_allocates() && !init && wrapper == NULL) return (NULL); return (&wrapper->val); } a_attr void a_name##tsd_set(a_type *val) { a_name##tsd_wrapper_t *wrapper; 




































































































































assert(a_name##tsd_booted); \
wrapper = a_name##tsd_wrapper_get(true); \
wrapper->val = *(val); \
if (a_cleanup != malloc_tsd_no_cleanup) \
wrapper->initialized = true; \
}
#endif

#endif 

#if defined(JEMALLOC_H_STRUCTS)

#if (!defined(JEMALLOC_MALLOC_THREAD_CLEANUP) && !defined(JEMALLOC_TLS) && !defined(_WIN32))

struct tsd_init_block_s {
ql_elm(tsd_init_block_t) link;
pthread_t thread;
void *data;
};
struct tsd_init_head_s {
ql_head(tsd_init_block_t) blocks;
malloc_mutex_t lock;
};
#endif

#define MALLOC_TSD O(tcache, tcache_t *) O(thread_allocated, uint64_t) O(thread_deallocated, uint64_t) O(prof_tdata, prof_tdata_t *) O(iarena, arena_t *) O(arena, arena_t *) O(arenas_tdata, arena_tdata_t *) O(narenas_tdata, unsigned) O(arenas_tdata_bypass, bool) O(tcache_enabled, tcache_enabled_t) O(quarantine, quarantine_t *) O(witnesses, witness_list_t) O(witness_fork, bool) 















#define TSD_INITIALIZER { tsd_state_uninitialized, NULL, 0, 0, NULL, NULL, NULL, NULL, 0, false, tcache_enabled_default, NULL, ql_head_initializer(witnesses), false }
















struct tsd_s {
tsd_state_t state;
#define O(n, t) t n;

MALLOC_TSD
#undef O
};






struct tsdn_s {
tsd_t tsd;
};

static const tsd_t tsd_initializer = TSD_INITIALIZER;

malloc_tsd_types(, tsd_t)

#endif 

#if defined(JEMALLOC_H_EXTERNS)

void *malloc_tsd_malloc(size_t size);
void malloc_tsd_dalloc(void *wrapper);
void malloc_tsd_no_cleanup(void *arg);
void malloc_tsd_cleanup_register(bool (*f)(void));
tsd_t *malloc_tsd_boot0(void);
void malloc_tsd_boot1(void);
#if (!defined(JEMALLOC_MALLOC_THREAD_CLEANUP) && !defined(JEMALLOC_TLS) && !defined(_WIN32))

void *tsd_init_check_recursion(tsd_init_head_t *head,
tsd_init_block_t *block);
void tsd_init_finish(tsd_init_head_t *head, tsd_init_block_t *block);
#endif
void tsd_cleanup(void *arg);

#endif 

#if defined(JEMALLOC_H_INLINES)

#if !defined(JEMALLOC_ENABLE_INLINE)
malloc_tsd_protos(JEMALLOC_ATTR(unused), , tsd_t)

tsd_t *tsd_fetch_impl(bool init);
tsd_t *tsd_fetch(void);
tsdn_t *tsd_tsdn(tsd_t *tsd);
bool tsd_nominal(tsd_t *tsd);
#define O(n, t) t *tsd_##n##p_get(tsd_t *tsd); t tsd_##n##_get(tsd_t *tsd); void tsd_##n##_set(tsd_t *tsd, t n);



MALLOC_TSD
#undef O
tsdn_t *tsdn_fetch(void);
bool tsdn_null(const tsdn_t *tsdn);
tsd_t *tsdn_tsd(tsdn_t *tsdn);
#endif

#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_TSD_C_))
malloc_tsd_externs(, tsd_t)
malloc_tsd_funcs(JEMALLOC_ALWAYS_INLINE, , tsd_t, tsd_initializer, tsd_cleanup)

JEMALLOC_ALWAYS_INLINE tsd_t *
tsd_fetch_impl(bool init)
{
tsd_t *tsd = tsd_get(init);

if (!init && tsd_get_allocates() && tsd == NULL)
return (NULL);

if (unlikely(tsd == NULL))
return (NULL)
if (unlikely(tsd->state != tsd_state_nominal)) {
if (tsd->state == tsd_state_uninitialized) {
tsd->state = tsd_state_nominal;

tsd_set(tsd);
} else if (tsd->state == tsd_state_purgatory) {
tsd->state = tsd_state_reincarnated;
tsd_set(tsd);
} else {
if (unlikely(tsd->state != tsd_state_reincarnated))
return (NULL);
}
}

return (tsd);
}

JEMALLOC_ALWAYS_INLINE tsd_t *
tsd_fetch(void)
{

return (tsd_fetch_impl(true));
}

JEMALLOC_ALWAYS_INLINE tsdn_t *
tsd_tsdn(tsd_t *tsd)
{

return ((tsdn_t *)tsd);
}

JEMALLOC_INLINE bool
tsd_nominal(tsd_t *tsd)
{

return (tsd->state == tsd_state_nominal);
}

#define O(n, t) JEMALLOC_ALWAYS_INLINE t * tsd_##n##p_get(tsd_t *tsd) { return (&tsd->n); } JEMALLOC_ALWAYS_INLINE t tsd_##n##_get(tsd_t *tsd) { return (*tsd_##n##p_get(tsd)); } JEMALLOC_ALWAYS_INLINE void tsd_##n##_set(tsd_t *tsd, t n) { assert(tsd->state == tsd_state_nominal); tsd->n = n; }





















MALLOC_TSD
#undef O

JEMALLOC_ALWAYS_INLINE tsdn_t *
tsdn_fetch(void)
{

if (!tsd_booted_get())
return (NULL);

return (tsd_tsdn(tsd_fetch_impl(false)));
}

JEMALLOC_ALWAYS_INLINE bool
tsdn_null(const tsdn_t *tsdn)
{

return (tsdn == NULL);
}

JEMALLOC_ALWAYS_INLINE tsd_t *
tsdn_tsd(tsdn_t *tsdn)
{
if (unlikely(tsdn_null(tsdn)))
return (NULL);
return (&tsdn->tsd);
}
#endif

#endif 

