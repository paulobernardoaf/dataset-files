
#if defined(JEMALLOC_H_TYPES)

typedef struct extent_node_s extent_node_t;

#endif 

#if defined(JEMALLOC_H_STRUCTS)


struct extent_node_s {

arena_t *en_arena;


void *en_addr;


size_t en_size;













size_t en_sn;





bool en_zeroed;






bool en_committed;





bool en_achunk;


prof_tctx_t *en_prof_tctx;


arena_runs_dirty_link_t rd;
qr(extent_node_t) cc_link;

union {

rb_node(extent_node_t) szsnad_link;


ql_elm(extent_node_t) ql_link;
};


rb_node(extent_node_t) ad_link;
};
typedef rb_tree(extent_node_t) extent_tree_t;

#endif 

#if defined(JEMALLOC_H_EXTERNS)

#if defined(JEMALLOC_JET)
size_t extent_size_quantize_floor(size_t size);
#endif
size_t extent_size_quantize_ceil(size_t size);

rb_proto(, extent_tree_szsnad_, extent_tree_t, extent_node_t)

rb_proto(, extent_tree_ad_, extent_tree_t, extent_node_t)

#endif 

#if defined(JEMALLOC_H_INLINES)

#if !defined(JEMALLOC_ENABLE_INLINE)
arena_t *extent_node_arena_get(const extent_node_t *node);
void *extent_node_addr_get(const extent_node_t *node);
size_t extent_node_size_get(const extent_node_t *node);
size_t extent_node_sn_get(const extent_node_t *node);
bool extent_node_zeroed_get(const extent_node_t *node);
bool extent_node_committed_get(const extent_node_t *node);
bool extent_node_achunk_get(const extent_node_t *node);
prof_tctx_t *extent_node_prof_tctx_get(const extent_node_t *node);
void extent_node_arena_set(extent_node_t *node, arena_t *arena);
void extent_node_addr_set(extent_node_t *node, void *addr);
void extent_node_size_set(extent_node_t *node, size_t size);
void extent_node_sn_set(extent_node_t *node, size_t sn);
void extent_node_zeroed_set(extent_node_t *node, bool zeroed);
void extent_node_committed_set(extent_node_t *node, bool committed);
void extent_node_achunk_set(extent_node_t *node, bool achunk);
void extent_node_prof_tctx_set(extent_node_t *node, prof_tctx_t *tctx);
void extent_node_init(extent_node_t *node, arena_t *arena, void *addr,
size_t size, size_t sn, bool zeroed, bool committed);
void extent_node_dirty_linkage_init(extent_node_t *node);
void extent_node_dirty_insert(extent_node_t *node,
arena_runs_dirty_link_t *runs_dirty, extent_node_t *chunks_dirty);
void extent_node_dirty_remove(extent_node_t *node);
#endif

#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_EXTENT_C_))
JEMALLOC_INLINE arena_t *
extent_node_arena_get(const extent_node_t *node)
{

return (node->en_arena);
}

JEMALLOC_INLINE void *
extent_node_addr_get(const extent_node_t *node)
{

return (node->en_addr);
}

JEMALLOC_INLINE size_t
extent_node_size_get(const extent_node_t *node)
{

return (node->en_size);
}

JEMALLOC_INLINE size_t
extent_node_sn_get(const extent_node_t *node)
{

return (node->en_sn);
}

JEMALLOC_INLINE bool
extent_node_zeroed_get(const extent_node_t *node)
{

return (node->en_zeroed);
}

JEMALLOC_INLINE bool
extent_node_committed_get(const extent_node_t *node)
{
if (unlikely(node->en_achunk))
return false;
return (node->en_committed);
}

JEMALLOC_INLINE bool
extent_node_achunk_get(const extent_node_t *node)
{

return (node->en_achunk);
}

JEMALLOC_INLINE prof_tctx_t *
extent_node_prof_tctx_get(const extent_node_t *node)
{

return (node->en_prof_tctx);
}

JEMALLOC_INLINE void
extent_node_arena_set(extent_node_t *node, arena_t *arena)
{

node->en_arena = arena;
}

JEMALLOC_INLINE void
extent_node_addr_set(extent_node_t *node, void *addr)
{

node->en_addr = addr;
}

JEMALLOC_INLINE void
extent_node_size_set(extent_node_t *node, size_t size)
{

node->en_size = size;
}

JEMALLOC_INLINE void
extent_node_sn_set(extent_node_t *node, size_t sn)
{

node->en_sn = sn;
}

JEMALLOC_INLINE void
extent_node_zeroed_set(extent_node_t *node, bool zeroed)
{

node->en_zeroed = zeroed;
}

JEMALLOC_INLINE void
extent_node_committed_set(extent_node_t *node, bool committed)
{

node->en_committed = committed;
}

JEMALLOC_INLINE void
extent_node_achunk_set(extent_node_t *node, bool achunk)
{

node->en_achunk = achunk;
}

JEMALLOC_INLINE void
extent_node_prof_tctx_set(extent_node_t *node, prof_tctx_t *tctx)
{

node->en_prof_tctx = tctx;
}

JEMALLOC_INLINE void
extent_node_init(extent_node_t *node, arena_t *arena, void *addr, size_t size,
size_t sn, bool zeroed, bool committed)
{

extent_node_arena_set(node, arena);
extent_node_addr_set(node, addr);
extent_node_size_set(node, size);
extent_node_sn_set(node, sn);
extent_node_zeroed_set(node, zeroed);
extent_node_committed_set(node, committed);
extent_node_achunk_set(node, false);
if (config_prof)
extent_node_prof_tctx_set(node, NULL);
}

JEMALLOC_INLINE void
extent_node_dirty_linkage_init(extent_node_t *node)
{

qr_new(&node->rd, rd_link);
qr_new(node, cc_link);
}

JEMALLOC_INLINE void
extent_node_dirty_insert(extent_node_t *node,
arena_runs_dirty_link_t *runs_dirty, extent_node_t *chunks_dirty)
{

qr_meld(runs_dirty, &node->rd, rd_link);
qr_meld(chunks_dirty, node, cc_link);
}

JEMALLOC_INLINE void
extent_node_dirty_remove(extent_node_t *node)
{

qr_remove(&node->rd, rd_link);
qr_remove(node, cc_link);
}

#endif

#endif 


