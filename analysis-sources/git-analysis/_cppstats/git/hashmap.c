#include "cache.h"
#include "hashmap.h"
#define FNV32_BASE ((unsigned int) 0x811c9dc5)
#define FNV32_PRIME ((unsigned int) 0x01000193)
unsigned int strhash(const char *str)
{
unsigned int c, hash = FNV32_BASE;
while ((c = (unsigned char) *str++))
hash = (hash * FNV32_PRIME) ^ c;
return hash;
}
unsigned int strihash(const char *str)
{
unsigned int c, hash = FNV32_BASE;
while ((c = (unsigned char) *str++)) {
if (c >= 'a' && c <= 'z')
c -= 'a' - 'A';
hash = (hash * FNV32_PRIME) ^ c;
}
return hash;
}
unsigned int memhash(const void *buf, size_t len)
{
unsigned int hash = FNV32_BASE;
unsigned char *ucbuf = (unsigned char *) buf;
while (len--) {
unsigned int c = *ucbuf++;
hash = (hash * FNV32_PRIME) ^ c;
}
return hash;
}
unsigned int memihash(const void *buf, size_t len)
{
unsigned int hash = FNV32_BASE;
unsigned char *ucbuf = (unsigned char *) buf;
while (len--) {
unsigned int c = *ucbuf++;
if (c >= 'a' && c <= 'z')
c -= 'a' - 'A';
hash = (hash * FNV32_PRIME) ^ c;
}
return hash;
}
unsigned int memihash_cont(unsigned int hash_seed, const void *buf, size_t len)
{
unsigned int hash = hash_seed;
unsigned char *ucbuf = (unsigned char *) buf;
while (len--) {
unsigned int c = *ucbuf++;
if (c >= 'a' && c <= 'z')
c -= 'a' - 'A';
hash = (hash * FNV32_PRIME) ^ c;
}
return hash;
}
#define HASHMAP_INITIAL_SIZE 64
#define HASHMAP_RESIZE_BITS 2
#define HASHMAP_LOAD_FACTOR 80
static void alloc_table(struct hashmap *map, unsigned int size)
{
map->tablesize = size;
map->table = xcalloc(size, sizeof(struct hashmap_entry *));
map->grow_at = (unsigned int) ((uint64_t) size * HASHMAP_LOAD_FACTOR / 100);
if (size <= HASHMAP_INITIAL_SIZE)
map->shrink_at = 0;
else
map->shrink_at = map->grow_at / ((1 << HASHMAP_RESIZE_BITS) + 1);
}
static inline int entry_equals(const struct hashmap *map,
const struct hashmap_entry *e1, const struct hashmap_entry *e2,
const void *keydata)
{
return (e1 == e2) ||
(e1->hash == e2->hash &&
!map->cmpfn(map->cmpfn_data, e1, e2, keydata));
}
static inline unsigned int bucket(const struct hashmap *map,
const struct hashmap_entry *key)
{
return key->hash & (map->tablesize - 1);
}
int hashmap_bucket(const struct hashmap *map, unsigned int hash)
{
return hash & (map->tablesize - 1);
}
static void rehash(struct hashmap *map, unsigned int newsize)
{
unsigned int i, oldsize = map->tablesize;
struct hashmap_entry **oldtable = map->table;
alloc_table(map, newsize);
for (i = 0; i < oldsize; i++) {
struct hashmap_entry *e = oldtable[i];
while (e) {
struct hashmap_entry *next = e->next;
unsigned int b = bucket(map, e);
e->next = map->table[b];
map->table[b] = e;
e = next;
}
}
free(oldtable);
}
static inline struct hashmap_entry **find_entry_ptr(const struct hashmap *map,
const struct hashmap_entry *key, const void *keydata)
{
struct hashmap_entry **e = &map->table[bucket(map, key)];
while (*e && !entry_equals(map, *e, key, keydata))
e = &(*e)->next;
return e;
}
static int always_equal(const void *unused_cmp_data,
const struct hashmap_entry *unused1,
const struct hashmap_entry *unused2,
const void *unused_keydata)
{
return 0;
}
void hashmap_init(struct hashmap *map, hashmap_cmp_fn equals_function,
const void *cmpfn_data, size_t initial_size)
{
unsigned int size = HASHMAP_INITIAL_SIZE;
memset(map, 0, sizeof(*map));
map->cmpfn = equals_function ? equals_function : always_equal;
map->cmpfn_data = cmpfn_data;
initial_size = (unsigned int) ((uint64_t) initial_size * 100
/ HASHMAP_LOAD_FACTOR);
while (initial_size > size)
size <<= HASHMAP_RESIZE_BITS;
alloc_table(map, size);
map->do_count_items = 1;
}
void hashmap_free_(struct hashmap *map, ssize_t entry_offset)
{
if (!map || !map->table)
return;
if (entry_offset >= 0) { 
struct hashmap_iter iter;
struct hashmap_entry *e;
hashmap_iter_init(map, &iter);
while ((e = hashmap_iter_next(&iter)))
free((char *)e - entry_offset);
}
free(map->table);
memset(map, 0, sizeof(*map));
}
struct hashmap_entry *hashmap_get(const struct hashmap *map,
const struct hashmap_entry *key,
const void *keydata)
{
return *find_entry_ptr(map, key, keydata);
}
struct hashmap_entry *hashmap_get_next(const struct hashmap *map,
const struct hashmap_entry *entry)
{
struct hashmap_entry *e = entry->next;
for (; e; e = e->next)
if (entry_equals(map, entry, e, NULL))
return e;
return NULL;
}
void hashmap_add(struct hashmap *map, struct hashmap_entry *entry)
{
unsigned int b = bucket(map, entry);
entry->next = map->table[b];
map->table[b] = entry;
if (map->do_count_items) {
map->private_size++;
if (map->private_size > map->grow_at)
rehash(map, map->tablesize << HASHMAP_RESIZE_BITS);
}
}
struct hashmap_entry *hashmap_remove(struct hashmap *map,
const struct hashmap_entry *key,
const void *keydata)
{
struct hashmap_entry *old;
struct hashmap_entry **e = find_entry_ptr(map, key, keydata);
if (!*e)
return NULL;
old = *e;
*e = old->next;
old->next = NULL;
if (map->do_count_items) {
map->private_size--;
if (map->private_size < map->shrink_at)
rehash(map, map->tablesize >> HASHMAP_RESIZE_BITS);
}
return old;
}
struct hashmap_entry *hashmap_put(struct hashmap *map,
struct hashmap_entry *entry)
{
struct hashmap_entry *old = hashmap_remove(map, entry, NULL);
hashmap_add(map, entry);
return old;
}
void hashmap_iter_init(struct hashmap *map, struct hashmap_iter *iter)
{
iter->map = map;
iter->tablepos = 0;
iter->next = NULL;
}
struct hashmap_entry *hashmap_iter_next(struct hashmap_iter *iter)
{
struct hashmap_entry *current = iter->next;
for (;;) {
if (current) {
iter->next = current->next;
return current;
}
if (iter->tablepos >= iter->map->tablesize)
return NULL;
current = iter->map->table[iter->tablepos++];
}
}
struct pool_entry {
struct hashmap_entry ent;
size_t len;
unsigned char data[FLEX_ARRAY];
};
static int pool_entry_cmp(const void *unused_cmp_data,
const struct hashmap_entry *eptr,
const struct hashmap_entry *entry_or_key,
const void *keydata)
{
const struct pool_entry *e1, *e2;
e1 = container_of(eptr, const struct pool_entry, ent);
e2 = container_of(entry_or_key, const struct pool_entry, ent);
return e1->data != keydata &&
(e1->len != e2->len || memcmp(e1->data, keydata, e1->len));
}
const void *memintern(const void *data, size_t len)
{
static struct hashmap map;
struct pool_entry key, *e;
if (!map.tablesize)
hashmap_init(&map, pool_entry_cmp, NULL, 0);
hashmap_entry_init(&key.ent, memhash(data, len));
key.len = len;
e = hashmap_get_entry(&map, &key, ent, data);
if (!e) {
FLEX_ALLOC_MEM(e, data, data, len);
hashmap_entry_init(&e->ent, key.ent.hash);
e->len = len;
hashmap_add(&map, &e->ent);
}
return e->data;
}
