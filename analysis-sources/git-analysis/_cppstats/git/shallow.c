#include "cache.h"
#include "repository.h"
#include "tempfile.h"
#include "lockfile.h"
#include "object-store.h"
#include "commit.h"
#include "tag.h"
#include "pkt-line.h"
#include "remote.h"
#include "refs.h"
#include "sha1-array.h"
#include "diff.h"
#include "revision.h"
#include "commit-slab.h"
#include "list-objects.h"
#include "commit-reach.h"
void set_alternate_shallow_file(struct repository *r, const char *path, int override)
{
if (r->parsed_objects->is_shallow != -1)
BUG("is_repository_shallow must not be called before set_alternate_shallow_file");
if (r->parsed_objects->alternate_shallow_file && !override)
return;
free(r->parsed_objects->alternate_shallow_file);
r->parsed_objects->alternate_shallow_file = xstrdup_or_null(path);
}
int register_shallow(struct repository *r, const struct object_id *oid)
{
struct commit_graft *graft =
xmalloc(sizeof(struct commit_graft));
struct commit *commit = lookup_commit(the_repository, oid);
oidcpy(&graft->oid, oid);
graft->nr_parent = -1;
if (commit && commit->object.parsed)
commit->parents = NULL;
return register_commit_graft(r, graft, 0);
}
int is_repository_shallow(struct repository *r)
{
FILE *fp;
char buf[1024];
const char *path = r->parsed_objects->alternate_shallow_file;
if (r->parsed_objects->is_shallow >= 0)
return r->parsed_objects->is_shallow;
if (!path)
path = git_path_shallow(r);
if (!*path || (fp = fopen(path, "r")) == NULL) {
stat_validity_clear(r->parsed_objects->shallow_stat);
r->parsed_objects->is_shallow = 0;
return r->parsed_objects->is_shallow;
}
stat_validity_update(r->parsed_objects->shallow_stat, fileno(fp));
r->parsed_objects->is_shallow = 1;
while (fgets(buf, sizeof(buf), fp)) {
struct object_id oid;
if (get_oid_hex(buf, &oid))
die("bad shallow line: %s", buf);
register_shallow(r, &oid);
}
fclose(fp);
return r->parsed_objects->is_shallow;
}
define_commit_slab(commit_depth, int *);
struct commit_list *get_shallow_commits(struct object_array *heads, int depth,
int shallow_flag, int not_shallow_flag)
{
int i = 0, cur_depth = 0;
struct commit_list *result = NULL;
struct object_array stack = OBJECT_ARRAY_INIT;
struct commit *commit = NULL;
struct commit_graft *graft;
struct commit_depth depths;
init_commit_depth(&depths);
while (commit || i < heads->nr || stack.nr) {
struct commit_list *p;
if (!commit) {
if (i < heads->nr) {
int **depth_slot;
commit = (struct commit *)
deref_tag(the_repository,
heads->objects[i++].item,
NULL, 0);
if (!commit || commit->object.type != OBJ_COMMIT) {
commit = NULL;
continue;
}
depth_slot = commit_depth_at(&depths, commit);
if (!*depth_slot)
*depth_slot = xmalloc(sizeof(int));
**depth_slot = 0;
cur_depth = 0;
} else {
commit = (struct commit *)
object_array_pop(&stack);
cur_depth = **commit_depth_at(&depths, commit);
}
}
parse_commit_or_die(commit);
cur_depth++;
if ((depth != INFINITE_DEPTH && cur_depth >= depth) ||
(is_repository_shallow(the_repository) && !commit->parents &&
(graft = lookup_commit_graft(the_repository, &commit->object.oid)) != NULL &&
graft->nr_parent < 0)) {
commit_list_insert(commit, &result);
commit->object.flags |= shallow_flag;
commit = NULL;
continue;
}
commit->object.flags |= not_shallow_flag;
for (p = commit->parents, commit = NULL; p; p = p->next) {
int **depth_slot = commit_depth_at(&depths, p->item);
if (!*depth_slot) {
*depth_slot = xmalloc(sizeof(int));
**depth_slot = cur_depth;
} else {
if (cur_depth >= **depth_slot)
continue;
**depth_slot = cur_depth;
}
if (p->next)
add_object_array(&p->item->object,
NULL, &stack);
else {
commit = p->item;
cur_depth = **commit_depth_at(&depths, commit);
}
}
}
for (i = 0; i < depths.slab_count; i++) {
int j;
if (!depths.slab[i])
continue;
for (j = 0; j < depths.slab_size; j++)
free(depths.slab[i][j]);
}
clear_commit_depth(&depths);
return result;
}
static void show_commit(struct commit *commit, void *data)
{
commit_list_insert(commit, data);
}
struct commit_list *get_shallow_commits_by_rev_list(int ac, const char **av,
int shallow_flag,
int not_shallow_flag)
{
struct commit_list *result = NULL, *p;
struct commit_list *not_shallow_list = NULL;
struct rev_info revs;
int both_flags = shallow_flag | not_shallow_flag;
clear_object_flags(both_flags);
is_repository_shallow(the_repository); 
repo_init_revisions(the_repository, &revs, NULL);
save_commit_buffer = 0;
setup_revisions(ac, av, &revs, NULL);
if (prepare_revision_walk(&revs))
die("revision walk setup failed");
traverse_commit_list(&revs, show_commit, NULL, &not_shallow_list);
if (!not_shallow_list)
die("no commits selected for shallow requests");
for (p = not_shallow_list; p; p = p->next)
p->item->object.flags |= not_shallow_flag;
for (p = not_shallow_list; p; p = p->next) {
struct commit *c = p->item;
struct commit_list *parent;
if (parse_commit(c))
die("unable to parse commit %s",
oid_to_hex(&c->object.oid));
for (parent = c->parents; parent; parent = parent->next)
if (!(parent->item->object.flags & not_shallow_flag)) {
c->object.flags |= shallow_flag;
commit_list_insert(c, &result);
break;
}
}
free_commit_list(not_shallow_list);
for (p = result; p; p = p->next) {
struct object *o = &p->item->object;
if ((o->flags & both_flags) == both_flags)
o->flags &= ~not_shallow_flag;
}
return result;
}
static void check_shallow_file_for_update(struct repository *r)
{
if (r->parsed_objects->is_shallow == -1)
BUG("shallow must be initialized by now");
if (!stat_validity_check(r->parsed_objects->shallow_stat,
git_path_shallow(r)))
die("shallow file has changed since we read it");
}
#define SEEN_ONLY 1
#define VERBOSE 2
#define QUICK 4
struct write_shallow_data {
struct strbuf *out;
int use_pack_protocol;
int count;
unsigned flags;
};
static int write_one_shallow(const struct commit_graft *graft, void *cb_data)
{
struct write_shallow_data *data = cb_data;
const char *hex = oid_to_hex(&graft->oid);
if (graft->nr_parent != -1)
return 0;
if (data->flags & QUICK) {
if (!has_object_file(&graft->oid))
return 0;
} else if (data->flags & SEEN_ONLY) {
struct commit *c = lookup_commit(the_repository, &graft->oid);
if (!c || !(c->object.flags & SEEN)) {
if (data->flags & VERBOSE)
printf("Removing %s from .git/shallow\n",
oid_to_hex(&c->object.oid));
return 0;
}
}
data->count++;
if (data->use_pack_protocol)
packet_buf_write(data->out, "shallow %s", hex);
else {
strbuf_addstr(data->out, hex);
strbuf_addch(data->out, '\n');
}
return 0;
}
static int write_shallow_commits_1(struct strbuf *out, int use_pack_protocol,
const struct oid_array *extra,
unsigned flags)
{
struct write_shallow_data data;
int i;
data.out = out;
data.use_pack_protocol = use_pack_protocol;
data.count = 0;
data.flags = flags;
for_each_commit_graft(write_one_shallow, &data);
if (!extra)
return data.count;
for (i = 0; i < extra->nr; i++) {
strbuf_addstr(out, oid_to_hex(extra->oid + i));
strbuf_addch(out, '\n');
data.count++;
}
return data.count;
}
int write_shallow_commits(struct strbuf *out, int use_pack_protocol,
const struct oid_array *extra)
{
return write_shallow_commits_1(out, use_pack_protocol, extra, 0);
}
const char *setup_temporary_shallow(const struct oid_array *extra)
{
struct tempfile *temp;
struct strbuf sb = STRBUF_INIT;
if (write_shallow_commits(&sb, 0, extra)) {
temp = xmks_tempfile(git_path("shallow_XXXXXX"));
if (write_in_full(temp->fd, sb.buf, sb.len) < 0 ||
close_tempfile_gently(temp) < 0)
die_errno("failed to write to %s",
get_tempfile_path(temp));
strbuf_release(&sb);
return get_tempfile_path(temp);
}
return "";
}
void setup_alternate_shallow(struct lock_file *shallow_lock,
const char **alternate_shallow_file,
const struct oid_array *extra)
{
struct strbuf sb = STRBUF_INIT;
int fd;
fd = hold_lock_file_for_update(shallow_lock,
git_path_shallow(the_repository),
LOCK_DIE_ON_ERROR);
check_shallow_file_for_update(the_repository);
if (write_shallow_commits(&sb, 0, extra)) {
if (write_in_full(fd, sb.buf, sb.len) < 0)
die_errno("failed to write to %s",
get_lock_file_path(shallow_lock));
*alternate_shallow_file = get_lock_file_path(shallow_lock);
} else
*alternate_shallow_file = "";
strbuf_release(&sb);
}
static int advertise_shallow_grafts_cb(const struct commit_graft *graft, void *cb)
{
int fd = *(int *)cb;
if (graft->nr_parent == -1)
packet_write_fmt(fd, "shallow %s\n", oid_to_hex(&graft->oid));
return 0;
}
void advertise_shallow_grafts(int fd)
{
if (!is_repository_shallow(the_repository))
return;
for_each_commit_graft(advertise_shallow_grafts_cb, &fd);
}
void prune_shallow(unsigned options)
{
struct lock_file shallow_lock = LOCK_INIT;
struct strbuf sb = STRBUF_INIT;
unsigned flags = SEEN_ONLY;
int fd;
if (options & PRUNE_QUICK)
flags |= QUICK;
if (options & PRUNE_SHOW_ONLY) {
flags |= VERBOSE;
write_shallow_commits_1(&sb, 0, NULL, flags);
strbuf_release(&sb);
return;
}
fd = hold_lock_file_for_update(&shallow_lock,
git_path_shallow(the_repository),
LOCK_DIE_ON_ERROR);
check_shallow_file_for_update(the_repository);
if (write_shallow_commits_1(&sb, 0, NULL, flags)) {
if (write_in_full(fd, sb.buf, sb.len) < 0)
die_errno("failed to write to %s",
get_lock_file_path(&shallow_lock));
commit_lock_file(&shallow_lock);
} else {
unlink(git_path_shallow(the_repository));
rollback_lock_file(&shallow_lock);
}
strbuf_release(&sb);
}
struct trace_key trace_shallow = TRACE_KEY_INIT(SHALLOW);
void prepare_shallow_info(struct shallow_info *info, struct oid_array *sa)
{
int i;
trace_printf_key(&trace_shallow, "shallow: prepare_shallow_info\n");
memset(info, 0, sizeof(*info));
info->shallow = sa;
if (!sa)
return;
ALLOC_ARRAY(info->ours, sa->nr);
ALLOC_ARRAY(info->theirs, sa->nr);
for (i = 0; i < sa->nr; i++) {
if (has_object_file(sa->oid + i)) {
struct commit_graft *graft;
graft = lookup_commit_graft(the_repository,
&sa->oid[i]);
if (graft && graft->nr_parent < 0)
continue;
info->ours[info->nr_ours++] = i;
} else
info->theirs[info->nr_theirs++] = i;
}
}
void clear_shallow_info(struct shallow_info *info)
{
free(info->ours);
free(info->theirs);
}
void remove_nonexistent_theirs_shallow(struct shallow_info *info)
{
struct object_id *oid = info->shallow->oid;
int i, dst;
trace_printf_key(&trace_shallow, "shallow: remove_nonexistent_theirs_shallow\n");
for (i = dst = 0; i < info->nr_theirs; i++) {
if (i != dst)
info->theirs[dst] = info->theirs[i];
if (has_object_file(oid + info->theirs[i]))
dst++;
}
info->nr_theirs = dst;
}
define_commit_slab(ref_bitmap, uint32_t *);
#define POOL_SIZE (512 * 1024)
struct paint_info {
struct ref_bitmap ref_bitmap;
unsigned nr_bits;
char **pools;
char *free, *end;
unsigned pool_count;
};
static uint32_t *paint_alloc(struct paint_info *info)
{
unsigned nr = DIV_ROUND_UP(info->nr_bits, 32);
unsigned size = nr * sizeof(uint32_t);
void *p;
if (!info->pool_count || size > info->end - info->free) {
if (size > POOL_SIZE)
BUG("pool size too small for %d in paint_alloc()",
size);
info->pool_count++;
REALLOC_ARRAY(info->pools, info->pool_count);
info->free = xmalloc(POOL_SIZE);
info->pools[info->pool_count - 1] = info->free;
info->end = info->free + POOL_SIZE;
}
p = info->free;
info->free += size;
return p;
}
static void paint_down(struct paint_info *info, const struct object_id *oid,
unsigned int id)
{
unsigned int i, nr;
struct commit_list *head = NULL;
int bitmap_nr = DIV_ROUND_UP(info->nr_bits, 32);
size_t bitmap_size = st_mult(sizeof(uint32_t), bitmap_nr);
struct commit *c = lookup_commit_reference_gently(the_repository, oid,
1);
uint32_t *tmp; 
uint32_t *bitmap;
if (!c)
return;
tmp = xmalloc(bitmap_size);
bitmap = paint_alloc(info);
memset(bitmap, 0, bitmap_size);
bitmap[id / 32] |= (1U << (id % 32));
commit_list_insert(c, &head);
while (head) {
struct commit_list *p;
struct commit *c = pop_commit(&head);
uint32_t **refs = ref_bitmap_at(&info->ref_bitmap, c);
if (c->object.flags & (SEEN | UNINTERESTING))
continue;
else
c->object.flags |= SEEN;
if (*refs == NULL)
*refs = bitmap;
else {
memcpy(tmp, *refs, bitmap_size);
for (i = 0; i < bitmap_nr; i++)
tmp[i] |= bitmap[i];
if (memcmp(tmp, *refs, bitmap_size)) {
*refs = paint_alloc(info);
memcpy(*refs, tmp, bitmap_size);
}
}
if (c->object.flags & BOTTOM)
continue;
if (parse_commit(c))
die("unable to parse commit %s",
oid_to_hex(&c->object.oid));
for (p = c->parents; p; p = p->next) {
if (p->item->object.flags & SEEN)
continue;
commit_list_insert(p->item, &head);
}
}
nr = get_max_object_index();
for (i = 0; i < nr; i++) {
struct object *o = get_indexed_object(i);
if (o && o->type == OBJ_COMMIT)
o->flags &= ~SEEN;
}
free(tmp);
}
static int mark_uninteresting(const char *refname, const struct object_id *oid,
int flags, void *cb_data)
{
struct commit *commit = lookup_commit_reference_gently(the_repository,
oid, 1);
if (!commit)
return 0;
commit->object.flags |= UNINTERESTING;
mark_parents_uninteresting(commit);
return 0;
}
static void post_assign_shallow(struct shallow_info *info,
struct ref_bitmap *ref_bitmap,
int *ref_status);
void assign_shallow_commits_to_refs(struct shallow_info *info,
uint32_t **used, int *ref_status)
{
struct object_id *oid = info->shallow->oid;
struct oid_array *ref = info->ref;
unsigned int i, nr;
int *shallow, nr_shallow = 0;
struct paint_info pi;
trace_printf_key(&trace_shallow, "shallow: assign_shallow_commits_to_refs\n");
ALLOC_ARRAY(shallow, info->nr_ours + info->nr_theirs);
for (i = 0; i < info->nr_ours; i++)
shallow[nr_shallow++] = info->ours[i];
for (i = 0; i < info->nr_theirs; i++)
shallow[nr_shallow++] = info->theirs[i];
nr = get_max_object_index();
for (i = 0; i < nr; i++) {
struct object *o = get_indexed_object(i);
if (!o || o->type != OBJ_COMMIT)
continue;
o->flags &= ~(UNINTERESTING | BOTTOM | SEEN);
}
memset(&pi, 0, sizeof(pi));
init_ref_bitmap(&pi.ref_bitmap);
pi.nr_bits = ref->nr;
head_ref(mark_uninteresting, NULL);
for_each_ref(mark_uninteresting, NULL);
for (i = 0; i < nr_shallow; i++) {
struct commit *c = lookup_commit(the_repository,
&oid[shallow[i]]);
c->object.flags |= BOTTOM;
}
for (i = 0; i < ref->nr; i++)
paint_down(&pi, ref->oid + i, i);
if (used) {
int bitmap_size = DIV_ROUND_UP(pi.nr_bits, 32) * sizeof(uint32_t);
memset(used, 0, sizeof(*used) * info->shallow->nr);
for (i = 0; i < nr_shallow; i++) {
const struct commit *c = lookup_commit(the_repository,
&oid[shallow[i]]);
uint32_t **map = ref_bitmap_at(&pi.ref_bitmap, c);
if (*map)
used[shallow[i]] = xmemdupz(*map, bitmap_size);
}
} else
post_assign_shallow(info, &pi.ref_bitmap, ref_status);
clear_ref_bitmap(&pi.ref_bitmap);
for (i = 0; i < pi.pool_count; i++)
free(pi.pools[i]);
free(pi.pools);
free(shallow);
}
struct commit_array {
struct commit **commits;
int nr, alloc;
};
static int add_ref(const char *refname, const struct object_id *oid,
int flags, void *cb_data)
{
struct commit_array *ca = cb_data;
ALLOC_GROW(ca->commits, ca->nr + 1, ca->alloc);
ca->commits[ca->nr] = lookup_commit_reference_gently(the_repository,
oid, 1);
if (ca->commits[ca->nr])
ca->nr++;
return 0;
}
static void update_refstatus(int *ref_status, int nr, uint32_t *bitmap)
{
unsigned int i;
if (!ref_status)
return;
for (i = 0; i < nr; i++)
if (bitmap[i / 32] & (1U << (i % 32)))
ref_status[i]++;
}
static void post_assign_shallow(struct shallow_info *info,
struct ref_bitmap *ref_bitmap,
int *ref_status)
{
struct object_id *oid = info->shallow->oid;
struct commit *c;
uint32_t **bitmap;
int dst, i, j;
int bitmap_nr = DIV_ROUND_UP(info->ref->nr, 32);
struct commit_array ca;
trace_printf_key(&trace_shallow, "shallow: post_assign_shallow\n");
if (ref_status)
memset(ref_status, 0, sizeof(*ref_status) * info->ref->nr);
for (i = dst = 0; i < info->nr_theirs; i++) {
if (i != dst)
info->theirs[dst] = info->theirs[i];
c = lookup_commit(the_repository, &oid[info->theirs[i]]);
bitmap = ref_bitmap_at(ref_bitmap, c);
if (!*bitmap)
continue;
for (j = 0; j < bitmap_nr; j++)
if (bitmap[0][j]) {
update_refstatus(ref_status, info->ref->nr, *bitmap);
dst++;
break;
}
}
info->nr_theirs = dst;
memset(&ca, 0, sizeof(ca));
head_ref(add_ref, &ca);
for_each_ref(add_ref, &ca);
for (i = dst = 0; i < info->nr_ours; i++) {
if (i != dst)
info->ours[dst] = info->ours[i];
c = lookup_commit(the_repository, &oid[info->ours[i]]);
bitmap = ref_bitmap_at(ref_bitmap, c);
if (!*bitmap)
continue;
for (j = 0; j < bitmap_nr; j++)
if (bitmap[0][j] &&
!in_merge_bases_many(c, ca.nr, ca.commits)) {
update_refstatus(ref_status, info->ref->nr, *bitmap);
dst++;
break;
}
}
info->nr_ours = dst;
free(ca.commits);
}
int delayed_reachability_test(struct shallow_info *si, int c)
{
if (si->need_reachability_test[c]) {
struct commit *commit = lookup_commit(the_repository,
&si->shallow->oid[c]);
if (!si->commits) {
struct commit_array ca;
memset(&ca, 0, sizeof(ca));
head_ref(add_ref, &ca);
for_each_ref(add_ref, &ca);
si->commits = ca.commits;
si->nr_commits = ca.nr;
}
si->reachable[c] = in_merge_bases_many(commit,
si->nr_commits,
si->commits);
si->need_reachability_test[c] = 0;
}
return si->reachable[c];
}
