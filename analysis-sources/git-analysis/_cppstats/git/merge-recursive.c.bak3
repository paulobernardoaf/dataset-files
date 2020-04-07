




#include "cache.h"
#include "merge-recursive.h"

#include "advice.h"
#include "alloc.h"
#include "attr.h"
#include "blob.h"
#include "builtin.h"
#include "cache-tree.h"
#include "commit.h"
#include "commit-reach.h"
#include "config.h"
#include "diff.h"
#include "diffcore.h"
#include "dir.h"
#include "ll-merge.h"
#include "lockfile.h"
#include "object-store.h"
#include "repository.h"
#include "revision.h"
#include "string-list.h"
#include "submodule.h"
#include "tag.h"
#include "tree-walk.h"
#include "unpack-trees.h"
#include "xdiff-interface.h"

struct merge_options_internal {
int call_depth;
int needed_rename_limit;
struct hashmap current_file_dir_set;
struct string_list df_conflict_file_set;
struct unpack_trees_options unpack_opts;
struct index_state orig_index;
};

struct path_hashmap_entry {
struct hashmap_entry e;
char path[FLEX_ARRAY];
};

static int path_hashmap_cmp(const void *cmp_data,
const struct hashmap_entry *eptr,
const struct hashmap_entry *entry_or_key,
const void *keydata)
{
const struct path_hashmap_entry *a, *b;
const char *key = keydata;

a = container_of(eptr, const struct path_hashmap_entry, e);
b = container_of(entry_or_key, const struct path_hashmap_entry, e);

if (ignore_case)
return strcasecmp(a->path, key ? key : b->path);
else
return strcmp(a->path, key ? key : b->path);
}

static unsigned int path_hash(const char *path)
{
return ignore_case ? strihash(path) : strhash(path);
}











struct dir_rename_entry {
struct hashmap_entry ent;
char *dir;
unsigned non_unique_new_dir:1;
struct strbuf new_dir;
struct string_list possible_new_dirs;
};

static struct dir_rename_entry *dir_rename_find_entry(struct hashmap *hashmap,
char *dir)
{
struct dir_rename_entry key;

if (dir == NULL)
return NULL;
hashmap_entry_init(&key.ent, strhash(dir));
key.dir = dir;
return hashmap_get_entry(hashmap, &key, ent, NULL);
}

static int dir_rename_cmp(const void *unused_cmp_data,
const struct hashmap_entry *eptr,
const struct hashmap_entry *entry_or_key,
const void *unused_keydata)
{
const struct dir_rename_entry *e1, *e2;

e1 = container_of(eptr, const struct dir_rename_entry, ent);
e2 = container_of(entry_or_key, const struct dir_rename_entry, ent);

return strcmp(e1->dir, e2->dir);
}

static void dir_rename_init(struct hashmap *map)
{
hashmap_init(map, dir_rename_cmp, NULL, 0);
}

static void dir_rename_entry_init(struct dir_rename_entry *entry,
char *directory)
{
hashmap_entry_init(&entry->ent, strhash(directory));
entry->dir = directory;
entry->non_unique_new_dir = 0;
strbuf_init(&entry->new_dir, 0);
string_list_init(&entry->possible_new_dirs, 0);
}

struct collision_entry {
struct hashmap_entry ent;
char *target_file;
struct string_list source_files;
unsigned reported_already:1;
};

static struct collision_entry *collision_find_entry(struct hashmap *hashmap,
char *target_file)
{
struct collision_entry key;

hashmap_entry_init(&key.ent, strhash(target_file));
key.target_file = target_file;
return hashmap_get_entry(hashmap, &key, ent, NULL);
}

static int collision_cmp(const void *unused_cmp_data,
const struct hashmap_entry *eptr,
const struct hashmap_entry *entry_or_key,
const void *unused_keydata)
{
const struct collision_entry *e1, *e2;

e1 = container_of(eptr, const struct collision_entry, ent);
e2 = container_of(entry_or_key, const struct collision_entry, ent);

return strcmp(e1->target_file, e2->target_file);
}

static void collision_init(struct hashmap *map)
{
hashmap_init(map, collision_cmp, NULL, 0);
}

static void flush_output(struct merge_options *opt)
{
if (opt->buffer_output < 2 && opt->obuf.len) {
fputs(opt->obuf.buf, stdout);
strbuf_reset(&opt->obuf);
}
}

static int err(struct merge_options *opt, const char *err, ...)
{
va_list params;

if (opt->buffer_output < 2)
flush_output(opt);
else {
strbuf_complete(&opt->obuf, '\n');
strbuf_addstr(&opt->obuf, "error: ");
}
va_start(params, err);
strbuf_vaddf(&opt->obuf, err, params);
va_end(params);
if (opt->buffer_output > 1)
strbuf_addch(&opt->obuf, '\n');
else {
error("%s", opt->obuf.buf);
strbuf_reset(&opt->obuf);
}

return -1;
}

static struct tree *shift_tree_object(struct repository *repo,
struct tree *one, struct tree *two,
const char *subtree_shift)
{
struct object_id shifted;

if (!*subtree_shift) {
shift_tree(repo, &one->object.oid, &two->object.oid, &shifted, 0);
} else {
shift_tree_by(repo, &one->object.oid, &two->object.oid, &shifted,
subtree_shift);
}
if (oideq(&two->object.oid, &shifted))
return two;
return lookup_tree(repo, &shifted);
}

static inline void set_commit_tree(struct commit *c, struct tree *t)
{
c->maybe_tree = t;
}

static struct commit *make_virtual_commit(struct repository *repo,
struct tree *tree,
const char *comment)
{
struct commit *commit = alloc_commit_node(repo);

set_merge_remote_desc(commit, comment, (struct object *)commit);
set_commit_tree(commit, tree);
commit->object.parsed = 1;
return commit;
}

enum rename_type {
RENAME_NORMAL = 0,
RENAME_VIA_DIR,
RENAME_ADD,
RENAME_DELETE,
RENAME_ONE_FILE_TO_ONE,
RENAME_ONE_FILE_TO_TWO,
RENAME_TWO_FILES_TO_ONE
};





struct stage_data {
struct diff_filespec stages[4]; 
struct rename_conflict_info *rename_conflict_info;
unsigned processed:1;
};

struct rename {
unsigned processed:1;
struct diff_filepair *pair;
const char *branch; 





char dir_rename_original_type;
char *dir_rename_original_dest;



















struct stage_data *src_entry;
struct stage_data *dst_entry;
};

struct rename_conflict_info {
enum rename_type rename_type;
struct rename *ren1;
struct rename *ren2;
};

static inline void setup_rename_conflict_info(enum rename_type rename_type,
struct merge_options *opt,
struct rename *ren1,
struct rename *ren2)
{
struct rename_conflict_info *ci;








if (ren2 && ren1->branch != opt->branch1) {
setup_rename_conflict_info(rename_type, opt, ren2, ren1);
return;
}

ci = xcalloc(1, sizeof(struct rename_conflict_info));
ci->rename_type = rename_type;
ci->ren1 = ren1;
ci->ren2 = ren2;

ci->ren1->dst_entry->processed = 0;
ci->ren1->dst_entry->rename_conflict_info = ci;
if (ren2) {
ci->ren2->dst_entry->rename_conflict_info = ci;
}
}

static int show(struct merge_options *opt, int v)
{
return (!opt->priv->call_depth && opt->verbosity >= v) ||
opt->verbosity >= 5;
}

__attribute__((format (printf, 3, 4)))
static void output(struct merge_options *opt, int v, const char *fmt, ...)
{
va_list ap;

if (!show(opt, v))
return;

strbuf_addchars(&opt->obuf, ' ', opt->priv->call_depth * 2);

va_start(ap, fmt);
strbuf_vaddf(&opt->obuf, fmt, ap);
va_end(ap);

strbuf_addch(&opt->obuf, '\n');
if (!opt->buffer_output)
flush_output(opt);
}

static void output_commit_title(struct merge_options *opt, struct commit *commit)
{
struct merge_remote_desc *desc;

strbuf_addchars(&opt->obuf, ' ', opt->priv->call_depth * 2);
desc = merge_remote_util(commit);
if (desc)
strbuf_addf(&opt->obuf, "virtual %s\n", desc->name);
else {
strbuf_add_unique_abbrev(&opt->obuf, &commit->object.oid,
DEFAULT_ABBREV);
strbuf_addch(&opt->obuf, ' ');
if (parse_commit(commit) != 0)
strbuf_addstr(&opt->obuf, _("(bad commit)\n"));
else {
const char *title;
const char *msg = get_commit_buffer(commit, NULL);
int len = find_commit_subject(msg, &title);
if (len)
strbuf_addf(&opt->obuf, "%.*s\n", len, title);
unuse_commit_buffer(commit, msg);
}
}
flush_output(opt);
}

static int add_cacheinfo(struct merge_options *opt,
const struct diff_filespec *blob,
const char *path, int stage, int refresh, int options)
{
struct index_state *istate = opt->repo->index;
struct cache_entry *ce;
int ret;

ce = make_cache_entry(istate, blob->mode, &blob->oid, path, stage, 0);
if (!ce)
return err(opt, _("add_cacheinfo failed for path '%s'; merge aborting."), path);

ret = add_index_entry(istate, ce, options);
if (refresh) {
struct cache_entry *nce;

nce = refresh_cache_entry(istate, ce,
CE_MATCH_REFRESH | CE_MATCH_IGNORE_MISSING);
if (!nce)
return err(opt, _("add_cacheinfo failed to refresh for path '%s'; merge aborting."), path);
if (nce != ce)
ret = add_index_entry(istate, nce, options);
}
return ret;
}

static inline int merge_detect_rename(struct merge_options *opt)
{
return (opt->detect_renames >= 0) ? opt->detect_renames : 1;
}

static void init_tree_desc_from_tree(struct tree_desc *desc, struct tree *tree)
{
parse_tree(tree);
init_tree_desc(desc, tree->buffer, tree->size);
}

static int unpack_trees_start(struct merge_options *opt,
struct tree *common,
struct tree *head,
struct tree *merge)
{
int rc;
struct tree_desc t[3];
struct index_state tmp_index = { NULL };

memset(&opt->priv->unpack_opts, 0, sizeof(opt->priv->unpack_opts));
if (opt->priv->call_depth)
opt->priv->unpack_opts.index_only = 1;
else
opt->priv->unpack_opts.update = 1;
opt->priv->unpack_opts.merge = 1;
opt->priv->unpack_opts.head_idx = 2;
opt->priv->unpack_opts.fn = threeway_merge;
opt->priv->unpack_opts.src_index = opt->repo->index;
opt->priv->unpack_opts.dst_index = &tmp_index;
opt->priv->unpack_opts.aggressive = !merge_detect_rename(opt);
setup_unpack_trees_porcelain(&opt->priv->unpack_opts, "merge");

init_tree_desc_from_tree(t+0, common);
init_tree_desc_from_tree(t+1, head);
init_tree_desc_from_tree(t+2, merge);

rc = unpack_trees(3, t, &opt->priv->unpack_opts);
cache_tree_free(&opt->repo->index->cache_tree);







opt->priv->orig_index = *opt->repo->index;
*opt->repo->index = tmp_index;
opt->priv->unpack_opts.src_index = &opt->priv->orig_index;

return rc;
}

static void unpack_trees_finish(struct merge_options *opt)
{
discard_index(&opt->priv->orig_index);
clear_unpack_trees_porcelain(&opt->priv->unpack_opts);
}

static int save_files_dirs(const struct object_id *oid,
struct strbuf *base, const char *path,
unsigned int mode, int stage, void *context)
{
struct path_hashmap_entry *entry;
int baselen = base->len;
struct merge_options *opt = context;

strbuf_addstr(base, path);

FLEX_ALLOC_MEM(entry, path, base->buf, base->len);
hashmap_entry_init(&entry->e, path_hash(entry->path));
hashmap_add(&opt->priv->current_file_dir_set, &entry->e);

strbuf_setlen(base, baselen);
return (S_ISDIR(mode) ? READ_TREE_RECURSIVE : 0);
}

