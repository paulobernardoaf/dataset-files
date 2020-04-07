#include "cache.h"
#include "argv-array.h"
#include "repository.h"
#include "config.h"
#include "dir.h"
#include "tree.h"
#include "tree-walk.h"
#include "cache-tree.h"
#include "unpack-trees.h"
#include "progress.h"
#include "refs.h"
#include "attr.h"
#include "split-index.h"
#include "submodule.h"
#include "submodule-config.h"
#include "fsmonitor.h"
#include "object-store.h"
#include "promisor-remote.h"
static const char *unpack_plumbing_errors[NB_UNPACK_TREES_ERROR_TYPES] = {
"Entry '%s' would be overwritten by merge. Cannot merge.",
"Entry '%s' not uptodate. Cannot merge.",
"Updating '%s' would lose untracked files in it",
"Untracked working tree file '%s' would be overwritten by merge.",
"Untracked working tree file '%s' would be removed by merge.",
"Entry '%s' overlaps with '%s'. Cannot bind.",
"Entry '%s' not uptodate. Cannot update sparse checkout.",
"Working tree file '%s' would be overwritten by sparse checkout update.",
"Working tree file '%s' would be removed by sparse checkout update.",
"Submodule '%s' cannot checkout new HEAD.",
};
#define ERRORMSG(o,type) ( ((o) && (o)->msgs[(type)]) ? ((o)->msgs[(type)]) : (unpack_plumbing_errors[(type)]) )
static const char *super_prefixed(const char *path)
{
static struct strbuf buf[2] = {STRBUF_INIT, STRBUF_INIT};
static int super_prefix_len = -1;
static unsigned idx = ARRAY_SIZE(buf) - 1;
if (super_prefix_len < 0) {
const char *super_prefix = get_super_prefix();
if (!super_prefix) {
super_prefix_len = 0;
} else {
int i;
for (i = 0; i < ARRAY_SIZE(buf); i++)
strbuf_addstr(&buf[i], super_prefix);
super_prefix_len = buf[0].len;
}
}
if (!super_prefix_len)
return path;
if (++idx >= ARRAY_SIZE(buf))
idx = 0;
strbuf_setlen(&buf[idx], super_prefix_len);
strbuf_addstr(&buf[idx], path);
return buf[idx].buf;
}
void setup_unpack_trees_porcelain(struct unpack_trees_options *opts,
const char *cmd)
{
int i;
const char **msgs = opts->msgs;
const char *msg;
argv_array_init(&opts->msgs_to_free);
if (!strcmp(cmd, "checkout"))
msg = advice_commit_before_merge
? _("Your local changes to the following files would be overwritten by checkout:\n%%s"
"Please commit your changes or stash them before you switch branches.")
: _("Your local changes to the following files would be overwritten by checkout:\n%%s");
else if (!strcmp(cmd, "merge"))
msg = advice_commit_before_merge
? _("Your local changes to the following files would be overwritten by merge:\n%%s"
"Please commit your changes or stash them before you merge.")
: _("Your local changes to the following files would be overwritten by merge:\n%%s");
else
msg = advice_commit_before_merge
? _("Your local changes to the following files would be overwritten by %s:\n%%s"
"Please commit your changes or stash them before you %s.")
: _("Your local changes to the following files would be overwritten by %s:\n%%s");
msgs[ERROR_WOULD_OVERWRITE] = msgs[ERROR_NOT_UPTODATE_FILE] =
argv_array_pushf(&opts->msgs_to_free, msg, cmd, cmd);
msgs[ERROR_NOT_UPTODATE_DIR] =
_("Updating the following directories would lose untracked files in them:\n%s");
if (!strcmp(cmd, "checkout"))
msg = advice_commit_before_merge
? _("The following untracked working tree files would be removed by checkout:\n%%s"
"Please move or remove them before you switch branches.")
: _("The following untracked working tree files would be removed by checkout:\n%%s");
else if (!strcmp(cmd, "merge"))
msg = advice_commit_before_merge
? _("The following untracked working tree files would be removed by merge:\n%%s"
"Please move or remove them before you merge.")
: _("The following untracked working tree files would be removed by merge:\n%%s");
else
msg = advice_commit_before_merge
? _("The following untracked working tree files would be removed by %s:\n%%s"
"Please move or remove them before you %s.")
: _("The following untracked working tree files would be removed by %s:\n%%s");
msgs[ERROR_WOULD_LOSE_UNTRACKED_REMOVED] =
argv_array_pushf(&opts->msgs_to_free, msg, cmd, cmd);
if (!strcmp(cmd, "checkout"))
msg = advice_commit_before_merge
? _("The following untracked working tree files would be overwritten by checkout:\n%%s"
"Please move or remove them before you switch branches.")
: _("The following untracked working tree files would be overwritten by checkout:\n%%s");
else if (!strcmp(cmd, "merge"))
msg = advice_commit_before_merge
? _("The following untracked working tree files would be overwritten by merge:\n%%s"
"Please move or remove them before you merge.")
: _("The following untracked working tree files would be overwritten by merge:\n%%s");
else
msg = advice_commit_before_merge
? _("The following untracked working tree files would be overwritten by %s:\n%%s"
"Please move or remove them before you %s.")
: _("The following untracked working tree files would be overwritten by %s:\n%%s");
msgs[ERROR_WOULD_LOSE_UNTRACKED_OVERWRITTEN] =
argv_array_pushf(&opts->msgs_to_free, msg, cmd, cmd);
msgs[ERROR_BIND_OVERLAP] = _("Entry '%s' overlaps with '%s'. Cannot bind.");
msgs[ERROR_SPARSE_NOT_UPTODATE_FILE] =
_("Cannot update sparse checkout: the following entries are not up to date:\n%s");
msgs[ERROR_WOULD_LOSE_ORPHANED_OVERWRITTEN] =
_("The following working tree files would be overwritten by sparse checkout update:\n%s");
msgs[ERROR_WOULD_LOSE_ORPHANED_REMOVED] =
_("The following working tree files would be removed by sparse checkout update:\n%s");
msgs[ERROR_WOULD_LOSE_SUBMODULE] =
_("Cannot update submodule:\n%s");
opts->show_all_errors = 1;
for (i = 0; i < ARRAY_SIZE(opts->unpack_rejects); i++)
opts->unpack_rejects[i].strdup_strings = 1;
}
void clear_unpack_trees_porcelain(struct unpack_trees_options *opts)
{
argv_array_clear(&opts->msgs_to_free);
memset(opts->msgs, 0, sizeof(opts->msgs));
}
static int do_add_entry(struct unpack_trees_options *o, struct cache_entry *ce,
unsigned int set, unsigned int clear)
{
clear |= CE_HASHED;
if (set & CE_REMOVE)
set |= CE_WT_REMOVE;
ce->ce_flags = (ce->ce_flags & ~clear) | set;
return add_index_entry(&o->result, ce,
ADD_CACHE_OK_TO_ADD | ADD_CACHE_OK_TO_REPLACE);
}
static void add_entry(struct unpack_trees_options *o,
const struct cache_entry *ce,
unsigned int set, unsigned int clear)
{
do_add_entry(o, dup_cache_entry(ce, &o->result), set, clear);
}
static int add_rejected_path(struct unpack_trees_options *o,
enum unpack_trees_error_types e,
const char *path)
{
if (o->quiet)
return -1;
if (!o->show_all_errors)
return error(ERRORMSG(o, e), super_prefixed(path));
string_list_append(&o->unpack_rejects[e], path);
return -1;
}
static void display_error_msgs(struct unpack_trees_options *o)
{
int e, i;
int something_displayed = 0;
for (e = 0; e < NB_UNPACK_TREES_ERROR_TYPES; e++) {
struct string_list *rejects = &o->unpack_rejects[e];
if (rejects->nr > 0) {
struct strbuf path = STRBUF_INIT;
something_displayed = 1;
for (i = 0; i < rejects->nr; i++)
strbuf_addf(&path, "\t%s\n", rejects->items[i].string);
error(ERRORMSG(o, e), super_prefixed(path.buf));
strbuf_release(&path);
}
string_list_clear(rejects, 0);
}
if (something_displayed)
fprintf(stderr, _("Aborting\n"));
}
static int check_submodule_move_head(const struct cache_entry *ce,
const char *old_id,
const char *new_id,
struct unpack_trees_options *o)
{
unsigned flags = SUBMODULE_MOVE_HEAD_DRY_RUN;
const struct submodule *sub = submodule_from_ce(ce);
if (!sub)
return 0;
if (o->reset)
flags |= SUBMODULE_MOVE_HEAD_FORCE;
if (submodule_move_head(ce->name, old_id, new_id, flags))
return add_rejected_path(o, ERROR_WOULD_LOSE_SUBMODULE, ce->name);
return 0;
}
static void load_gitmodules_file(struct index_state *index,
struct checkout *state)
{
int pos = index_name_pos(index, GITMODULES_FILE, strlen(GITMODULES_FILE));
if (pos >= 0) {
struct cache_entry *ce = index->cache[pos];
if (!state && ce->ce_flags & CE_WT_REMOVE) {
repo_read_gitmodules(the_repository, 0);
} else if (state && (ce->ce_flags & CE_UPDATE)) {
submodule_free(the_repository);
checkout_entry(ce, state, NULL, NULL);
repo_read_gitmodules(the_repository, 0);
}
}
}
static struct progress *get_progress(struct unpack_trees_options *o)
{
unsigned cnt = 0, total = 0;
struct index_state *index = &o->result;
if (!o->update || !o->verbose_update)
return NULL;
for (; cnt < index->cache_nr; cnt++) {
const struct cache_entry *ce = index->cache[cnt];
if (ce->ce_flags & (CE_UPDATE | CE_WT_REMOVE))
total++;
}
return start_delayed_progress(_("Updating files"), total);
}
static void setup_collided_checkout_detection(struct checkout *state,
struct index_state *index)
{
int i;
state->clone = 1;
for (i = 0; i < index->cache_nr; i++)
index->cache[i]->ce_flags &= ~CE_MATCHED;
}
static void report_collided_checkout(struct index_state *index)
{
struct string_list list = STRING_LIST_INIT_NODUP;
int i;
for (i = 0; i < index->cache_nr; i++) {
struct cache_entry *ce = index->cache[i];
if (!(ce->ce_flags & CE_MATCHED))
continue;
string_list_append(&list, ce->name);
ce->ce_flags &= ~CE_MATCHED;
}
list.cmp = fspathcmp;
string_list_sort(&list);
if (list.nr) {
warning(_("the following paths have collided (e.g. case-sensitive paths\n"
"on a case-insensitive filesystem) and only one from the same\n"
"colliding group is in the working tree:\n"));
for (i = 0; i < list.nr; i++)
fprintf(stderr, " '%s'\n", list.items[i].string);
}
string_list_clear(&list, 0);
}
static int check_updates(struct unpack_trees_options *o)
{
unsigned cnt = 0;
int errs = 0;
struct progress *progress;
struct index_state *index = &o->result;
struct checkout state = CHECKOUT_INIT;
int i;
trace_performance_enter();
state.force = 1;
state.quiet = 1;
state.refresh_cache = 1;
state.istate = index;
clone_checkout_metadata(&state.meta, &o->meta, NULL);
if (!o->update || o->dry_run) {
remove_marked_cache_entries(index, 0);
trace_performance_leave("check_updates");
return 0;
}
if (o->clone)
setup_collided_checkout_detection(&state, index);
progress = get_progress(o);
git_attr_set_direction(GIT_ATTR_CHECKOUT);
if (should_update_submodules())
load_gitmodules_file(index, NULL);
for (i = 0; i < index->cache_nr; i++) {
const struct cache_entry *ce = index->cache[i];
if (ce->ce_flags & CE_WT_REMOVE) {
display_progress(progress, ++cnt);
unlink_entry(ce);
}
}
remove_marked_cache_entries(index, 0);
remove_scheduled_dirs();
if (should_update_submodules())
load_gitmodules_file(index, &state);
enable_delayed_checkout(&state);
if (has_promisor_remote()) {
struct oid_array to_fetch = OID_ARRAY_INIT;
for (i = 0; i < index->cache_nr; i++) {
struct cache_entry *ce = index->cache[i];
if (!(ce->ce_flags & CE_UPDATE) ||
S_ISGITLINK(ce->ce_mode))
continue;
if (!oid_object_info_extended(the_repository, &ce->oid,
NULL,
OBJECT_INFO_FOR_PREFETCH))
continue;
oid_array_append(&to_fetch, &ce->oid);
}
if (to_fetch.nr)
promisor_remote_get_direct(the_repository,
to_fetch.oid, to_fetch.nr);
oid_array_clear(&to_fetch);
}
for (i = 0; i < index->cache_nr; i++) {
struct cache_entry *ce = index->cache[i];
if (ce->ce_flags & CE_UPDATE) {
if (ce->ce_flags & CE_WT_REMOVE)
BUG("both update and delete flags are set on %s",
ce->name);
display_progress(progress, ++cnt);
ce->ce_flags &= ~CE_UPDATE;
errs |= checkout_entry(ce, &state, NULL, NULL);
}
}
stop_progress(&progress);
errs |= finish_delayed_checkout(&state, NULL);
git_attr_set_direction(GIT_ATTR_CHECKIN);
if (o->clone)
report_collided_checkout(index);
trace_performance_leave("check_updates");
return errs != 0;
}
static int verify_uptodate_sparse(const struct cache_entry *ce,
struct unpack_trees_options *o);
static int verify_absent_sparse(const struct cache_entry *ce,
enum unpack_trees_error_types,
struct unpack_trees_options *o);
static int apply_sparse_checkout(struct index_state *istate,
struct cache_entry *ce,
struct unpack_trees_options *o)
{
int was_skip_worktree = ce_skip_worktree(ce);
if (ce->ce_flags & CE_NEW_SKIP_WORKTREE)
ce->ce_flags |= CE_SKIP_WORKTREE;
else
ce->ce_flags &= ~CE_SKIP_WORKTREE;
if (was_skip_worktree != ce_skip_worktree(ce)) {
ce->ce_flags |= CE_UPDATE_IN_BASE;
mark_fsmonitor_invalid(istate, ce);
istate->cache_changed |= CE_ENTRY_CHANGED;
}
if (was_skip_worktree && ce_skip_worktree(ce)) {
ce->ce_flags &= ~CE_UPDATE;
if (ce->ce_flags & CE_REMOVE)
ce->ce_flags &= ~CE_WT_REMOVE;
}
if (!was_skip_worktree && ce_skip_worktree(ce)) {
if (!(ce->ce_flags & CE_UPDATE) && verify_uptodate_sparse(ce, o))
return -1;
ce->ce_flags |= CE_WT_REMOVE;
ce->ce_flags &= ~CE_UPDATE;
}
if (was_skip_worktree && !ce_skip_worktree(ce)) {
if (verify_absent_sparse(ce, ERROR_WOULD_LOSE_UNTRACKED_OVERWRITTEN, o))
return -1;
ce->ce_flags |= CE_UPDATE;
}
return 0;
}
static inline int call_unpack_fn(const struct cache_entry * const *src,
struct unpack_trees_options *o)
{
int ret = o->fn(src, o);
if (ret > 0)
ret = 0;
return ret;
}
static void mark_ce_used(struct cache_entry *ce, struct unpack_trees_options *o)
{
ce->ce_flags |= CE_UNPACKED;
if (o->cache_bottom < o->src_index->cache_nr &&
o->src_index->cache[o->cache_bottom] == ce) {
int bottom = o->cache_bottom;
while (bottom < o->src_index->cache_nr &&
o->src_index->cache[bottom]->ce_flags & CE_UNPACKED)
bottom++;
o->cache_bottom = bottom;
}
}
static void mark_all_ce_unused(struct index_state *index)
{
int i;
for (i = 0; i < index->cache_nr; i++)
index->cache[i]->ce_flags &= ~(CE_UNPACKED | CE_ADDED | CE_NEW_SKIP_WORKTREE);
}
static int locate_in_src_index(const struct cache_entry *ce,
struct unpack_trees_options *o)
{
struct index_state *index = o->src_index;
int len = ce_namelen(ce);
int pos = index_name_pos(index, ce->name, len);
if (pos < 0)
pos = -1 - pos;
return pos;
}
static void mark_ce_used_same_name(struct cache_entry *ce,
struct unpack_trees_options *o)
{
struct index_state *index = o->src_index;
int len = ce_namelen(ce);
int pos;
for (pos = locate_in_src_index(ce, o); pos < index->cache_nr; pos++) {
struct cache_entry *next = index->cache[pos];
if (len != ce_namelen(next) ||
memcmp(ce->name, next->name, len))
break;
mark_ce_used(next, o);
}
}
static struct cache_entry *next_cache_entry(struct unpack_trees_options *o)
{
const struct index_state *index = o->src_index;
int pos = o->cache_bottom;
while (pos < index->cache_nr) {
struct cache_entry *ce = index->cache[pos];
if (!(ce->ce_flags & CE_UNPACKED))
return ce;
pos++;
}
return NULL;
}
static void add_same_unmerged(const struct cache_entry *ce,
struct unpack_trees_options *o)
{
struct index_state *index = o->src_index;
int len = ce_namelen(ce);
int pos = index_name_pos(index, ce->name, len);
if (0 <= pos)
die("programming error in a caller of mark_ce_used_same_name");
for (pos = -pos - 1; pos < index->cache_nr; pos++) {
struct cache_entry *next = index->cache[pos];
if (len != ce_namelen(next) ||
memcmp(ce->name, next->name, len))
break;
add_entry(o, next, 0, 0);
mark_ce_used(next, o);
}
}
static int unpack_index_entry(struct cache_entry *ce,
struct unpack_trees_options *o)
{
const struct cache_entry *src[MAX_UNPACK_TREES + 1] = { NULL, };
int ret;
src[0] = ce;
mark_ce_used(ce, o);
if (ce_stage(ce)) {
if (o->skip_unmerged) {
add_entry(o, ce, 0, 0);
return 0;
}
}
ret = call_unpack_fn(src, o);
if (ce_stage(ce))
mark_ce_used_same_name(ce, o);
return ret;
}
static int find_cache_pos(struct traverse_info *, const char *p, size_t len);
static void restore_cache_bottom(struct traverse_info *info, int bottom)
{
struct unpack_trees_options *o = info->data;
if (o->diff_index_cached)
return;
o->cache_bottom = bottom;
}
static int switch_cache_bottom(struct traverse_info *info)
{
struct unpack_trees_options *o = info->data;
int ret, pos;
if (o->diff_index_cached)
return 0;
ret = o->cache_bottom;
pos = find_cache_pos(info->prev, info->name, info->namelen);
if (pos < -1)
o->cache_bottom = -2 - pos;
else if (pos < 0)
o->cache_bottom = o->src_index->cache_nr;
return ret;
}
static inline int are_same_oid(struct name_entry *name_j, struct name_entry *name_k)
{
return !is_null_oid(&name_j->oid) && !is_null_oid(&name_k->oid) && oideq(&name_j->oid, &name_k->oid);
}
static int all_trees_same_as_cache_tree(int n, unsigned long dirmask,
struct name_entry *names,
struct traverse_info *info)
{
struct unpack_trees_options *o = info->data;
int i;
if (!o->merge || dirmask != ((1 << n) - 1))
return 0;
for (i = 1; i < n; i++)
if (!are_same_oid(names, names + i))
return 0;
return cache_tree_matches_traversal(o->src_index->cache_tree, names, info);
}
static int index_pos_by_traverse_info(struct name_entry *names,
struct traverse_info *info)
{
struct unpack_trees_options *o = info->data;
struct strbuf name = STRBUF_INIT;
int pos;
strbuf_make_traverse_path(&name, info, names->path, names->pathlen);
strbuf_addch(&name, '/');
pos = index_name_pos(o->src_index, name.buf, name.len);
if (pos >= 0)
BUG("This is a directory and should not exist in index");
pos = -pos - 1;
if (pos >= o->src_index->cache_nr ||
!starts_with(o->src_index->cache[pos]->name, name.buf) ||
(pos > 0 && starts_with(o->src_index->cache[pos-1]->name, name.buf)))
BUG("pos %d doesn't point to the first entry of %s in index",
pos, name.buf);
strbuf_release(&name);
return pos;
}
static int traverse_by_cache_tree(int pos, int nr_entries, int nr_names,
struct traverse_info *info)
{
struct cache_entry *src[MAX_UNPACK_TREES + 1] = { NULL, };
struct unpack_trees_options *o = info->data;
struct cache_entry *tree_ce = NULL;
int ce_len = 0;
int i, d;
if (!o->merge)
BUG("We need cache-tree to do this optimization");
for (i = 0; i < nr_entries; i++) {
int new_ce_len, len, rc;
src[0] = o->src_index->cache[pos + i];
len = ce_namelen(src[0]);
new_ce_len = cache_entry_size(len);
if (new_ce_len > ce_len) {
new_ce_len <<= 1;
tree_ce = xrealloc(tree_ce, new_ce_len);
memset(tree_ce, 0, new_ce_len);
ce_len = new_ce_len;
tree_ce->ce_flags = create_ce_flags(0);
for (d = 1; d <= nr_names; d++)
src[d] = tree_ce;
}
tree_ce->ce_mode = src[0]->ce_mode;
tree_ce->ce_namelen = len;
oidcpy(&tree_ce->oid, &src[0]->oid);
memcpy(tree_ce->name, src[0]->name, len + 1);
rc = call_unpack_fn((const struct cache_entry * const *)src, o);
if (rc < 0) {
free(tree_ce);
return rc;
}
mark_ce_used(src[0], o);
}
free(tree_ce);
if (o->debug_unpack)
printf("Unpacked %d entries from %s to %s using cache-tree\n",
nr_entries,
o->src_index->cache[pos]->name,
o->src_index->cache[pos + nr_entries - 1]->name);
return 0;
}
static int traverse_trees_recursive(int n, unsigned long dirmask,
unsigned long df_conflicts,
struct name_entry *names,
struct traverse_info *info)
{
struct unpack_trees_options *o = info->data;
int i, ret, bottom;
int nr_buf = 0;
struct tree_desc t[MAX_UNPACK_TREES];
void *buf[MAX_UNPACK_TREES];
struct traverse_info newinfo;
struct name_entry *p;
int nr_entries;
nr_entries = all_trees_same_as_cache_tree(n, dirmask, names, info);
if (nr_entries > 0) {
int pos = index_pos_by_traverse_info(names, info);
if (!o->merge || df_conflicts)
BUG("Wrong condition to get here buddy");
bottom = o->cache_bottom;
ret = traverse_by_cache_tree(pos, nr_entries, n, info);
o->cache_bottom = bottom;
return ret;
}
p = names;
while (!p->mode)
p++;
newinfo = *info;
newinfo.prev = info;
newinfo.pathspec = info->pathspec;
newinfo.name = p->path;
newinfo.namelen = p->pathlen;
newinfo.mode = p->mode;
newinfo.pathlen = st_add3(newinfo.pathlen, tree_entry_len(p), 1);
newinfo.df_conflicts |= df_conflicts;
for (i = 0; i < n; i++, dirmask >>= 1) {
if (i > 0 && are_same_oid(&names[i], &names[i - 1]))
t[i] = t[i - 1];
else if (i > 1 && are_same_oid(&names[i], &names[i - 2]))
t[i] = t[i - 2];
else {
const struct object_id *oid = NULL;
if (dirmask & 1)
oid = &names[i].oid;
buf[nr_buf++] = fill_tree_descriptor(the_repository, t + i, oid);
}
}
bottom = switch_cache_bottom(&newinfo);
ret = traverse_trees(o->src_index, n, t, &newinfo);
restore_cache_bottom(&newinfo, bottom);
for (i = 0; i < nr_buf; i++)
free(buf[i]);
return ret;
}
static int do_compare_entry_piecewise(const struct cache_entry *ce,
const struct traverse_info *info,
const char *name, size_t namelen,
unsigned mode)
{
int pathlen, ce_len;
const char *ce_name;
if (info->prev) {
int cmp = do_compare_entry_piecewise(ce, info->prev,
info->name, info->namelen,
info->mode);
if (cmp)
return cmp;
}
pathlen = info->pathlen;
ce_len = ce_namelen(ce);
if (ce_len < pathlen)
return -1;
ce_len -= pathlen;
ce_name = ce->name + pathlen;
return df_name_compare(ce_name, ce_len, S_IFREG, name, namelen, mode);
}
static int do_compare_entry(const struct cache_entry *ce,
const struct traverse_info *info,
const char *name, size_t namelen,
unsigned mode)
{
int pathlen, ce_len;
const char *ce_name;
int cmp;
if (!info->traverse_path)
return do_compare_entry_piecewise(ce, info, name, namelen, mode);
cmp = strncmp(ce->name, info->traverse_path, info->pathlen);
if (cmp)
return cmp;
pathlen = info->pathlen;
ce_len = ce_namelen(ce);
if (ce_len < pathlen)
return -1;
ce_len -= pathlen;
ce_name = ce->name + pathlen;
return df_name_compare(ce_name, ce_len, S_IFREG, name, namelen, mode);
}
static int compare_entry(const struct cache_entry *ce, const struct traverse_info *info, const struct name_entry *n)
{
int cmp = do_compare_entry(ce, info, n->path, n->pathlen, n->mode);
if (cmp)
return cmp;
return ce_namelen(ce) > traverse_path_len(info, tree_entry_len(n));
}
static int ce_in_traverse_path(const struct cache_entry *ce,
const struct traverse_info *info)
{
if (!info->prev)
return 1;
if (do_compare_entry(ce, info->prev,
info->name, info->namelen, info->mode))
return 0;
return (info->pathlen < ce_namelen(ce));
}
static struct cache_entry *create_ce_entry(const struct traverse_info *info,
const struct name_entry *n,
int stage,
struct index_state *istate,
int is_transient)
{
size_t len = traverse_path_len(info, tree_entry_len(n));
struct cache_entry *ce =
is_transient ?
make_empty_transient_cache_entry(len) :
make_empty_cache_entry(istate, len);
ce->ce_mode = create_ce_mode(n->mode);
ce->ce_flags = create_ce_flags(stage);
ce->ce_namelen = len;
oidcpy(&ce->oid, &n->oid);
make_traverse_path(ce->name, len + 1, info, n->path, n->pathlen);
return ce;
}
static int unpack_nondirectories(int n, unsigned long mask,
unsigned long dirmask,
struct cache_entry **src,
const struct name_entry *names,
const struct traverse_info *info)
{
int i;
struct unpack_trees_options *o = info->data;
unsigned long conflicts = info->df_conflicts | dirmask;
if (mask == dirmask && !src[0])
return 0;
for (i = 0; i < n; i++) {
int stage;
unsigned int bit = 1ul << i;
if (conflicts & bit) {
src[i + o->merge] = o->df_conflict_entry;
continue;
}
if (!(mask & bit))
continue;
if (!o->merge)
stage = 0;
else if (i + 1 < o->head_idx)
stage = 1;
else if (i + 1 > o->head_idx)
stage = 3;
else
stage = 2;
src[i + o->merge] = create_ce_entry(info, names + i, stage, &o->result, o->merge);
}
if (o->merge) {
int rc = call_unpack_fn((const struct cache_entry * const *)src,
o);
for (i = 0; i < n; i++) {
struct cache_entry *ce = src[i + o->merge];
if (ce != o->df_conflict_entry)
discard_cache_entry(ce);
}
return rc;
}
for (i = 0; i < n; i++)
if (src[i] && src[i] != o->df_conflict_entry)
if (do_add_entry(o, src[i], 0, 0))
return -1;
return 0;
}
static int unpack_failed(struct unpack_trees_options *o, const char *message)
{
discard_index(&o->result);
if (!o->quiet && !o->exiting_early) {
if (message)
return error("%s", message);
return -1;
}
return -1;
}
static int find_cache_pos(struct traverse_info *info,
const char *p, size_t p_len)
{
int pos;
struct unpack_trees_options *o = info->data;
struct index_state *index = o->src_index;
int pfxlen = info->pathlen;
for (pos = o->cache_bottom; pos < index->cache_nr; pos++) {
const struct cache_entry *ce = index->cache[pos];
const char *ce_name, *ce_slash;
int cmp, ce_len;
if (ce->ce_flags & CE_UNPACKED) {
if (pos == o->cache_bottom)
++o->cache_bottom;
continue;
}
if (!ce_in_traverse_path(ce, info)) {
if (info->traverse_path) {
if (strncmp(ce->name, info->traverse_path,
info->pathlen) > 0)
break;
}
continue;
}
ce_name = ce->name + pfxlen;
ce_slash = strchr(ce_name, '/');
if (ce_slash)
ce_len = ce_slash - ce_name;
else
ce_len = ce_namelen(ce) - pfxlen;
cmp = name_compare(p, p_len, ce_name, ce_len);
if (!cmp)
return ce_slash ? -2 - pos : pos;
if (0 < cmp)
continue; 
if (p_len < ce_len && !memcmp(ce_name, p, p_len) &&
ce_name[p_len] < '/')
continue; 
break;
}
return -1;
}
static struct cache_entry *find_cache_entry(struct traverse_info *info,
const struct name_entry *p)
{
int pos = find_cache_pos(info, p->path, p->pathlen);
struct unpack_trees_options *o = info->data;
if (0 <= pos)
return o->src_index->cache[pos];
else
return NULL;
}
static void debug_path(struct traverse_info *info)
{
if (info->prev) {
debug_path(info->prev);
if (*info->prev->name)
putchar('/');
}
printf("%s", info->name);
}
static void debug_name_entry(int i, struct name_entry *n)
{
printf("ent#%d %06o %s\n", i,
n->path ? n->mode : 0,
n->path ? n->path : "(missing)");
}
static void debug_unpack_callback(int n,
unsigned long mask,
unsigned long dirmask,
struct name_entry *names,
struct traverse_info *info)
{
int i;
printf("* unpack mask %lu, dirmask %lu, cnt %d ",
mask, dirmask, n);
debug_path(info);
putchar('\n');
for (i = 0; i < n; i++)
debug_name_entry(i, names + i);
}
static int unpack_callback(int n, unsigned long mask, unsigned long dirmask, struct name_entry *names, struct traverse_info *info)
{
struct cache_entry *src[MAX_UNPACK_TREES + 1] = { NULL, };
struct unpack_trees_options *o = info->data;
const struct name_entry *p = names;
while (!p->mode)
p++;
if (o->debug_unpack)
debug_unpack_callback(n, mask, dirmask, names, info);
if (o->merge) {
while (1) {
int cmp;
struct cache_entry *ce;
if (o->diff_index_cached)
ce = next_cache_entry(o);
else
ce = find_cache_entry(info, p);
if (!ce)
break;
cmp = compare_entry(ce, info, p);
if (cmp < 0) {
if (unpack_index_entry(ce, o) < 0)
return unpack_failed(o, NULL);
continue;
}
if (!cmp) {
if (ce_stage(ce)) {
if (o->skip_unmerged) {
add_same_unmerged(ce, o);
return mask;
}
}
src[0] = ce;
}
break;
}
}
if (unpack_nondirectories(n, mask, dirmask, src, names, info) < 0)
return -1;
if (o->merge && src[0]) {
if (ce_stage(src[0]))
mark_ce_used_same_name(src[0], o);
else
mark_ce_used(src[0], o);
}
if (dirmask) {
if (o->diff_index_cached &&
n == 1 && dirmask == 1 && S_ISDIR(names->mode)) {
int matches;
matches = cache_tree_matches_traversal(o->src_index->cache_tree,
names, info);
if (matches) {
o->cache_bottom += matches;
return mask;
}
}
if (traverse_trees_recursive(n, dirmask, mask & ~dirmask,
names, info) < 0)
return -1;
return mask;
}
return mask;
}
static int clear_ce_flags_1(struct index_state *istate,
struct cache_entry **cache, int nr,
struct strbuf *prefix,
int select_mask, int clear_mask,
struct pattern_list *pl,
enum pattern_match_result default_match,
int progress_nr);
static int clear_ce_flags_dir(struct index_state *istate,
struct cache_entry **cache, int nr,
struct strbuf *prefix,
char *basename,
int select_mask, int clear_mask,
struct pattern_list *pl,
enum pattern_match_result default_match,
int progress_nr)
{
struct cache_entry **cache_end;
int dtype = DT_DIR;
int rc;
enum pattern_match_result ret, orig_ret;
orig_ret = path_matches_pattern_list(prefix->buf, prefix->len,
basename, &dtype, pl, istate);
strbuf_addch(prefix, '/');
if (orig_ret == UNDECIDED)
ret = default_match;
else
ret = orig_ret;
for (cache_end = cache; cache_end != cache + nr; cache_end++) {
struct cache_entry *ce = *cache_end;
if (strncmp(ce->name, prefix->buf, prefix->len))
break;
}
if (pl->use_cone_patterns && orig_ret == MATCHED_RECURSIVE) {
struct cache_entry **ce = cache;
rc = cache_end - cache;
while (ce < cache_end) {
(*ce)->ce_flags &= ~clear_mask;
ce++;
}
} else if (pl->use_cone_patterns && orig_ret == NOT_MATCHED) {
rc = cache_end - cache;
} else {
rc = clear_ce_flags_1(istate, cache, cache_end - cache,
prefix,
select_mask, clear_mask,
pl, ret,
progress_nr);
}
strbuf_setlen(prefix, prefix->len - 1);
return rc;
}
static int clear_ce_flags_1(struct index_state *istate,
struct cache_entry **cache, int nr,
struct strbuf *prefix,
int select_mask, int clear_mask,
struct pattern_list *pl,
enum pattern_match_result default_match,
int progress_nr)
{
struct cache_entry **cache_end = nr ? cache + nr : cache;
while(cache != cache_end) {
struct cache_entry *ce = *cache;
const char *name, *slash;
int len, dtype;
enum pattern_match_result ret;
display_progress(istate->progress, progress_nr);
if (select_mask && !(ce->ce_flags & select_mask)) {
cache++;
progress_nr++;
continue;
}
if (prefix->len && strncmp(ce->name, prefix->buf, prefix->len))
break;
name = ce->name + prefix->len;
slash = strchr(name, '/');
if (slash) {
int processed;
len = slash - name;
strbuf_add(prefix, name, len);
processed = clear_ce_flags_dir(istate, cache, cache_end - cache,
prefix,
prefix->buf + prefix->len - len,
select_mask, clear_mask,
pl, default_match,
progress_nr);
if (processed) {
cache += processed;
progress_nr += processed;
strbuf_setlen(prefix, prefix->len - len);
continue;
}
strbuf_addch(prefix, '/');
processed = clear_ce_flags_1(istate, cache, cache_end - cache,
prefix,
select_mask, clear_mask, pl,
default_match, progress_nr);
cache += processed;
progress_nr += processed;
strbuf_setlen(prefix, prefix->len - len - 1);
continue;
}
dtype = ce_to_dtype(ce);
ret = path_matches_pattern_list(ce->name,
ce_namelen(ce),
name, &dtype, pl, istate);
if (ret == UNDECIDED)
ret = default_match;
if (ret == MATCHED || ret == MATCHED_RECURSIVE)
ce->ce_flags &= ~clear_mask;
cache++;
progress_nr++;
}
display_progress(istate->progress, progress_nr);
return nr - (cache_end - cache);
}
static int clear_ce_flags(struct index_state *istate,
int select_mask, int clear_mask,
struct pattern_list *pl,
int show_progress)
{
static struct strbuf prefix = STRBUF_INIT;
char label[100];
int rval;
strbuf_reset(&prefix);
if (show_progress)
istate->progress = start_delayed_progress(
_("Updating index flags"),
istate->cache_nr);
xsnprintf(label, sizeof(label), "clear_ce_flags(0x%08lx,0x%08lx)",
(unsigned long)select_mask, (unsigned long)clear_mask);
trace2_region_enter("unpack_trees", label, the_repository);
rval = clear_ce_flags_1(istate,
istate->cache,
istate->cache_nr,
&prefix,
select_mask, clear_mask,
pl, 0, 0);
trace2_region_leave("unpack_trees", label, the_repository);
stop_progress(&istate->progress);
return rval;
}
static void mark_new_skip_worktree(struct pattern_list *pl,
struct index_state *istate,
int select_flag, int skip_wt_flag,
int show_progress)
{
int i;
for (i = 0; i < istate->cache_nr; i++) {
struct cache_entry *ce = istate->cache[i];
if (select_flag && !(ce->ce_flags & select_flag))
continue;
if (!ce_stage(ce) && !(ce->ce_flags & CE_CONFLICTED))
ce->ce_flags |= skip_wt_flag;
else
ce->ce_flags &= ~skip_wt_flag;
}
clear_ce_flags(istate, select_flag, skip_wt_flag, pl, show_progress);
}
static int verify_absent(const struct cache_entry *,
enum unpack_trees_error_types,
struct unpack_trees_options *);
int unpack_trees(unsigned len, struct tree_desc *t, struct unpack_trees_options *o)
{
int i, ret;
static struct cache_entry *dfc;
struct pattern_list pl;
if (len > MAX_UNPACK_TREES)
die("unpack_trees takes at most %d trees", MAX_UNPACK_TREES);
trace_performance_enter();
memset(&pl, 0, sizeof(pl));
if (!core_apply_sparse_checkout || !o->update)
o->skip_sparse_checkout = 1;
if (!o->skip_sparse_checkout && !o->pl) {
char *sparse = git_pathdup("info/sparse-checkout");
pl.use_cone_patterns = core_sparse_checkout_cone;
if (add_patterns_from_file_to_list(sparse, "", 0, &pl, NULL) < 0)
o->skip_sparse_checkout = 1;
else
o->pl = &pl;
free(sparse);
}
memset(&o->result, 0, sizeof(o->result));
o->result.initialized = 1;
o->result.timestamp.sec = o->src_index->timestamp.sec;
o->result.timestamp.nsec = o->src_index->timestamp.nsec;
o->result.version = o->src_index->version;
if (!o->src_index->split_index) {
o->result.split_index = NULL;
} else if (o->src_index == o->dst_index) {
o->result.split_index = o->src_index->split_index;
o->result.split_index->refcount++;
} else {
o->result.split_index = init_split_index(&o->result);
}
oidcpy(&o->result.oid, &o->src_index->oid);
o->merge_size = len;
mark_all_ce_unused(o->src_index);
if (o->src_index->fsmonitor_last_update)
o->result.fsmonitor_last_update = o->src_index->fsmonitor_last_update;
if (!o->skip_sparse_checkout)
mark_new_skip_worktree(o->pl, o->src_index, 0,
CE_NEW_SKIP_WORKTREE, o->verbose_update);
if (!dfc)
dfc = xcalloc(1, cache_entry_size(0));
o->df_conflict_entry = dfc;
if (len) {
const char *prefix = o->prefix ? o->prefix : "";
struct traverse_info info;
setup_traverse_info(&info, prefix);
info.fn = unpack_callback;
info.data = o;
info.show_all_errors = o->show_all_errors;
info.pathspec = o->pathspec;
if (o->prefix) {
while (1) {
struct cache_entry *ce = next_cache_entry(o);
if (!ce)
break;
if (ce_in_traverse_path(ce, &info))
break;
if (unpack_index_entry(ce, o) < 0)
goto return_failed;
}
}
trace_performance_enter();
ret = traverse_trees(o->src_index, len, t, &info);
trace_performance_leave("traverse_trees");
if (ret < 0)
goto return_failed;
}
if (o->merge) {
while (1) {
struct cache_entry *ce = next_cache_entry(o);
if (!ce)
break;
if (unpack_index_entry(ce, o) < 0)
goto return_failed;
}
}
mark_all_ce_unused(o->src_index);
if (o->trivial_merges_only && o->nontrivial_merge) {
ret = unpack_failed(o, "Merge requires file-level merging");
goto done;
}
if (!o->skip_sparse_checkout) {
int empty_worktree = 1;
mark_new_skip_worktree(o->pl, &o->result,
CE_ADDED, CE_SKIP_WORKTREE | CE_NEW_SKIP_WORKTREE,
o->verbose_update);
ret = 0;
for (i = 0; i < o->result.cache_nr; i++) {
struct cache_entry *ce = o->result.cache[i];
if (ce->ce_flags & CE_ADDED &&
verify_absent(ce, ERROR_WOULD_LOSE_UNTRACKED_OVERWRITTEN, o)) {
if (!o->show_all_errors)
goto return_failed;
ret = -1;
}
if (apply_sparse_checkout(&o->result, ce, o)) {
if (!o->show_all_errors)
goto return_failed;
ret = -1;
}
if (!ce_skip_worktree(ce))
empty_worktree = 0;
}
if (ret < 0)
goto return_failed;
if (o->result.cache_nr && empty_worktree) {
ret = unpack_failed(o, "Sparse checkout leaves no entry on working directory");
goto done;
}
}
ret = check_updates(o) ? (-2) : 0;
if (o->dst_index) {
move_index_extensions(&o->result, o->src_index);
if (!ret) {
if (git_env_bool("GIT_TEST_CHECK_CACHE_TREE", 0))
cache_tree_verify(the_repository, &o->result);
if (!o->result.cache_tree)
o->result.cache_tree = cache_tree();
if (!cache_tree_fully_valid(o->result.cache_tree))
cache_tree_update(&o->result,
WRITE_TREE_SILENT |
WRITE_TREE_REPAIR);
}
o->result.updated_workdir = 1;
discard_index(o->dst_index);
*o->dst_index = o->result;
} else {
discard_index(&o->result);
}
o->src_index = NULL;
done:
trace_performance_leave("unpack_trees");
if (!o->keep_pattern_list)
clear_pattern_list(&pl);
return ret;
return_failed:
if (o->show_all_errors)
display_error_msgs(o);
mark_all_ce_unused(o->src_index);
ret = unpack_failed(o, NULL);
if (o->exiting_early)
ret = 0;
goto done;
}
static int reject_merge(const struct cache_entry *ce,
struct unpack_trees_options *o)
{
return add_rejected_path(o, ERROR_WOULD_OVERWRITE, ce->name);
}
static int same(const struct cache_entry *a, const struct cache_entry *b)
{
if (!!a != !!b)
return 0;
if (!a && !b)
return 1;
if ((a->ce_flags | b->ce_flags) & CE_CONFLICTED)
return 0;
return a->ce_mode == b->ce_mode &&
oideq(&a->oid, &b->oid);
}
static int verify_uptodate_1(const struct cache_entry *ce,
struct unpack_trees_options *o,
enum unpack_trees_error_types error_type)
{
struct stat st;
if (o->index_only)
return 0;
if ((ce->ce_flags & CE_VALID) || ce_skip_worktree(ce))
; 
else if (o->reset || ce_uptodate(ce))
return 0;
if (!lstat(ce->name, &st)) {
int flags = CE_MATCH_IGNORE_VALID|CE_MATCH_IGNORE_SKIP_WORKTREE;
unsigned changed = ie_match_stat(o->src_index, ce, &st, flags);
if (submodule_from_ce(ce)) {
int r = check_submodule_move_head(ce,
"HEAD", oid_to_hex(&ce->oid), o);
if (r)
return add_rejected_path(o, error_type, ce->name);
return 0;
}
if (!changed)
return 0;
if (S_ISGITLINK(ce->ce_mode))
return 0;
errno = 0;
}
if (errno == ENOENT)
return 0;
return add_rejected_path(o, error_type, ce->name);
}
int verify_uptodate(const struct cache_entry *ce,
struct unpack_trees_options *o)
{
if (!o->skip_sparse_checkout && (ce->ce_flags & CE_NEW_SKIP_WORKTREE))
return 0;
return verify_uptodate_1(ce, o, ERROR_NOT_UPTODATE_FILE);
}
static int verify_uptodate_sparse(const struct cache_entry *ce,
struct unpack_trees_options *o)
{
return verify_uptodate_1(ce, o, ERROR_SPARSE_NOT_UPTODATE_FILE);
}
static void invalidate_ce_path(const struct cache_entry *ce,
struct unpack_trees_options *o)
{
if (!ce)
return;
cache_tree_invalidate_path(o->src_index, ce->name);
untracked_cache_invalidate_path(o->src_index, ce->name, 1);
}
static int verify_clean_submodule(const char *old_sha1,
const struct cache_entry *ce,
struct unpack_trees_options *o)
{
if (!submodule_from_ce(ce))
return 0;
return check_submodule_move_head(ce, old_sha1,
oid_to_hex(&ce->oid), o);
}
static int verify_clean_subdirectory(const struct cache_entry *ce,
struct unpack_trees_options *o)
{
int namelen;
int i;
struct dir_struct d;
char *pathbuf;
int cnt = 0;
if (S_ISGITLINK(ce->ce_mode)) {
struct object_id oid;
int sub_head = resolve_gitlink_ref(ce->name, "HEAD", &oid);
if (!sub_head && oideq(&oid, &ce->oid))
return 0;
return verify_clean_submodule(sub_head ? NULL : oid_to_hex(&oid),
ce, o);
}
namelen = ce_namelen(ce);
for (i = locate_in_src_index(ce, o);
i < o->src_index->cache_nr;
i++) {
struct cache_entry *ce2 = o->src_index->cache[i];
int len = ce_namelen(ce2);
if (len < namelen ||
strncmp(ce->name, ce2->name, namelen) ||
ce2->name[namelen] != '/')
break;
if (!ce_stage(ce2)) {
if (verify_uptodate(ce2, o))
return -1;
add_entry(o, ce2, CE_REMOVE, 0);
invalidate_ce_path(ce, o);
mark_ce_used(ce2, o);
}
cnt++;
}
pathbuf = xstrfmt("%.*s/", namelen, ce->name);
memset(&d, 0, sizeof(d));
if (o->dir)
d.exclude_per_dir = o->dir->exclude_per_dir;
i = read_directory(&d, o->src_index, pathbuf, namelen+1, NULL);
if (i)
return add_rejected_path(o, ERROR_NOT_UPTODATE_DIR, ce->name);
free(pathbuf);
return cnt;
}
static int icase_exists(struct unpack_trees_options *o, const char *name, int len, struct stat *st)
{
const struct cache_entry *src;
src = index_file_exists(o->src_index, name, len, 1);
return src && !ie_match_stat(o->src_index, src, st, CE_MATCH_IGNORE_VALID|CE_MATCH_IGNORE_SKIP_WORKTREE);
}
static int check_ok_to_remove(const char *name, int len, int dtype,
const struct cache_entry *ce, struct stat *st,
enum unpack_trees_error_types error_type,
struct unpack_trees_options *o)
{
const struct cache_entry *result;
if (ignore_case && icase_exists(o, name, len, st))
return 0;
if (o->dir &&
is_excluded(o->dir, o->src_index, name, &dtype))
return 0;
if (S_ISDIR(st->st_mode)) {
if (verify_clean_subdirectory(ce, o) < 0)
return -1;
return 0;
}
result = index_file_exists(&o->result, name, len, 0);
if (result) {
if (result->ce_flags & CE_REMOVE)
return 0;
}
return add_rejected_path(o, error_type, name);
}
static int verify_absent_1(const struct cache_entry *ce,
enum unpack_trees_error_types error_type,
struct unpack_trees_options *o)
{
int len;
struct stat st;
if (o->index_only || o->reset || !o->update)
return 0;
len = check_leading_path(ce->name, ce_namelen(ce));
if (!len)
return 0;
else if (len > 0) {
char *path;
int ret;
path = xmemdupz(ce->name, len);
if (lstat(path, &st))
ret = error_errno("cannot stat '%s'", path);
else {
if (submodule_from_ce(ce))
ret = check_submodule_move_head(ce,
oid_to_hex(&ce->oid),
NULL, o);
else
ret = check_ok_to_remove(path, len, DT_UNKNOWN, NULL,
&st, error_type, o);
}
free(path);
return ret;
} else if (lstat(ce->name, &st)) {
if (errno != ENOENT)
return error_errno("cannot stat '%s'", ce->name);
return 0;
} else {
if (submodule_from_ce(ce))
return check_submodule_move_head(ce, oid_to_hex(&ce->oid),
NULL, o);
return check_ok_to_remove(ce->name, ce_namelen(ce),
ce_to_dtype(ce), ce, &st,
error_type, o);
}
}
static int verify_absent(const struct cache_entry *ce,
enum unpack_trees_error_types error_type,
struct unpack_trees_options *o)
{
if (!o->skip_sparse_checkout && (ce->ce_flags & CE_NEW_SKIP_WORKTREE))
return 0;
return verify_absent_1(ce, error_type, o);
}
static int verify_absent_sparse(const struct cache_entry *ce,
enum unpack_trees_error_types error_type,
struct unpack_trees_options *o)
{
enum unpack_trees_error_types orphaned_error = error_type;
if (orphaned_error == ERROR_WOULD_LOSE_UNTRACKED_OVERWRITTEN)
orphaned_error = ERROR_WOULD_LOSE_ORPHANED_OVERWRITTEN;
return verify_absent_1(ce, orphaned_error, o);
}
static int merged_entry(const struct cache_entry *ce,
const struct cache_entry *old,
struct unpack_trees_options *o)
{
int update = CE_UPDATE;
struct cache_entry *merge = dup_cache_entry(ce, &o->result);
if (!old) {
update |= CE_ADDED;
merge->ce_flags |= CE_NEW_SKIP_WORKTREE;
if (verify_absent(merge,
ERROR_WOULD_LOSE_UNTRACKED_OVERWRITTEN, o)) {
discard_cache_entry(merge);
return -1;
}
invalidate_ce_path(merge, o);
if (submodule_from_ce(ce) && file_exists(ce->name)) {
int ret = check_submodule_move_head(ce, NULL,
oid_to_hex(&ce->oid),
o);
if (ret)
return ret;
}
} else if (!(old->ce_flags & CE_CONFLICTED)) {
if (same(old, merge)) {
copy_cache_entry(merge, old);
update = 0;
} else {
if (verify_uptodate(old, o)) {
discard_cache_entry(merge);
return -1;
}
update |= old->ce_flags & (CE_SKIP_WORKTREE | CE_NEW_SKIP_WORKTREE);
invalidate_ce_path(old, o);
}
if (submodule_from_ce(ce) && file_exists(ce->name)) {
int ret = check_submodule_move_head(ce, oid_to_hex(&old->oid),
oid_to_hex(&ce->oid),
o);
if (ret)
return ret;
}
} else {
invalidate_ce_path(old, o);
}
if (do_add_entry(o, merge, update, CE_STAGEMASK) < 0)
return -1;
return 1;
}
static int deleted_entry(const struct cache_entry *ce,
const struct cache_entry *old,
struct unpack_trees_options *o)
{
if (!old) {
if (verify_absent(ce, ERROR_WOULD_LOSE_UNTRACKED_REMOVED, o))
return -1;
return 0;
}
if (!(old->ce_flags & CE_CONFLICTED) && verify_uptodate(old, o))
return -1;
add_entry(o, ce, CE_REMOVE, 0);
invalidate_ce_path(ce, o);
return 1;
}
static int keep_entry(const struct cache_entry *ce,
struct unpack_trees_options *o)
{
add_entry(o, ce, 0, 0);
if (ce_stage(ce))
invalidate_ce_path(ce, o);
return 1;
}
#if DBRT_DEBUG
static void show_stage_entry(FILE *o,
const char *label, const struct cache_entry *ce)
{
if (!ce)
fprintf(o, "%s (missing)\n", label);
else
fprintf(o, "%s%06o %s %d\t%s\n",
label,
ce->ce_mode,
oid_to_hex(&ce->oid),
ce_stage(ce),
ce->name);
}
#endif
int threeway_merge(const struct cache_entry * const *stages,
struct unpack_trees_options *o)
{
const struct cache_entry *index;
const struct cache_entry *head;
const struct cache_entry *remote = stages[o->head_idx + 1];
int count;
int head_match = 0;
int remote_match = 0;
int df_conflict_head = 0;
int df_conflict_remote = 0;
int any_anc_missing = 0;
int no_anc_exists = 1;
int i;
for (i = 1; i < o->head_idx; i++) {
if (!stages[i] || stages[i] == o->df_conflict_entry)
any_anc_missing = 1;
else
no_anc_exists = 0;
}
index = stages[0];
head = stages[o->head_idx];
if (head == o->df_conflict_entry) {
df_conflict_head = 1;
head = NULL;
}
if (remote == o->df_conflict_entry) {
df_conflict_remote = 1;
remote = NULL;
}
if (!same(remote, head)) {
for (i = 1; i < o->head_idx; i++) {
if (same(stages[i], head)) {
head_match = i;
}
if (same(stages[i], remote)) {
remote_match = i;
}
}
}
if (remote && !df_conflict_head && head_match && !remote_match) {
if (index && !same(index, remote) && !same(index, head))
return reject_merge(index, o);
return merged_entry(remote, index, o);
}
if (index && !same(index, head))
return reject_merge(index, o);
if (head) {
if (same(head, remote))
return merged_entry(head, index, o);
if (!df_conflict_remote && remote_match && !head_match)
return merged_entry(head, index, o);
}
if (!head && !remote && any_anc_missing)
return 0;
if (o->aggressive) {
int head_deleted = !head;
int remote_deleted = !remote;
const struct cache_entry *ce = NULL;
if (index)
ce = index;
else if (head)
ce = head;
else if (remote)
ce = remote;
else {
for (i = 1; i < o->head_idx; i++) {
if (stages[i] && stages[i] != o->df_conflict_entry) {
ce = stages[i];
break;
}
}
}
if ((head_deleted && remote_deleted) ||
(head_deleted && remote && remote_match) ||
(remote_deleted && head && head_match)) {
if (index)
return deleted_entry(index, index, o);
if (ce && !head_deleted) {
if (verify_absent(ce, ERROR_WOULD_LOSE_UNTRACKED_REMOVED, o))
return -1;
}
return 0;
}
if (no_anc_exists && head && remote && same(head, remote))
return merged_entry(head, index, o);
}
if (index) {
if (verify_uptodate(index, o))
return -1;
}
o->nontrivial_merge = 1;
count = 0;
if (!head_match || !remote_match) {
for (i = 1; i < o->head_idx; i++) {
if (stages[i] && stages[i] != o->df_conflict_entry) {
keep_entry(stages[i], o);
count++;
break;
}
}
}
#if DBRT_DEBUG
else {
fprintf(stderr, "read-tree: warning #16 detected\n");
show_stage_entry(stderr, "head ", stages[head_match]);
show_stage_entry(stderr, "remote ", stages[remote_match]);
}
#endif
if (head) { count += keep_entry(head, o); }
if (remote) { count += keep_entry(remote, o); }
return count;
}
int twoway_merge(const struct cache_entry * const *src,
struct unpack_trees_options *o)
{
const struct cache_entry *current = src[0];
const struct cache_entry *oldtree = src[1];
const struct cache_entry *newtree = src[2];
if (o->merge_size != 2)
return error("Cannot do a twoway merge of %d trees",
o->merge_size);
if (oldtree == o->df_conflict_entry)
oldtree = NULL;
if (newtree == o->df_conflict_entry)
newtree = NULL;
if (current) {
if (current->ce_flags & CE_CONFLICTED) {
if (same(oldtree, newtree) || o->reset) {
if (!newtree)
return deleted_entry(current, current, o);
else
return merged_entry(newtree, current, o);
}
return reject_merge(current, o);
} else if ((!oldtree && !newtree) || 
(!oldtree && newtree &&
same(current, newtree)) || 
(oldtree && newtree &&
same(oldtree, newtree)) || 
(oldtree && newtree &&
!same(oldtree, newtree) && 
same(current, newtree))) {
return keep_entry(current, o);
} else if (oldtree && !newtree && same(current, oldtree)) {
return deleted_entry(oldtree, current, o);
} else if (oldtree && newtree &&
same(current, oldtree) && !same(current, newtree)) {
return merged_entry(newtree, current, o);
} else
return reject_merge(current, o);
}
else if (newtree) {
if (oldtree && !o->initial_checkout) {
if (same(oldtree, newtree))
return 1;
return reject_merge(oldtree, o);
}
return merged_entry(newtree, current, o);
}
return deleted_entry(oldtree, current, o);
}
int bind_merge(const struct cache_entry * const *src,
struct unpack_trees_options *o)
{
const struct cache_entry *old = src[0];
const struct cache_entry *a = src[1];
if (o->merge_size != 1)
return error("Cannot do a bind merge of %d trees",
o->merge_size);
if (a && old)
return o->quiet ? -1 :
error(ERRORMSG(o, ERROR_BIND_OVERLAP),
super_prefixed(a->name),
super_prefixed(old->name));
if (!a)
return keep_entry(old, o);
else
return merged_entry(a, NULL, o);
}
int oneway_merge(const struct cache_entry * const *src,
struct unpack_trees_options *o)
{
const struct cache_entry *old = src[0];
const struct cache_entry *a = src[1];
if (o->merge_size != 1)
return error("Cannot do a oneway merge of %d trees",
o->merge_size);
if (!a || a == o->df_conflict_entry)
return deleted_entry(old, old, o);
if (old && same(old, a)) {
int update = 0;
if (o->reset && o->update && !ce_uptodate(old) && !ce_skip_worktree(old) &&
!(old->ce_flags & CE_FSMONITOR_VALID)) {
struct stat st;
if (lstat(old->name, &st) ||
ie_match_stat(o->src_index, old, &st, CE_MATCH_IGNORE_VALID|CE_MATCH_IGNORE_SKIP_WORKTREE))
update |= CE_UPDATE;
}
if (o->update && S_ISGITLINK(old->ce_mode) &&
should_update_submodules() && !verify_uptodate(old, o))
update |= CE_UPDATE;
add_entry(o, old, update, CE_STAGEMASK);
return 0;
}
return merged_entry(a, old, o);
}
