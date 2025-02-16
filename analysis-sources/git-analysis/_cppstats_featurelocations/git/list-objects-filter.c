#include "cache.h"
#include "dir.h"
#include "tag.h"
#include "commit.h"
#include "tree.h"
#include "blob.h"
#include "diff.h"
#include "tree-walk.h"
#include "revision.h"
#include "list-objects.h"
#include "list-objects-filter.h"
#include "list-objects-filter-options.h"
#include "oidmap.h"
#include "oidset.h"
#include "object-store.h"











#define FILTER_SHOWN_BUT_REVISIT (1<<21)

struct subfilter {
struct filter *filter;
struct oidset seen;
struct oidset omits;
struct object_id skip_tree;
unsigned is_skipping_tree : 1;
};

struct filter {
enum list_objects_filter_result (*filter_object_fn)(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct oidset *omits,
void *filter_data);
















void (*finalize_omits_fn)(struct oidset *omits, void *filter_data);

void (*free_fn)(void *filter_data);

void *filter_data;


struct oidset *omits;
};

static enum list_objects_filter_result filter_blobs_none(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct oidset *omits,
void *filter_data_)
{
switch (filter_situation) {
default:
BUG("unknown filter_situation: %d", filter_situation);

case LOFS_BEGIN_TREE:
assert(obj->type == OBJ_TREE);

return LOFR_MARK_SEEN | LOFR_DO_SHOW;

case LOFS_END_TREE:
assert(obj->type == OBJ_TREE);
return LOFR_ZERO;

case LOFS_BLOB:
assert(obj->type == OBJ_BLOB);
assert((obj->flags & SEEN) == 0);

if (omits)
oidset_insert(omits, &obj->oid);
return LOFR_MARK_SEEN; 
}
}

static void filter_blobs_none__init(
struct list_objects_filter_options *filter_options,
struct filter *filter)
{
filter->filter_object_fn = filter_blobs_none;
filter->free_fn = free;
}





struct filter_trees_depth_data {








struct oidmap seen_at_depth;

unsigned long exclude_depth;
unsigned long current_depth;
};

struct seen_map_entry {
struct oidmap_entry base;
size_t depth;
};


static int filter_trees_update_omits(
struct object *obj,
struct oidset *omits,
int include_it)
{
if (!omits)
return 0;

if (include_it)
return oidset_remove(omits, &obj->oid);
else
return oidset_insert(omits, &obj->oid);
}

static enum list_objects_filter_result filter_trees_depth(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct oidset *omits,
void *filter_data_)
{
struct filter_trees_depth_data *filter_data = filter_data_;
struct seen_map_entry *seen_info;
int include_it = filter_data->current_depth <
filter_data->exclude_depth;
int filter_res;
int already_seen;






switch (filter_situation) {
default:
BUG("unknown filter_situation: %d", filter_situation);

case LOFS_END_TREE:
assert(obj->type == OBJ_TREE);
filter_data->current_depth--;
return LOFR_ZERO;

case LOFS_BLOB:
filter_trees_update_omits(obj, omits, include_it);
return include_it ? LOFR_MARK_SEEN | LOFR_DO_SHOW : LOFR_ZERO;

case LOFS_BEGIN_TREE:
seen_info = oidmap_get(
&filter_data->seen_at_depth, &obj->oid);
if (!seen_info) {
seen_info = xcalloc(1, sizeof(*seen_info));
oidcpy(&seen_info->base.oid, &obj->oid);
seen_info->depth = filter_data->current_depth;
oidmap_put(&filter_data->seen_at_depth, seen_info);
already_seen = 0;
} else {
already_seen =
filter_data->current_depth >= seen_info->depth;
}

if (already_seen) {
filter_res = LOFR_SKIP_TREE;
} else {
int been_omitted = filter_trees_update_omits(
obj, omits, include_it);
seen_info->depth = filter_data->current_depth;

if (include_it)
filter_res = LOFR_DO_SHOW;
else if (omits && !been_omitted)




filter_res = LOFR_ZERO;
else
filter_res = LOFR_SKIP_TREE;
}

filter_data->current_depth++;
return filter_res;
}
}

static void filter_trees_free(void *filter_data) {
struct filter_trees_depth_data *d = filter_data;
if (!d)
return;
oidmap_free(&d->seen_at_depth, 1);
free(d);
}

static void filter_trees_depth__init(
struct list_objects_filter_options *filter_options,
struct filter *filter)
{
struct filter_trees_depth_data *d = xcalloc(1, sizeof(*d));
oidmap_init(&d->seen_at_depth, 0);
d->exclude_depth = filter_options->tree_exclude_depth;
d->current_depth = 0;

filter->filter_data = d;
filter->filter_object_fn = filter_trees_depth;
filter->free_fn = filter_trees_free;
}





struct filter_blobs_limit_data {
unsigned long max_bytes;
};

