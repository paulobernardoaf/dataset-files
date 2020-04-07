#if !defined(REPLACE_OBJECT_H)
#define REPLACE_OBJECT_H

#include "oidmap.h"
#include "repository.h"
#include "object-store.h"

struct replace_object {
struct oidmap_entry original;
struct object_id replacement;
};

void prepare_replace_object(struct repository *r);





const struct object_id *do_lookup_replace_object(struct repository *r,
const struct object_id *oid);












static inline const struct object_id *lookup_replace_object(struct repository *r,
const struct object_id *oid)
{
if (!read_replace_refs ||
(r->objects->replace_map_initialized &&
r->objects->replace_map->map.tablesize == 0))
return oid;
return do_lookup_replace_object(r, oid);
}

#endif 
