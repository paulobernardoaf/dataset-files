#if defined(JEMALLOC_H_TYPES)
typedef struct witness_s witness_t;
typedef unsigned witness_rank_t;
typedef ql_head(witness_t) witness_list_t;
typedef int witness_comp_t (const witness_t *, const witness_t *);
#define WITNESS_RANK_OMIT 0U
#define WITNESS_RANK_MIN 1U
#define WITNESS_RANK_INIT 1U
#define WITNESS_RANK_CTL 1U
#define WITNESS_RANK_TCACHES 2U
#define WITNESS_RANK_ARENAS 3U
#define WITNESS_RANK_PROF_DUMP 4U
#define WITNESS_RANK_PROF_BT2GCTX 5U
#define WITNESS_RANK_PROF_TDATAS 6U
#define WITNESS_RANK_PROF_TDATA 7U
#define WITNESS_RANK_PROF_GCTX 8U
#define WITNESS_RANK_CORE 9U
#define WITNESS_RANK_ARENA 9U
#define WITNESS_RANK_ARENA_CHUNKS 10U
#define WITNESS_RANK_ARENA_NODE_CACHE 11U
#define WITNESS_RANK_BASE 12U
#define WITNESS_RANK_LEAF 0xffffffffU
#define WITNESS_RANK_ARENA_BIN WITNESS_RANK_LEAF
#define WITNESS_RANK_ARENA_HUGE WITNESS_RANK_LEAF
#define WITNESS_RANK_DSS WITNESS_RANK_LEAF
#define WITNESS_RANK_PROF_ACTIVE WITNESS_RANK_LEAF
#define WITNESS_RANK_PROF_DUMP_SEQ WITNESS_RANK_LEAF
#define WITNESS_RANK_PROF_GDUMP WITNESS_RANK_LEAF
#define WITNESS_RANK_PROF_NEXT_THR_UID WITNESS_RANK_LEAF
#define WITNESS_RANK_PROF_THREAD_ACTIVE_INIT WITNESS_RANK_LEAF
#define WITNESS_INITIALIZER(rank) {"initializer", rank, NULL, {NULL, NULL}}
#endif 
#if defined(JEMALLOC_H_STRUCTS)
struct witness_s {
const char *name;
witness_rank_t rank;
witness_comp_t *comp;
ql_elm(witness_t) link;
};
#endif 
#if defined(JEMALLOC_H_EXTERNS)
void witness_init(witness_t *witness, const char *name, witness_rank_t rank,
witness_comp_t *comp);
#if defined(JEMALLOC_JET)
typedef void (witness_lock_error_t)(const witness_list_t *, const witness_t *);
extern witness_lock_error_t *witness_lock_error;
#else
void witness_lock_error(const witness_list_t *witnesses,
const witness_t *witness);
#endif
#if defined(JEMALLOC_JET)
typedef void (witness_owner_error_t)(const witness_t *);
extern witness_owner_error_t *witness_owner_error;
#else
void witness_owner_error(const witness_t *witness);
#endif
#if defined(JEMALLOC_JET)
typedef void (witness_not_owner_error_t)(const witness_t *);
extern witness_not_owner_error_t *witness_not_owner_error;
#else
void witness_not_owner_error(const witness_t *witness);
#endif
#if defined(JEMALLOC_JET)
typedef void (witness_depth_error_t)(const witness_list_t *,
witness_rank_t rank_inclusive, unsigned depth);
extern witness_depth_error_t *witness_depth_error;
#else
void witness_depth_error(const witness_list_t *witnesses,
witness_rank_t rank_inclusive, unsigned depth);
#endif
void witnesses_cleanup(tsd_t *tsd);
void witness_fork_cleanup(tsd_t *tsd);
void witness_prefork(tsd_t *tsd);
void witness_postfork_parent(tsd_t *tsd);
void witness_postfork_child(tsd_t *tsd);
#endif 
#if defined(JEMALLOC_H_INLINES)
#if !defined(JEMALLOC_ENABLE_INLINE)
bool witness_owner(tsd_t *tsd, const witness_t *witness);
void witness_assert_owner(tsdn_t *tsdn, const witness_t *witness);
void witness_assert_not_owner(tsdn_t *tsdn, const witness_t *witness);
void witness_assert_depth_to_rank(tsdn_t *tsdn, witness_rank_t rank_inclusive,
unsigned depth);
void witness_assert_depth(tsdn_t *tsdn, unsigned depth);
void witness_assert_lockless(tsdn_t *tsdn);
void witness_lock(tsdn_t *tsdn, witness_t *witness);
void witness_unlock(tsdn_t *tsdn, witness_t *witness);
#endif
#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_MUTEX_C_))
JEMALLOC_INLINE bool
witness_owner(tsd_t *tsd, const witness_t *witness)
{
witness_list_t *witnesses;
witness_t *w;
cassert(config_debug);
witnesses = tsd_witnessesp_get(tsd);
ql_foreach(w, witnesses, link) {
if (w == witness)
return (true);
}
return (false);
}
JEMALLOC_INLINE void
witness_assert_owner(tsdn_t *tsdn, const witness_t *witness)
{
tsd_t *tsd;
if (!config_debug)
return;
if (tsdn_null(tsdn))
return;
tsd = tsdn_tsd(tsdn);
if (witness->rank == WITNESS_RANK_OMIT)
return;
if (witness_owner(tsd, witness))
return;
witness_owner_error(witness);
}
JEMALLOC_INLINE void
witness_assert_not_owner(tsdn_t *tsdn, const witness_t *witness)
{
tsd_t *tsd;
witness_list_t *witnesses;
witness_t *w;
if (!config_debug)
return;
if (tsdn_null(tsdn))
return;
tsd = tsdn_tsd(tsdn);
if (witness->rank == WITNESS_RANK_OMIT)
return;
witnesses = tsd_witnessesp_get(tsd);
ql_foreach(w, witnesses, link) {
if (w == witness)
witness_not_owner_error(witness);
}
}
JEMALLOC_INLINE void
witness_assert_depth_to_rank(tsdn_t *tsdn, witness_rank_t rank_inclusive,
unsigned depth) {
tsd_t *tsd;
unsigned d;
witness_list_t *witnesses;
witness_t *w;
if (!config_debug)
return;
if (tsdn_null(tsdn))
return;
tsd = tsdn_tsd(tsdn);
d = 0;
witnesses = tsd_witnessesp_get(tsd);
w = ql_last(witnesses, link);
if (w != NULL) {
ql_reverse_foreach(w, witnesses, link) {
if (w->rank < rank_inclusive) {
break;
}
d++;
}
}
if (d != depth)
witness_depth_error(witnesses, rank_inclusive, depth);
}
JEMALLOC_INLINE void
witness_assert_depth(tsdn_t *tsdn, unsigned depth) {
witness_assert_depth_to_rank(tsdn, WITNESS_RANK_MIN, depth);
}
JEMALLOC_INLINE void
witness_assert_lockless(tsdn_t *tsdn) {
witness_assert_depth(tsdn, 0);
}
JEMALLOC_INLINE void
witness_lock(tsdn_t *tsdn, witness_t *witness)
{
tsd_t *tsd;
witness_list_t *witnesses;
witness_t *w;
if (!config_debug)
return;
if (tsdn_null(tsdn))
return;
tsd = tsdn_tsd(tsdn);
if (witness->rank == WITNESS_RANK_OMIT)
return;
witness_assert_not_owner(tsdn, witness);
witnesses = tsd_witnessesp_get(tsd);
w = ql_last(witnesses, link);
if (w == NULL) {
} else if (tsd_witness_fork_get(tsd) && w->rank <= witness->rank) {
} else if (w->rank > witness->rank) {
witness_lock_error(witnesses, witness);
} else if (w->rank == witness->rank && (w->comp == NULL || w->comp !=
witness->comp || w->comp(w, witness) > 0)) {
witness_lock_error(witnesses, witness);
}
ql_elm_new(witness, link);
ql_tail_insert(witnesses, witness, link);
}
JEMALLOC_INLINE void
witness_unlock(tsdn_t *tsdn, witness_t *witness)
{
tsd_t *tsd;
witness_list_t *witnesses;
if (!config_debug)
return;
if (tsdn_null(tsdn))
return;
tsd = tsdn_tsd(tsdn);
if (witness->rank == WITNESS_RANK_OMIT)
return;
if (witness_owner(tsd, witness)) {
witnesses = tsd_witnessesp_get(tsd);
ql_remove(witnesses, witness, link);
} else
witness_assert_owner(tsdn, witness);
}
#endif
#endif 