static enum list_objects_filter_result filter_blobs_limit(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct oidset *omits,
void *filter_data_)
{
struct filter_blobs_limit_data *filter_data = filter_data_;
unsigned long object_length;
enum object_type t;

switch (filter_situation) {
default:
BUG("unknown filter_situation: %d", filter_situation);

case LOFS_BEGIN_TREE:
assert(obj->type == OBJ_TREE);

return LOFR_MARK_SEEN | LOFR_DO_SHOW;

case LOFS_END_TREE:
assert(obj->type == OBJ_TREE);
return LOFR_ZERO;

case LOFS_BLOB:
assert(obj->type == OBJ_BLOB);
assert((obj->flags & SEEN) == 0);

t = oid_object_info(r, &obj->oid, &object_length);
if (t != OBJ_BLOB) { 






goto include_it;
}

if (object_length < filter_data->max_bytes)
goto include_it;

if (omits)
oidset_insert(omits, &obj->oid);
return LOFR_MARK_SEEN; 
}

include_it:
if (omits)
oidset_remove(omits, &obj->oid);
return LOFR_MARK_SEEN | LOFR_DO_SHOW;
}

static void filter_blobs_limit__init(
struct list_objects_filter_options *filter_options,
struct filter *filter)
{
struct filter_blobs_limit_data *d = xcalloc(1, sizeof(*d));
d->max_bytes = filter_options->blob_limit_value;

filter->filter_data = d;
filter->filter_object_fn = filter_blobs_limit;
filter->free_fn = free;
}










struct frame {






enum pattern_match_result default_match;









unsigned child_prov_omit : 1;
};

struct filter_sparse_data {
struct pattern_list pl;

size_t nr, alloc;
struct frame *array_frame;
};

static enum list_objects_filter_result filter_sparse(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct oidset *omits,
void *filter_data_)
{
struct filter_sparse_data *filter_data = filter_data_;
int dtype;
struct frame *frame;
enum pattern_match_result match;

switch (filter_situation) {
default:
BUG("unknown filter_situation: %d", filter_situation);

case LOFS_BEGIN_TREE:
assert(obj->type == OBJ_TREE);
dtype = DT_DIR;
match = path_matches_pattern_list(pathname, strlen(pathname),
filename, &dtype, &filter_data->pl,
r->index);
if (match == UNDECIDED)
match = filter_data->array_frame[filter_data->nr - 1].default_match;

ALLOC_GROW(filter_data->array_frame, filter_data->nr + 1,
filter_data->alloc);
filter_data->array_frame[filter_data->nr].default_match = match;
filter_data->array_frame[filter_data->nr].child_prov_omit = 0;
filter_data->nr++;

















if (obj->flags & FILTER_SHOWN_BUT_REVISIT)
return LOFR_ZERO;
obj->flags |= FILTER_SHOWN_BUT_REVISIT;
return LOFR_DO_SHOW;

case LOFS_END_TREE:
assert(obj->type == OBJ_TREE);
assert(filter_data->nr > 1);

frame = &filter_data->array_frame[--filter_data->nr];





filter_data->array_frame[filter_data->nr - 1].child_prov_omit |=
frame->child_prov_omit;






if (!frame->child_prov_omit)
return LOFR_MARK_SEEN;
return LOFR_ZERO;

case LOFS_BLOB:
assert(obj->type == OBJ_BLOB);
assert((obj->flags & SEEN) == 0);

frame = &filter_data->array_frame[filter_data->nr - 1];

dtype = DT_REG;
match = path_matches_pattern_list(pathname, strlen(pathname),
filename, &dtype, &filter_data->pl,
r->index);
if (match == UNDECIDED)
match = frame->default_match;
if (match == MATCHED) {
if (omits)
oidset_remove(omits, &obj->oid);
return LOFR_MARK_SEEN | LOFR_DO_SHOW;
}











if (omits)
oidset_insert(omits, &obj->oid);






frame->child_prov_omit = 1;
return LOFR_ZERO;
}
}


static void filter_sparse_free(void *filter_data)
{
struct filter_sparse_data *d = filter_data;
free(d->array_frame);
free(d);
}

static void filter_sparse_oid__init(
struct list_objects_filter_options *filter_options,
struct filter *filter)
{
struct filter_sparse_data *d = xcalloc(1, sizeof(*d));
struct object_context oc;
struct object_id sparse_oid;

if (get_oid_with_context(the_repository,
filter_options->sparse_oid_name,
GET_OID_BLOB, &sparse_oid, &oc))
die(_("unable to access sparse blob in '%s'"),
filter_options->sparse_oid_name);
if (add_patterns_from_blob_to_list(&sparse_oid, "", 0, &d->pl) < 0)
die(_("unable to parse sparse filter data in %s"),
oid_to_hex(&sparse_oid));

ALLOC_GROW(d->array_frame, d->nr + 1, d->alloc);
d->array_frame[d->nr].default_match = 0; 
d->array_frame[d->nr].child_prov_omit = 0;
d->nr++;

filter->filter_data = d;
filter->filter_object_fn = filter_sparse;
filter->free_fn = filter_sparse_free;
}


