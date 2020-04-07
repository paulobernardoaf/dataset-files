#include "object-store.h"
#include "thread-utils.h"
#include "pack.h"
struct repository;
#define DEFAULT_DELTA_CACHE_SIZE (256 * 1024 * 1024)
#define OE_DFS_STATE_BITS 2
#define OE_DEPTH_BITS 12
#define OE_IN_PACK_BITS 10
#define OE_Z_DELTA_BITS 20
#define OE_SIZE_BITS 31
#define OE_DELTA_SIZE_BITS 23
enum dfs_state {
DFS_NONE = 0,
DFS_ACTIVE,
DFS_DONE,
DFS_NUM_STATES
};
struct object_entry {
struct pack_idx_entry idx;
void *delta_data; 
off_t in_pack_offset;
uint32_t hash; 
unsigned size_:OE_SIZE_BITS;
unsigned size_valid:1;
uint32_t delta_idx; 
uint32_t delta_child_idx; 
uint32_t delta_sibling_idx; 
unsigned delta_size_:OE_DELTA_SIZE_BITS; 
unsigned delta_size_valid:1;
unsigned char in_pack_header_size;
unsigned in_pack_idx:OE_IN_PACK_BITS; 
unsigned z_delta_size:OE_Z_DELTA_BITS;
unsigned type_valid:1;
unsigned no_try_delta:1;
unsigned type_:TYPE_BITS;
unsigned in_pack_type:TYPE_BITS; 
unsigned preferred_base:1; 
unsigned tagged:1; 
unsigned filled:1; 
unsigned dfs_state:OE_DFS_STATE_BITS;
unsigned depth:OE_DEPTH_BITS;
unsigned ext_base:1; 
};
struct packing_data {
struct repository *repo;
struct object_entry *objects;
uint32_t nr_objects, nr_alloc;
int32_t *index;
uint32_t index_size;
unsigned int *in_pack_pos;
unsigned long *delta_size;
struct packed_git **in_pack_by_idx;
struct packed_git **in_pack;
pthread_mutex_t odb_lock;
struct object_entry *ext_bases;
uint32_t nr_ext, alloc_ext;
uintmax_t oe_size_limit;
uintmax_t oe_delta_size_limit;
unsigned int *tree_depth;
unsigned char *layer;
};
void prepare_packing_data(struct repository *r, struct packing_data *pdata);
static inline void packing_data_lock(struct packing_data *pdata)
{
pthread_mutex_lock(&pdata->odb_lock);
}
static inline void packing_data_unlock(struct packing_data *pdata)
{
pthread_mutex_unlock(&pdata->odb_lock);
}
struct object_entry *packlist_alloc(struct packing_data *pdata,
const struct object_id *oid);
struct object_entry *packlist_find(struct packing_data *pdata,
const struct object_id *oid);
static inline uint32_t pack_name_hash(const char *name)
{
uint32_t c, hash = 0;
if (!name)
return 0;
while ((c = *name++) != 0) {
if (isspace(c))
continue;
hash = (hash >> 2) + (c << 24);
}
return hash;
}
static inline enum object_type oe_type(const struct object_entry *e)
{
return e->type_valid ? e->type_ : OBJ_BAD;
}
static inline void oe_set_type(struct object_entry *e,
enum object_type type)
{
if (type >= OBJ_ANY)
BUG("OBJ_ANY cannot be set in pack-objects code");
e->type_valid = type >= OBJ_NONE;
e->type_ = (unsigned)type;
}
static inline unsigned int oe_in_pack_pos(const struct packing_data *pack,
const struct object_entry *e)
{
return pack->in_pack_pos[e - pack->objects];
}
static inline void oe_set_in_pack_pos(const struct packing_data *pack,
const struct object_entry *e,
unsigned int pos)
{
pack->in_pack_pos[e - pack->objects] = pos;
}
static inline struct packed_git *oe_in_pack(const struct packing_data *pack,
const struct object_entry *e)
{
if (pack->in_pack_by_idx)
return pack->in_pack_by_idx[e->in_pack_idx];
else
return pack->in_pack[e - pack->objects];
}
void oe_map_new_pack(struct packing_data *pack);
static inline void oe_set_in_pack(struct packing_data *pack,
struct object_entry *e,
struct packed_git *p)
{
if (pack->in_pack_by_idx) {
if (p->index) {
e->in_pack_idx = p->index;
return;
}
oe_map_new_pack(pack);
}
pack->in_pack[e - pack->objects] = p;
}
static inline struct object_entry *oe_delta(
const struct packing_data *pack,
const struct object_entry *e)
{
if (!e->delta_idx)
return NULL;
if (e->ext_base)
return &pack->ext_bases[e->delta_idx - 1];
else
return &pack->objects[e->delta_idx - 1];
}
static inline void oe_set_delta(struct packing_data *pack,
struct object_entry *e,
struct object_entry *delta)
{
if (delta)
e->delta_idx = (delta - pack->objects) + 1;
else
e->delta_idx = 0;
}
void oe_set_delta_ext(struct packing_data *pack,
struct object_entry *e,
const struct object_id *oid);
static inline struct object_entry *oe_delta_child(
const struct packing_data *pack,
const struct object_entry *e)
{
if (e->delta_child_idx)
return &pack->objects[e->delta_child_idx - 1];
return NULL;
}
static inline void oe_set_delta_child(struct packing_data *pack,
struct object_entry *e,
struct object_entry *delta)
{
if (delta)
e->delta_child_idx = (delta - pack->objects) + 1;
else
e->delta_child_idx = 0;
}
static inline struct object_entry *oe_delta_sibling(
const struct packing_data *pack,
const struct object_entry *e)
{
if (e->delta_sibling_idx)
return &pack->objects[e->delta_sibling_idx - 1];
return NULL;
}
static inline void oe_set_delta_sibling(struct packing_data *pack,
struct object_entry *e,
struct object_entry *delta)
{
if (delta)
e->delta_sibling_idx = (delta - pack->objects) + 1;
else
e->delta_sibling_idx = 0;
}
unsigned long oe_get_size_slow(struct packing_data *pack,
const struct object_entry *e);
static inline unsigned long oe_size(struct packing_data *pack,
const struct object_entry *e)
{
if (e->size_valid)
return e->size_;
return oe_get_size_slow(pack, e);
}
static inline int oe_size_less_than(struct packing_data *pack,
const struct object_entry *lhs,
unsigned long rhs)
{
if (lhs->size_valid)
return lhs->size_ < rhs;
if (rhs < pack->oe_size_limit) 
return 0;
return oe_get_size_slow(pack, lhs) < rhs;
}
static inline int oe_size_greater_than(struct packing_data *pack,
const struct object_entry *lhs,
unsigned long rhs)
{
if (lhs->size_valid)
return lhs->size_ > rhs;
if (rhs < pack->oe_size_limit) 
return 1;
return oe_get_size_slow(pack, lhs) > rhs;
}
static inline void oe_set_size(struct packing_data *pack,
struct object_entry *e,
unsigned long size)
{
if (size < pack->oe_size_limit) {
e->size_ = size;
e->size_valid = 1;
} else {
e->size_valid = 0;
if (oe_get_size_slow(pack, e) != size)
BUG("'size' is supposed to be the object size!");
}
}
static inline unsigned long oe_delta_size(struct packing_data *pack,
const struct object_entry *e)
{
if (e->delta_size_valid)
return e->delta_size_;
return pack->delta_size[e - pack->objects];
}
static inline void oe_set_delta_size(struct packing_data *pack,
struct object_entry *e,
unsigned long size)
{
if (size < pack->oe_delta_size_limit) {
e->delta_size_ = size;
e->delta_size_valid = 1;
} else {
packing_data_lock(pack);
if (!pack->delta_size)
ALLOC_ARRAY(pack->delta_size, pack->nr_alloc);
packing_data_unlock(pack);
pack->delta_size[e - pack->objects] = size;
e->delta_size_valid = 0;
}
}
static inline unsigned int oe_tree_depth(struct packing_data *pack,
struct object_entry *e)
{
if (!pack->tree_depth)
return 0;
return pack->tree_depth[e - pack->objects];
}
static inline void oe_set_tree_depth(struct packing_data *pack,
struct object_entry *e,
unsigned int tree_depth)
{
if (!pack->tree_depth)
CALLOC_ARRAY(pack->tree_depth, pack->nr_alloc);
pack->tree_depth[e - pack->objects] = tree_depth;
}
static inline unsigned char oe_layer(struct packing_data *pack,
struct object_entry *e)
{
if (!pack->layer)
return 0;
return pack->layer[e - pack->objects];
}
static inline void oe_set_layer(struct packing_data *pack,
struct object_entry *e,
unsigned char layer)
{
if (!pack->layer)
CALLOC_ARRAY(pack->layer, pack->nr_alloc);
pack->layer[e - pack->objects] = layer;
}
