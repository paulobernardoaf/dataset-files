#include "cache.h"
#include "quote.h"
#include "commit.h"
#include "diff.h"
#include "diffcore.h"
#include "revision.h"
#include "cache-tree.h"
#include "unpack-trees.h"
#include "refs.h"
#include "submodule.h"
#include "dir.h"
#include "fsmonitor.h"
static int check_removed(const struct cache_entry *ce, struct stat *st)
{
if (lstat(ce->name, st) < 0) {
if (!is_missing_file_error(errno))
return -1;
return 1;
}
if (has_symlink_leading_path(ce->name, ce_namelen(ce)))
return 1;
if (S_ISDIR(st->st_mode)) {
struct object_id sub;
if (!S_ISGITLINK(ce->ce_mode) &&
resolve_gitlink_ref(ce->name, "HEAD", &sub))
return 1;
}
return 0;
}
static int match_stat_with_submodule(struct diff_options *diffopt,
const struct cache_entry *ce,
struct stat *st, unsigned ce_option,
unsigned *dirty_submodule)
{
int changed = ie_match_stat(diffopt->repo->index, ce, st, ce_option);
if (S_ISGITLINK(ce->ce_mode)) {
struct diff_flags orig_flags = diffopt->flags;
if (!diffopt->flags.override_submodule_config)
set_diffopt_flags_from_submodule_config(diffopt, ce->name);
if (diffopt->flags.ignore_submodules)
changed = 0;
else if (!diffopt->flags.ignore_dirty_submodules &&
(!changed || diffopt->flags.dirty_submodules))
*dirty_submodule = is_submodule_modified(ce->name,
diffopt->flags.ignore_untracked_in_submodules);
diffopt->flags = orig_flags;
}
return changed;
}
int run_diff_files(struct rev_info *revs, unsigned int option)
{
int entries, i;
int diff_unmerged_stage = revs->max_count;
unsigned ce_option = ((option & DIFF_RACY_IS_MODIFIED)
? CE_MATCH_RACY_IS_DIRTY : 0);
uint64_t start = getnanotime();
struct index_state *istate = revs->diffopt.repo->index;
diff_set_mnemonic_prefix(&revs->diffopt, "i/", "w/");
if (diff_unmerged_stage < 0)
diff_unmerged_stage = 2;
entries = istate->cache_nr;
for (i = 0; i < entries; i++) {
unsigned int oldmode, newmode;
struct cache_entry *ce = istate->cache[i];
int changed;
unsigned dirty_submodule = 0;
const struct object_id *old_oid, *new_oid;
if (diff_can_quit_early(&revs->diffopt))
break;
if (!ce_path_match(istate, ce, &revs->prune_data, NULL))
continue;
if (ce_stage(ce)) {
struct combine_diff_path *dpath;
struct diff_filepair *pair;
unsigned int wt_mode = 0;
int num_compare_stages = 0;
size_t path_len;
struct stat st;
path_len = ce_namelen(ce);
dpath = xmalloc(combine_diff_path_size(5, path_len));
dpath->path = (char *) &(dpath->parent[5]);
dpath->next = NULL;
memcpy(dpath->path, ce->name, path_len);
dpath->path[path_len] = '\0';
oidclr(&dpath->oid);
memset(&(dpath->parent[0]), 0,
sizeof(struct combine_diff_parent)*5);
changed = check_removed(ce, &st);
if (!changed)
wt_mode = ce_mode_from_stat(ce, st.st_mode);
else {
if (changed < 0) {
perror(ce->name);
continue;
}
wt_mode = 0;
}
dpath->mode = wt_mode;
while (i < entries) {
struct cache_entry *nce = istate->cache[i];
int stage;
if (strcmp(ce->name, nce->name))
break;
stage = ce_stage(nce);
if (2 <= stage) {
int mode = nce->ce_mode;
num_compare_stages++;
oidcpy(&dpath->parent[stage - 2].oid,
&nce->oid);
dpath->parent[stage-2].mode = ce_mode_from_stat(nce, mode);
dpath->parent[stage-2].status =
DIFF_STATUS_MODIFIED;
}
if (stage == diff_unmerged_stage)
ce = nce;
i++;
}
i--;
if (revs->combine_merges && num_compare_stages == 2) {
show_combined_diff(dpath, 2,
revs->dense_combined_merges,
revs);
free(dpath);
continue;
}
FREE_AND_NULL(dpath);
pair = diff_unmerge(&revs->diffopt, ce->name);
if (wt_mode)
pair->two->mode = wt_mode;
if (ce_stage(ce) != diff_unmerged_stage)
continue;
}
if (ce_uptodate(ce) || ce_skip_worktree(ce))
continue;
if (ce->ce_flags & CE_VALID) {
changed = 0;
newmode = ce->ce_mode;
} else {
struct stat st;
changed = check_removed(ce, &st);
if (changed) {
if (changed < 0) {
perror(ce->name);
continue;
}
diff_addremove(&revs->diffopt, '-', ce->ce_mode,
&ce->oid,
!is_null_oid(&ce->oid),
ce->name, 0);
continue;
} else if (revs->diffopt.ita_invisible_in_index &&
ce_intent_to_add(ce)) {
diff_addremove(&revs->diffopt, '+', ce->ce_mode,
the_hash_algo->empty_tree, 0,
ce->name, 0);
continue;
}
changed = match_stat_with_submodule(&revs->diffopt, ce, &st,
ce_option, &dirty_submodule);
newmode = ce_mode_from_stat(ce, st.st_mode);
}
if (!changed && !dirty_submodule) {
ce_mark_uptodate(ce);
mark_fsmonitor_valid(istate, ce);
if (!revs->diffopt.flags.find_copies_harder)
continue;
}
oldmode = ce->ce_mode;
old_oid = &ce->oid;
new_oid = changed ? &null_oid : &ce->oid;
diff_change(&revs->diffopt, oldmode, newmode,
old_oid, new_oid,
!is_null_oid(old_oid),
!is_null_oid(new_oid),
ce->name, 0, dirty_submodule);
}
diffcore_std(&revs->diffopt);
diff_flush(&revs->diffopt);
trace_performance_since(start, "diff-files");
return 0;
}
static void diff_index_show_file(struct rev_info *revs,
const char *prefix,
const struct cache_entry *ce,
const struct object_id *oid, int oid_valid,
unsigned int mode,
unsigned dirty_submodule)
{
diff_addremove(&revs->diffopt, prefix[0], mode,
oid, oid_valid, ce->name, dirty_submodule);
}
static int get_stat_data(const struct cache_entry *ce,
const struct object_id **oidp,
unsigned int *modep,
int cached, int match_missing,
unsigned *dirty_submodule, struct diff_options *diffopt)
{
const struct object_id *oid = &ce->oid;
unsigned int mode = ce->ce_mode;
if (!cached && !ce_uptodate(ce)) {
int changed;
struct stat st;
changed = check_removed(ce, &st);
if (changed < 0)
return -1;
else if (changed) {
if (match_missing) {
*oidp = oid;
*modep = mode;
return 0;
}
return -1;
}
changed = match_stat_with_submodule(diffopt, ce, &st,
0, dirty_submodule);
if (changed) {
mode = ce_mode_from_stat(ce, st.st_mode);
oid = &null_oid;
}
}
*oidp = oid;
*modep = mode;
return 0;
}
static void show_new_file(struct rev_info *revs,
const struct cache_entry *new_file,
int cached, int match_missing)
{
const struct object_id *oid;
unsigned int mode;
unsigned dirty_submodule = 0;
if (get_stat_data(new_file, &oid, &mode, cached, match_missing,
&dirty_submodule, &revs->diffopt) < 0)
return;
diff_index_show_file(revs, "+", new_file, oid, !is_null_oid(oid), mode, dirty_submodule);
}
static int show_modified(struct rev_info *revs,
const struct cache_entry *old_entry,
const struct cache_entry *new_entry,
int report_missing,
int cached, int match_missing)
{
unsigned int mode, oldmode;
const struct object_id *oid;
unsigned dirty_submodule = 0;
if (get_stat_data(new_entry, &oid, &mode, cached, match_missing,
&dirty_submodule, &revs->diffopt) < 0) {
if (report_missing)
diff_index_show_file(revs, "-", old_entry,
&old_entry->oid, 1, old_entry->ce_mode,
0);
return -1;
}
if (revs->combine_merges && !cached &&
(!oideq(oid, &old_entry->oid) || !oideq(&old_entry->oid, &new_entry->oid))) {
struct combine_diff_path *p;
int pathlen = ce_namelen(new_entry);
p = xmalloc(combine_diff_path_size(2, pathlen));
p->path = (char *) &p->parent[2];
p->next = NULL;
memcpy(p->path, new_entry->name, pathlen);
p->path[pathlen] = 0;
p->mode = mode;
oidclr(&p->oid);
memset(p->parent, 0, 2 * sizeof(struct combine_diff_parent));
p->parent[0].status = DIFF_STATUS_MODIFIED;
p->parent[0].mode = new_entry->ce_mode;
oidcpy(&p->parent[0].oid, &new_entry->oid);
p->parent[1].status = DIFF_STATUS_MODIFIED;
p->parent[1].mode = old_entry->ce_mode;
oidcpy(&p->parent[1].oid, &old_entry->oid);
show_combined_diff(p, 2, revs->dense_combined_merges, revs);
free(p);
return 0;
}
oldmode = old_entry->ce_mode;
if (mode == oldmode && oideq(oid, &old_entry->oid) && !dirty_submodule &&
!revs->diffopt.flags.find_copies_harder)
return 0;
diff_change(&revs->diffopt, oldmode, mode,
&old_entry->oid, oid, 1, !is_null_oid(oid),
old_entry->name, 0, dirty_submodule);
return 0;
}
static void do_oneway_diff(struct unpack_trees_options *o,
const struct cache_entry *idx,
const struct cache_entry *tree)
{
struct rev_info *revs = o->unpack_data;
int match_missing, cached;
if (o->index_only &&
revs->diffopt.ita_invisible_in_index &&
idx && ce_intent_to_add(idx)) {
idx = NULL;
if (!tree)
return; 
}
cached = o->index_only ||
(idx && ((idx->ce_flags & CE_VALID) || ce_skip_worktree(idx)));
match_missing = !revs->ignore_merges;
if (cached && idx && ce_stage(idx)) {
struct diff_filepair *pair;
pair = diff_unmerge(&revs->diffopt, idx->name);
if (tree)
fill_filespec(pair->one, &tree->oid, 1,
tree->ce_mode);
return;
}
if (!tree) {
show_new_file(revs, idx, cached, match_missing);
return;
}
if (!idx) {
diff_index_show_file(revs, "-", tree, &tree->oid, 1,
tree->ce_mode, 0);
return;
}
show_modified(revs, tree, idx, 1, cached, match_missing);
}
static int oneway_diff(const struct cache_entry * const *src,
struct unpack_trees_options *o)
{
const struct cache_entry *idx = src[0];
const struct cache_entry *tree = src[1];
struct rev_info *revs = o->unpack_data;
if (tree == o->df_conflict_entry)
tree = NULL;
if (ce_path_match(revs->diffopt.repo->index,
idx ? idx : tree,
&revs->prune_data, NULL)) {
do_oneway_diff(o, idx, tree);
if (diff_can_quit_early(&revs->diffopt)) {
o->exiting_early = 1;
return -1;
}
}
return 0;
}
static int diff_cache(struct rev_info *revs,
const struct object_id *tree_oid,
const char *tree_name,
int cached)
{
struct tree *tree;
struct tree_desc t;
struct unpack_trees_options opts;
tree = parse_tree_indirect(tree_oid);
if (!tree)
return error("bad tree object %s",
tree_name ? tree_name : oid_to_hex(tree_oid));
memset(&opts, 0, sizeof(opts));
opts.head_idx = 1;
opts.index_only = cached;
opts.diff_index_cached = (cached &&
!revs->diffopt.flags.find_copies_harder);
opts.merge = 1;
opts.fn = oneway_diff;
opts.unpack_data = revs;
opts.src_index = revs->diffopt.repo->index;
opts.dst_index = NULL;
opts.pathspec = &revs->diffopt.pathspec;
opts.pathspec->recursive = 1;
init_tree_desc(&t, tree->buffer, tree->size);
return unpack_trees(1, &t, &opts);
}
int run_diff_index(struct rev_info *revs, int cached)
{
struct object_array_entry *ent;
if (revs->pending.nr != 1)
BUG("run_diff_index must be passed exactly one tree");
trace_performance_enter();
ent = revs->pending.objects;
if (diff_cache(revs, &ent->item->oid, ent->name, cached))
exit(128);
diff_set_mnemonic_prefix(&revs->diffopt, "c/", cached ? "i/" : "w/");
diffcore_fix_diff_index();
diffcore_std(&revs->diffopt);
diff_flush(&revs->diffopt);
trace_performance_leave("diff-index");
return 0;
}
int do_diff_cache(const struct object_id *tree_oid, struct diff_options *opt)
{
struct rev_info revs;
repo_init_revisions(opt->repo, &revs, NULL);
copy_pathspec(&revs.prune_data, &opt->pathspec);
revs.diffopt = *opt;
if (diff_cache(&revs, tree_oid, NULL, 1))
exit(128);
return 0;
}
int index_differs_from(struct repository *r,
const char *def, const struct diff_flags *flags,
int ita_invisible_in_index)
{
struct rev_info rev;
struct setup_revision_opt opt;
repo_init_revisions(r, &rev, NULL);
memset(&opt, 0, sizeof(opt));
opt.def = def;
setup_revisions(0, NULL, &rev, &opt);
rev.diffopt.flags.quick = 1;
rev.diffopt.flags.exit_with_status = 1;
if (flags)
diff_flags_or(&rev.diffopt.flags, flags);
rev.diffopt.ita_invisible_in_index = ita_invisible_in_index;
run_diff_index(&rev, 1);
object_array_clear(&rev.pending);
return (rev.diffopt.flags.has_changes != 0);
}