struct combine_filter_data {
struct subfilter *sub;
size_t nr;
};

static enum list_objects_filter_result process_subfilter(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct subfilter *sub)
{
enum list_objects_filter_result result;










if (sub->is_skipping_tree) {
if (filter_situation == LOFS_END_TREE &&
oideq(&obj->oid, &sub->skip_tree))
sub->is_skipping_tree = 0;
else
return LOFR_ZERO;
}
if (oidset_contains(&sub->seen, &obj->oid))
return LOFR_ZERO;

result = list_objects_filter__filter_object(
r, filter_situation, obj, pathname, filename, sub->filter);

if (result & LOFR_MARK_SEEN)
oidset_insert(&sub->seen, &obj->oid);

if (result & LOFR_SKIP_TREE) {
sub->is_skipping_tree = 1;
sub->skip_tree = obj->oid;
}

return result;
}

static enum list_objects_filter_result filter_combine(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct oidset *omits,
void *filter_data)
{
struct combine_filter_data *d = filter_data;
enum list_objects_filter_result combined_result =
LOFR_DO_SHOW | LOFR_MARK_SEEN | LOFR_SKIP_TREE;
size_t sub;

for (sub = 0; sub < d->nr; sub++) {
enum list_objects_filter_result sub_result = process_subfilter(
r, filter_situation, obj, pathname, filename,
&d->sub[sub]);
if (!(sub_result & LOFR_DO_SHOW))
combined_result &= ~LOFR_DO_SHOW;
if (!(sub_result & LOFR_MARK_SEEN))
combined_result &= ~LOFR_MARK_SEEN;
if (!d->sub[sub].is_skipping_tree)
combined_result &= ~LOFR_SKIP_TREE;
}

return combined_result;
}

static void filter_combine__free(void *filter_data)
{
struct combine_filter_data *d = filter_data;
size_t sub;
for (sub = 0; sub < d->nr; sub++) {
list_objects_filter__free(d->sub[sub].filter);
oidset_clear(&d->sub[sub].seen);
if (d->sub[sub].omits.set.size)
BUG("expected oidset to be cleared already");
}
free(d->sub);
}

static void add_all(struct oidset *dest, struct oidset *src) {
struct oidset_iter iter;
struct object_id *src_oid;

oidset_iter_init(src, &iter);
while ((src_oid = oidset_iter_next(&iter)) != NULL)
oidset_insert(dest, src_oid);
}

static void filter_combine__finalize_omits(
struct oidset *omits,
void *filter_data)
{
struct combine_filter_data *d = filter_data;
size_t sub;

for (sub = 0; sub < d->nr; sub++) {
add_all(omits, &d->sub[sub].omits);
oidset_clear(&d->sub[sub].omits);
}
}

static void filter_combine__init(
struct list_objects_filter_options *filter_options,
struct filter* filter)
{
struct combine_filter_data *d = xcalloc(1, sizeof(*d));
size_t sub;

d->nr = filter_options->sub_nr;
d->sub = xcalloc(d->nr, sizeof(*d->sub));
for (sub = 0; sub < d->nr; sub++)
d->sub[sub].filter = list_objects_filter__init(
filter->omits ? &d->sub[sub].omits : NULL,
&filter_options->sub[sub]);

filter->filter_data = d;
filter->filter_object_fn = filter_combine;
filter->free_fn = filter_combine__free;
filter->finalize_omits_fn = filter_combine__finalize_omits;
}

typedef void (*filter_init_fn)(
struct list_objects_filter_options *filter_options,
struct filter *filter);




static filter_init_fn s_filters[] = {
NULL,
filter_blobs_none__init,
filter_blobs_limit__init,
filter_trees_depth__init,
filter_sparse_oid__init,
filter_combine__init,
};

struct filter *list_objects_filter__init(
struct oidset *omitted,
struct list_objects_filter_options *filter_options)
{
struct filter *filter;
filter_init_fn init_fn;

assert((sizeof(s_filters) / sizeof(s_filters[0])) == LOFC__COUNT);

if (filter_options->choice >= LOFC__COUNT)
BUG("invalid list-objects filter choice: %d",
filter_options->choice);

init_fn = s_filters[filter_options->choice];
if (!init_fn)
return NULL;

filter = xcalloc(1, sizeof(*filter));
filter->omits = omitted;
init_fn(filter_options, filter);
return filter;
}

enum list_objects_filter_result list_objects_filter__filter_object(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct filter *filter)
{
if (filter && (obj->flags & NOT_USER_GIVEN))
return filter->filter_object_fn(r, filter_situation, obj,
pathname, filename,
filter->omits,
filter->filter_data);





if (filter_situation == LOFS_END_TREE)
return 0;
return LOFR_MARK_SEEN | LOFR_DO_SHOW;
}

void list_objects_filter__free(struct filter *filter)
{
if (!filter)
return;
if (filter->finalize_omits_fn && filter->omits)
filter->finalize_omits_fn(filter->omits, filter->filter_data);
filter->free_fn(filter->filter_data);
free(filter);
}
