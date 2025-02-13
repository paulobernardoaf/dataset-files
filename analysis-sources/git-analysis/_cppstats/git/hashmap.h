#include "hash.h"
unsigned int strhash(const char *buf);
unsigned int strihash(const char *buf);
unsigned int memhash(const void *buf, size_t len);
unsigned int memihash(const void *buf, size_t len);
unsigned int memihash_cont(unsigned int hash_seed, const void *buf, size_t len);
static inline unsigned int oidhash(const struct object_id *oid)
{
unsigned int hash;
memcpy(&hash, oid->hash, sizeof(hash));
return hash;
}
struct hashmap_entry {
struct hashmap_entry *next;
unsigned int hash;
};
typedef int (*hashmap_cmp_fn)(const void *hashmap_cmp_fn_data,
const struct hashmap_entry *entry,
const struct hashmap_entry *entry_or_key,
const void *keydata);
struct hashmap {
struct hashmap_entry **table;
hashmap_cmp_fn cmpfn;
const void *cmpfn_data;
unsigned int private_size; 
unsigned int tablesize;
unsigned int grow_at;
unsigned int shrink_at;
unsigned int do_count_items : 1;
};
void hashmap_init(struct hashmap *map,
hashmap_cmp_fn equals_function,
const void *equals_function_data,
size_t initial_size);
void hashmap_free_(struct hashmap *map, ssize_t offset);
#define hashmap_free(map) hashmap_free_(map, -1)
#define hashmap_free_entries(map, type, member) hashmap_free_(map, offsetof(type, member));
static inline void hashmap_entry_init(struct hashmap_entry *e,
unsigned int hash)
{
e->hash = hash;
e->next = NULL;
}
static inline unsigned int hashmap_get_size(struct hashmap *map)
{
if (map->do_count_items)
return map->private_size;
BUG("hashmap_get_size: size not set");
return 0;
}
struct hashmap_entry *hashmap_get(const struct hashmap *map,
const struct hashmap_entry *key,
const void *keydata);
static inline struct hashmap_entry *hashmap_get_from_hash(
const struct hashmap *map,
unsigned int hash,
const void *keydata)
{
struct hashmap_entry key;
hashmap_entry_init(&key, hash);
return hashmap_get(map, &key, keydata);
}
struct hashmap_entry *hashmap_get_next(const struct hashmap *map,
const struct hashmap_entry *entry);
void hashmap_add(struct hashmap *map, struct hashmap_entry *entry);
struct hashmap_entry *hashmap_put(struct hashmap *map,
struct hashmap_entry *entry);
#define hashmap_put_entry(map, keyvar, member) container_of_or_null_offset(hashmap_put(map, &(keyvar)->member), OFFSETOF_VAR(keyvar, member))
struct hashmap_entry *hashmap_remove(struct hashmap *map,
const struct hashmap_entry *key,
const void *keydata);
#define hashmap_remove_entry(map, keyvar, member, keydata) container_of_or_null_offset( hashmap_remove(map, &(keyvar)->member, keydata), OFFSETOF_VAR(keyvar, member))
int hashmap_bucket(const struct hashmap *map, unsigned int hash);
struct hashmap_iter {
struct hashmap *map;
struct hashmap_entry *next;
unsigned int tablepos;
};
void hashmap_iter_init(struct hashmap *map, struct hashmap_iter *iter);
struct hashmap_entry *hashmap_iter_next(struct hashmap_iter *iter);
static inline struct hashmap_entry *hashmap_iter_first(struct hashmap *map,
struct hashmap_iter *iter)
{
hashmap_iter_init(map, iter);
return hashmap_iter_next(iter);
}
#define hashmap_iter_first_entry(map, iter, type, member) container_of_or_null(hashmap_iter_first(map, iter), type, member)
#define hashmap_iter_next_entry_offset(iter, offset) container_of_or_null_offset(hashmap_iter_next(iter), offset)
#define hashmap_iter_first_entry_offset(map, iter, offset) container_of_or_null_offset(hashmap_iter_first(map, iter), offset)
#define hashmap_for_each_entry(map, iter, var, member) for (var = hashmap_iter_first_entry_offset(map, iter, OFFSETOF_VAR(var, member)); var; var = hashmap_iter_next_entry_offset(iter, OFFSETOF_VAR(var, member)))
#define hashmap_get_entry(map, keyvar, member, keydata) container_of_or_null_offset( hashmap_get(map, &(keyvar)->member, keydata), OFFSETOF_VAR(keyvar, member))
#define hashmap_get_entry_from_hash(map, hash, keydata, type, member) container_of_or_null(hashmap_get_from_hash(map, hash, keydata), type, member)
#define hashmap_get_next_entry(map, var, member) container_of_or_null_offset(hashmap_get_next(map, &(var)->member), OFFSETOF_VAR(var, member))
#define hashmap_for_each_entry_from(map, var, member) for (; var; var = hashmap_get_next_entry(map, var, member))
static inline void hashmap_disable_item_counting(struct hashmap *map)
{
map->do_count_items = 0;
}
static inline void hashmap_enable_item_counting(struct hashmap *map)
{
unsigned int n = 0;
struct hashmap_iter iter;
if (map->do_count_items)
return;
hashmap_iter_init(map, &iter);
while (hashmap_iter_next(&iter))
n++;
map->do_count_items = 1;
map->private_size = n;
}
const void *memintern(const void *data, size_t len);
static inline const char *strintern(const char *string)
{
return memintern(string, strlen(string));
}
