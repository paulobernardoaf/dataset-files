#if defined(JEMALLOC_H_TYPES)
typedef struct prof_bt_s prof_bt_t;
typedef struct prof_cnt_s prof_cnt_t;
typedef struct prof_tctx_s prof_tctx_t;
typedef struct prof_gctx_s prof_gctx_t;
typedef struct prof_tdata_s prof_tdata_t;
#if defined(JEMALLOC_PROF)
#define PROF_PREFIX_DEFAULT "jeprof"
#else
#define PROF_PREFIX_DEFAULT ""
#endif
#define LG_PROF_SAMPLE_DEFAULT 19
#define LG_PROF_INTERVAL_DEFAULT -1
#define PROF_BT_MAX 128
#define PROF_CKH_MINITEMS 64
#define PROF_DUMP_BUFSIZE 65536
#define PROF_PRINTF_BUFSIZE 128
#define PROF_NCTX_LOCKS 1024
#define PROF_NTDATA_LOCKS 256
#define PROF_TDATA_STATE_REINCARNATED ((prof_tdata_t *)(uintptr_t)1)
#define PROF_TDATA_STATE_PURGATORY ((prof_tdata_t *)(uintptr_t)2)
#define PROF_TDATA_STATE_MAX PROF_TDATA_STATE_PURGATORY
#endif 
#if defined(JEMALLOC_H_STRUCTS)
struct prof_bt_s {
void **vec;
unsigned len;
};
#if defined(JEMALLOC_PROF_LIBGCC)
typedef struct {
prof_bt_t *bt;
unsigned max;
} prof_unwind_data_t;
#endif
struct prof_cnt_s {
uint64_t curobjs;
uint64_t curbytes;
uint64_t accumobjs;
uint64_t accumbytes;
};
typedef enum {
prof_tctx_state_initializing,
prof_tctx_state_nominal,
prof_tctx_state_dumping,
prof_tctx_state_purgatory 
} prof_tctx_state_t;
struct prof_tctx_s {
prof_tdata_t *tdata;
uint64_t thr_uid;
uint64_t thr_discrim;
prof_cnt_t cnts;
prof_gctx_t *gctx;
uint64_t tctx_uid;
rb_node(prof_tctx_t) tctx_link;
bool prepared;
prof_tctx_state_t state;
prof_cnt_t dump_cnts;
};
typedef rb_tree(prof_tctx_t) prof_tctx_tree_t;
struct prof_gctx_s {
malloc_mutex_t *lock;
unsigned nlimbo;
prof_tctx_tree_t tctxs;
rb_node(prof_gctx_t) dump_link;
prof_cnt_t cnt_summed;
prof_bt_t bt;
void *vec[1];
};
typedef rb_tree(prof_gctx_t) prof_gctx_tree_t;
struct prof_tdata_s {
malloc_mutex_t *lock;
uint64_t thr_uid;
uint64_t thr_discrim;
char *thread_name;
bool attached;
bool expired;
rb_node(prof_tdata_t) tdata_link;
uint64_t tctx_uid_next;
ckh_t bt2tctx;
uint64_t prng_state;
uint64_t bytes_until_sample;
bool enq;
bool enq_idump;
bool enq_gdump;
bool dumping;
bool active;
prof_cnt_t cnt_summed;
void *vec[PROF_BT_MAX];
};
typedef rb_tree(prof_tdata_t) prof_tdata_tree_t;
#endif 
#if defined(JEMALLOC_H_EXTERNS)
extern bool opt_prof;
extern bool opt_prof_active;
extern bool opt_prof_thread_active_init;
extern size_t opt_lg_prof_sample; 
extern ssize_t opt_lg_prof_interval; 
extern bool opt_prof_gdump; 
extern bool opt_prof_final; 
extern bool opt_prof_leak; 
extern bool opt_prof_accum; 
extern char opt_prof_prefix[
#if defined(JEMALLOC_PROF)
PATH_MAX +
#endif
1];
extern bool prof_active;
extern bool prof_gdump_val;
extern uint64_t prof_interval;
extern size_t lg_prof_sample;
void prof_alloc_rollback(tsd_t *tsd, prof_tctx_t *tctx, bool updated);
void prof_malloc_sample_object(tsdn_t *tsdn, const void *ptr, size_t usize,
prof_tctx_t *tctx);
void prof_free_sampled_object(tsd_t *tsd, size_t usize, prof_tctx_t *tctx);
void bt_init(prof_bt_t *bt, void **vec);
void prof_backtrace(prof_bt_t *bt);
prof_tctx_t *prof_lookup(tsd_t *tsd, prof_bt_t *bt);
#if defined(JEMALLOC_JET)
size_t prof_tdata_count(void);
size_t prof_bt_count(void);
const prof_cnt_t *prof_cnt_all(void);
typedef int (prof_dump_open_t)(bool, const char *);
extern prof_dump_open_t *prof_dump_open;
typedef bool (prof_dump_header_t)(tsdn_t *, bool, const prof_cnt_t *);
extern prof_dump_header_t *prof_dump_header;
#endif
void prof_idump(tsdn_t *tsdn);
bool prof_mdump(tsd_t *tsd, const char *filename);
void prof_gdump(tsdn_t *tsdn);
prof_tdata_t *prof_tdata_init(tsd_t *tsd);
prof_tdata_t *prof_tdata_reinit(tsd_t *tsd, prof_tdata_t *tdata);
void prof_reset(tsd_t *tsd, size_t lg_sample);
void prof_tdata_cleanup(tsd_t *tsd);
bool prof_active_get(tsdn_t *tsdn);
bool prof_active_set(tsdn_t *tsdn, bool active);
const char *prof_thread_name_get(tsd_t *tsd);
int prof_thread_name_set(tsd_t *tsd, const char *thread_name);
bool prof_thread_active_get(tsd_t *tsd);
bool prof_thread_active_set(tsd_t *tsd, bool active);
bool prof_thread_active_init_get(tsdn_t *tsdn);
bool prof_thread_active_init_set(tsdn_t *tsdn, bool active_init);
bool prof_gdump_get(tsdn_t *tsdn);
bool prof_gdump_set(tsdn_t *tsdn, bool active);
void prof_boot0(void);
void prof_boot1(void);
bool prof_boot2(tsd_t *tsd);
void prof_prefork0(tsdn_t *tsdn);
void prof_prefork1(tsdn_t *tsdn);
void prof_postfork_parent(tsdn_t *tsdn);
void prof_postfork_child(tsdn_t *tsdn);
void prof_sample_threshold_update(prof_tdata_t *tdata);
#endif 
#if defined(JEMALLOC_H_INLINES)
#if !defined(JEMALLOC_ENABLE_INLINE)
bool prof_active_get_unlocked(void);
bool prof_gdump_get_unlocked(void);
prof_tdata_t *prof_tdata_get(tsd_t *tsd, bool create);
prof_tctx_t *prof_tctx_get(tsdn_t *tsdn, const void *ptr);
void prof_tctx_set(tsdn_t *tsdn, const void *ptr, size_t usize,
prof_tctx_t *tctx);
void prof_tctx_reset(tsdn_t *tsdn, const void *ptr, size_t usize,
const void *old_ptr, prof_tctx_t *tctx);
bool prof_sample_accum_update(tsd_t *tsd, size_t usize, bool commit,
prof_tdata_t **tdata_out);
prof_tctx_t *prof_alloc_prep(tsd_t *tsd, size_t usize, bool prof_active,
bool update);
void prof_malloc(tsdn_t *tsdn, const void *ptr, size_t usize,
prof_tctx_t *tctx);
void prof_realloc(tsd_t *tsd, const void *ptr, size_t usize,
prof_tctx_t *tctx, bool prof_active, bool updated, const void *old_ptr,
size_t old_usize, prof_tctx_t *old_tctx);
void prof_free(tsd_t *tsd, const void *ptr, size_t usize);
#endif
#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_PROF_C_))
JEMALLOC_ALWAYS_INLINE bool
prof_active_get_unlocked(void)
{
return (prof_active);
}
JEMALLOC_ALWAYS_INLINE bool
prof_gdump_get_unlocked(void)
{
return (prof_gdump_val);
}
JEMALLOC_ALWAYS_INLINE prof_tdata_t *
prof_tdata_get(tsd_t *tsd, bool create)
{
prof_tdata_t *tdata;
if (unlikely(!config_prof))
return NULL;
tdata = tsd_prof_tdata_get(tsd);
if (create) {
if (unlikely(tdata == NULL)) {
if (tsd_nominal(tsd)) {
tdata = prof_tdata_init(tsd);
tsd_prof_tdata_set(tsd, tdata);
}
} else if (unlikely(tdata->expired)) {
tdata = prof_tdata_reinit(tsd, tdata);
tsd_prof_tdata_set(tsd, tdata);
}
if (unlikely ( ( (tdata != NULL) || ! (tdata->attached) ) ) )
return NULL;
}
return (tdata);
}
JEMALLOC_ALWAYS_INLINE prof_tctx_t *
prof_tctx_get(tsdn_t *tsdn, const void *ptr)
{
if (unlikely (!config_prof || ptr == NULL))
return NULL;
return (arena_prof_tctx_get(tsdn, ptr));
}
JEMALLOC_ALWAYS_INLINE void
prof_tctx_set(tsdn_t *tsdn, const void *ptr, size_t usize, prof_tctx_t *tctx)
{
cassert(config_prof);
assert(ptr != NULL);
arena_prof_tctx_set(tsdn, ptr, usize, tctx);
}
JEMALLOC_ALWAYS_INLINE void
prof_tctx_reset(tsdn_t *tsdn, const void *ptr, size_t usize, const void *old_ptr,
prof_tctx_t *old_tctx)
{
cassert(config_prof);
assert(ptr != NULL);
arena_prof_tctx_reset(tsdn, ptr, usize, old_ptr, old_tctx);
}
JEMALLOC_ALWAYS_INLINE bool
prof_sample_accum_update(tsd_t *tsd, size_t usize, bool update,
prof_tdata_t **tdata_out)
{
prof_tdata_t *tdata;
if (unlikely(!config_prof))
return false;
tdata = prof_tdata_get(tsd, true);
if (unlikely((uintptr_t)tdata <= (uintptr_t)PROF_TDATA_STATE_MAX))
tdata = NULL;
if (tdata_out != NULL)
*tdata_out = tdata;
if (unlikely(tdata == NULL))
return (true);
if (likely(tdata->bytes_until_sample >= usize)) {
if (update)
tdata->bytes_until_sample -= usize;
return (true);
} else {
if (update)
prof_sample_threshold_update(tdata);
return (!tdata->active);
}
}
JEMALLOC_ALWAYS_INLINE prof_tctx_t *
prof_alloc_prep(tsd_t *tsd, size_t usize, bool prof_active, bool update)
{
prof_tctx_t *ret;
prof_tdata_t *tdata;
prof_bt_t bt;
if (unlikely(usize != s2u(usize)))
return NULL;
if (!prof_active || likely(prof_sample_accum_update(tsd, usize, update,
&tdata)))
ret = (prof_tctx_t *)(uintptr_t)1U;
else {
bt_init(&bt, tdata->vec);
prof_backtrace(&bt);
ret = prof_lookup(tsd, &bt);
}
return (ret);
}
JEMALLOC_ALWAYS_INLINE void
prof_malloc(tsdn_t *tsdn, const void *ptr, size_t usize, prof_tctx_t *tctx)
{
cassert(config_prof);
assert(ptr != NULL);
assert(usize == isalloc(tsdn, ptr, true));
if (unlikely((uintptr_t)tctx > (uintptr_t)1U))
prof_malloc_sample_object(tsdn, ptr, usize, tctx);
else
prof_tctx_set(tsdn, ptr, usize, (prof_tctx_t *)(uintptr_t)1U);
}
JEMALLOC_ALWAYS_INLINE void
prof_realloc(tsd_t *tsd, const void *ptr, size_t usize, prof_tctx_t *tctx,
bool prof_active, bool updated, const void *old_ptr, size_t old_usize,
prof_tctx_t *old_tctx)
{
bool sampled, old_sampled;
cassert(config_prof);
assert(ptr != NULL || (uintptr_t)tctx <= (uintptr_t)1U);
if (prof_active && !updated && ptr != NULL) {
assert(usize == isalloc(tsd_tsdn(tsd), ptr, true));
if (prof_sample_accum_update(tsd, usize, true, NULL)) {
prof_alloc_rollback(tsd, tctx, true);
tctx = (prof_tctx_t *)(uintptr_t)1U;
}
}
sampled = ((uintptr_t)tctx > (uintptr_t)1U);
old_sampled = ((uintptr_t)old_tctx > (uintptr_t)1U);
if (unlikely(sampled))
prof_malloc_sample_object(tsd_tsdn(tsd), ptr, usize, tctx);
else
prof_tctx_reset(tsd_tsdn(tsd), ptr, usize, old_ptr, old_tctx);
if (unlikely(old_sampled))
prof_free_sampled_object(tsd, old_usize, old_tctx);
}
JEMALLOC_ALWAYS_INLINE void
prof_free(tsd_t *tsd, const void *ptr, size_t usize)
{
prof_tctx_t *tctx = prof_tctx_get(tsd_tsdn(tsd), ptr);
cassert(config_prof);
assert(usize == isalloc(tsd_tsdn(tsd), ptr, true));
if (unlikely((uintptr_t)tctx > (uintptr_t)1U))
prof_free_sampled_object(tsd, usize, tctx);
}
#endif
#endif 