static void get_files_dirs(struct merge_options *opt, struct tree *tree)
{
struct pathspec match_all;
memset(&match_all, 0, sizeof(match_all));
read_tree_recursive(opt->repo, tree, "", 0, 0,
&match_all, save_files_dirs, opt);
}

static int get_tree_entry_if_blob(struct repository *r,
const struct object_id *tree,
const char *path,
struct diff_filespec *dfs)
{
int ret;

ret = get_tree_entry(r, tree, path, &dfs->oid, &dfs->mode);
if (S_ISDIR(dfs->mode)) {
oidcpy(&dfs->oid, &null_oid);
dfs->mode = 0;
}
return ret;
}





static struct stage_data *insert_stage_data(struct repository *r,
const char *path,
struct tree *o, struct tree *a, struct tree *b,
struct string_list *entries)
{
struct string_list_item *item;
struct stage_data *e = xcalloc(1, sizeof(struct stage_data));
get_tree_entry_if_blob(r, &o->object.oid, path, &e->stages[1]);
get_tree_entry_if_blob(r, &a->object.oid, path, &e->stages[2]);
get_tree_entry_if_blob(r, &b->object.oid, path, &e->stages[3]);
item = string_list_insert(entries, path);
item->util = e;
return e;
}





static struct string_list *get_unmerged(struct index_state *istate)
{
struct string_list *unmerged = xcalloc(1, sizeof(struct string_list));
int i;

unmerged->strdup_strings = 1;

for (i = 0; i < istate->cache_nr; i++) {
struct string_list_item *item;
struct stage_data *e;
const struct cache_entry *ce = istate->cache[i];
if (!ce_stage(ce))
continue;

item = string_list_lookup(unmerged, ce->name);
if (!item) {
item = string_list_insert(unmerged, ce->name);
item->util = xcalloc(1, sizeof(struct stage_data));
}
e = item->util;
e->stages[ce_stage(ce)].mode = ce->ce_mode;
oidcpy(&e->stages[ce_stage(ce)].oid, &ce->oid);
}

return unmerged;
}

static int string_list_df_name_compare(const char *one, const char *two)
{
int onelen = strlen(one);
int twolen = strlen(two);












int cmp = df_name_compare(one, onelen, S_IFDIR,
two, twolen, S_IFDIR);




if (cmp)
return cmp;
return onelen - twolen;
}

static void record_df_conflict_files(struct merge_options *opt,
struct string_list *entries)
{













struct string_list df_sorted_entries = STRING_LIST_INIT_NODUP;
const char *last_file = NULL;
int last_len = 0;
int i;





if (opt->priv->call_depth)
return;


for (i = 0; i < entries->nr; i++) {
struct string_list_item *next = &entries->items[i];
string_list_append(&df_sorted_entries, next->string)->util =
next->util;
}
df_sorted_entries.cmp = string_list_df_name_compare;
string_list_sort(&df_sorted_entries);

string_list_clear(&opt->priv->df_conflict_file_set, 1);
for (i = 0; i < df_sorted_entries.nr; i++) {
const char *path = df_sorted_entries.items[i].string;
int len = strlen(path);
struct stage_data *e = df_sorted_entries.items[i].util;







if (last_file &&
len > last_len &&
memcmp(path, last_file, last_len) == 0 &&
path[last_len] == '/') {
string_list_insert(&opt->priv->df_conflict_file_set, last_file);
}







if (S_ISREG(e->stages[2].mode) || S_ISLNK(e->stages[2].mode)) {
last_file = path;
last_len = len;
} else {
last_file = NULL;
}
}
string_list_clear(&df_sorted_entries, 0);
}

static int update_stages(struct merge_options *opt, const char *path,
const struct diff_filespec *o,
const struct diff_filespec *a,
const struct diff_filespec *b)
{









int clear = 1;
int options = ADD_CACHE_OK_TO_ADD | ADD_CACHE_SKIP_DFCHECK;
if (clear)
if (remove_file_from_index(opt->repo->index, path))
return -1;
if (o)
if (add_cacheinfo(opt, o, path, 1, 0, options))
return -1;
if (a)
if (add_cacheinfo(opt, a, path, 2, 0, options))
return -1;
if (b)
if (add_cacheinfo(opt, b, path, 3, 0, options))
return -1;
return 0;
}

static void update_entry(struct stage_data *entry,
struct diff_filespec *o,
struct diff_filespec *a,
struct diff_filespec *b)
{
entry->processed = 0;
entry->stages[1].mode = o->mode;
entry->stages[2].mode = a->mode;
entry->stages[3].mode = b->mode;
oidcpy(&entry->stages[1].oid, &o->oid);
oidcpy(&entry->stages[2].oid, &a->oid);
oidcpy(&entry->stages[3].oid, &b->oid);
}

static int remove_file(struct merge_options *opt, int clean,
const char *path, int no_wd)
{
int update_cache = opt->priv->call_depth || clean;
int update_working_directory = !opt->priv->call_depth && !no_wd;

if (update_cache) {
if (remove_file_from_index(opt->repo->index, path))
return -1;
}
if (update_working_directory) {
if (ignore_case) {
struct cache_entry *ce;
ce = index_file_exists(opt->repo->index, path, strlen(path),
ignore_case);
if (ce && ce_stage(ce) == 0 && strcmp(path, ce->name))
return 0;
}
if (remove_path(path))
return -1;
}
return 0;
}


static void add_flattened_path(struct strbuf *out, const char *s)
{
size_t i = out->len;
strbuf_addstr(out, s);
for (; i < out->len; i++)
if (out->buf[i] == '/')
out->buf[i] = '_';
}

static char *unique_path(struct merge_options *opt,
const char *path,
const char *branch)
{
struct path_hashmap_entry *entry;
struct strbuf newpath = STRBUF_INIT;
int suffix = 0;
size_t base_len;

strbuf_addf(&newpath, "%s~", path);
add_flattened_path(&newpath, branch);

base_len = newpath.len;
while (hashmap_get_from_hash(&opt->priv->current_file_dir_set,
path_hash(newpath.buf), newpath.buf) ||
(!opt->priv->call_depth && file_exists(newpath.buf))) {
strbuf_setlen(&newpath, base_len);
strbuf_addf(&newpath, "_%d", suffix++);
}

FLEX_ALLOC_MEM(entry, path, newpath.buf, newpath.len);
hashmap_entry_init(&entry->e, path_hash(entry->path));
hashmap_add(&opt->priv->current_file_dir_set, &entry->e);
return strbuf_detach(&newpath, NULL);
}







static int dir_in_way(struct index_state *istate, const char *path,
int check_working_copy, int empty_ok)
{
int pos;
struct strbuf dirpath = STRBUF_INIT;
struct stat st;

strbuf_addstr(&dirpath, path);
strbuf_addch(&dirpath, '/');

pos = index_name_pos(istate, dirpath.buf, dirpath.len);

if (pos < 0)
pos = -1 - pos;
if (pos < istate->cache_nr &&
!strncmp(dirpath.buf, istate->cache[pos]->name, dirpath.len)) {
strbuf_release(&dirpath);
return 1;
}

strbuf_release(&dirpath);
return check_working_copy && !lstat(path, &st) && S_ISDIR(st.st_mode) &&
!(empty_ok && is_empty_dir(path)) &&
!has_symlink_leading_path(path, strlen(path));
}





static int was_tracked_and_matches(struct merge_options *opt, const char *path,
const struct diff_filespec *blob)
{
int pos = index_name_pos(&opt->priv->orig_index, path, strlen(path));
struct cache_entry *ce;

if (0 > pos)

return 0;


ce = opt->priv->orig_index.cache[pos];
return (oideq(&ce->oid, &blob->oid) && ce->ce_mode == blob->mode);
}




static int was_tracked(struct merge_options *opt, const char *path)
{
int pos = index_name_pos(&opt->priv->orig_index, path, strlen(path));

if (0 <= pos)

return 1;

return 0;
}

static int would_lose_untracked(struct merge_options *opt, const char *path)
{
struct index_state *istate = opt->repo->index;


















int pos = index_name_pos(istate, path, strlen(path));

if (pos < 0)
pos = -1 - pos;
while (pos < istate->cache_nr &&
!strcmp(path, istate->cache[pos]->name)) {






switch (ce_stage(istate->cache[pos])) {
case 0:
case 2:
return 0;
}
pos++;
}
return file_exists(path);
}

static int was_dirty(struct merge_options *opt, const char *path)
{
struct cache_entry *ce;
int dirty = 1;

if (opt->priv->call_depth || !was_tracked(opt, path))
return !dirty;

ce = index_file_exists(opt->priv->unpack_opts.src_index,
path, strlen(path), ignore_case);
dirty = verify_uptodate(ce, &opt->priv->unpack_opts) != 0;
return dirty;
}

static int make_room_for_path(struct merge_options *opt, const char *path)
{
int status, i;
const char *msg = _("failed to create path '%s'%s");


for (i = 0; i < opt->priv->df_conflict_file_set.nr; i++) {
const char *df_path = opt->priv->df_conflict_file_set.items[i].string;
size_t pathlen = strlen(path);
size_t df_pathlen = strlen(df_path);
if (df_pathlen < pathlen &&
path[df_pathlen] == '/' &&
strncmp(path, df_path, df_pathlen) == 0) {
output(opt, 3,
_("Removing %s to make room for subdirectory\n"),
df_path);
unlink(df_path);
unsorted_string_list_delete_item(&opt->priv->df_conflict_file_set,
i, 0);
break;
}
}


status = safe_create_leading_directories_const(path);
if (status) {
if (status == SCLD_EXISTS)

return err(opt, msg, path, _(": perhaps a D/F conflict?"));
return err(opt, msg, path, "");
}





if (would_lose_untracked(opt, path))
return err(opt, _("refusing to lose untracked file at '%s'"),
path);


if (!unlink(path))
return 0;

if (errno == ENOENT)
return 0;

return err(opt, msg, path, _(": perhaps a D/F conflict?"));
}

static int update_file_flags(struct merge_options *opt,
const struct diff_filespec *contents,
const char *path,
int update_cache,
int update_wd)
{
int ret = 0;

if (opt->priv->call_depth)
update_wd = 0;

if (update_wd) {
enum object_type type;
void *buf;
unsigned long size;

if (S_ISGITLINK(contents->mode)) {





update_wd = 0;
goto update_index;
}

buf = read_object_file(&contents->oid, &type, &size);
if (!buf) {
ret = err(opt, _("cannot read object %s '%s'"),
oid_to_hex(&contents->oid), path);
goto free_buf;
}
if (type != OBJ_BLOB) {
ret = err(opt, _("blob expected for %s '%s'"),
oid_to_hex(&contents->oid), path);
goto free_buf;
}
if (S_ISREG(contents->mode)) {
struct strbuf strbuf = STRBUF_INIT;
if (convert_to_working_tree(opt->repo->index,
path, buf, size, &strbuf, NULL)) {
free(buf);
size = strbuf.len;
buf = strbuf_detach(&strbuf, NULL);
}
}

if (make_room_for_path(opt, path) < 0) {
update_wd = 0;
goto free_buf;
}
if (S_ISREG(contents->mode) ||
(!has_symlinks && S_ISLNK(contents->mode))) {
int fd;
int mode = (contents->mode & 0100 ? 0777 : 0666);

fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, mode);
if (fd < 0) {
ret = err(opt, _("failed to open '%s': %s"),
path, strerror(errno));
goto free_buf;
}
write_in_full(fd, buf, size);
close(fd);
} else if (S_ISLNK(contents->mode)) {
char *lnk = xmemdupz(buf, size);
safe_create_leading_directories_const(path);
unlink(path);
if (symlink(lnk, path))
ret = err(opt, _("failed to symlink '%s': %s"),
path, strerror(errno));
free(lnk);
} else
ret = err(opt,
_("do not know what to do with %06o %s '%s'"),
contents->mode, oid_to_hex(&contents->oid), path);
free_buf:
free(buf);
}
update_index:
if (!ret && update_cache) {
int refresh = (!opt->priv->call_depth &&
contents->mode != S_IFGITLINK);
if (add_cacheinfo(opt, contents, path, 0, refresh,
ADD_CACHE_OK_TO_ADD))
return -1;
}
return ret;
}

