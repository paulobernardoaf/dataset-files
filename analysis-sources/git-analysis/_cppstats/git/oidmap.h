#include "cache.h"
#include "hashmap.h"
struct oidmap_entry {
struct hashmap_entry internal_entry;
struct object_id oid;
};
struct oidmap {
struct hashmap map;
};
#define OIDMAP_INIT { { NULL } }
void oidmap_init(struct oidmap *map, size_t initial_size);
void oidmap_free(struct oidmap *map, int free_entries);
void *oidmap_get(const struct oidmap *map,
const struct object_id *key);
void *oidmap_put(struct oidmap *map, void *entry);
void *oidmap_remove(struct oidmap *map, const struct object_id *key);
struct oidmap_iter {
struct hashmap_iter h_iter;
};
static inline void oidmap_iter_init(struct oidmap *map, struct oidmap_iter *iter)
{
hashmap_iter_init(&map->map, &iter->h_iter);
}
static inline void *oidmap_iter_next(struct oidmap_iter *iter)
{
return (void *)hashmap_iter_next(&iter->h_iter);
}
static inline void *oidmap_iter_first(struct oidmap *map,
struct oidmap_iter *iter)
{
oidmap_iter_init(map, iter);
return (void *)oidmap_iter_next(iter);
}
