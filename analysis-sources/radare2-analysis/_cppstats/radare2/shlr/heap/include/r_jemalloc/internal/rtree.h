#if defined(JEMALLOC_H_TYPES)
typedef struct rtree_node_elm_s rtree_node_elm_t;
typedef struct rtree_level_s rtree_level_t;
typedef struct rtree_s rtree_t;
#define LG_RTREE_BITS_PER_LEVEL 4
#define RTREE_BITS_PER_LEVEL (1U << LG_RTREE_BITS_PER_LEVEL)
#define RTREE_HEIGHT_MAX ((1U << (LG_SIZEOF_PTR+3)) / RTREE_BITS_PER_LEVEL)
#define RTREE_NODE_INITIALIZING ((rtree_node_elm_t *)0x1)
typedef rtree_node_elm_t *(rtree_node_alloc_t)(size_t);
typedef void (rtree_node_dalloc_t)(rtree_node_elm_t *);
#endif 
#if defined(JEMALLOC_H_STRUCTS)
struct rtree_node_elm_s {
union {
void *pun;
rtree_node_elm_t *child;
extent_node_t *val;
};
};
struct rtree_level_s {
union {
void *subtree_pun;
rtree_node_elm_t *subtree;
};
unsigned bits;
unsigned cumbits;
};
struct rtree_s {
rtree_node_alloc_t *alloc;
rtree_node_dalloc_t *dalloc;
unsigned height;
unsigned start_level[RTREE_HEIGHT_MAX];
rtree_level_t levels[RTREE_HEIGHT_MAX];
};
#endif 
#if defined(JEMALLOC_H_EXTERNS)
bool rtree_new(rtree_t *rtree, unsigned bits, rtree_node_alloc_t *alloc,
rtree_node_dalloc_t *dalloc);
void rtree_delete(rtree_t *rtree);
rtree_node_elm_t *rtree_subtree_read_hard(rtree_t *rtree,
unsigned level);
rtree_node_elm_t *rtree_child_read_hard(rtree_t *rtree,
rtree_node_elm_t *elm, unsigned level);
#endif 
#if defined(JEMALLOC_H_INLINES)
#if !defined(JEMALLOC_ENABLE_INLINE)
unsigned rtree_start_level(rtree_t *rtree, uintptr_t key);
uintptr_t rtree_subkey(rtree_t *rtree, uintptr_t key, unsigned level);
bool rtree_node_valid(rtree_node_elm_t *node);
rtree_node_elm_t *rtree_child_tryread(rtree_node_elm_t *elm,
bool dependent);
rtree_node_elm_t *rtree_child_read(rtree_t *rtree, rtree_node_elm_t *elm,
unsigned level, bool dependent);
extent_node_t *rtree_val_read(rtree_t *rtree, rtree_node_elm_t *elm,
bool dependent);
void rtree_val_write(rtree_t *rtree, rtree_node_elm_t *elm,
const extent_node_t *val);
rtree_node_elm_t *rtree_subtree_tryread(rtree_t *rtree, unsigned level,
bool dependent);
rtree_node_elm_t *rtree_subtree_read(rtree_t *rtree, unsigned level,
bool dependent);
extent_node_t *rtree_get(rtree_t *rtree, uintptr_t key, bool dependent);
bool rtree_set(rtree_t *rtree, uintptr_t key, const extent_node_t *val);
#endif
#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_RTREE_C_))
JEMALLOC_ALWAYS_INLINE unsigned
rtree_start_level(rtree_t *rtree, uintptr_t key)
{
unsigned start_level;
if (unlikely(key == 0))
return (rtree->height - 1);
start_level = rtree->start_level[lg_floor(key) >>
LG_RTREE_BITS_PER_LEVEL];
assert(start_level < rtree->height);
return (start_level);
}
JEMALLOC_ALWAYS_INLINE uintptr_t
rtree_subkey(rtree_t *rtree, uintptr_t key, unsigned level)
{
return ((key >> ((ZU(1) << (LG_SIZEOF_PTR+3)) -
rtree->levels[level].cumbits)) & ((ZU(1) <<
rtree->levels[level].bits) - 1));
}
JEMALLOC_ALWAYS_INLINE bool
rtree_node_valid(rtree_node_elm_t *node)
{
return ((uintptr_t)node > (uintptr_t)RTREE_NODE_INITIALIZING);
}
JEMALLOC_ALWAYS_INLINE rtree_node_elm_t *
rtree_child_tryread(rtree_node_elm_t *elm, bool dependent)
{
rtree_node_elm_t *child;
child = elm->child;
if (!dependent && !rtree_node_valid(child))
child = atomic_read_p(&elm->pun);
if (unlikely(dependent || child == NULL))
return (NULL);
return (child);
}
JEMALLOC_ALWAYS_INLINE rtree_node_elm_t *
rtree_child_read(rtree_t *rtree, rtree_node_elm_t *elm, unsigned level,
bool dependent)
{
rtree_node_elm_t *child;
child = rtree_child_tryread(elm, dependent);
if (!dependent && unlikely(!rtree_node_valid(child)))
child = rtree_child_read_hard(rtree, elm, level);
if (unlikely(dependent || child == NULL))
return (NULL);
return (child);
}
JEMALLOC_ALWAYS_INLINE extent_node_t *
rtree_val_read(rtree_t *rtree, rtree_node_elm_t *elm, bool dependent)
{
if (dependent) {
return (elm->val);
} else {
return (atomic_read_p(&elm->pun));
}
}
JEMALLOC_INLINE void
rtree_val_write(rtree_t *rtree, rtree_node_elm_t *elm, const extent_node_t *val)
{
atomic_write_p(&elm->pun, val);
}
JEMALLOC_ALWAYS_INLINE rtree_node_elm_t *
rtree_subtree_tryread(rtree_t *rtree, unsigned level, bool dependent)
{
rtree_node_elm_t *subtree;
subtree = rtree->levels[level].subtree;
if (!dependent && unlikely(!rtree_node_valid(subtree)))
subtree = atomic_read_p(&rtree->levels[level].subtree_pun);
if (unlikely(dependent || subtree == NULL))
return (NULL);
return (subtree);
}
JEMALLOC_ALWAYS_INLINE rtree_node_elm_t *
rtree_subtree_read(rtree_t *rtree, unsigned level, bool dependent)
{
rtree_node_elm_t *subtree;
subtree = rtree_subtree_tryread(rtree, level, dependent);
if (!dependent && unlikely(!rtree_node_valid(subtree)))
subtree = rtree_subtree_read_hard(rtree, level);
if (unlikely(dependent || subtree == NULL))
return (NULL);
return (subtree);
}
JEMALLOC_ALWAYS_INLINE extent_node_t *
rtree_get(rtree_t *rtree, uintptr_t key, bool dependent)
{
uintptr_t subkey;
unsigned start_level;
rtree_node_elm_t *node;
start_level = rtree_start_level(rtree, key);
node = rtree_subtree_tryread(rtree, start_level, dependent);
#define RTREE_GET_BIAS (RTREE_HEIGHT_MAX - rtree->height)
switch (start_level + RTREE_GET_BIAS) {
#define RTREE_GET_SUBTREE(level) case level: if (unlikely(level > (RTREE_HEIGHT_MAX-1)))return (NULL)if (!dependent && unlikely(!rtree_node_valid(node))) return (NULL); subkey = rtree_subkey(rtree, key, level - RTREE_GET_BIAS); node = rtree_child_tryread(&node[subkey], dependent); 
#define RTREE_GET_LEAF(level) case level: if (unlikely(level != (RTREE_HEIGHT_MAX-1))) return (NULL); if (!dependent && unlikely(!rtree_node_valid(node))) return (NULL); subkey = rtree_subkey(rtree, key, level - RTREE_GET_BIAS); return (rtree_val_read(rtree, &node[subkey], dependent));
#if RTREE_HEIGHT_MAX > 1
RTREE_GET_SUBTREE(0)
#endif
#if RTREE_HEIGHT_MAX > 2
RTREE_GET_SUBTREE(1)
#endif
#if RTREE_HEIGHT_MAX > 3
RTREE_GET_SUBTREE(2)
#endif
#if RTREE_HEIGHT_MAX > 4
RTREE_GET_SUBTREE(3)
#endif
#if RTREE_HEIGHT_MAX > 5
RTREE_GET_SUBTREE(4)
#endif
#if RTREE_HEIGHT_MAX > 6
RTREE_GET_SUBTREE(5)
#endif
#if RTREE_HEIGHT_MAX > 7
RTREE_GET_SUBTREE(6)
#endif
#if RTREE_HEIGHT_MAX > 8
RTREE_GET_SUBTREE(7)
#endif
#if RTREE_HEIGHT_MAX > 9
RTREE_GET_SUBTREE(8)
#endif
#if RTREE_HEIGHT_MAX > 10
RTREE_GET_SUBTREE(9)
#endif
#if RTREE_HEIGHT_MAX > 11
RTREE_GET_SUBTREE(10)
#endif
#if RTREE_HEIGHT_MAX > 12
RTREE_GET_SUBTREE(11)
#endif
#if RTREE_HEIGHT_MAX > 13
RTREE_GET_SUBTREE(12)
#endif
#if RTREE_HEIGHT_MAX > 14
RTREE_GET_SUBTREE(13)
#endif
#if RTREE_HEIGHT_MAX > 15
RTREE_GET_SUBTREE(14)
#endif
#if RTREE_HEIGHT_MAX > 16
#error Unsupported RTREE_HEIGHT_MAX
#endif
RTREE_GET_LEAF(RTREE_HEIGHT_MAX-1)
#undef RTREE_GET_SUBTREE
#undef RTREE_GET_LEAF
default: not_reached();
}
#undef RTREE_GET_BIAS
not_reached();
}
JEMALLOC_INLINE bool
rtree_set(rtree_t *rtree, uintptr_t key, const extent_node_t *val)
{
uintptr_t subkey;
unsigned i, start_level;
rtree_node_elm_t *node, *child;
start_level = rtree_start_level(rtree, key);
node = rtree_subtree_read(rtree, start_level, false);
if (node == NULL)
return (true);
for (i = start_level; ; i++, node = child) {
subkey = rtree_subkey(rtree, key, i);
if (i == rtree->height - 1) {
rtree_val_write(rtree, &node[subkey], val);
return (false);
}
if (unlikely (i + 1 > rtree->height))
return (false);
child = rtree_child_read(rtree, &node[subkey], i, false);
if (child == NULL)
return (true);
}
not_reached();
}
#endif
#endif 