static int update_file(struct merge_options *opt,
int clean,
const struct diff_filespec *contents,
const char *path)
{
return update_file_flags(opt, contents, path,
opt->priv->call_depth || clean, !opt->priv->call_depth);
}



struct merge_file_info {
struct diff_filespec blob; 
unsigned clean:1,
merge:1;
};

static int merge_3way(struct merge_options *opt,
mmbuffer_t *result_buf,
const struct diff_filespec *o,
const struct diff_filespec *a,
const struct diff_filespec *b,
const char *branch1,
const char *branch2,
const int extra_marker_size)
{
mmfile_t orig, src1, src2;
struct ll_merge_options ll_opts = {0};
char *base, *name1, *name2;
int merge_status;

ll_opts.renormalize = opt->renormalize;
ll_opts.extra_marker_size = extra_marker_size;
ll_opts.xdl_opts = opt->xdl_opts;

if (opt->priv->call_depth) {
ll_opts.virtual_ancestor = 1;
ll_opts.variant = 0;
} else {
switch (opt->recursive_variant) {
case MERGE_VARIANT_OURS:
ll_opts.variant = XDL_MERGE_FAVOR_OURS;
break;
case MERGE_VARIANT_THEIRS:
ll_opts.variant = XDL_MERGE_FAVOR_THEIRS;
break;
default:
ll_opts.variant = 0;
break;
}
}

assert(a->path && b->path && o->path && opt->ancestor);
if (strcmp(a->path, b->path) || strcmp(a->path, o->path) != 0) {
base = mkpathdup("%s:%s", opt->ancestor, o->path);
name1 = mkpathdup("%s:%s", branch1, a->path);
name2 = mkpathdup("%s:%s", branch2, b->path);
} else {
base = mkpathdup("%s", opt->ancestor);
name1 = mkpathdup("%s", branch1);
name2 = mkpathdup("%s", branch2);
}

read_mmblob(&orig, &o->oid);
read_mmblob(&src1, &a->oid);
read_mmblob(&src2, &b->oid);

merge_status = ll_merge(result_buf, a->path, &orig, base,
&src1, name1, &src2, name2,
opt->repo->index, &ll_opts);

free(base);
free(name1);
free(name2);
free(orig.ptr);
free(src1.ptr);
free(src2.ptr);
return merge_status;
}

static int find_first_merges(struct repository *repo,
struct object_array *result, const char *path,
struct commit *a, struct commit *b)
{
int i, j;
struct object_array merges = OBJECT_ARRAY_INIT;
struct commit *commit;
int contains_another;

char merged_revision[GIT_MAX_HEXSZ + 2];
const char *rev_args[] = { "rev-list", "--merges", "--ancestry-path",
"--all", merged_revision, NULL };
struct rev_info revs;
struct setup_revision_opt rev_opts;

memset(result, 0, sizeof(struct object_array));
memset(&rev_opts, 0, sizeof(rev_opts));


xsnprintf(merged_revision, sizeof(merged_revision), "^%s",
oid_to_hex(&a->object.oid));
repo_init_revisions(repo, &revs, NULL);
rev_opts.submodule = path;

revs.single_worktree = path != NULL;
setup_revisions(ARRAY_SIZE(rev_args)-1, rev_args, &revs, &rev_opts);


if (prepare_revision_walk(&revs))
die("revision walk setup failed");
while ((commit = get_revision(&revs)) != NULL) {
struct object *o = &(commit->object);
if (in_merge_bases(b, commit))
add_object_array(o, NULL, &merges);
}
reset_revision_walk();





for (i = 0; i < merges.nr; i++) {
struct commit *m1 = (struct commit *) merges.objects[i].item;

contains_another = 0;
for (j = 0; j < merges.nr; j++) {
struct commit *m2 = (struct commit *) merges.objects[j].item;
if (i != j && in_merge_bases(m2, m1)) {
contains_another = 1;
break;
}
}

if (!contains_another)
add_object_array(merges.objects[i].item, NULL, result);
}

object_array_clear(&merges);
return result->nr;
}

static void print_commit(struct commit *commit)
{
struct strbuf sb = STRBUF_INIT;
struct pretty_print_context ctx = {0};
ctx.date_mode.type = DATE_NORMAL;
format_commit_message(commit, " %h: %m %s", &sb, &ctx);
fprintf(stderr, "%s\n", sb.buf);
strbuf_release(&sb);
}

static int is_valid(const struct diff_filespec *dfs)
{
return dfs->mode != 0 && !is_null_oid(&dfs->oid);
}

static int merge_submodule(struct merge_options *opt,
struct object_id *result, const char *path,
const struct object_id *base, const struct object_id *a,
const struct object_id *b)
{
struct commit *commit_base, *commit_a, *commit_b;
int parent_count;
struct object_array merges;

int i;
int search = !opt->priv->call_depth;


oidcpy(result, a);


if (is_null_oid(base))
return 0;
if (is_null_oid(a))
return 0;
if (is_null_oid(b))
return 0;

if (add_submodule_odb(path)) {
output(opt, 1, _("Failed to merge submodule %s (not checked out)"), path);
return 0;
}

if (!(commit_base = lookup_commit_reference(opt->repo, base)) ||
!(commit_a = lookup_commit_reference(opt->repo, a)) ||
!(commit_b = lookup_commit_reference(opt->repo, b))) {
output(opt, 1, _("Failed to merge submodule %s (commits not present)"), path);
return 0;
}


if (!in_merge_bases(commit_base, commit_a) ||
!in_merge_bases(commit_base, commit_b)) {
output(opt, 1, _("Failed to merge submodule %s (commits don't follow merge-base)"), path);
return 0;
}


if (in_merge_bases(commit_a, commit_b)) {
oidcpy(result, b);
if (show(opt, 3)) {
output(opt, 3, _("Fast-forwarding submodule %s to the following commit:"), path);
output_commit_title(opt, commit_b);
} else if (show(opt, 2))
output(opt, 2, _("Fast-forwarding submodule %s"), path);
else
; 

return 1;
}
if (in_merge_bases(commit_b, commit_a)) {
oidcpy(result, a);
if (show(opt, 3)) {
output(opt, 3, _("Fast-forwarding submodule %s to the following commit:"), path);
output_commit_title(opt, commit_a);
} else if (show(opt, 2))
output(opt, 2, _("Fast-forwarding submodule %s"), path);
else
; 

return 1;
}









if (!search)
return 0;


parent_count = find_first_merges(opt->repo, &merges, path,
commit_a, commit_b);
switch (parent_count) {
case 0:
output(opt, 1, _("Failed to merge submodule %s (merge following commits not found)"), path);
break;

case 1:
output(opt, 1, _("Failed to merge submodule %s (not fast-forward)"), path);
output(opt, 2, _("Found a possible merge resolution for the submodule:\n"));
print_commit((struct commit *) merges.objects[0].item);
output(opt, 2, _(
"If this is correct simply add it to the index "
"for example\n"
"by using:\n\n"
" git update-index --cacheinfo 160000 %s \"%s\"\n\n"
"which will accept this suggestion.\n"),
oid_to_hex(&merges.objects[0].item->oid), path);
break;

default:
output(opt, 1, _("Failed to merge submodule %s (multiple merges found)"), path);
for (i = 0; i < merges.nr; i++)
print_commit((struct commit *) merges.objects[i].item);
}

object_array_clear(&merges);
return 0;
}

static int merge_mode_and_contents(struct merge_options *opt,
const struct diff_filespec *o,
const struct diff_filespec *a,
const struct diff_filespec *b,
const char *filename,
const char *branch1,
const char *branch2,
const int extra_marker_size,
struct merge_file_info *result)
{
if (opt->branch1 != branch1) {





return merge_mode_and_contents(opt, o, b, a,
filename,
branch2, branch1,
extra_marker_size, result);
}

result->merge = 0;
result->clean = 1;

if ((S_IFMT & a->mode) != (S_IFMT & b->mode)) {
result->clean = 0;
if (S_ISREG(a->mode)) {
result->blob.mode = a->mode;
oidcpy(&result->blob.oid, &a->oid);
} else {
result->blob.mode = b->mode;
oidcpy(&result->blob.oid, &b->oid);
}
} else {
if (!oideq(&a->oid, &o->oid) && !oideq(&b->oid, &o->oid))
result->merge = 1;




if (a->mode == b->mode || a->mode == o->mode)
result->blob.mode = b->mode;
else {
result->blob.mode = a->mode;
if (b->mode != o->mode) {
result->clean = 0;
result->merge = 1;
}
}

if (oideq(&a->oid, &b->oid) || oideq(&a->oid, &o->oid))
oidcpy(&result->blob.oid, &b->oid);
else if (oideq(&b->oid, &o->oid))
oidcpy(&result->blob.oid, &a->oid);
else if (S_ISREG(a->mode)) {
mmbuffer_t result_buf;
int ret = 0, merge_status;

merge_status = merge_3way(opt, &result_buf, o, a, b,
branch1, branch2,
extra_marker_size);

if ((merge_status < 0) || !result_buf.ptr)
ret = err(opt, _("Failed to execute internal merge"));

if (!ret &&
write_object_file(result_buf.ptr, result_buf.size,
blob_type, &result->blob.oid))
ret = err(opt, _("Unable to add %s to database"),
a->path);

free(result_buf.ptr);
if (ret)
return ret;
result->clean = (merge_status == 0);
} else if (S_ISGITLINK(a->mode)) {
result->clean = merge_submodule(opt, &result->blob.oid,
o->path,
&o->oid,
&a->oid,
&b->oid);
} else if (S_ISLNK(a->mode)) {
switch (opt->recursive_variant) {
case MERGE_VARIANT_NORMAL:
oidcpy(&result->blob.oid, &a->oid);
if (!oideq(&a->oid, &b->oid))
result->clean = 0;
break;
case MERGE_VARIANT_OURS:
oidcpy(&result->blob.oid, &a->oid);
break;
case MERGE_VARIANT_THEIRS:
oidcpy(&result->blob.oid, &b->oid);
break;
}
} else
BUG("unsupported object type in the tree");
}

if (result->merge)
output(opt, 2, _("Auto-merging %s"), filename);

return 0;
}

static int handle_rename_via_dir(struct merge_options *opt,
struct rename_conflict_info *ci)
{






const struct rename *ren = ci->ren1;
const struct diff_filespec *dest = ren->pair->two;
char *file_path = dest->path;
int mark_conflicted = (opt->detect_directory_renames ==
MERGE_DIRECTORY_RENAMES_CONFLICT);
assert(ren->dir_rename_original_dest);

if (!opt->priv->call_depth && would_lose_untracked(opt, dest->path)) {
mark_conflicted = 1;
file_path = unique_path(opt, dest->path, ren->branch);
output(opt, 1, _("Error: Refusing to lose untracked file at %s; "
"writing to %s instead."),
dest->path, file_path);
}

if (mark_conflicted) {





if (update_file(opt, 0, dest, file_path))
return -1;
if (file_path != dest->path)
free(file_path);
if (update_stages(opt, dest->path, NULL,
ren->branch == opt->branch1 ? dest : NULL,
ren->branch == opt->branch1 ? NULL : dest))
return -1;
return 0; 
} else {

if (update_file(opt, 1, dest, dest->path))
return -1;
return 1; 
}
}

static int handle_change_delete(struct merge_options *opt,
const char *path, const char *old_path,
const struct diff_filespec *o,
const struct diff_filespec *changed,
const char *change_branch,
const char *delete_branch,
const char *change, const char *change_past)
{
char *alt_path = NULL;
const char *update_path = path;
int ret = 0;

if (dir_in_way(opt->repo->index, path, !opt->priv->call_depth, 0) ||
(!opt->priv->call_depth && would_lose_untracked(opt, path))) {
update_path = alt_path = unique_path(opt, path, change_branch);
}

if (opt->priv->call_depth) {





ret = remove_file_from_index(opt->repo->index, path);
if (!ret)
ret = update_file(opt, 0, o, update_path);
} else {















if (!alt_path) {
if (!old_path) {
output(opt, 1, _("CONFLICT (%s/delete): %s deleted in %s "
"and %s in %s. Version %s of %s left in tree."),
change, path, delete_branch, change_past,
change_branch, change_branch, path);
} else {
output(opt, 1, _("CONFLICT (%s/delete): %s deleted in %s "
"and %s to %s in %s. Version %s of %s left in tree."),
change, old_path, delete_branch, change_past, path,
change_branch, change_branch, path);
}
} else {
if (!old_path) {
output(opt, 1, _("CONFLICT (%s/delete): %s deleted in %s "
"and %s in %s. Version %s of %s left in tree at %s."),
change, path, delete_branch, change_past,
change_branch, change_branch, path, alt_path);
} else {
output(opt, 1, _("CONFLICT (%s/delete): %s deleted in %s "
"and %s to %s in %s. Version %s of %s left in tree at %s."),
change, old_path, delete_branch, change_past, path,
change_branch, change_branch, path, alt_path);
}
}






if (change_branch != opt->branch1 || alt_path)
ret = update_file(opt, 0, changed, update_path);
}
free(alt_path);

return ret;
}

