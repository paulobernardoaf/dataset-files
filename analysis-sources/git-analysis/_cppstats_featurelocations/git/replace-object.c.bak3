#include "cache.h"
#include "oidmap.h"
#include "object-store.h"
#include "replace-object.h"
#include "refs.h"
#include "repository.h"
#include "commit.h"

static int register_replace_ref(struct repository *r,
const char *refname,
const struct object_id *oid,
int flag, void *cb_data)
{

const char *slash = strrchr(refname, '/');
const char *hash = slash ? slash + 1 : refname;
struct replace_object *repl_obj = xmalloc(sizeof(*repl_obj));

if (get_oid_hex(hash, &repl_obj->original.oid)) {
free(repl_obj);
warning(_("bad replace ref name: %s"), refname);
return 0;
}


oidcpy(&repl_obj->replacement, oid);


if (oidmap_put(r->objects->replace_map, repl_obj))
die(_("duplicate replace ref: %s"), refname);

return 0;
}

void prepare_replace_object(struct repository *r)
{
if (r->objects->replace_map_initialized)
return;

pthread_mutex_lock(&r->objects->replace_mutex);
if (r->objects->replace_map_initialized) {
pthread_mutex_unlock(&r->objects->replace_mutex);
return;
}

r->objects->replace_map =
xmalloc(sizeof(*r->objects->replace_map));
oidmap_init(r->objects->replace_map, 0);

for_each_replace_ref(r, register_replace_ref, NULL);
r->objects->replace_map_initialized = 1;

pthread_mutex_unlock(&r->objects->replace_mutex);
}


#define MAXREPLACEDEPTH 5








const struct object_id *do_lookup_replace_object(struct repository *r,
const struct object_id *oid)
{
int depth = MAXREPLACEDEPTH;
const struct object_id *cur = oid;

prepare_replace_object(r);


while (depth-- > 0) {
struct replace_object *repl_obj =
oidmap_get(r->objects->replace_map, cur);
if (!repl_obj)
return cur;
cur = &repl_obj->replacement;
}
die(_("replace depth too high for object %s"), oid_to_hex(oid));
}