static int handle_rename_delete(struct merge_options *opt,
struct rename_conflict_info *ci)
{
const struct rename *ren = ci->ren1;
const struct diff_filespec *orig = ren->pair->one;
const struct diff_filespec *dest = ren->pair->two;
const char *rename_branch = ren->branch;
const char *delete_branch = (opt->branch1 == ren->branch ?
opt->branch2 : opt->branch1);

if (handle_change_delete(opt,
opt->priv->call_depth ? orig->path : dest->path,
opt->priv->call_depth ? NULL : orig->path,
orig, dest,
rename_branch, delete_branch,
_("rename"), _("renamed")))
return -1;

if (opt->priv->call_depth)
return remove_file_from_index(opt->repo->index, dest->path);
else
return update_stages(opt, dest->path, NULL,
rename_branch == opt->branch1 ? dest : NULL,
rename_branch == opt->branch1 ? NULL : dest);
}

static int handle_file_collision(struct merge_options *opt,
const char *collide_path,
const char *prev_path1,
const char *prev_path2,
const char *branch1, const char *branch2,
struct diff_filespec *a,
struct diff_filespec *b)
{
struct merge_file_info mfi;
struct diff_filespec null;
char *alt_path = NULL;
const char *update_path = collide_path;







if (branch1 != opt->branch1) {
return handle_file_collision(opt, collide_path,
prev_path2, prev_path1,
branch2, branch1,
b, a);
}


if (prev_path1)
remove_file(opt, 1, prev_path1,
opt->priv->call_depth || would_lose_untracked(opt, prev_path1));
if (prev_path2)
remove_file(opt, 1, prev_path2,
opt->priv->call_depth || would_lose_untracked(opt, prev_path2));






if (was_dirty(opt, collide_path)) {
output(opt, 1, _("Refusing to lose dirty file at %s"),
collide_path);
update_path = alt_path = unique_path(opt, collide_path, "merged");
} else if (would_lose_untracked(opt, collide_path)) {






output(opt, 1, _("Refusing to lose untracked file at "
"%s, even though it's in the way."),
collide_path);
update_path = alt_path = unique_path(opt, collide_path, "merged");
} else {










remove_file(opt, 0, collide_path, 0);
}


null.path = (char *)collide_path;
oidcpy(&null.oid, &null_oid);
null.mode = 0;

if (merge_mode_and_contents(opt, &null, a, b, collide_path,
branch1, branch2, opt->priv->call_depth * 2, &mfi))
return -1;
mfi.clean &= !alt_path;
if (update_file(opt, mfi.clean, &mfi.blob, update_path))
return -1;
if (!mfi.clean && !opt->priv->call_depth &&
update_stages(opt, collide_path, NULL, a, b))
return -1;
free(alt_path);







return mfi.clean;
}

static int handle_rename_add(struct merge_options *opt,
struct rename_conflict_info *ci)
{

struct diff_filespec *a = ci->ren1->pair->one;
struct diff_filespec *c = ci->ren1->pair->two;
char *path = c->path;
char *prev_path_desc;
struct merge_file_info mfi;

const char *rename_branch = ci->ren1->branch;
const char *add_branch = (opt->branch1 == rename_branch ?
opt->branch2 : opt->branch1);
int other_stage = (ci->ren1->branch == opt->branch1 ? 3 : 2);

output(opt, 1, _("CONFLICT (rename/add): "
"Rename %s->%s in %s. Added %s in %s"),
a->path, c->path, rename_branch,
c->path, add_branch);

prev_path_desc = xstrfmt("version of %s from %s", path, a->path);
ci->ren1->src_entry->stages[other_stage].path = a->path;
if (merge_mode_and_contents(opt, a, c,
&ci->ren1->src_entry->stages[other_stage],
prev_path_desc,
opt->branch1, opt->branch2,
1 + opt->priv->call_depth * 2, &mfi))
return -1;
free(prev_path_desc);

ci->ren1->dst_entry->stages[other_stage].path = mfi.blob.path = c->path;
return handle_file_collision(opt,
c->path, a->path, NULL,
rename_branch, add_branch,
&mfi.blob,
&ci->ren1->dst_entry->stages[other_stage]);
}

static char *find_path_for_conflict(struct merge_options *opt,
const char *path,
const char *branch1,
const char *branch2)
{
char *new_path = NULL;
if (dir_in_way(opt->repo->index, path, !opt->priv->call_depth, 0)) {
new_path = unique_path(opt, path, branch1);
output(opt, 1, _("%s is a directory in %s adding "
"as %s instead"),
path, branch2, new_path);
} else if (would_lose_untracked(opt, path)) {
new_path = unique_path(opt, path, branch1);
output(opt, 1, _("Refusing to lose untracked file"
" at %s; adding as %s instead"),
path, new_path);
}

return new_path;
}




static inline int flip_stage(int stage)
{
return (2 + 3) - stage;
}

static int handle_rename_rename_1to2(struct merge_options *opt,
struct rename_conflict_info *ci)
{

struct merge_file_info mfi;
struct diff_filespec *add;
struct diff_filespec *o = ci->ren1->pair->one;
struct diff_filespec *a = ci->ren1->pair->two;
struct diff_filespec *b = ci->ren2->pair->two;
char *path_desc;

output(opt, 1, _("CONFLICT (rename/rename): "
"Rename \"%s\"->\"%s\" in branch \"%s\" "
"rename \"%s\"->\"%s\" in \"%s\"%s"),
o->path, a->path, ci->ren1->branch,
o->path, b->path, ci->ren2->branch,
opt->priv->call_depth ? _(" (left unresolved)") : "");

path_desc = xstrfmt("%s and %s, both renamed from %s",
a->path, b->path, o->path);
if (merge_mode_and_contents(opt, o, a, b, path_desc,
ci->ren1->branch, ci->ren2->branch,
opt->priv->call_depth * 2, &mfi))
return -1;
free(path_desc);

if (opt->priv->call_depth)
remove_file_from_index(opt->repo->index, o->path);






add = &ci->ren1->dst_entry->stages[flip_stage(2)];
if (is_valid(add)) {
add->path = mfi.blob.path = a->path;
if (handle_file_collision(opt, a->path,
NULL, NULL,
ci->ren1->branch,
ci->ren2->branch,
&mfi.blob, add) < 0)
return -1;
} else {
char *new_path = find_path_for_conflict(opt, a->path,
ci->ren1->branch,
ci->ren2->branch);
if (update_file(opt, 0, &mfi.blob,
new_path ? new_path : a->path))
return -1;
free(new_path);
if (!opt->priv->call_depth &&
update_stages(opt, a->path, NULL, a, NULL))
return -1;
}

add = &ci->ren2->dst_entry->stages[flip_stage(3)];
if (is_valid(add)) {
add->path = mfi.blob.path = b->path;
if (handle_file_collision(opt, b->path,
NULL, NULL,
ci->ren1->branch,
ci->ren2->branch,
add, &mfi.blob) < 0)
return -1;
} else {
char *new_path = find_path_for_conflict(opt, b->path,
ci->ren2->branch,
ci->ren1->branch);
if (update_file(opt, 0, &mfi.blob,
new_path ? new_path : b->path))
return -1;
free(new_path);
if (!opt->priv->call_depth &&
update_stages(opt, b->path, NULL, NULL, b))
return -1;
}

return 0;
}

static int handle_rename_rename_2to1(struct merge_options *opt,
struct rename_conflict_info *ci)
{

struct diff_filespec *a = ci->ren1->pair->one;
struct diff_filespec *b = ci->ren2->pair->one;
struct diff_filespec *c1 = ci->ren1->pair->two;
struct diff_filespec *c2 = ci->ren2->pair->two;
char *path = c1->path; 
char *path_side_1_desc;
char *path_side_2_desc;
struct merge_file_info mfi_c1;
struct merge_file_info mfi_c2;
int ostage1, ostage2;

output(opt, 1, _("CONFLICT (rename/rename): "
"Rename %s->%s in %s. "
"Rename %s->%s in %s"),
a->path, c1->path, ci->ren1->branch,
b->path, c2->path, ci->ren2->branch);

path_side_1_desc = xstrfmt("version of %s from %s", path, a->path);
path_side_2_desc = xstrfmt("version of %s from %s", path, b->path);
ostage1 = ci->ren1->branch == opt->branch1 ? 3 : 2;
ostage2 = flip_stage(ostage1);
ci->ren1->src_entry->stages[ostage1].path = a->path;
ci->ren2->src_entry->stages[ostage2].path = b->path;
if (merge_mode_and_contents(opt, a, c1,
&ci->ren1->src_entry->stages[ostage1],
path_side_1_desc,
opt->branch1, opt->branch2,
1 + opt->priv->call_depth * 2, &mfi_c1) ||
merge_mode_and_contents(opt, b,
&ci->ren2->src_entry->stages[ostage2],
c2, path_side_2_desc,
opt->branch1, opt->branch2,
1 + opt->priv->call_depth * 2, &mfi_c2))
return -1;
free(path_side_1_desc);
free(path_side_2_desc);
mfi_c1.blob.path = path;
mfi_c2.blob.path = path;

return handle_file_collision(opt, path, a->path, b->path,
ci->ren1->branch, ci->ren2->branch,
&mfi_c1.blob, &mfi_c2.blob);
}




static struct diff_queue_struct *get_diffpairs(struct merge_options *opt,
struct tree *o_tree,
struct tree *tree)
{
struct diff_queue_struct *ret;
struct diff_options opts;

repo_diff_setup(opt->repo, &opts);
opts.flags.recursive = 1;
opts.flags.rename_empty = 0;
opts.detect_rename = merge_detect_rename(opt);






if (opts.detect_rename > DIFF_DETECT_RENAME)
opts.detect_rename = DIFF_DETECT_RENAME;
opts.rename_limit = (opt->rename_limit >= 0) ? opt->rename_limit : 1000;
opts.rename_score = opt->rename_score;
opts.show_rename_progress = opt->show_rename_progress;
opts.output_format = DIFF_FORMAT_NO_OUTPUT;
diff_setup_done(&opts);
diff_tree_oid(&o_tree->object.oid, &tree->object.oid, "", &opts);
diffcore_std(&opts);
if (opts.needed_rename_limit > opt->priv->needed_rename_limit)
opt->priv->needed_rename_limit = opts.needed_rename_limit;

ret = xmalloc(sizeof(*ret));
*ret = diff_queued_diff;

opts.output_format = DIFF_FORMAT_NO_OUTPUT;
diff_queued_diff.nr = 0;
diff_queued_diff.queue = NULL;
diff_flush(&opts);
return ret;
}

static int tree_has_path(struct repository *r, struct tree *tree,
const char *path)
{
struct object_id hashy;
unsigned short mode_o;

return !get_tree_entry(r,
&tree->object.oid, path,
&hashy, &mode_o);
}








static char *apply_dir_rename(struct dir_rename_entry *entry,
const char *old_path)
{
struct strbuf new_path = STRBUF_INIT;
int oldlen, newlen;

if (entry->non_unique_new_dir)
return NULL;

oldlen = strlen(entry->dir);
if (entry->new_dir.len == 0)








oldlen++;
newlen = entry->new_dir.len + (strlen(old_path) - oldlen) + 1;
strbuf_grow(&new_path, newlen);
strbuf_addbuf(&new_path, &entry->new_dir);
strbuf_addstr(&new_path, &old_path[oldlen]);

return strbuf_detach(&new_path, NULL);
}

static void get_renamed_dir_portion(const char *old_path, const char *new_path,
char **old_dir, char **new_dir)
{
char *end_of_old, *end_of_new;


*old_dir = NULL;
*new_dir = NULL;














end_of_old = strrchr(old_path, '/');
end_of_new = strrchr(new_path, '/');








if (end_of_old == NULL)
return; 





if (end_of_new == NULL) {
*old_dir = xstrndup(old_path, end_of_old - old_path);
*new_dir = xstrdup("");
return;
}


while (*--end_of_new == *--end_of_old &&
end_of_old != old_path &&
end_of_new != new_path)
; 





if (end_of_old == old_path && end_of_new == new_path &&
*end_of_old == *end_of_new)
return; 












if (end_of_new == new_path &&
end_of_old != old_path && end_of_old[-1] == '/') {
*old_dir = xstrndup(old_path, --end_of_old - old_path);
*new_dir = xstrdup("");
return;
}



















end_of_old = strchr(++end_of_old, '/');
end_of_new = strchr(++end_of_new, '/');


*old_dir = xstrndup(old_path, end_of_old - old_path);
*new_dir = xstrndup(new_path, end_of_new - new_path);
}

static void remove_hashmap_entries(struct hashmap *dir_renames,
struct string_list *items_to_remove)
{
int i;
struct dir_rename_entry *entry;

for (i = 0; i < items_to_remove->nr; i++) {
entry = items_to_remove->items[i].util;
hashmap_remove(dir_renames, &entry->ent, NULL);
}
string_list_clear(items_to_remove, 0);
}






static char *handle_path_level_conflicts(struct merge_options *opt,
const char *path,
struct dir_rename_entry *entry,
struct hashmap *collisions,
struct tree *tree)
{
char *new_path = NULL;
struct collision_entry *collision_ent;
int clean = 1;
struct strbuf collision_paths = STRBUF_INIT;





new_path = apply_dir_rename(entry, path);

if (!new_path) {

if (!entry->non_unique_new_dir)
BUG("entry->non_unqiue_dir not set and !new_path");
output(opt, 1, _("CONFLICT (directory rename split): "
"Unclear where to place %s because directory "
"%s was renamed to multiple other directories, "
"with no destination getting a majority of the "
"files."),
path, entry->dir);
clean = 0;
return NULL;
}






collision_ent = collision_find_entry(collisions, new_path);
if (collision_ent == NULL)
BUG("collision_ent is NULL");








if (collision_ent->reported_already) {
clean = 0;
} else if (tree_has_path(opt->repo, tree, new_path)) {
collision_ent->reported_already = 1;
strbuf_add_separated_string_list(&collision_paths, ", ",
&collision_ent->source_files);
output(opt, 1, _("CONFLICT (implicit dir rename): Existing "
"file/dir at %s in the way of implicit "
"directory rename(s) putting the following "
"path(s) there: %s."),
new_path, collision_paths.buf);
clean = 0;
} else if (collision_ent->source_files.nr > 1) {
collision_ent->reported_already = 1;
strbuf_add_separated_string_list(&collision_paths, ", ",
&collision_ent->source_files);
output(opt, 1, _("CONFLICT (implicit dir rename): Cannot map "
"more than one path to %s; implicit directory "
"renames tried to put these paths there: %s"),
new_path, collision_paths.buf);
clean = 0;
}


strbuf_release(&collision_paths);
if (!clean && new_path) {
free(new_path);
return NULL;
}

return new_path;
}





















static void handle_directory_level_conflicts(struct merge_options *opt,
struct hashmap *dir_re_head,
struct tree *head,
struct hashmap *dir_re_merge,
struct tree *merge)
{
struct hashmap_iter iter;
struct dir_rename_entry *head_ent;
struct dir_rename_entry *merge_ent;

struct string_list remove_from_head = STRING_LIST_INIT_NODUP;
struct string_list remove_from_merge = STRING_LIST_INIT_NODUP;

hashmap_for_each_entry(dir_re_head, &iter, head_ent,
ent ) {
merge_ent = dir_rename_find_entry(dir_re_merge, head_ent->dir);
if (merge_ent &&
!head_ent->non_unique_new_dir &&
!merge_ent->non_unique_new_dir &&
!strbuf_cmp(&head_ent->new_dir, &merge_ent->new_dir)) {

string_list_append(&remove_from_head,
head_ent->dir)->util = head_ent;
strbuf_release(&head_ent->new_dir);
string_list_append(&remove_from_merge,
merge_ent->dir)->util = merge_ent;
strbuf_release(&merge_ent->new_dir);
} else if (tree_has_path(opt->repo, head, head_ent->dir)) {

string_list_append(&remove_from_head,
head_ent->dir)->util = head_ent;
strbuf_release(&head_ent->new_dir);
}
}

remove_hashmap_entries(dir_re_head, &remove_from_head);
remove_hashmap_entries(dir_re_merge, &remove_from_merge);

hashmap_for_each_entry(dir_re_merge, &iter, merge_ent,
ent ) {
head_ent = dir_rename_find_entry(dir_re_head, merge_ent->dir);
if (tree_has_path(opt->repo, merge, merge_ent->dir)) {

string_list_append(&remove_from_merge,
merge_ent->dir)->util = merge_ent;
} else if (head_ent &&
!head_ent->non_unique_new_dir &&
!merge_ent->non_unique_new_dir) {







output(opt, 1, _("CONFLICT (rename/rename): "
"Rename directory %s->%s in %s. "
"Rename directory %s->%s in %s"),
head_ent->dir, head_ent->new_dir.buf, opt->branch1,
head_ent->dir, merge_ent->new_dir.buf, opt->branch2);
string_list_append(&remove_from_head,
head_ent->dir)->util = head_ent;
strbuf_release(&head_ent->new_dir);
string_list_append(&remove_from_merge,
merge_ent->dir)->util = merge_ent;
strbuf_release(&merge_ent->new_dir);
}
}

remove_hashmap_entries(dir_re_head, &remove_from_head);
remove_hashmap_entries(dir_re_merge, &remove_from_merge);
}

static struct hashmap *get_directory_renames(struct diff_queue_struct *pairs)
{
struct hashmap *dir_renames;
struct hashmap_iter iter;
struct dir_rename_entry *entry;
int i;

















dir_renames = xmalloc(sizeof(*dir_renames));
dir_rename_init(dir_renames);
for (i = 0; i < pairs->nr; ++i) {
struct string_list_item *item;
int *count;
struct diff_filepair *pair = pairs->queue[i];
char *old_dir, *new_dir;


if (pair->status != 'R')
continue;

get_renamed_dir_portion(pair->one->path, pair->two->path,
&old_dir, &new_dir);
if (!old_dir)

continue;

entry = dir_rename_find_entry(dir_renames, old_dir);
if (!entry) {
entry = xmalloc(sizeof(*entry));
dir_rename_entry_init(entry, old_dir);
hashmap_put(dir_renames, &entry->ent);
} else {
free(old_dir);
}
item = string_list_lookup(&entry->possible_new_dirs, new_dir);
if (!item) {
item = string_list_insert(&entry->possible_new_dirs,
new_dir);
item->util = xcalloc(1, sizeof(int));
} else {
free(new_dir);
}
count = item->util;
*count += 1;
}










hashmap_for_each_entry(dir_renames, &iter, entry,
ent ) {
int max = 0;
int bad_max = 0;
char *best = NULL;

for (i = 0; i < entry->possible_new_dirs.nr; i++) {
int *count = entry->possible_new_dirs.items[i].util;

if (*count == max)
bad_max = max;
else if (*count > max) {
max = *count;
best = entry->possible_new_dirs.items[i].string;
}
}
if (bad_max == max)
entry->non_unique_new_dir = 1;
else {
assert(entry->new_dir.len == 0);
strbuf_addstr(&entry->new_dir, best);
}








entry->possible_new_dirs.strdup_strings = 1;
string_list_clear(&entry->possible_new_dirs, 1);
}

return dir_renames;
}

static struct dir_rename_entry *check_dir_renamed(const char *path,
struct hashmap *dir_renames)
{
char *temp = xstrdup(path);
char *end;
struct dir_rename_entry *entry = NULL;

while ((end = strrchr(temp, '/'))) {
*end = '\0';
entry = dir_rename_find_entry(dir_renames, temp);
if (entry)
break;
}
free(temp);
return entry;
}

static void compute_collisions(struct hashmap *collisions,
struct hashmap *dir_renames,
struct diff_queue_struct *pairs)
{
int i;

















collision_init(collisions);

for (i = 0; i < pairs->nr; ++i) {
struct dir_rename_entry *dir_rename_ent;
struct collision_entry *collision_ent;
char *new_path;
struct diff_filepair *pair = pairs->queue[i];

if (pair->status != 'A' && pair->status != 'R')
continue;
dir_rename_ent = check_dir_renamed(pair->two->path,
dir_renames);
if (!dir_rename_ent)
continue;

new_path = apply_dir_rename(dir_rename_ent, pair->two->path);
if (!new_path)






continue;
collision_ent = collision_find_entry(collisions, new_path);
if (!collision_ent) {
collision_ent = xcalloc(1,
sizeof(struct collision_entry));
hashmap_entry_init(&collision_ent->ent,
strhash(new_path));
hashmap_put(collisions, &collision_ent->ent);
collision_ent->target_file = new_path;
} else {
free(new_path);
}
string_list_insert(&collision_ent->source_files,
pair->two->path);
}
}

static char *check_for_directory_rename(struct merge_options *opt,
const char *path,
struct tree *tree,
struct hashmap *dir_renames,
struct hashmap *dir_rename_exclusions,
struct hashmap *collisions,
int *clean_merge)
{
char *new_path = NULL;
struct dir_rename_entry *entry = check_dir_renamed(path, dir_renames);
struct dir_rename_entry *oentry = NULL;

if (!entry)
return new_path;

























oentry = dir_rename_find_entry(dir_rename_exclusions, entry->new_dir.buf);
if (oentry) {
output(opt, 1, _("WARNING: Avoiding applying %s -> %s rename "
"to %s, because %s itself was renamed."),
entry->dir, entry->new_dir.buf, path, entry->new_dir.buf);
} else {
new_path = handle_path_level_conflicts(opt, path, entry,
collisions, tree);
*clean_merge &= (new_path != NULL);
}

return new_path;
}

static void apply_directory_rename_modifications(struct merge_options *opt,
struct diff_filepair *pair,
char *new_path,
struct rename *re,
struct tree *tree,
struct tree *o_tree,
struct tree *a_tree,
struct tree *b_tree,
struct string_list *entries)
{
struct string_list_item *item;
int stage = (tree == a_tree ? 2 : 3);
int update_wd;










update_wd = !was_dirty(opt, pair->two->path);
if (!update_wd)
output(opt, 1, _("Refusing to lose dirty file at %s"),
pair->two->path);
remove_file(opt, 1, pair->two->path, !update_wd);


item = string_list_lookup(entries, new_path);
if (item) {








re->dst_entry->processed = 1;




re->dst_entry = item->util;
} else {









if (pair->status == 'R')
re->dst_entry->processed = 1;

re->dst_entry = insert_stage_data(opt->repo, new_path,
o_tree, a_tree, b_tree,
entries);
item = string_list_insert(entries, new_path);
item->util = re->dst_entry;
}

















get_tree_entry(opt->repo,
&tree->object.oid,
pair->two->path,
&re->dst_entry->stages[stage].oid,
&re->dst_entry->stages[stage].mode);










re->dir_rename_original_type = pair->status;
re->dir_rename_original_dest = pair->two->path;





pair->status = 'R';




pair->two->path = new_path;
}








static struct string_list *get_renames(struct merge_options *opt,
const char *branch,
struct diff_queue_struct *pairs,
struct hashmap *dir_renames,
struct hashmap *dir_rename_exclusions,
struct tree *tree,
struct tree *o_tree,
struct tree *a_tree,
struct tree *b_tree,
struct string_list *entries,
int *clean_merge)
{
int i;
struct hashmap collisions;
struct hashmap_iter iter;
struct collision_entry *e;
struct string_list *renames;

compute_collisions(&collisions, dir_renames, pairs);
renames = xcalloc(1, sizeof(struct string_list));

for (i = 0; i < pairs->nr; ++i) {
struct string_list_item *item;
struct rename *re;
struct diff_filepair *pair = pairs->queue[i];
char *new_path; 

if (pair->status != 'A' && pair->status != 'R') {
diff_free_filepair(pair);
continue;
}
new_path = check_for_directory_rename(opt, pair->two->path, tree,
dir_renames,
dir_rename_exclusions,
&collisions,
clean_merge);
if (pair->status != 'R' && !new_path) {
diff_free_filepair(pair);
continue;
}

re = xmalloc(sizeof(*re));
re->processed = 0;
re->pair = pair;
re->branch = branch;
re->dir_rename_original_type = '\0';
re->dir_rename_original_dest = NULL;
item = string_list_lookup(entries, re->pair->one->path);
if (!item)
re->src_entry = insert_stage_data(opt->repo,
re->pair->one->path,
o_tree, a_tree, b_tree, entries);
else
re->src_entry = item->util;

item = string_list_lookup(entries, re->pair->two->path);
if (!item)
re->dst_entry = insert_stage_data(opt->repo,
re->pair->two->path,
o_tree, a_tree, b_tree, entries);
else
re->dst_entry = item->util;
item = string_list_insert(renames, pair->one->path);
item->util = re;
if (new_path)
apply_directory_rename_modifications(opt, pair, new_path,
re, tree, o_tree,
a_tree, b_tree,
entries);
}

hashmap_for_each_entry(&collisions, &iter, e,
ent ) {
free(e->target_file);
string_list_clear(&e->source_files, 0);
}
hashmap_free_entries(&collisions, struct collision_entry, ent);
return renames;
}

static int process_renames(struct merge_options *opt,
struct string_list *a_renames,
struct string_list *b_renames)
{
int clean_merge = 1, i, j;
struct string_list a_by_dst = STRING_LIST_INIT_NODUP;
struct string_list b_by_dst = STRING_LIST_INIT_NODUP;
const struct rename *sre;

for (i = 0; i < a_renames->nr; i++) {
sre = a_renames->items[i].util;
string_list_insert(&a_by_dst, sre->pair->two->path)->util
= (void *)sre;
}
for (i = 0; i < b_renames->nr; i++) {
sre = b_renames->items[i].util;
string_list_insert(&b_by_dst, sre->pair->two->path)->util
= (void *)sre;
}

for (i = 0, j = 0; i < a_renames->nr || j < b_renames->nr;) {
struct string_list *renames1, *renames2Dst;
struct rename *ren1 = NULL, *ren2 = NULL;
const char *ren1_src, *ren1_dst;
struct string_list_item *lookup;

if (i >= a_renames->nr) {
ren2 = b_renames->items[j++].util;
} else if (j >= b_renames->nr) {
ren1 = a_renames->items[i++].util;
} else {
int compare = strcmp(a_renames->items[i].string,
b_renames->items[j].string);
if (compare <= 0)
ren1 = a_renames->items[i++].util;
if (compare >= 0)
ren2 = b_renames->items[j++].util;
}


if (ren1) {
renames1 = a_renames;
renames2Dst = &b_by_dst;
} else {
renames1 = b_renames;
renames2Dst = &a_by_dst;
SWAP(ren2, ren1);
}

if (ren1->processed)
continue;
ren1->processed = 1;
ren1->dst_entry->processed = 1;



ren1->src_entry->processed = 1;

ren1_src = ren1->pair->one->path;
ren1_dst = ren1->pair->two->path;

if (ren2) {

const char *ren2_src = ren2->pair->one->path;
const char *ren2_dst = ren2->pair->two->path;
enum rename_type rename_type;
if (strcmp(ren1_src, ren2_src) != 0)
BUG("ren1_src != ren2_src");
ren2->dst_entry->processed = 1;
ren2->processed = 1;
if (strcmp(ren1_dst, ren2_dst) != 0) {
rename_type = RENAME_ONE_FILE_TO_TWO;
clean_merge = 0;
} else {
rename_type = RENAME_ONE_FILE_TO_ONE;




remove_file(opt, 1, ren1_src, 1);
update_entry(ren1->dst_entry,
ren1->pair->one,
ren1->pair->two,
ren2->pair->two);
}
setup_rename_conflict_info(rename_type, opt, ren1, ren2);
} else if ((lookup = string_list_lookup(renames2Dst, ren1_dst))) {

char *ren2_dst;
ren2 = lookup->util;
ren2_dst = ren2->pair->two->path;
if (strcmp(ren1_dst, ren2_dst) != 0)
BUG("ren1_dst != ren2_dst");

clean_merge = 0;
ren2->processed = 1;





ren2->src_entry->processed = 1;

setup_rename_conflict_info(RENAME_TWO_FILES_TO_ONE,
opt, ren1, ren2);
} else {


struct diff_filespec src_other, dst_other;
int try_merge;







int renamed_stage = a_renames == renames1 ? 2 : 3;
int other_stage = a_renames == renames1 ? 3 : 2;





remove_file(opt, 1, ren1_src,
renamed_stage == 2 || !was_tracked(opt, ren1_src));

oidcpy(&src_other.oid,
&ren1->src_entry->stages[other_stage].oid);
src_other.mode = ren1->src_entry->stages[other_stage].mode;
oidcpy(&dst_other.oid,
&ren1->dst_entry->stages[other_stage].oid);
dst_other.mode = ren1->dst_entry->stages[other_stage].mode;
try_merge = 0;

if (oideq(&src_other.oid, &null_oid) &&
ren1->dir_rename_original_type == 'A') {
setup_rename_conflict_info(RENAME_VIA_DIR,
opt, ren1, NULL);
} else if (oideq(&src_other.oid, &null_oid)) {
setup_rename_conflict_info(RENAME_DELETE,
opt, ren1, NULL);
} else if ((dst_other.mode == ren1->pair->two->mode) &&
oideq(&dst_other.oid, &ren1->pair->two->oid)) {








if (update_file_flags(opt,
ren1->pair->two,
ren1_dst,
1, 
0 ))
clean_merge = -1;
} else if (!oideq(&dst_other.oid, &null_oid)) {







setup_rename_conflict_info(RENAME_ADD,
opt, ren1, NULL);
} else
try_merge = 1;

if (clean_merge < 0)
goto cleanup_and_return;
if (try_merge) {
struct diff_filespec *o, *a, *b;
src_other.path = (char *)ren1_src;

o = ren1->pair->one;
if (a_renames == renames1) {
a = ren1->pair->two;
b = &src_other;
} else {
b = ren1->pair->two;
a = &src_other;
}
update_entry(ren1->dst_entry, o, a, b);
setup_rename_conflict_info(RENAME_NORMAL,
opt, ren1, NULL);
}
}
}
cleanup_and_return:
string_list_clear(&a_by_dst, 0);
string_list_clear(&b_by_dst, 0);

return clean_merge;
}

struct rename_info {
struct string_list *head_renames;
struct string_list *merge_renames;
};

static void initial_cleanup_rename(struct diff_queue_struct *pairs,
struct hashmap *dir_renames)
{
struct hashmap_iter iter;
struct dir_rename_entry *e;

hashmap_for_each_entry(dir_renames, &iter, e,
ent ) {
free(e->dir);
strbuf_release(&e->new_dir);

}
hashmap_free_entries(dir_renames, struct dir_rename_entry, ent);
free(dir_renames);

free(pairs->queue);
free(pairs);
}

static int detect_and_process_renames(struct merge_options *opt,
struct tree *common,
struct tree *head,
struct tree *merge,
struct string_list *entries,
struct rename_info *ri)
{
struct diff_queue_struct *head_pairs, *merge_pairs;
struct hashmap *dir_re_head, *dir_re_merge;
int clean = 1;

ri->head_renames = NULL;
ri->merge_renames = NULL;

if (!merge_detect_rename(opt))
return 1;

head_pairs = get_diffpairs(opt, common, head);
merge_pairs = get_diffpairs(opt, common, merge);

if ((opt->detect_directory_renames == MERGE_DIRECTORY_RENAMES_TRUE) ||
(opt->detect_directory_renames == MERGE_DIRECTORY_RENAMES_CONFLICT &&
!opt->priv->call_depth)) {
dir_re_head = get_directory_renames(head_pairs);
dir_re_merge = get_directory_renames(merge_pairs);

handle_directory_level_conflicts(opt,
dir_re_head, head,
dir_re_merge, merge);
} else {
dir_re_head = xmalloc(sizeof(*dir_re_head));
dir_re_merge = xmalloc(sizeof(*dir_re_merge));
dir_rename_init(dir_re_head);
dir_rename_init(dir_re_merge);
}

ri->head_renames = get_renames(opt, opt->branch1, head_pairs,
dir_re_merge, dir_re_head, head,
common, head, merge, entries,
&clean);
if (clean < 0)
goto cleanup;
ri->merge_renames = get_renames(opt, opt->branch2, merge_pairs,
dir_re_head, dir_re_merge, merge,
common, head, merge, entries,
&clean);
if (clean < 0)
goto cleanup;
clean &= process_renames(opt, ri->head_renames, ri->merge_renames);

cleanup:





initial_cleanup_rename(head_pairs, dir_re_head);
initial_cleanup_rename(merge_pairs, dir_re_merge);

return clean;
}

static void final_cleanup_rename(struct string_list *rename)
{
const struct rename *re;
int i;

if (rename == NULL)
return;

for (i = 0; i < rename->nr; i++) {
re = rename->items[i].util;
diff_free_filepair(re->pair);
}
string_list_clear(rename, 1);
free(rename);
}

static void final_cleanup_renames(struct rename_info *re_info)
{
final_cleanup_rename(re_info->head_renames);
final_cleanup_rename(re_info->merge_renames);
}

static int read_oid_strbuf(struct merge_options *opt,
const struct object_id *oid,
struct strbuf *dst)
{
void *buf;
enum object_type type;
unsigned long size;
buf = read_object_file(oid, &type, &size);
if (!buf)
return err(opt, _("cannot read object %s"), oid_to_hex(oid));
if (type != OBJ_BLOB) {
free(buf);
return err(opt, _("object %s is not a blob"), oid_to_hex(oid));
}
strbuf_attach(dst, buf, size, size + 1);
return 0;
}

static int blob_unchanged(struct merge_options *opt,
const struct diff_filespec *o,
const struct diff_filespec *a,
int renormalize, const char *path)
{
struct strbuf obuf = STRBUF_INIT;
struct strbuf abuf = STRBUF_INIT;
int ret = 0; 
const struct index_state *idx = opt->repo->index;

if (a->mode != o->mode)
return 0;
if (oideq(&o->oid, &a->oid))
return 1;
if (!renormalize)
return 0;

if (read_oid_strbuf(opt, &o->oid, &obuf) ||
read_oid_strbuf(opt, &a->oid, &abuf))
goto error_return;





if (renormalize_buffer(idx, path, obuf.buf, obuf.len, &obuf) |
renormalize_buffer(idx, path, abuf.buf, abuf.len, &abuf))
ret = (obuf.len == abuf.len && !memcmp(obuf.buf, abuf.buf, obuf.len));

error_return:
strbuf_release(&obuf);
strbuf_release(&abuf);
return ret;
}

static int handle_modify_delete(struct merge_options *opt,
const char *path,
const struct diff_filespec *o,
const struct diff_filespec *a,
const struct diff_filespec *b)
{
const char *modify_branch, *delete_branch;
const struct diff_filespec *changed;

if (is_valid(a)) {
modify_branch = opt->branch1;
delete_branch = opt->branch2;
changed = a;
} else {
modify_branch = opt->branch2;
delete_branch = opt->branch1;
changed = b;
}

return handle_change_delete(opt,
path, NULL,
o, changed,
modify_branch, delete_branch,
_("modify"), _("modified"));
}

static int handle_content_merge(struct merge_file_info *mfi,
struct merge_options *opt,
const char *path,
int is_dirty,
const struct diff_filespec *o,
const struct diff_filespec *a,
const struct diff_filespec *b,
struct rename_conflict_info *ci)
{
const char *reason = _("content");
unsigned df_conflict_remains = 0;

if (!is_valid(o))
reason = _("add/add");

assert(o->path && a->path && b->path);
if (ci && dir_in_way(opt->repo->index, path, !opt->priv->call_depth,
S_ISGITLINK(ci->ren1->pair->two->mode)))
df_conflict_remains = 1;

if (merge_mode_and_contents(opt, o, a, b, path,
opt->branch1, opt->branch2,
opt->priv->call_depth * 2, mfi))
return -1;







if (mfi->clean && was_tracked_and_matches(opt, path, &mfi->blob) &&
!df_conflict_remains) {
int pos;
struct cache_entry *ce;

output(opt, 3, _("Skipped %s (merged same as existing)"), path);
if (add_cacheinfo(opt, &mfi->blob, path,
0, (!opt->priv->call_depth && !is_dirty), 0))
return -1;






pos = index_name_pos(&opt->priv->orig_index, path, strlen(path));
ce = opt->priv->orig_index.cache[pos];
if (ce_skip_worktree(ce)) {
pos = index_name_pos(opt->repo->index, path, strlen(path));
ce = opt->repo->index->cache[pos];
ce->ce_flags |= CE_SKIP_WORKTREE;
}
return mfi->clean;
}

if (!mfi->clean) {
if (S_ISGITLINK(mfi->blob.mode))
reason = _("submodule");
output(opt, 1, _("CONFLICT (%s): Merge conflict in %s"),
reason, path);
if (ci && !df_conflict_remains)
if (update_stages(opt, path, o, a, b))
return -1;
}

if (df_conflict_remains || is_dirty) {
char *new_path;
if (opt->priv->call_depth) {
remove_file_from_index(opt->repo->index, path);
} else {
if (!mfi->clean) {
if (update_stages(opt, path, o, a, b))
return -1;
} else {
int file_from_stage2 = was_tracked(opt, path);

if (update_stages(opt, path, NULL,
file_from_stage2 ? &mfi->blob : NULL,
file_from_stage2 ? NULL : &mfi->blob))
return -1;
}

}
new_path = unique_path(opt, path, ci->ren1->branch);
if (is_dirty) {
output(opt, 1, _("Refusing to lose dirty file at %s"),
path);
}
output(opt, 1, _("Adding as %s instead"), new_path);
if (update_file(opt, 0, &mfi->blob, new_path)) {
free(new_path);
return -1;
}
free(new_path);
mfi->clean = 0;
} else if (update_file(opt, mfi->clean, &mfi->blob, path))
return -1;
return !is_dirty && mfi->clean;
}

static int handle_rename_normal(struct merge_options *opt,
const char *path,
const struct diff_filespec *o,
const struct diff_filespec *a,
const struct diff_filespec *b,
struct rename_conflict_info *ci)
{
struct rename *ren = ci->ren1;
struct merge_file_info mfi;
int clean;
int side = (ren->branch == opt->branch1 ? 2 : 3);


clean = handle_content_merge(&mfi, opt, path, was_dirty(opt, path),
o, a, b, ci);

if (clean &&
opt->detect_directory_renames == MERGE_DIRECTORY_RENAMES_CONFLICT &&
ren->dir_rename_original_dest) {
if (update_stages(opt, path,
NULL,
side == 2 ? &mfi.blob : NULL,
side == 2 ? NULL : &mfi.blob))
return -1;
clean = 0; 
}
return clean;
}

static void dir_rename_warning(const char *msg,
int is_add,
int clean,
struct merge_options *opt,
struct rename *ren)
{
const char *other_branch;
other_branch = (ren->branch == opt->branch1 ?
opt->branch2 : opt->branch1);
if (is_add) {
output(opt, clean ? 2 : 1, msg,
ren->pair->one->path, ren->branch,
other_branch, ren->pair->two->path);
return;
}
output(opt, clean ? 2 : 1, msg,
ren->pair->one->path, ren->dir_rename_original_dest, ren->branch,
other_branch, ren->pair->two->path);
}
static int warn_about_dir_renamed_entries(struct merge_options *opt,
struct rename *ren)
{
const char *msg;
int clean = 1, is_add;

if (!ren)
return clean;


if (!ren->dir_rename_original_dest)
return clean;


assert(opt->detect_directory_renames > MERGE_DIRECTORY_RENAMES_NONE);
assert(ren->dir_rename_original_type == 'A' ||
ren->dir_rename_original_type == 'R');


clean = (opt->detect_directory_renames == MERGE_DIRECTORY_RENAMES_TRUE);

is_add = (ren->dir_rename_original_type == 'A');
if (ren->dir_rename_original_type == 'A' && clean) {
msg = _("Path updated: %s added in %s inside a "
"directory that was renamed in %s; moving it to %s.");
} else if (ren->dir_rename_original_type == 'A' && !clean) {
msg = _("CONFLICT (file location): %s added in %s "
"inside a directory that was renamed in %s, "
"suggesting it should perhaps be moved to %s.");
} else if (ren->dir_rename_original_type == 'R' && clean) {
msg = _("Path updated: %s renamed to %s in %s, inside a "
"directory that was renamed in %s; moving it to %s.");
} else if (ren->dir_rename_original_type == 'R' && !clean) {
msg = _("CONFLICT (file location): %s renamed to %s in %s, "
"inside a directory that was renamed in %s, "
"suggesting it should perhaps be moved to %s.");
} else {
BUG("Impossible dir_rename_original_type/clean combination");
}
dir_rename_warning(msg, is_add, clean, opt, ren);

return clean;
}


static int process_entry(struct merge_options *opt,
const char *path, struct stage_data *entry)
{
int clean_merge = 1;
int normalize = opt->renormalize;

struct diff_filespec *o = &entry->stages[1];
struct diff_filespec *a = &entry->stages[2];
struct diff_filespec *b = &entry->stages[3];
int o_valid = is_valid(o);
int a_valid = is_valid(a);
int b_valid = is_valid(b);
o->path = a->path = b->path = (char*)path;

entry->processed = 1;
if (entry->rename_conflict_info) {
struct rename_conflict_info *ci = entry->rename_conflict_info;
struct diff_filespec *temp;
int path_clean;

path_clean = warn_about_dir_renamed_entries(opt, ci->ren1);
path_clean &= warn_about_dir_renamed_entries(opt, ci->ren2);







temp = (opt->branch1 == ci->ren1->branch) ? b : a;
o->path = temp->path = ci->ren1->pair->one->path;
if (ci->ren2) {
assert(opt->branch1 == ci->ren1->branch);
}

switch (ci->rename_type) {
case RENAME_NORMAL:
case RENAME_ONE_FILE_TO_ONE:
clean_merge = handle_rename_normal(opt, path, o, a, b,
ci);
break;
case RENAME_VIA_DIR:
clean_merge = handle_rename_via_dir(opt, ci);
break;
case RENAME_ADD:






clean_merge = handle_rename_add(opt, ci);
break;
case RENAME_DELETE:
clean_merge = 0;
if (handle_rename_delete(opt, ci))
clean_merge = -1;
break;
case RENAME_ONE_FILE_TO_TWO:




o->path = ci->ren1->pair->one->path;
a->path = ci->ren1->pair->two->path;
b->path = ci->ren2->pair->two->path;

clean_merge = 0;
if (handle_rename_rename_1to2(opt, ci))
clean_merge = -1;
break;
case RENAME_TWO_FILES_TO_ONE:




o->path = NULL;
a->path = ci->ren1->pair->two->path;
b->path = ci->ren2->pair->two->path;







clean_merge = handle_rename_rename_2to1(opt, ci);
break;
default:
entry->processed = 0;
break;
}
if (path_clean < clean_merge)
clean_merge = path_clean;
} else if (o_valid && (!a_valid || !b_valid)) {

if ((!a_valid && !b_valid) ||
(!b_valid && blob_unchanged(opt, o, a, normalize, path)) ||
(!a_valid && blob_unchanged(opt, o, b, normalize, path))) {


if (a_valid)
output(opt, 2, _("Removing %s"), path);

remove_file(opt, 1, path, !a_valid);
} else {

clean_merge = 0;
if (handle_modify_delete(opt, path, o, a, b))
clean_merge = -1;
}
} else if ((!o_valid && a_valid && !b_valid) ||
(!o_valid && !a_valid && b_valid)) {



const char *add_branch;
const char *other_branch;
const char *conf;
const struct diff_filespec *contents;

if (a_valid) {
add_branch = opt->branch1;
other_branch = opt->branch2;
contents = a;
conf = _("file/directory");
} else {
add_branch = opt->branch2;
other_branch = opt->branch1;
contents = b;
conf = _("directory/file");
}
if (dir_in_way(opt->repo->index, path,
!opt->priv->call_depth && !S_ISGITLINK(a->mode),
0)) {
char *new_path = unique_path(opt, path, add_branch);
clean_merge = 0;
output(opt, 1, _("CONFLICT (%s): There is a directory with name %s in %s. "
"Adding %s as %s"),
conf, path, other_branch, path, new_path);
if (update_file(opt, 0, contents, new_path))
clean_merge = -1;
else if (opt->priv->call_depth)
remove_file_from_index(opt->repo->index, path);
free(new_path);
} else {
output(opt, 2, _("Adding %s"), path);

if (update_file_flags(opt, contents, path, 1, !a_valid))
clean_merge = -1;
}
} else if (a_valid && b_valid) {
if (!o_valid) {

output(opt, 1,
_("CONFLICT (add/add): Merge conflict in %s"),
path);
clean_merge = handle_file_collision(opt,
path, NULL, NULL,
opt->branch1,
opt->branch2,
a, b);
} else {

struct merge_file_info mfi;
int is_dirty = 0; 
clean_merge = handle_content_merge(&mfi, opt, path,
is_dirty,
o, a, b, NULL);
}
} else if (!o_valid && !a_valid && !b_valid) {




remove_file(opt, 1, path, !a->mode);
} else
BUG("fatal merge failure, shouldn't happen.");

return clean_merge;
}

static int merge_trees_internal(struct merge_options *opt,
struct tree *head,
struct tree *merge,
struct tree *merge_base,
struct tree **result)
{
struct index_state *istate = opt->repo->index;
int code, clean;

if (opt->subtree_shift) {
merge = shift_tree_object(opt->repo, head, merge,
opt->subtree_shift);
merge_base = shift_tree_object(opt->repo, head, merge_base,
opt->subtree_shift);
}

if (oideq(&merge_base->object.oid, &merge->object.oid)) {
output(opt, 0, _("Already up to date!"));
*result = head;
return 1;
}

code = unpack_trees_start(opt, merge_base, head, merge);

if (code != 0) {
if (show(opt, 4) || opt->priv->call_depth)
err(opt, _("merging of trees %s and %s failed"),
oid_to_hex(&head->object.oid),
oid_to_hex(&merge->object.oid));
unpack_trees_finish(opt);
return -1;
}

if (unmerged_index(istate)) {
struct string_list *entries;
struct rename_info re_info;
int i;







hashmap_init(&opt->priv->current_file_dir_set, path_hashmap_cmp,
NULL, 512);
get_files_dirs(opt, head);
get_files_dirs(opt, merge);

entries = get_unmerged(opt->repo->index);
clean = detect_and_process_renames(opt, merge_base, head, merge,
entries, &re_info);
record_df_conflict_files(opt, entries);
if (clean < 0)
goto cleanup;
for (i = entries->nr-1; 0 <= i; i--) {
const char *path = entries->items[i].string;
struct stage_data *e = entries->items[i].util;
if (!e->processed) {
int ret = process_entry(opt, path, e);
if (!ret)
clean = 0;
else if (ret < 0) {
clean = ret;
goto cleanup;
}
}
}
for (i = 0; i < entries->nr; i++) {
struct stage_data *e = entries->items[i].util;
if (!e->processed)
BUG("unprocessed path??? %s",
entries->items[i].string);
}

cleanup:
final_cleanup_renames(&re_info);

string_list_clear(entries, 1);
free(entries);

hashmap_free_entries(&opt->priv->current_file_dir_set,
struct path_hashmap_entry, e);

if (clean < 0) {
unpack_trees_finish(opt);
return clean;
}
}
else
clean = 1;

unpack_trees_finish(opt);

if (opt->priv->call_depth &&
!(*result = write_in_core_index_as_tree(opt->repo)))
return -1;

return clean;
}

static struct commit_list *reverse_commit_list(struct commit_list *list)
{
struct commit_list *next = NULL, *current, *backup;
for (current = list; current; current = backup) {
backup = current->next;
current->next = next;
next = current;
}
return next;
}





static int merge_recursive_internal(struct merge_options *opt,
struct commit *h1,
struct commit *h2,
struct commit_list *merge_bases,
struct commit **result)
{
struct commit_list *iter;
struct commit *merged_merge_bases;
struct tree *result_tree;
int clean;
const char *ancestor_name;
struct strbuf merge_base_abbrev = STRBUF_INIT;

if (show(opt, 4)) {
output(opt, 4, _("Merging:"));
output_commit_title(opt, h1);
output_commit_title(opt, h2);
}

if (!merge_bases) {
merge_bases = get_merge_bases(h1, h2);
merge_bases = reverse_commit_list(merge_bases);
}

if (show(opt, 5)) {
unsigned cnt = commit_list_count(merge_bases);

output(opt, 5, Q_("found %u common ancestor:",
"found %u common ancestors:", cnt), cnt);
for (iter = merge_bases; iter; iter = iter->next)
output_commit_title(opt, iter->item);
}

merged_merge_bases = pop_commit(&merge_bases);
if (merged_merge_bases == NULL) {

struct tree *tree;

tree = lookup_tree(opt->repo, opt->repo->hash_algo->empty_tree);
merged_merge_bases = make_virtual_commit(opt->repo, tree,
"ancestor");
ancestor_name = "empty tree";
} else if (opt->ancestor && !opt->priv->call_depth) {
ancestor_name = opt->ancestor;
} else if (merge_bases) {
ancestor_name = "merged common ancestors";
} else {
strbuf_add_unique_abbrev(&merge_base_abbrev,
&merged_merge_bases->object.oid,
DEFAULT_ABBREV);
ancestor_name = merge_base_abbrev.buf;
}

for (iter = merge_bases; iter; iter = iter->next) {
const char *saved_b1, *saved_b2;
opt->priv->call_depth++;








discard_index(opt->repo->index);
saved_b1 = opt->branch1;
saved_b2 = opt->branch2;
opt->branch1 = "Temporary merge branch 1";
opt->branch2 = "Temporary merge branch 2";
if (merge_recursive_internal(opt, merged_merge_bases, iter->item,
NULL, &merged_merge_bases) < 0)
return -1;
opt->branch1 = saved_b1;
opt->branch2 = saved_b2;
opt->priv->call_depth--;

if (!merged_merge_bases)
return err(opt, _("merge returned no commit"));
}

discard_index(opt->repo->index);
if (!opt->priv->call_depth)
repo_read_index(opt->repo);

opt->ancestor = ancestor_name;
clean = merge_trees_internal(opt,
repo_get_commit_tree(opt->repo, h1),
repo_get_commit_tree(opt->repo, h2),
repo_get_commit_tree(opt->repo,
merged_merge_bases),
&result_tree);
strbuf_release(&merge_base_abbrev);
opt->ancestor = NULL; 
if (clean < 0) {
flush_output(opt);
return clean;
}

if (opt->priv->call_depth) {
*result = make_virtual_commit(opt->repo, result_tree,
"merged tree");
commit_list_insert(h1, &(*result)->parents);
commit_list_insert(h2, &(*result)->parents->next);
}
return clean;
}

static int merge_start(struct merge_options *opt, struct tree *head)
{
struct strbuf sb = STRBUF_INIT;


assert(opt->repo);

assert(opt->branch1 && opt->branch2);

assert(opt->detect_renames >= -1 &&
opt->detect_renames <= DIFF_DETECT_COPY);
assert(opt->detect_directory_renames >= MERGE_DIRECTORY_RENAMES_NONE &&
opt->detect_directory_renames <= MERGE_DIRECTORY_RENAMES_TRUE);
assert(opt->rename_limit >= -1);
assert(opt->rename_score >= 0 && opt->rename_score <= MAX_SCORE);
assert(opt->show_rename_progress >= 0 && opt->show_rename_progress <= 1);

assert(opt->xdl_opts >= 0);
assert(opt->recursive_variant >= MERGE_VARIANT_NORMAL &&
opt->recursive_variant <= MERGE_VARIANT_THEIRS);

assert(opt->verbosity >= 0 && opt->verbosity <= 5);
assert(opt->buffer_output <= 2);
assert(opt->obuf.len == 0);

assert(opt->priv == NULL);


if (repo_index_has_changes(opt->repo, head, &sb)) {
err(opt, _("Your local changes to the following files would be overwritten by merge:\n %s"),
sb.buf);
strbuf_release(&sb);
return -1;
}

opt->priv = xcalloc(1, sizeof(*opt->priv));
string_list_init(&opt->priv->df_conflict_file_set, 1);
return 0;
}

static void merge_finalize(struct merge_options *opt)
{
flush_output(opt);
if (!opt->priv->call_depth && opt->buffer_output < 2)
strbuf_release(&opt->obuf);
if (show(opt, 2))
diff_warn_rename_limit("merge.renamelimit",
opt->priv->needed_rename_limit, 0);
FREE_AND_NULL(opt->priv);
}

int merge_trees(struct merge_options *opt,
struct tree *head,
struct tree *merge,
struct tree *merge_base)
{
int clean;
struct tree *ignored;

assert(opt->ancestor != NULL);

if (merge_start(opt, head))
return -1;
clean = merge_trees_internal(opt, head, merge, merge_base, &ignored);
merge_finalize(opt);

return clean;
}

int merge_recursive(struct merge_options *opt,
struct commit *h1,
struct commit *h2,
struct commit_list *merge_bases,
struct commit **result)
{
int clean;

assert(opt->ancestor == NULL ||
!strcmp(opt->ancestor, "constructed merge base"));

if (merge_start(opt, repo_get_commit_tree(opt->repo, h1)))
return -1;
clean = merge_recursive_internal(opt, h1, h2, merge_bases, result);
merge_finalize(opt);

return clean;
}

static struct commit *get_ref(struct repository *repo,
const struct object_id *oid,
const char *name)
{
struct object *object;

object = deref_tag(repo, parse_object(repo, oid),
name, strlen(name));
if (!object)
return NULL;
if (object->type == OBJ_TREE)
return make_virtual_commit(repo, (struct tree*)object, name);
if (object->type != OBJ_COMMIT)
return NULL;
if (parse_commit((struct commit *)object))
return NULL;
return (struct commit *)object;
}

int merge_recursive_generic(struct merge_options *opt,
const struct object_id *head,
const struct object_id *merge,
int num_merge_bases,
const struct object_id **merge_bases,
struct commit **result)
{
int clean;
struct lock_file lock = LOCK_INIT;
struct commit *head_commit = get_ref(opt->repo, head, opt->branch1);
struct commit *next_commit = get_ref(opt->repo, merge, opt->branch2);
struct commit_list *ca = NULL;

if (merge_bases) {
int i;
for (i = 0; i < num_merge_bases; ++i) {
struct commit *base;
if (!(base = get_ref(opt->repo, merge_bases[i],
oid_to_hex(merge_bases[i]))))
return err(opt, _("Could not parse object '%s'"),
oid_to_hex(merge_bases[i]));
commit_list_insert(base, &ca);
}
if (num_merge_bases == 1)
opt->ancestor = "constructed merge base";
}

repo_hold_locked_index(opt->repo, &lock, LOCK_DIE_ON_ERROR);
clean = merge_recursive(opt, head_commit, next_commit, ca,
result);
if (clean < 0) {
rollback_lock_file(&lock);
return clean;
}

if (write_locked_index(opt->repo->index, &lock,
COMMIT_LOCK | SKIP_IF_UNCHANGED))
return err(opt, _("Unable to write index."));

return clean ? 0 : 1;
}

static void merge_recursive_config(struct merge_options *opt)
{
char *value = NULL;
git_config_get_int("merge.verbosity", &opt->verbosity);
git_config_get_int("diff.renamelimit", &opt->rename_limit);
git_config_get_int("merge.renamelimit", &opt->rename_limit);
if (!git_config_get_string("diff.renames", &value)) {
opt->detect_renames = git_config_rename("diff.renames", value);
free(value);
}
if (!git_config_get_string("merge.renames", &value)) {
opt->detect_renames = git_config_rename("merge.renames", value);
free(value);
}
if (!git_config_get_string("merge.directoryrenames", &value)) {
int boolval = git_parse_maybe_bool(value);
if (0 <= boolval) {
opt->detect_directory_renames = boolval ?
MERGE_DIRECTORY_RENAMES_TRUE :
MERGE_DIRECTORY_RENAMES_NONE;
} else if (!strcasecmp(value, "conflict")) {
opt->detect_directory_renames =
MERGE_DIRECTORY_RENAMES_CONFLICT;
} 
free(value);
}
git_config(git_xmerge_config, NULL);
}

void init_merge_options(struct merge_options *opt,
struct repository *repo)
{
const char *merge_verbosity;
memset(opt, 0, sizeof(struct merge_options));

opt->repo = repo;

opt->detect_renames = -1;
opt->detect_directory_renames = MERGE_DIRECTORY_RENAMES_CONFLICT;
opt->rename_limit = -1;

opt->verbosity = 2;
opt->buffer_output = 1;
strbuf_init(&opt->obuf, 0);

opt->renormalize = 0;

merge_recursive_config(opt);
merge_verbosity = getenv("GIT_MERGE_VERBOSITY");
if (merge_verbosity)
opt->verbosity = strtol(merge_verbosity, NULL, 10);
if (opt->verbosity >= 5)
opt->buffer_output = 0;
}

int parse_merge_opt(struct merge_options *opt, const char *s)
{
const char *arg;

if (!s || !*s)
return -1;
if (!strcmp(s, "ours"))
opt->recursive_variant = MERGE_VARIANT_OURS;
else if (!strcmp(s, "theirs"))
opt->recursive_variant = MERGE_VARIANT_THEIRS;
else if (!strcmp(s, "subtree"))
opt->subtree_shift = "";
else if (skip_prefix(s, "subtree=", &arg))
opt->subtree_shift = arg;
else if (!strcmp(s, "patience"))
opt->xdl_opts = DIFF_WITH_ALG(opt, PATIENCE_DIFF);
else if (!strcmp(s, "histogram"))
opt->xdl_opts = DIFF_WITH_ALG(opt, HISTOGRAM_DIFF);
else if (skip_prefix(s, "diff-algorithm=", &arg)) {
long value = parse_algorithm_value(arg);
if (value < 0)
return -1;

DIFF_XDL_CLR(opt, NEED_MINIMAL);
opt->xdl_opts &= ~XDF_DIFF_ALGORITHM_MASK;
opt->xdl_opts |= value;
}
else if (!strcmp(s, "ignore-space-change"))
DIFF_XDL_SET(opt, IGNORE_WHITESPACE_CHANGE);
else if (!strcmp(s, "ignore-all-space"))
DIFF_XDL_SET(opt, IGNORE_WHITESPACE);
else if (!strcmp(s, "ignore-space-at-eol"))
DIFF_XDL_SET(opt, IGNORE_WHITESPACE_AT_EOL);
else if (!strcmp(s, "ignore-cr-at-eol"))
DIFF_XDL_SET(opt, IGNORE_CR_AT_EOL);
else if (!strcmp(s, "renormalize"))
opt->renormalize = 1;
else if (!strcmp(s, "no-renormalize"))
opt->renormalize = 0;
else if (!strcmp(s, "no-renames"))
opt->detect_renames = 0;
else if (!strcmp(s, "find-renames")) {
opt->detect_renames = 1;
opt->rename_score = 0;
}
else if (skip_prefix(s, "find-renames=", &arg) ||
skip_prefix(s, "rename-threshold=", &arg)) {
if ((opt->rename_score = parse_rename_score(&arg)) == -1 || *arg != 0)
return -1;
opt->detect_renames = 1;
}




else
return -1;
return 0;
}
