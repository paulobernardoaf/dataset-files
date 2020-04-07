




#include "cache.h"
#include "config.h"
#include "diff.h"
#include "diffcore.h"
#include "tempfile.h"
#include "lockfile.h"
#include "cache-tree.h"
#include "refs.h"
#include "dir.h"
#include "object-store.h"
#include "tree.h"
#include "commit.h"
#include "blob.h"
#include "resolve-undo.h"
#include "run-command.h"
#include "strbuf.h"
#include "varint.h"
#include "split-index.h"
#include "utf8.h"
#include "fsmonitor.h"
#include "thread-utils.h"
#include "progress.h"



#define CE_NAMEMASK (0x0fff)










#define CACHE_EXT(s) ( (s[0]<<24)|(s[1]<<16)|(s[2]<<8)|(s[3]) )
#define CACHE_EXT_TREE 0x54524545 
#define CACHE_EXT_RESOLVE_UNDO 0x52455543 
#define CACHE_EXT_LINK 0x6c696e6b 
#define CACHE_EXT_UNTRACKED 0x554E5452 
#define CACHE_EXT_FSMONITOR 0x46534D4E 
#define CACHE_EXT_ENDOFINDEXENTRIES 0x454F4945 
#define CACHE_EXT_INDEXENTRYOFFSETTABLE 0x49454F54 


#define EXTMASK (RESOLVE_UNDO_CHANGED | CACHE_TREE_CHANGED | CE_ENTRY_ADDED | CE_ENTRY_REMOVED | CE_ENTRY_CHANGED | SPLIT_INDEX_ORDERED | UNTRACKED_CHANGED | FSMONITOR_CHANGED)












#define CACHE_ENTRY_PATH_LENGTH 80

static inline struct cache_entry *mem_pool__ce_alloc(struct mem_pool *mem_pool, size_t len)
{
struct cache_entry *ce;
ce = mem_pool_alloc(mem_pool, cache_entry_size(len));
ce->mem_pool_allocated = 1;
return ce;
}

static inline struct cache_entry *mem_pool__ce_calloc(struct mem_pool *mem_pool, size_t len)
{
struct cache_entry * ce;
ce = mem_pool_calloc(mem_pool, 1, cache_entry_size(len));
ce->mem_pool_allocated = 1;
return ce;
}

static struct mem_pool *find_mem_pool(struct index_state *istate)
{
struct mem_pool **pool_ptr;

if (istate->split_index && istate->split_index->base)
pool_ptr = &istate->split_index->base->ce_mem_pool;
else
pool_ptr = &istate->ce_mem_pool;

if (!*pool_ptr)
mem_pool_init(pool_ptr, 0);

return *pool_ptr;
}

static const char *alternate_index_output;

static void set_index_entry(struct index_state *istate, int nr, struct cache_entry *ce)
{
istate->cache[nr] = ce;
add_name_hash(istate, ce);
}

static void replace_index_entry(struct index_state *istate, int nr, struct cache_entry *ce)
{
struct cache_entry *old = istate->cache[nr];

replace_index_entry_in_base(istate, old, ce);
remove_name_hash(istate, old);
discard_cache_entry(old);
ce->ce_flags &= ~CE_HASHED;
set_index_entry(istate, nr, ce);
ce->ce_flags |= CE_UPDATE_IN_BASE;
mark_fsmonitor_invalid(istate, ce);
istate->cache_changed |= CE_ENTRY_CHANGED;
}

void rename_index_entry_at(struct index_state *istate, int nr, const char *new_name)
{
struct cache_entry *old_entry = istate->cache[nr], *new_entry;
int namelen = strlen(new_name);

new_entry = make_empty_cache_entry(istate, namelen);
copy_cache_entry(new_entry, old_entry);
new_entry->ce_flags &= ~CE_HASHED;
new_entry->ce_namelen = namelen;
new_entry->index = 0;
memcpy(new_entry->name, new_name, namelen + 1);

cache_tree_invalidate_path(istate, old_entry->name);
untracked_cache_remove_from_index(istate, old_entry->name);
remove_index_entry_at(istate, nr);
add_index_entry(istate, new_entry, ADD_CACHE_OK_TO_ADD|ADD_CACHE_OK_TO_REPLACE);
}

void fill_stat_data(struct stat_data *sd, struct stat *st)
{
sd->sd_ctime.sec = (unsigned int)st->st_ctime;
sd->sd_mtime.sec = (unsigned int)st->st_mtime;
sd->sd_ctime.nsec = ST_CTIME_NSEC(*st);
sd->sd_mtime.nsec = ST_MTIME_NSEC(*st);
sd->sd_dev = st->st_dev;
sd->sd_ino = st->st_ino;
sd->sd_uid = st->st_uid;
sd->sd_gid = st->st_gid;
sd->sd_size = st->st_size;
}

int match_stat_data(const struct stat_data *sd, struct stat *st)
{
int changed = 0;

if (sd->sd_mtime.sec != (unsigned int)st->st_mtime)
changed |= MTIME_CHANGED;
if (trust_ctime && check_stat &&
sd->sd_ctime.sec != (unsigned int)st->st_ctime)
changed |= CTIME_CHANGED;

#if defined(USE_NSEC)
if (check_stat && sd->sd_mtime.nsec != ST_MTIME_NSEC(*st))
changed |= MTIME_CHANGED;
if (trust_ctime && check_stat &&
sd->sd_ctime.nsec != ST_CTIME_NSEC(*st))
changed |= CTIME_CHANGED;
#endif

if (check_stat) {
if (sd->sd_uid != (unsigned int) st->st_uid ||
sd->sd_gid != (unsigned int) st->st_gid)
changed |= OWNER_CHANGED;
if (sd->sd_ino != (unsigned int) st->st_ino)
changed |= INODE_CHANGED;
}

#if defined(USE_STDEV)





if (check_stat && sd->sd_dev != (unsigned int) st->st_dev)
changed |= INODE_CHANGED;
#endif

if (sd->sd_size != (unsigned int) st->st_size)
changed |= DATA_CHANGED;

return changed;
}






void fill_stat_cache_info(struct index_state *istate, struct cache_entry *ce, struct stat *st)
{
fill_stat_data(&ce->ce_stat_data, st);

if (assume_unchanged)
ce->ce_flags |= CE_VALID;

if (S_ISREG(st->st_mode)) {
ce_mark_uptodate(ce);
mark_fsmonitor_valid(istate, ce);
}
}

static int ce_compare_data(struct index_state *istate,
const struct cache_entry *ce,
struct stat *st)
{
int match = -1;
int fd = git_open_cloexec(ce->name, O_RDONLY);

if (fd >= 0) {
struct object_id oid;
if (!index_fd(istate, &oid, fd, st, OBJ_BLOB, ce->name, 0))
match = !oideq(&oid, &ce->oid);

}
return match;
}

static int ce_compare_link(const struct cache_entry *ce, size_t expected_size)
{
int match = -1;
void *buffer;
unsigned long size;
enum object_type type;
struct strbuf sb = STRBUF_INIT;

if (strbuf_readlink(&sb, ce->name, expected_size))
return -1;

buffer = read_object_file(&ce->oid, &type, &size);
if (buffer) {
if (size == sb.len)
match = memcmp(buffer, sb.buf, size);
free(buffer);
}
strbuf_release(&sb);
return match;
}

static int ce_compare_gitlink(const struct cache_entry *ce)
{
struct object_id oid;









if (resolve_gitlink_ref(ce->name, "HEAD", &oid) < 0)
return 0;
return !oideq(&oid, &ce->oid);
}

static int ce_modified_check_fs(struct index_state *istate,
const struct cache_entry *ce,
struct stat *st)
{
switch (st->st_mode & S_IFMT) {
case S_IFREG:
if (ce_compare_data(istate, ce, st))
return DATA_CHANGED;
break;
case S_IFLNK:
if (ce_compare_link(ce, xsize_t(st->st_size)))
return DATA_CHANGED;
break;
case S_IFDIR:
if (S_ISGITLINK(ce->ce_mode))
return ce_compare_gitlink(ce) ? DATA_CHANGED : 0;

default:
return TYPE_CHANGED;
}
return 0;
}

static int ce_match_stat_basic(const struct cache_entry *ce, struct stat *st)
{
unsigned int changed = 0;

if (ce->ce_flags & CE_REMOVE)
return MODE_CHANGED | DATA_CHANGED | TYPE_CHANGED;

switch (ce->ce_mode & S_IFMT) {
case S_IFREG:
changed |= !S_ISREG(st->st_mode) ? TYPE_CHANGED : 0;



if (trust_executable_bit &&
(0100 & (ce->ce_mode ^ st->st_mode)))
changed |= MODE_CHANGED;
break;
case S_IFLNK:
if (!S_ISLNK(st->st_mode) &&
(has_symlinks || !S_ISREG(st->st_mode)))
changed |= TYPE_CHANGED;
break;
case S_IFGITLINK:

if (!S_ISDIR(st->st_mode))
changed |= TYPE_CHANGED;
else if (ce_compare_gitlink(ce))
changed |= DATA_CHANGED;
return changed;
default:
BUG("unsupported ce_mode: %o", ce->ce_mode);
}

changed |= match_stat_data(&ce->ce_stat_data, st);


if (!ce->ce_stat_data.sd_size) {
if (!is_empty_blob_sha1(ce->oid.hash))
changed |= DATA_CHANGED;
}

return changed;
}

static int is_racy_stat(const struct index_state *istate,
const struct stat_data *sd)
{
return (istate->timestamp.sec &&
#if defined(USE_NSEC)

(istate->timestamp.sec < sd->sd_mtime.sec ||
(istate->timestamp.sec == sd->sd_mtime.sec &&
istate->timestamp.nsec <= sd->sd_mtime.nsec))
#else
istate->timestamp.sec <= sd->sd_mtime.sec
#endif
);
}

int is_racy_timestamp(const struct index_state *istate,
const struct cache_entry *ce)
{
return (!S_ISGITLINK(ce->ce_mode) &&
is_racy_stat(istate, &ce->ce_stat_data));
}

int match_stat_data_racy(const struct index_state *istate,
const struct stat_data *sd, struct stat *st)
{
if (is_racy_stat(istate, sd))
return MTIME_CHANGED;
return match_stat_data(sd, st);
}

int ie_match_stat(struct index_state *istate,
const struct cache_entry *ce, struct stat *st,
unsigned int options)
{
unsigned int changed;
int ignore_valid = options & CE_MATCH_IGNORE_VALID;
int ignore_skip_worktree = options & CE_MATCH_IGNORE_SKIP_WORKTREE;
int assume_racy_is_modified = options & CE_MATCH_RACY_IS_DIRTY;
int ignore_fsmonitor = options & CE_MATCH_IGNORE_FSMONITOR;

if (!ignore_fsmonitor)
refresh_fsmonitor(istate);






if (!ignore_skip_worktree && ce_skip_worktree(ce))
return 0;
if (!ignore_valid && (ce->ce_flags & CE_VALID))
return 0;
if (!ignore_fsmonitor && (ce->ce_flags & CE_FSMONITOR_VALID))
return 0;






if (ce_intent_to_add(ce))
return DATA_CHANGED | TYPE_CHANGED | MODE_CHANGED;

changed = ce_match_stat_basic(ce, st);

















if (!changed && is_racy_timestamp(istate, ce)) {
if (assume_racy_is_modified)
changed |= DATA_CHANGED;
else
changed |= ce_modified_check_fs(istate, ce, st);
}

return changed;
}

int ie_modified(struct index_state *istate,
const struct cache_entry *ce,
struct stat *st, unsigned int options)
{
int changed, changed_fs;

changed = ie_match_stat(istate, ce, st, options);
if (!changed)
return 0;




if (changed & (MODE_CHANGED | TYPE_CHANGED))
return changed;















if ((changed & DATA_CHANGED) &&
(S_ISGITLINK(ce->ce_mode) || ce->ce_stat_data.sd_size != 0))
return changed;

changed_fs = ce_modified_check_fs(istate, ce, st);
if (changed_fs)
return changed | changed_fs;
return 0;
}

int base_name_compare(const char *name1, int len1, int mode1,
const char *name2, int len2, int mode2)
{
unsigned char c1, c2;
int len = len1 < len2 ? len1 : len2;
int cmp;

cmp = memcmp(name1, name2, len);
if (cmp)
return cmp;
c1 = name1[len];
c2 = name2[len];
if (!c1 && S_ISDIR(mode1))
c1 = '/';
if (!c2 && S_ISDIR(mode2))
c2 = '/';
return (c1 < c2) ? -1 : (c1 > c2) ? 1 : 0;
}











int df_name_compare(const char *name1, int len1, int mode1,
const char *name2, int len2, int mode2)
{
int len = len1 < len2 ? len1 : len2, cmp;
unsigned char c1, c2;

cmp = memcmp(name1, name2, len);
if (cmp)
return cmp;

if (len1 == len2)
return 0;
c1 = name1[len];
if (!c1 && S_ISDIR(mode1))
c1 = '/';
c2 = name2[len];
if (!c2 && S_ISDIR(mode2))
c2 = '/';
if (c1 == '/' && !c2)
return 0;
if (c2 == '/' && !c1)
return 0;
return c1 - c2;
}

int name_compare(const char *name1, size_t len1, const char *name2, size_t len2)
{
size_t min_len = (len1 < len2) ? len1 : len2;
int cmp = memcmp(name1, name2, min_len);
if (cmp)
return cmp;
if (len1 < len2)
return -1;
if (len1 > len2)
return 1;
return 0;
}

int cache_name_stage_compare(const char *name1, int len1, int stage1, const char *name2, int len2, int stage2)
{
int cmp;

cmp = name_compare(name1, len1, name2, len2);
if (cmp)
return cmp;

if (stage1 < stage2)
return -1;
if (stage1 > stage2)
return 1;
return 0;
}

static int index_name_stage_pos(const struct index_state *istate, const char *name, int namelen, int stage)
{
int first, last;

first = 0;
last = istate->cache_nr;
while (last > first) {
int next = first + ((last - first) >> 1);
struct cache_entry *ce = istate->cache[next];
int cmp = cache_name_stage_compare(name, namelen, stage, ce->name, ce_namelen(ce), ce_stage(ce));
if (!cmp)
return next;
if (cmp < 0) {
last = next;
continue;
}
first = next+1;
}
return -first-1;
}

int index_name_pos(const struct index_state *istate, const char *name, int namelen)
{
return index_name_stage_pos(istate, name, namelen, 0);
}

int remove_index_entry_at(struct index_state *istate, int pos)
{
struct cache_entry *ce = istate->cache[pos];

record_resolve_undo(istate, ce);
remove_name_hash(istate, ce);
save_or_free_index_entry(istate, ce);
istate->cache_changed |= CE_ENTRY_REMOVED;
istate->cache_nr--;
if (pos >= istate->cache_nr)
return 0;
MOVE_ARRAY(istate->cache + pos, istate->cache + pos + 1,
istate->cache_nr - pos);
return 1;
}






void remove_marked_cache_entries(struct index_state *istate, int invalidate)
{
struct cache_entry **ce_array = istate->cache;
unsigned int i, j;

for (i = j = 0; i < istate->cache_nr; i++) {
if (ce_array[i]->ce_flags & CE_REMOVE) {
if (invalidate) {
cache_tree_invalidate_path(istate,
ce_array[i]->name);
untracked_cache_remove_from_index(istate,
ce_array[i]->name);
}
remove_name_hash(istate, ce_array[i]);
save_or_free_index_entry(istate, ce_array[i]);
}
else
ce_array[j++] = ce_array[i];
}
if (j == istate->cache_nr)
return;
istate->cache_changed |= CE_ENTRY_REMOVED;
istate->cache_nr = j;
}

int remove_file_from_index(struct index_state *istate, const char *path)
{
int pos = index_name_pos(istate, path, strlen(path));
if (pos < 0)
pos = -pos-1;
cache_tree_invalidate_path(istate, path);
untracked_cache_remove_from_index(istate, path);
while (pos < istate->cache_nr && !strcmp(istate->cache[pos]->name, path))
remove_index_entry_at(istate, pos);
return 0;
}

static int compare_name(struct cache_entry *ce, const char *path, int namelen)
{
return namelen != ce_namelen(ce) || memcmp(path, ce->name, namelen);
}

static int index_name_pos_also_unmerged(struct index_state *istate,
const char *path, int namelen)
{
int pos = index_name_pos(istate, path, namelen);
struct cache_entry *ce;

if (pos >= 0)
return pos;


pos = -1 - pos;
if (pos >= istate->cache_nr ||
compare_name((ce = istate->cache[pos]), path, namelen))
return -1;


if (ce_stage(ce) == 1 && pos + 1 < istate->cache_nr &&
ce_stage((ce = istate->cache[pos + 1])) == 2 &&
!compare_name(ce, path, namelen))
pos++;
return pos;
}

static int different_name(struct cache_entry *ce, struct cache_entry *alias)
{
int len = ce_namelen(ce);
return ce_namelen(alias) != len || memcmp(ce->name, alias->name, len);
}










static struct cache_entry *create_alias_ce(struct index_state *istate,
struct cache_entry *ce,
struct cache_entry *alias)
{
int len;
struct cache_entry *new_entry;

if (alias->ce_flags & CE_ADDED)
die(_("will not add file alias '%s' ('%s' already exists in index)"),
ce->name, alias->name);


len = ce_namelen(alias);
new_entry = make_empty_cache_entry(istate, len);
memcpy(new_entry->name, alias->name, len);
copy_cache_entry(new_entry, ce);
save_or_free_index_entry(istate, ce);
return new_entry;
}

void set_object_name_for_intent_to_add_entry(struct cache_entry *ce)
{
struct object_id oid;
if (write_object_file("", 0, blob_type, &oid))
die(_("cannot create an empty blob in the object database"));
oidcpy(&ce->oid, &oid);
}

int add_to_index(struct index_state *istate, const char *path, struct stat *st, int flags)
{
int namelen, was_same;
mode_t st_mode = st->st_mode;
struct cache_entry *ce, *alias = NULL;
unsigned ce_option = CE_MATCH_IGNORE_VALID|CE_MATCH_IGNORE_SKIP_WORKTREE|CE_MATCH_RACY_IS_DIRTY;
int verbose = flags & (ADD_CACHE_VERBOSE | ADD_CACHE_PRETEND);
int pretend = flags & ADD_CACHE_PRETEND;
int intent_only = flags & ADD_CACHE_INTENT;
int add_option = (ADD_CACHE_OK_TO_ADD|ADD_CACHE_OK_TO_REPLACE|
(intent_only ? ADD_CACHE_NEW_ONLY : 0));
int hash_flags = HASH_WRITE_OBJECT;
struct object_id oid;

if (flags & ADD_CACHE_RENORMALIZE)
hash_flags |= HASH_RENORMALIZE;

if (!S_ISREG(st_mode) && !S_ISLNK(st_mode) && !S_ISDIR(st_mode))
return error(_("%s: can only add regular files, symbolic links or git-directories"), path);

namelen = strlen(path);
if (S_ISDIR(st_mode)) {
if (resolve_gitlink_ref(path, "HEAD", &oid) < 0)
return error(_("'%s' does not have a commit checked out"), path);
while (namelen && path[namelen-1] == '/')
namelen--;
}
ce = make_empty_cache_entry(istate, namelen);
memcpy(ce->name, path, namelen);
ce->ce_namelen = namelen;
if (!intent_only)
fill_stat_cache_info(istate, ce, st);
else
ce->ce_flags |= CE_INTENT_TO_ADD;


if (trust_executable_bit && has_symlinks) {
ce->ce_mode = create_ce_mode(st_mode);
} else {



struct cache_entry *ent;
int pos = index_name_pos_also_unmerged(istate, path, namelen);

ent = (0 <= pos) ? istate->cache[pos] : NULL;
ce->ce_mode = ce_mode_from_stat(ent, st_mode);
}






if (ignore_case) {
adjust_dirname_case(istate, ce->name);
}
if (!(flags & ADD_CACHE_RENORMALIZE)) {
alias = index_file_exists(istate, ce->name,
ce_namelen(ce), ignore_case);
if (alias &&
!ce_stage(alias) &&
!ie_match_stat(istate, alias, st, ce_option)) {

if (!S_ISGITLINK(alias->ce_mode))
ce_mark_uptodate(alias);
alias->ce_flags |= CE_ADDED;

discard_cache_entry(ce);
return 0;
}
}
if (!intent_only) {
if (index_path(istate, &ce->oid, path, st, hash_flags)) {
discard_cache_entry(ce);
return error(_("unable to index file '%s'"), path);
}
} else
set_object_name_for_intent_to_add_entry(ce);

if (ignore_case && alias && different_name(ce, alias))
ce = create_alias_ce(istate, ce, alias);
ce->ce_flags |= CE_ADDED;


was_same = (alias &&
!ce_stage(alias) &&
oideq(&alias->oid, &ce->oid) &&
ce->ce_mode == alias->ce_mode);

if (pretend)
discard_cache_entry(ce);
else if (add_index_entry(istate, ce, add_option)) {
discard_cache_entry(ce);
return error(_("unable to add '%s' to index"), path);
}
if (verbose && !was_same)
printf("add '%s'\n", path);
return 0;
}

int add_file_to_index(struct index_state *istate, const char *path, int flags)
{
struct stat st;
if (lstat(path, &st))
die_errno(_("unable to stat '%s'"), path);
return add_to_index(istate, path, &st, flags);
}

struct cache_entry *make_empty_cache_entry(struct index_state *istate, size_t len)
{
return mem_pool__ce_calloc(find_mem_pool(istate), len);
}

struct cache_entry *make_empty_transient_cache_entry(size_t len)
{
return xcalloc(1, cache_entry_size(len));
}

struct cache_entry *make_cache_entry(struct index_state *istate,
unsigned int mode,
const struct object_id *oid,
const char *path,
int stage,
unsigned int refresh_options)
{
struct cache_entry *ce, *ret;
int len;

if (!verify_path(path, mode)) {
error(_("invalid path '%s'"), path);
return NULL;
}

len = strlen(path);
ce = make_empty_cache_entry(istate, len);

oidcpy(&ce->oid, oid);
memcpy(ce->name, path, len);
ce->ce_flags = create_ce_flags(stage);
ce->ce_namelen = len;
ce->ce_mode = create_ce_mode(mode);

ret = refresh_cache_entry(istate, ce, refresh_options);
if (ret != ce)
discard_cache_entry(ce);
return ret;
}

struct cache_entry *make_transient_cache_entry(unsigned int mode, const struct object_id *oid,
const char *path, int stage)
{
struct cache_entry *ce;
int len;

if (!verify_path(path, mode)) {
error(_("invalid path '%s'"), path);
return NULL;
}

len = strlen(path);
ce = make_empty_transient_cache_entry(len);

oidcpy(&ce->oid, oid);
memcpy(ce->name, path, len);
ce->ce_flags = create_ce_flags(stage);
ce->ce_namelen = len;
ce->ce_mode = create_ce_mode(mode);

return ce;
}








int chmod_index_entry(struct index_state *istate, struct cache_entry *ce,
char flip)
{
if (!S_ISREG(ce->ce_mode))
return -1;
switch (flip) {
case '+':
ce->ce_mode |= 0111;
break;
case '-':
ce->ce_mode &= ~0111;
break;
default:
return -2;
}
cache_tree_invalidate_path(istate, ce->name);
ce->ce_flags |= CE_UPDATE_IN_BASE;
mark_fsmonitor_invalid(istate, ce);
istate->cache_changed |= CE_ENTRY_CHANGED;

return 0;
}

int ce_same_name(const struct cache_entry *a, const struct cache_entry *b)
{
int len = ce_namelen(a);
return ce_namelen(b) == len && !memcmp(a->name, b->name, len);
}









static int verify_dotfile(const char *rest, unsigned mode)
{







if (*rest == '\0' || is_dir_sep(*rest))
return 0;

switch (*rest) {









case 'g':
case 'G':
if (rest[1] != 'i' && rest[1] != 'I')
break;
if (rest[2] != 't' && rest[2] != 'T')
break;
if (rest[3] == '\0' || is_dir_sep(rest[3]))
return 0;
if (S_ISLNK(mode)) {
rest += 3;
if (skip_iprefix(rest, "modules", &rest) &&
(*rest == '\0' || is_dir_sep(*rest)))
return 0;
}
break;
case '.':
if (rest[1] == '\0' || is_dir_sep(rest[1]))
return 0;
}
return 1;
}

int verify_path(const char *path, unsigned mode)
{
char c = 0;

if (has_dos_drive_prefix(path))
return 0;

if (!is_valid_path(path))
return 0;

goto inside;
for (;;) {
if (!c)
return 1;
if (is_dir_sep(c)) {
inside:
if (protect_hfs) {

if (is_hfs_dotgit(path))
return 0;
if (S_ISLNK(mode)) {
if (is_hfs_dotgitmodules(path))
return 0;
}
}
if (protect_ntfs) {
#if defined(GIT_WINDOWS_NATIVE)
if (c == '\\')
return 0;
#endif
if (is_ntfs_dotgit(path))
return 0;
if (S_ISLNK(mode)) {
if (is_ntfs_dotgitmodules(path))
return 0;
}
}

c = *path++;
if ((c == '.' && !verify_dotfile(path, mode)) ||
is_dir_sep(c) || c == '\0')
return 0;
} else if (c == '\\' && protect_ntfs) {
if (is_ntfs_dotgit(path))
return 0;
if (S_ISLNK(mode)) {
if (is_ntfs_dotgitmodules(path))
return 0;
}
}

c = *path++;
}
}





static int has_file_name(struct index_state *istate,
const struct cache_entry *ce, int pos, int ok_to_replace)
{
int retval = 0;
int len = ce_namelen(ce);
int stage = ce_stage(ce);
const char *name = ce->name;

while (pos < istate->cache_nr) {
struct cache_entry *p = istate->cache[pos++];

if (len >= ce_namelen(p))
break;
if (memcmp(name, p->name, len))
break;
if (ce_stage(p) != stage)
continue;
if (p->name[len] != '/')
continue;
if (p->ce_flags & CE_REMOVE)
continue;
retval = -1;
if (!ok_to_replace)
break;
remove_index_entry_at(istate, --pos);
}
return retval;
}






int strcmp_offset(const char *s1, const char *s2, size_t *first_change)
{
size_t k;

if (!first_change)
return strcmp(s1, s2);

for (k = 0; s1[k] == s2[k]; k++)
if (s1[k] == '\0')
break;

*first_change = k;
return (unsigned char)s1[k] - (unsigned char)s2[k];
}








static int has_dir_name(struct index_state *istate,
const struct cache_entry *ce, int pos, int ok_to_replace)
{
int retval = 0;
int stage = ce_stage(ce);
const char *name = ce->name;
const char *slash = name + ce_namelen(ce);
size_t len_eq_last;
int cmp_last = 0;











if (istate->cache_nr > 0) {
cmp_last = strcmp_offset(name,
istate->cache[istate->cache_nr - 1]->name,
&len_eq_last);
if (cmp_last > 0) {
if (len_eq_last == 0) {





return retval;
} else {






}
} else if (cmp_last == 0) {






}
}

for (;;) {
size_t len;

for (;;) {
if (*--slash == '/')
break;
if (slash <= ce->name)
return retval;
}
len = slash - name;

if (cmp_last > 0) {








if (len + 1 <= len_eq_last) {












return retval;
}

if (len > len_eq_last) {









return retval;
}

if (istate->cache_nr > 0 &&
ce_namelen(istate->cache[istate->cache_nr - 1]) > len) {









return retval;
}








}

pos = index_name_stage_pos(istate, name, len, stage);
if (pos >= 0) {








if (!(istate->cache[pos]->ce_flags & CE_REMOVE)) {
retval = -1;
if (!ok_to_replace)
break;
remove_index_entry_at(istate, pos);
continue;
}
}
else
pos = -pos-1;






while (pos < istate->cache_nr) {
struct cache_entry *p = istate->cache[pos];
if ((ce_namelen(p) <= len) ||
(p->name[len] != '/') ||
memcmp(p->name, name, len))
break; 
if (ce_stage(p) == stage && !(p->ce_flags & CE_REMOVE))






return retval;
pos++;
}
}
return retval;
}










static int check_file_directory_conflict(struct index_state *istate,
const struct cache_entry *ce,
int pos, int ok_to_replace)
{
int retval;




if (ce->ce_flags & CE_REMOVE)
return 0;






retval = has_file_name(istate, ce, pos, ok_to_replace);





return retval + has_dir_name(istate, ce, pos, ok_to_replace);
}

static int add_index_entry_with_check(struct index_state *istate, struct cache_entry *ce, int option)
{
int pos;
int ok_to_add = option & ADD_CACHE_OK_TO_ADD;
int ok_to_replace = option & ADD_CACHE_OK_TO_REPLACE;
int skip_df_check = option & ADD_CACHE_SKIP_DFCHECK;
int new_only = option & ADD_CACHE_NEW_ONLY;

if (!(option & ADD_CACHE_KEEP_CACHE_TREE))
cache_tree_invalidate_path(istate, ce->name);





if (istate->cache_nr > 0 &&
strcmp(ce->name, istate->cache[istate->cache_nr - 1]->name) > 0)
pos = index_pos_to_insert_pos(istate->cache_nr);
else
pos = index_name_stage_pos(istate, ce->name, ce_namelen(ce), ce_stage(ce));


if (pos >= 0) {
if (!new_only)
replace_index_entry(istate, pos, ce);
return 0;
}
pos = -pos-1;

if (!(option & ADD_CACHE_KEEP_CACHE_TREE))
untracked_cache_add_to_index(istate, ce->name);





if (pos < istate->cache_nr && ce_stage(ce) == 0) {
while (ce_same_name(istate->cache[pos], ce)) {
ok_to_add = 1;
if (!remove_index_entry_at(istate, pos))
break;
}
}

if (!ok_to_add)
return -1;
if (!verify_path(ce->name, ce->ce_mode))
return error(_("invalid path '%s'"), ce->name);

if (!skip_df_check &&
check_file_directory_conflict(istate, ce, pos, ok_to_replace)) {
if (!ok_to_replace)
return error(_("'%s' appears as both a file and as a directory"),
ce->name);
pos = index_name_stage_pos(istate, ce->name, ce_namelen(ce), ce_stage(ce));
pos = -pos-1;
}
return pos + 1;
}

int add_index_entry(struct index_state *istate, struct cache_entry *ce, int option)
{
int pos;

if (option & ADD_CACHE_JUST_APPEND)
pos = istate->cache_nr;
else {
int ret;
ret = add_index_entry_with_check(istate, ce, option);
if (ret <= 0)
return ret;
pos = ret - 1;
}


ALLOC_GROW(istate->cache, istate->cache_nr + 1, istate->cache_alloc);


istate->cache_nr++;
if (istate->cache_nr > pos + 1)
MOVE_ARRAY(istate->cache + pos + 1, istate->cache + pos,
istate->cache_nr - pos - 1);
set_index_entry(istate, pos, ce);
istate->cache_changed |= CE_ENTRY_ADDED;
return 0;
}












static struct cache_entry *refresh_cache_ent(struct index_state *istate,
struct cache_entry *ce,
unsigned int options, int *err,
int *changed_ret)
{
struct stat st;
struct cache_entry *updated;
int changed;
int refresh = options & CE_MATCH_REFRESH;
int ignore_valid = options & CE_MATCH_IGNORE_VALID;
int ignore_skip_worktree = options & CE_MATCH_IGNORE_SKIP_WORKTREE;
int ignore_missing = options & CE_MATCH_IGNORE_MISSING;
int ignore_fsmonitor = options & CE_MATCH_IGNORE_FSMONITOR;

if (!refresh || ce_uptodate(ce))
return ce;

if (!ignore_fsmonitor)
refresh_fsmonitor(istate);





if (!ignore_skip_worktree && ce_skip_worktree(ce)) {
ce_mark_uptodate(ce);
return ce;
}
if (!ignore_valid && (ce->ce_flags & CE_VALID)) {
ce_mark_uptodate(ce);
return ce;
}
if (!ignore_fsmonitor && (ce->ce_flags & CE_FSMONITOR_VALID)) {
ce_mark_uptodate(ce);
return ce;
}

if (has_symlink_leading_path(ce->name, ce_namelen(ce))) {
if (ignore_missing)
return ce;
if (err)
*err = ENOENT;
return NULL;
}

if (lstat(ce->name, &st) < 0) {
if (ignore_missing && errno == ENOENT)
return ce;
if (err)
*err = errno;
return NULL;
}

changed = ie_match_stat(istate, ce, &st, options);
if (changed_ret)
*changed_ret = changed;
if (!changed) {







if (ignore_valid && assume_unchanged &&
!(ce->ce_flags & CE_VALID))
; 
else {





if (!S_ISGITLINK(ce->ce_mode)) {
ce_mark_uptodate(ce);
mark_fsmonitor_valid(istate, ce);
}
return ce;
}
}

if (ie_modified(istate, ce, &st, options)) {
if (err)
*err = EINVAL;
return NULL;
}

updated = make_empty_cache_entry(istate, ce_namelen(ce));
copy_cache_entry(updated, ce);
memcpy(updated->name, ce->name, ce->ce_namelen + 1);
fill_stat_cache_info(istate, updated, &st);






if (!ignore_valid && assume_unchanged &&
!(ce->ce_flags & CE_VALID))
updated->ce_flags &= ~CE_VALID;


return updated;
}

static void show_file(const char * fmt, const char * name, int in_porcelain,
int * first, const char *header_msg)
{
if (in_porcelain && *first && header_msg) {
printf("%s\n", header_msg);
*first = 0;
}
printf(fmt, name);
}

int repo_refresh_and_write_index(struct repository *repo,
unsigned int refresh_flags,
unsigned int write_flags,
int gentle,
const struct pathspec *pathspec,
char *seen, const char *header_msg)
{
struct lock_file lock_file = LOCK_INIT;
int fd, ret = 0;

fd = repo_hold_locked_index(repo, &lock_file, 0);
if (!gentle && fd < 0)
return -1;
if (refresh_index(repo->index, refresh_flags, pathspec, seen, header_msg))
ret = 1;
if (0 <= fd && write_locked_index(repo->index, &lock_file, COMMIT_LOCK | write_flags))
ret = -1;
return ret;
}


int refresh_index(struct index_state *istate, unsigned int flags,
const struct pathspec *pathspec,
char *seen, const char *header_msg)
{
int i;
int has_errors = 0;
int really = (flags & REFRESH_REALLY) != 0;
int allow_unmerged = (flags & REFRESH_UNMERGED) != 0;
int quiet = (flags & REFRESH_QUIET) != 0;
int not_new = (flags & REFRESH_IGNORE_MISSING) != 0;
int ignore_submodules = (flags & REFRESH_IGNORE_SUBMODULES) != 0;
int first = 1;
int in_porcelain = (flags & REFRESH_IN_PORCELAIN);
unsigned int options = (CE_MATCH_REFRESH |
(really ? CE_MATCH_IGNORE_VALID : 0) |
(not_new ? CE_MATCH_IGNORE_MISSING : 0));
const char *modified_fmt;
const char *deleted_fmt;
const char *typechange_fmt;
const char *added_fmt;
const char *unmerged_fmt;
struct progress *progress = NULL;

if (flags & REFRESH_PROGRESS && isatty(2))
progress = start_delayed_progress(_("Refresh index"),
istate->cache_nr);

trace_performance_enter();
modified_fmt = in_porcelain ? "M\t%s\n" : "%s: needs update\n";
deleted_fmt = in_porcelain ? "D\t%s\n" : "%s: needs update\n";
typechange_fmt = in_porcelain ? "T\t%s\n" : "%s: needs update\n";
added_fmt = in_porcelain ? "A\t%s\n" : "%s: needs update\n";
unmerged_fmt = in_porcelain ? "U\t%s\n" : "%s: needs merge\n";





preload_index(istate, pathspec, 0);
for (i = 0; i < istate->cache_nr; i++) {
struct cache_entry *ce, *new_entry;
int cache_errno = 0;
int changed = 0;
int filtered = 0;

ce = istate->cache[i];
if (ignore_submodules && S_ISGITLINK(ce->ce_mode))
continue;

if (pathspec && !ce_path_match(istate, ce, pathspec, seen))
filtered = 1;

if (ce_stage(ce)) {
while ((i < istate->cache_nr) &&
! strcmp(istate->cache[i]->name, ce->name))
i++;
i--;
if (allow_unmerged)
continue;
if (!filtered)
show_file(unmerged_fmt, ce->name, in_porcelain,
&first, header_msg);
has_errors = 1;
continue;
}

if (filtered)
continue;

new_entry = refresh_cache_ent(istate, ce, options, &cache_errno, &changed);
if (new_entry == ce)
continue;
if (progress)
display_progress(progress, i);
if (!new_entry) {
const char *fmt;

if (really && cache_errno == EINVAL) {



ce->ce_flags &= ~CE_VALID;
ce->ce_flags |= CE_UPDATE_IN_BASE;
mark_fsmonitor_invalid(istate, ce);
istate->cache_changed |= CE_ENTRY_CHANGED;
}
if (quiet)
continue;

if (cache_errno == ENOENT)
fmt = deleted_fmt;
else if (ce_intent_to_add(ce))
fmt = added_fmt; 
else if (changed & TYPE_CHANGED)
fmt = typechange_fmt;
else
fmt = modified_fmt;
show_file(fmt,
ce->name, in_porcelain, &first, header_msg);
has_errors = 1;
continue;
}

replace_index_entry(istate, i, new_entry);
}
if (progress) {
display_progress(progress, istate->cache_nr);
stop_progress(&progress);
}
trace_performance_leave("refresh index");
return has_errors;
}

struct cache_entry *refresh_cache_entry(struct index_state *istate,
struct cache_entry *ce,
unsigned int options)
{
return refresh_cache_ent(istate, ce, options, NULL, NULL);
}






#define INDEX_FORMAT_DEFAULT 3

static unsigned int get_index_format_default(struct repository *r)
{
char *envversion = getenv("GIT_INDEX_VERSION");
char *endp;
unsigned int version = INDEX_FORMAT_DEFAULT;

if (!envversion) {
prepare_repo_settings(r);

if (r->settings.index_version >= 0)
version = r->settings.index_version;
if (version < INDEX_FORMAT_LB || INDEX_FORMAT_UB < version) {
warning(_("index.version set, but the value is invalid.\n"
"Using version %i"), INDEX_FORMAT_DEFAULT);
return INDEX_FORMAT_DEFAULT;
}
return version;
}

version = strtoul(envversion, &endp, 10);
if (*endp ||
version < INDEX_FORMAT_LB || INDEX_FORMAT_UB < version) {
warning(_("GIT_INDEX_VERSION set, but the value is invalid.\n"
"Using version %i"), INDEX_FORMAT_DEFAULT);
version = INDEX_FORMAT_DEFAULT;
}
return version;
}









struct ondisk_cache_entry {
struct cache_time ctime;
struct cache_time mtime;
uint32_t dev;
uint32_t ino;
uint32_t mode;
uint32_t uid;
uint32_t gid;
uint32_t size;






unsigned char data[GIT_MAX_RAWSZ + 2 * sizeof(uint16_t)];
char name[FLEX_ARRAY];
};


#define align_padding_size(size, len) ((size + (len) + 8) & ~7) - (size + len)
#define align_flex_name(STRUCT,len) ((offsetof(struct STRUCT,data) + (len) + 8) & ~7)
#define ondisk_cache_entry_size(len) align_flex_name(ondisk_cache_entry,len)
#define ondisk_data_size(flags, len) (the_hash_algo->rawsz + ((flags & CE_EXTENDED) ? 2 : 1) * sizeof(uint16_t) + len)

#define ondisk_data_size_max(len) (ondisk_data_size(CE_EXTENDED, len))
#define ondisk_ce_size(ce) (ondisk_cache_entry_size(ondisk_data_size((ce)->ce_flags, ce_namelen(ce))))


int verify_index_checksum;


int verify_ce_order;

static int verify_hdr(const struct cache_header *hdr, unsigned long size)
{
git_hash_ctx c;
unsigned char hash[GIT_MAX_RAWSZ];
int hdr_version;

if (hdr->hdr_signature != htonl(CACHE_SIGNATURE))
return error(_("bad signature 0x%08x"), hdr->hdr_signature);
hdr_version = ntohl(hdr->hdr_version);
if (hdr_version < INDEX_FORMAT_LB || INDEX_FORMAT_UB < hdr_version)
return error(_("bad index version %d"), hdr_version);

if (!verify_index_checksum)
return 0;

the_hash_algo->init_fn(&c);
the_hash_algo->update_fn(&c, hdr, size - the_hash_algo->rawsz);
the_hash_algo->final_fn(hash, &c);
if (!hasheq(hash, (unsigned char *)hdr + size - the_hash_algo->rawsz))
return error(_("bad index file sha1 signature"));
return 0;
}

static int read_index_extension(struct index_state *istate,
const char *ext, const char *data, unsigned long sz)
{
switch (CACHE_EXT(ext)) {
case CACHE_EXT_TREE:
istate->cache_tree = cache_tree_read(data, sz);
break;
case CACHE_EXT_RESOLVE_UNDO:
istate->resolve_undo = resolve_undo_read(data, sz);
break;
case CACHE_EXT_LINK:
if (read_link_extension(istate, data, sz))
return -1;
break;
case CACHE_EXT_UNTRACKED:
istate->untracked = read_untracked_extension(data, sz);
break;
case CACHE_EXT_FSMONITOR:
read_fsmonitor_extension(istate, data, sz);
break;
case CACHE_EXT_ENDOFINDEXENTRIES:
case CACHE_EXT_INDEXENTRYOFFSETTABLE:

break;
default:
if (*ext < 'A' || 'Z' < *ext)
return error(_("index uses %.4s extension, which we do not understand"),
ext);
fprintf_ln(stderr, _("ignoring %.4s extension"), ext);
break;
}
return 0;
}

static struct cache_entry *create_from_disk(struct mem_pool *ce_mem_pool,
unsigned int version,
struct ondisk_cache_entry *ondisk,
unsigned long *ent_size,
const struct cache_entry *previous_ce)
{
struct cache_entry *ce;
size_t len;
const char *name;
const unsigned hashsz = the_hash_algo->rawsz;
const uint16_t *flagsp = (const uint16_t *)(ondisk->data + hashsz);
unsigned int flags;
size_t copy_len = 0;







int expand_name_field = version == 4;


flags = get_be16(flagsp);
len = flags & CE_NAMEMASK;

if (flags & CE_EXTENDED) {
int extended_flags;
extended_flags = get_be16(flagsp + 1) << 16;

if (extended_flags & ~CE_EXTENDED_FLAGS)
die(_("unknown index entry format 0x%08x"), extended_flags);
flags |= extended_flags;
name = (const char *)(flagsp + 2);
}
else
name = (const char *)(flagsp + 1);

if (expand_name_field) {
const unsigned char *cp = (const unsigned char *)name;
size_t strip_len, previous_len;


strip_len = decode_varint(&cp);
if (previous_ce) {
previous_len = previous_ce->ce_namelen;
if (previous_len < strip_len)
die(_("malformed name field in the index, near path '%s'"),
previous_ce->name);
copy_len = previous_len - strip_len;
}
name = (const char *)cp;
}

if (len == CE_NAMEMASK) {
len = strlen(name);
if (expand_name_field)
len += copy_len;
}

ce = mem_pool__ce_alloc(ce_mem_pool, len);

ce->ce_stat_data.sd_ctime.sec = get_be32(&ondisk->ctime.sec);
ce->ce_stat_data.sd_mtime.sec = get_be32(&ondisk->mtime.sec);
ce->ce_stat_data.sd_ctime.nsec = get_be32(&ondisk->ctime.nsec);
ce->ce_stat_data.sd_mtime.nsec = get_be32(&ondisk->mtime.nsec);
ce->ce_stat_data.sd_dev = get_be32(&ondisk->dev);
ce->ce_stat_data.sd_ino = get_be32(&ondisk->ino);
ce->ce_mode = get_be32(&ondisk->mode);
ce->ce_stat_data.sd_uid = get_be32(&ondisk->uid);
ce->ce_stat_data.sd_gid = get_be32(&ondisk->gid);
ce->ce_stat_data.sd_size = get_be32(&ondisk->size);
ce->ce_flags = flags & ~CE_NAMEMASK;
ce->ce_namelen = len;
ce->index = 0;
hashcpy(ce->oid.hash, ondisk->data);
memcpy(ce->name, name, len);
ce->name[len] = '\0';

if (expand_name_field) {
if (copy_len)
memcpy(ce->name, previous_ce->name, copy_len);
memcpy(ce->name + copy_len, name, len + 1 - copy_len);
*ent_size = (name - ((char *)ondisk)) + len + 1 - copy_len;
} else {
memcpy(ce->name, name, len + 1);
*ent_size = ondisk_ce_size(ce);
}
return ce;
}

static void check_ce_order(struct index_state *istate)
{
unsigned int i;

if (!verify_ce_order)
return;

for (i = 1; i < istate->cache_nr; i++) {
struct cache_entry *ce = istate->cache[i - 1];
struct cache_entry *next_ce = istate->cache[i];
int name_compare = strcmp(ce->name, next_ce->name);

if (0 < name_compare)
die(_("unordered stage entries in index"));
if (!name_compare) {
if (!ce_stage(ce))
die(_("multiple stage entries for merged file '%s'"),
ce->name);
if (ce_stage(ce) > ce_stage(next_ce))
die(_("unordered stage entries for '%s'"),
ce->name);
}
}
}

static void tweak_untracked_cache(struct index_state *istate)
{
struct repository *r = the_repository;

prepare_repo_settings(r);

if (r->settings.core_untracked_cache == UNTRACKED_CACHE_REMOVE) {
remove_untracked_cache(istate);
return;
}

if (r->settings.core_untracked_cache == UNTRACKED_CACHE_WRITE)
add_untracked_cache(istate);
}

static void tweak_split_index(struct index_state *istate)
{
switch (git_config_get_split_index()) {
case -1: 
break;
case 0: 
remove_split_index(istate);
break;
case 1: 
add_split_index(istate);
break;
default: 
break;
}
}

static void post_read_index_from(struct index_state *istate)
{
check_ce_order(istate);
tweak_untracked_cache(istate);
tweak_split_index(istate);
tweak_fsmonitor(istate);
}

static size_t estimate_cache_size_from_compressed(unsigned int entries)
{
return entries * (sizeof(struct cache_entry) + CACHE_ENTRY_PATH_LENGTH);
}

static size_t estimate_cache_size(size_t ondisk_size, unsigned int entries)
{
long per_entry = sizeof(struct cache_entry) - sizeof(struct ondisk_cache_entry);




per_entry += align_padding_size(per_entry, 0);
return ondisk_size + entries * per_entry;
}

struct index_entry_offset
{

int offset, nr;
};

struct index_entry_offset_table
{
int nr;
struct index_entry_offset entries[FLEX_ARRAY];
};

static struct index_entry_offset_table *read_ieot_extension(const char *mmap, size_t mmap_size, size_t offset);
static void write_ieot_extension(struct strbuf *sb, struct index_entry_offset_table *ieot);

static size_t read_eoie_extension(const char *mmap, size_t mmap_size);
static void write_eoie_extension(struct strbuf *sb, git_hash_ctx *eoie_context, size_t offset);

struct load_index_extensions
{
pthread_t pthread;
struct index_state *istate;
const char *mmap;
size_t mmap_size;
unsigned long src_offset;
};

static void *load_index_extensions(void *_data)
{
struct load_index_extensions *p = _data;
unsigned long src_offset = p->src_offset;

while (src_offset <= p->mmap_size - the_hash_algo->rawsz - 8) {






uint32_t extsize = get_be32(p->mmap + src_offset + 4);
if (read_index_extension(p->istate,
p->mmap + src_offset,
p->mmap + src_offset + 8,
extsize) < 0) {
munmap((void *)p->mmap, p->mmap_size);
die(_("index file corrupt"));
}
src_offset += 8;
src_offset += extsize;
}

return NULL;
}





static unsigned long load_cache_entry_block(struct index_state *istate,
struct mem_pool *ce_mem_pool, int offset, int nr, const char *mmap,
unsigned long start_offset, const struct cache_entry *previous_ce)
{
int i;
unsigned long src_offset = start_offset;

for (i = offset; i < offset + nr; i++) {
struct ondisk_cache_entry *disk_ce;
struct cache_entry *ce;
unsigned long consumed;

disk_ce = (struct ondisk_cache_entry *)(mmap + src_offset);
ce = create_from_disk(ce_mem_pool, istate->version, disk_ce, &consumed, previous_ce);
set_index_entry(istate, i, ce);

src_offset += consumed;
previous_ce = ce;
}
return src_offset - start_offset;
}

static unsigned long load_all_cache_entries(struct index_state *istate,
const char *mmap, size_t mmap_size, unsigned long src_offset)
{
unsigned long consumed;

if (istate->version == 4) {
mem_pool_init(&istate->ce_mem_pool,
estimate_cache_size_from_compressed(istate->cache_nr));
} else {
mem_pool_init(&istate->ce_mem_pool,
estimate_cache_size(mmap_size, istate->cache_nr));
}

consumed = load_cache_entry_block(istate, istate->ce_mem_pool,
0, istate->cache_nr, mmap, src_offset, NULL);
return consumed;
}








#define THREAD_COST (10000)

struct load_cache_entries_thread_data
{
pthread_t pthread;
struct index_state *istate;
struct mem_pool *ce_mem_pool;
int offset;
const char *mmap;
struct index_entry_offset_table *ieot;
int ieot_start; 
int ieot_blocks; 
unsigned long consumed; 
};





static void *load_cache_entries_thread(void *_data)
{
struct load_cache_entries_thread_data *p = _data;
int i;


for (i = p->ieot_start; i < p->ieot_start + p->ieot_blocks; i++) {
p->consumed += load_cache_entry_block(p->istate, p->ce_mem_pool,
p->offset, p->ieot->entries[i].nr, p->mmap, p->ieot->entries[i].offset, NULL);
p->offset += p->ieot->entries[i].nr;
}
return NULL;
}

static unsigned long load_cache_entries_threaded(struct index_state *istate, const char *mmap, size_t mmap_size,
int nr_threads, struct index_entry_offset_table *ieot)
{
int i, offset, ieot_blocks, ieot_start, err;
struct load_cache_entries_thread_data *data;
unsigned long consumed = 0;


if (istate->name_hash_initialized)
BUG("the name hash isn't thread safe");

mem_pool_init(&istate->ce_mem_pool, 0);


if (nr_threads > ieot->nr)
nr_threads = ieot->nr;
data = xcalloc(nr_threads, sizeof(*data));

offset = ieot_start = 0;
ieot_blocks = DIV_ROUND_UP(ieot->nr, nr_threads);
for (i = 0; i < nr_threads; i++) {
struct load_cache_entries_thread_data *p = &data[i];
int nr, j;

if (ieot_start + ieot_blocks > ieot->nr)
ieot_blocks = ieot->nr - ieot_start;

p->istate = istate;
p->offset = offset;
p->mmap = mmap;
p->ieot = ieot;
p->ieot_start = ieot_start;
p->ieot_blocks = ieot_blocks;


nr = 0;
for (j = p->ieot_start; j < p->ieot_start + p->ieot_blocks; j++)
nr += p->ieot->entries[j].nr;
if (istate->version == 4) {
mem_pool_init(&p->ce_mem_pool,
estimate_cache_size_from_compressed(nr));
} else {
mem_pool_init(&p->ce_mem_pool,
estimate_cache_size(mmap_size, nr));
}

err = pthread_create(&p->pthread, NULL, load_cache_entries_thread, p);
if (err)
die(_("unable to create load_cache_entries thread: %s"), strerror(err));


for (j = 0; j < ieot_blocks; j++)
offset += ieot->entries[ieot_start + j].nr;
ieot_start += ieot_blocks;
}

for (i = 0; i < nr_threads; i++) {
struct load_cache_entries_thread_data *p = &data[i];

err = pthread_join(p->pthread, NULL);
if (err)
die(_("unable to join load_cache_entries thread: %s"), strerror(err));
mem_pool_combine(istate->ce_mem_pool, p->ce_mem_pool);
consumed += p->consumed;
}

free(data);

return consumed;
}


int do_read_index(struct index_state *istate, const char *path, int must_exist)
{
int fd;
struct stat st;
unsigned long src_offset;
const struct cache_header *hdr;
const char *mmap;
size_t mmap_size;
struct load_index_extensions p;
size_t extension_offset = 0;
int nr_threads, cpus;
struct index_entry_offset_table *ieot = NULL;

if (istate->initialized)
return istate->cache_nr;

istate->timestamp.sec = 0;
istate->timestamp.nsec = 0;
fd = open(path, O_RDONLY);
if (fd < 0) {
if (!must_exist && errno == ENOENT)
return 0;
die_errno(_("%s: index file open failed"), path);
}

if (fstat(fd, &st))
die_errno(_("%s: cannot stat the open index"), path);

mmap_size = xsize_t(st.st_size);
if (mmap_size < sizeof(struct cache_header) + the_hash_algo->rawsz)
die(_("%s: index file smaller than expected"), path);

mmap = xmmap_gently(NULL, mmap_size, PROT_READ, MAP_PRIVATE, fd, 0);
if (mmap == MAP_FAILED)
die_errno(_("%s: unable to map index file"), path);
close(fd);

hdr = (const struct cache_header *)mmap;
if (verify_hdr(hdr, mmap_size) < 0)
goto unmap;

hashcpy(istate->oid.hash, (const unsigned char *)hdr + mmap_size - the_hash_algo->rawsz);
istate->version = ntohl(hdr->hdr_version);
istate->cache_nr = ntohl(hdr->hdr_entries);
istate->cache_alloc = alloc_nr(istate->cache_nr);
istate->cache = xcalloc(istate->cache_alloc, sizeof(*istate->cache));
istate->initialized = 1;

p.istate = istate;
p.mmap = mmap;
p.mmap_size = mmap_size;

src_offset = sizeof(*hdr);

if (git_config_get_index_threads(&nr_threads))
nr_threads = 1;


if (!nr_threads) {
nr_threads = istate->cache_nr / THREAD_COST;
cpus = online_cpus();
if (nr_threads > cpus)
nr_threads = cpus;
}

if (!HAVE_THREADS)
nr_threads = 1;

if (nr_threads > 1) {
extension_offset = read_eoie_extension(mmap, mmap_size);
if (extension_offset) {
int err;

p.src_offset = extension_offset;
err = pthread_create(&p.pthread, NULL, load_index_extensions, &p);
if (err)
die(_("unable to create load_index_extensions thread: %s"), strerror(err));

nr_threads--;
}
}





if (extension_offset && nr_threads > 1)
ieot = read_ieot_extension(mmap, mmap_size, extension_offset);

if (ieot) {
src_offset += load_cache_entries_threaded(istate, mmap, mmap_size, nr_threads, ieot);
free(ieot);
} else {
src_offset += load_all_cache_entries(istate, mmap, mmap_size, src_offset);
}

istate->timestamp.sec = st.st_mtime;
istate->timestamp.nsec = ST_MTIME_NSEC(st);


if (extension_offset) {
int ret = pthread_join(p.pthread, NULL);
if (ret)
die(_("unable to join load_index_extensions thread: %s"), strerror(ret));
} else {
p.src_offset = src_offset;
load_index_extensions(&p);
}
munmap((void *)mmap, mmap_size);





trace2_data_intmax("index", the_repository, "read/version",
istate->version);
trace2_data_intmax("index", the_repository, "read/cache_nr",
istate->cache_nr);

return istate->cache_nr;

unmap:
munmap((void *)mmap, mmap_size);
die(_("index file corrupt"));
}







static void freshen_shared_index(const char *shared_index, int warn)
{
if (!check_and_freshen_file(shared_index, 1) && warn)
warning(_("could not freshen shared index '%s'"), shared_index);
}

int read_index_from(struct index_state *istate, const char *path,
const char *gitdir)
{
struct split_index *split_index;
int ret;
char *base_oid_hex;
char *base_path;


if (istate->initialized)
return istate->cache_nr;





trace2_region_enter_printf("index", "do_read_index", the_repository,
"%s", path);
trace_performance_enter();
ret = do_read_index(istate, path, 0);
trace_performance_leave("read cache %s", path);
trace2_region_leave_printf("index", "do_read_index", the_repository,
"%s", path);

split_index = istate->split_index;
if (!split_index || is_null_oid(&split_index->base_oid)) {
post_read_index_from(istate);
return ret;
}

trace_performance_enter();
if (split_index->base)
discard_index(split_index->base);
else
split_index->base = xcalloc(1, sizeof(*split_index->base));

base_oid_hex = oid_to_hex(&split_index->base_oid);
base_path = xstrfmt("%s/sharedindex.%s", gitdir, base_oid_hex);
trace2_region_enter_printf("index", "shared/do_read_index",
the_repository, "%s", base_path);
ret = do_read_index(split_index->base, base_path, 1);
trace2_region_leave_printf("index", "shared/do_read_index",
the_repository, "%s", base_path);
if (!oideq(&split_index->base_oid, &split_index->base->oid))
die(_("broken index, expect %s in %s, got %s"),
base_oid_hex, base_path,
oid_to_hex(&split_index->base->oid));

freshen_shared_index(base_path, 0);
merge_base_index(istate);
post_read_index_from(istate);
trace_performance_leave("read cache %s", base_path);
free(base_path);
return ret;
}

int is_index_unborn(struct index_state *istate)
{
return (!istate->cache_nr && !istate->timestamp.sec);
}

int discard_index(struct index_state *istate)
{







validate_cache_entries(istate);

resolve_undo_clear_index(istate);
istate->cache_nr = 0;
istate->cache_changed = 0;
istate->timestamp.sec = 0;
istate->timestamp.nsec = 0;
free_name_hash(istate);
cache_tree_free(&(istate->cache_tree));
istate->initialized = 0;
istate->fsmonitor_has_run_once = 0;
FREE_AND_NULL(istate->cache);
istate->cache_alloc = 0;
discard_split_index(istate);
free_untracked_cache(istate->untracked);
istate->untracked = NULL;

if (istate->ce_mem_pool) {
mem_pool_discard(istate->ce_mem_pool, should_validate_cache_entries());
istate->ce_mem_pool = NULL;
}

return 0;
}








void validate_cache_entries(const struct index_state *istate)
{
int i;

if (!should_validate_cache_entries() ||!istate || !istate->initialized)
return;

for (i = 0; i < istate->cache_nr; i++) {
if (!istate) {
BUG("cache entry is not allocated from expected memory pool");
} else if (!istate->ce_mem_pool ||
!mem_pool_contains(istate->ce_mem_pool, istate->cache[i])) {
if (!istate->split_index ||
!istate->split_index->base ||
!istate->split_index->base->ce_mem_pool ||
!mem_pool_contains(istate->split_index->base->ce_mem_pool, istate->cache[i])) {
BUG("cache entry is not allocated from expected memory pool");
}
}
}

if (istate->split_index)
validate_cache_entries(istate->split_index->base);
}

int unmerged_index(const struct index_state *istate)
{
int i;
for (i = 0; i < istate->cache_nr; i++) {
if (ce_stage(istate->cache[i]))
return 1;
}
return 0;
}

int repo_index_has_changes(struct repository *repo,
struct tree *tree,
struct strbuf *sb)
{
struct index_state *istate = repo->index;
struct object_id cmp;
int i;

if (tree)
cmp = tree->object.oid;
if (tree || !get_oid_tree("HEAD", &cmp)) {
struct diff_options opt;

repo_diff_setup(repo, &opt);
opt.flags.exit_with_status = 1;
if (!sb)
opt.flags.quick = 1;
do_diff_cache(&cmp, &opt);
diffcore_std(&opt);
for (i = 0; sb && i < diff_queued_diff.nr; i++) {
if (i)
strbuf_addch(sb, ' ');
strbuf_addstr(sb, diff_queued_diff.queue[i]->two->path);
}
diff_flush(&opt);
return opt.flags.has_changes != 0;
} else {
for (i = 0; sb && i < istate->cache_nr; i++) {
if (i)
strbuf_addch(sb, ' ');
strbuf_addstr(sb, istate->cache[i]->name);
}
return !!istate->cache_nr;
}
}

#define WRITE_BUFFER_SIZE 8192
static unsigned char write_buffer[WRITE_BUFFER_SIZE];
static unsigned long write_buffer_len;

static int ce_write_flush(git_hash_ctx *context, int fd)
{
unsigned int buffered = write_buffer_len;
if (buffered) {
the_hash_algo->update_fn(context, write_buffer, buffered);
if (write_in_full(fd, write_buffer, buffered) < 0)
return -1;
write_buffer_len = 0;
}
return 0;
}

static int ce_write(git_hash_ctx *context, int fd, void *data, unsigned int len)
{
while (len) {
unsigned int buffered = write_buffer_len;
unsigned int partial = WRITE_BUFFER_SIZE - buffered;
if (partial > len)
partial = len;
memcpy(write_buffer + buffered, data, partial);
buffered += partial;
if (buffered == WRITE_BUFFER_SIZE) {
write_buffer_len = buffered;
if (ce_write_flush(context, fd))
return -1;
buffered = 0;
}
write_buffer_len = buffered;
len -= partial;
data = (char *) data + partial;
}
return 0;
}

static int write_index_ext_header(git_hash_ctx *context, git_hash_ctx *eoie_context,
int fd, unsigned int ext, unsigned int sz)
{
ext = htonl(ext);
sz = htonl(sz);
if (eoie_context) {
the_hash_algo->update_fn(eoie_context, &ext, 4);
the_hash_algo->update_fn(eoie_context, &sz, 4);
}
return ((ce_write(context, fd, &ext, 4) < 0) ||
(ce_write(context, fd, &sz, 4) < 0)) ? -1 : 0;
}

static int ce_flush(git_hash_ctx *context, int fd, unsigned char *hash)
{
unsigned int left = write_buffer_len;

if (left) {
write_buffer_len = 0;
the_hash_algo->update_fn(context, write_buffer, left);
}


if (left + the_hash_algo->rawsz > WRITE_BUFFER_SIZE) {
if (write_in_full(fd, write_buffer, left) < 0)
return -1;
left = 0;
}


the_hash_algo->final_fn(write_buffer + left, context);
hashcpy(hash, write_buffer + left);
left += the_hash_algo->rawsz;
return (write_in_full(fd, write_buffer, left) < 0) ? -1 : 0;
}

static void ce_smudge_racily_clean_entry(struct index_state *istate,
struct cache_entry *ce)
{











struct stat st;

if (lstat(ce->name, &st) < 0)
return;
if (ce_match_stat_basic(ce, &st))
return;
if (ce_modified_check_fs(istate, ce, &st)) {

























ce->ce_stat_data.sd_size = 0;
}
}


static void copy_cache_entry_to_ondisk(struct ondisk_cache_entry *ondisk,
struct cache_entry *ce)
{
short flags;
const unsigned hashsz = the_hash_algo->rawsz;
uint16_t *flagsp = (uint16_t *)(ondisk->data + hashsz);

ondisk->ctime.sec = htonl(ce->ce_stat_data.sd_ctime.sec);
ondisk->mtime.sec = htonl(ce->ce_stat_data.sd_mtime.sec);
ondisk->ctime.nsec = htonl(ce->ce_stat_data.sd_ctime.nsec);
ondisk->mtime.nsec = htonl(ce->ce_stat_data.sd_mtime.nsec);
ondisk->dev = htonl(ce->ce_stat_data.sd_dev);
ondisk->ino = htonl(ce->ce_stat_data.sd_ino);
ondisk->mode = htonl(ce->ce_mode);
ondisk->uid = htonl(ce->ce_stat_data.sd_uid);
ondisk->gid = htonl(ce->ce_stat_data.sd_gid);
ondisk->size = htonl(ce->ce_stat_data.sd_size);
hashcpy(ondisk->data, ce->oid.hash);

flags = ce->ce_flags & ~CE_NAMEMASK;
flags |= (ce_namelen(ce) >= CE_NAMEMASK ? CE_NAMEMASK : ce_namelen(ce));
flagsp[0] = htons(flags);
if (ce->ce_flags & CE_EXTENDED) {
flagsp[1] = htons((ce->ce_flags & CE_EXTENDED_FLAGS) >> 16);
}
}

static int ce_write_entry(git_hash_ctx *c, int fd, struct cache_entry *ce,
struct strbuf *previous_name, struct ondisk_cache_entry *ondisk)
{
int size;
int result;
unsigned int saved_namelen;
int stripped_name = 0;
static unsigned char padding[8] = { 0x00 };

if (ce->ce_flags & CE_STRIP_NAME) {
saved_namelen = ce_namelen(ce);
ce->ce_namelen = 0;
stripped_name = 1;
}

size = offsetof(struct ondisk_cache_entry,data) + ondisk_data_size(ce->ce_flags, 0);

if (!previous_name) {
int len = ce_namelen(ce);
copy_cache_entry_to_ondisk(ondisk, ce);
result = ce_write(c, fd, ondisk, size);
if (!result)
result = ce_write(c, fd, ce->name, len);
if (!result)
result = ce_write(c, fd, padding, align_padding_size(size, len));
} else {
int common, to_remove, prefix_size;
unsigned char to_remove_vi[16];
for (common = 0;
(ce->name[common] &&
common < previous_name->len &&
ce->name[common] == previous_name->buf[common]);
common++)
; 
to_remove = previous_name->len - common;
prefix_size = encode_varint(to_remove, to_remove_vi);

copy_cache_entry_to_ondisk(ondisk, ce);
result = ce_write(c, fd, ondisk, size);
if (!result)
result = ce_write(c, fd, to_remove_vi, prefix_size);
if (!result)
result = ce_write(c, fd, ce->name + common, ce_namelen(ce) - common);
if (!result)
result = ce_write(c, fd, padding, 1);

strbuf_splice(previous_name, common, to_remove,
ce->name + common, ce_namelen(ce) - common);
}
if (stripped_name) {
ce->ce_namelen = saved_namelen;
ce->ce_flags &= ~CE_STRIP_NAME;
}

return result;
}





static int verify_index_from(const struct index_state *istate, const char *path)
{
int fd;
ssize_t n;
struct stat st;
unsigned char hash[GIT_MAX_RAWSZ];

if (!istate->initialized)
return 0;

fd = open(path, O_RDONLY);
if (fd < 0)
return 0;

if (fstat(fd, &st))
goto out;

if (st.st_size < sizeof(struct cache_header) + the_hash_algo->rawsz)
goto out;

n = pread_in_full(fd, hash, the_hash_algo->rawsz, st.st_size - the_hash_algo->rawsz);
if (n != the_hash_algo->rawsz)
goto out;

if (!hasheq(istate->oid.hash, hash))
goto out;

close(fd);
return 1;

out:
close(fd);
return 0;
}

static int repo_verify_index(struct repository *repo)
{
return verify_index_from(repo->index, repo->index_file);
}

static int has_racy_timestamp(struct index_state *istate)
{
int entries = istate->cache_nr;
int i;

for (i = 0; i < entries; i++) {
struct cache_entry *ce = istate->cache[i];
if (is_racy_timestamp(istate, ce))
return 1;
}
return 0;
}

void repo_update_index_if_able(struct repository *repo,
struct lock_file *lockfile)
{
if ((repo->index->cache_changed ||
has_racy_timestamp(repo->index)) &&
repo_verify_index(repo))
write_locked_index(repo->index, lockfile, COMMIT_LOCK);
else
rollback_lock_file(lockfile);
}

static int record_eoie(void)
{
int val;

if (!git_config_get_bool("index.recordendofindexentries", &val))
return val;






return !git_config_get_index_threads(&val) && val != 1;
}

static int record_ieot(void)
{
int val;

if (!git_config_get_bool("index.recordoffsettable", &val))
return val;






return !git_config_get_index_threads(&val) && val != 1;
}








static int do_write_index(struct index_state *istate, struct tempfile *tempfile,
int strip_extensions)
{
uint64_t start = getnanotime();
int newfd = tempfile->fd;
git_hash_ctx c, eoie_c;
struct cache_header hdr;
int i, err = 0, removed, extended, hdr_version;
struct cache_entry **cache = istate->cache;
int entries = istate->cache_nr;
struct stat st;
struct ondisk_cache_entry ondisk;
struct strbuf previous_name_buf = STRBUF_INIT, *previous_name;
int drop_cache_tree = istate->drop_cache_tree;
off_t offset;
int ieot_entries = 1;
struct index_entry_offset_table *ieot = NULL;
int nr, nr_threads;

for (i = removed = extended = 0; i < entries; i++) {
if (cache[i]->ce_flags & CE_REMOVE)
removed++;


cache[i]->ce_flags &= ~CE_EXTENDED;
if (cache[i]->ce_flags & CE_EXTENDED_FLAGS) {
extended++;
cache[i]->ce_flags |= CE_EXTENDED;
}
}

if (!istate->version) {
istate->version = get_index_format_default(the_repository);
if (git_env_bool("GIT_TEST_SPLIT_INDEX", 0))
init_split_index(istate);
}


if (istate->version == 3 || istate->version == 2)
istate->version = extended ? 3 : 2;

hdr_version = istate->version;

hdr.hdr_signature = htonl(CACHE_SIGNATURE);
hdr.hdr_version = htonl(hdr_version);
hdr.hdr_entries = htonl(entries - removed);

the_hash_algo->init_fn(&c);
if (ce_write(&c, newfd, &hdr, sizeof(hdr)) < 0)
return -1;

if (!HAVE_THREADS || git_config_get_index_threads(&nr_threads))
nr_threads = 1;

if (nr_threads != 1 && record_ieot()) {
int ieot_blocks, cpus;






if (!nr_threads) {
ieot_blocks = istate->cache_nr / THREAD_COST;
cpus = online_cpus();
if (ieot_blocks > cpus - 1)
ieot_blocks = cpus - 1;
} else {
ieot_blocks = nr_threads;
if (ieot_blocks > istate->cache_nr)
ieot_blocks = istate->cache_nr;
}





if (ieot_blocks > 1) {
ieot = xcalloc(1, sizeof(struct index_entry_offset_table)
+ (ieot_blocks * sizeof(struct index_entry_offset)));
ieot_entries = DIV_ROUND_UP(entries, ieot_blocks);
}
}

offset = lseek(newfd, 0, SEEK_CUR);
if (offset < 0) {
free(ieot);
return -1;
}
offset += write_buffer_len;
nr = 0;
previous_name = (hdr_version == 4) ? &previous_name_buf : NULL;

for (i = 0; i < entries; i++) {
struct cache_entry *ce = cache[i];
if (ce->ce_flags & CE_REMOVE)
continue;
if (!ce_uptodate(ce) && is_racy_timestamp(istate, ce))
ce_smudge_racily_clean_entry(istate, ce);
if (is_null_oid(&ce->oid)) {
static const char msg[] = "cache entry has null sha1: %s";
static int allow = -1;

if (allow < 0)
allow = git_env_bool("GIT_ALLOW_NULL_SHA1", 0);
if (allow)
warning(msg, ce->name);
else
err = error(msg, ce->name);

drop_cache_tree = 1;
}
if (ieot && i && (i % ieot_entries == 0)) {
ieot->entries[ieot->nr].nr = nr;
ieot->entries[ieot->nr].offset = offset;
ieot->nr++;





if (previous_name)
previous_name->buf[0] = 0;
nr = 0;
offset = lseek(newfd, 0, SEEK_CUR);
if (offset < 0) {
free(ieot);
return -1;
}
offset += write_buffer_len;
}
if (ce_write_entry(&c, newfd, ce, previous_name, (struct ondisk_cache_entry *)&ondisk) < 0)
err = -1;

if (err)
break;
nr++;
}
if (ieot && nr) {
ieot->entries[ieot->nr].nr = nr;
ieot->entries[ieot->nr].offset = offset;
ieot->nr++;
}
strbuf_release(&previous_name_buf);

if (err) {
free(ieot);
return err;
}


offset = lseek(newfd, 0, SEEK_CUR);
if (offset < 0) {
free(ieot);
return -1;
}
offset += write_buffer_len;
the_hash_algo->init_fn(&eoie_c);








if (ieot) {
struct strbuf sb = STRBUF_INIT;

write_ieot_extension(&sb, ieot);
err = write_index_ext_header(&c, &eoie_c, newfd, CACHE_EXT_INDEXENTRYOFFSETTABLE, sb.len) < 0
|| ce_write(&c, newfd, sb.buf, sb.len) < 0;
strbuf_release(&sb);
free(ieot);
if (err)
return -1;
}

if (!strip_extensions && istate->split_index &&
!is_null_oid(&istate->split_index->base_oid)) {
struct strbuf sb = STRBUF_INIT;

err = write_link_extension(&sb, istate) < 0 ||
write_index_ext_header(&c, &eoie_c, newfd, CACHE_EXT_LINK,
sb.len) < 0 ||
ce_write(&c, newfd, sb.buf, sb.len) < 0;
strbuf_release(&sb);
if (err)
return -1;
}
if (!strip_extensions && !drop_cache_tree && istate->cache_tree) {
struct strbuf sb = STRBUF_INIT;

cache_tree_write(&sb, istate->cache_tree);
err = write_index_ext_header(&c, &eoie_c, newfd, CACHE_EXT_TREE, sb.len) < 0
|| ce_write(&c, newfd, sb.buf, sb.len) < 0;
strbuf_release(&sb);
if (err)
return -1;
}
if (!strip_extensions && istate->resolve_undo) {
struct strbuf sb = STRBUF_INIT;

resolve_undo_write(&sb, istate->resolve_undo);
err = write_index_ext_header(&c, &eoie_c, newfd, CACHE_EXT_RESOLVE_UNDO,
sb.len) < 0
|| ce_write(&c, newfd, sb.buf, sb.len) < 0;
strbuf_release(&sb);
if (err)
return -1;
}
if (!strip_extensions && istate->untracked) {
struct strbuf sb = STRBUF_INIT;

write_untracked_extension(&sb, istate->untracked);
err = write_index_ext_header(&c, &eoie_c, newfd, CACHE_EXT_UNTRACKED,
sb.len) < 0 ||
ce_write(&c, newfd, sb.buf, sb.len) < 0;
strbuf_release(&sb);
if (err)
return -1;
}
if (!strip_extensions && istate->fsmonitor_last_update) {
struct strbuf sb = STRBUF_INIT;

write_fsmonitor_extension(&sb, istate);
err = write_index_ext_header(&c, &eoie_c, newfd, CACHE_EXT_FSMONITOR, sb.len) < 0
|| ce_write(&c, newfd, sb.buf, sb.len) < 0;
strbuf_release(&sb);
if (err)
return -1;
}







if (offset && record_eoie()) {
struct strbuf sb = STRBUF_INIT;

write_eoie_extension(&sb, &eoie_c, offset);
err = write_index_ext_header(&c, NULL, newfd, CACHE_EXT_ENDOFINDEXENTRIES, sb.len) < 0
|| ce_write(&c, newfd, sb.buf, sb.len) < 0;
strbuf_release(&sb);
if (err)
return -1;
}

if (ce_flush(&c, newfd, istate->oid.hash))
return -1;
if (close_tempfile_gently(tempfile)) {
error(_("could not close '%s'"), tempfile->filename.buf);
return -1;
}
if (stat(tempfile->filename.buf, &st))
return -1;
istate->timestamp.sec = (unsigned int)st.st_mtime;
istate->timestamp.nsec = ST_MTIME_NSEC(st);
trace_performance_since(start, "write index, changed mask = %x", istate->cache_changed);





trace2_data_intmax("index", the_repository, "write/version",
istate->version);
trace2_data_intmax("index", the_repository, "write/cache_nr",
istate->cache_nr);

return 0;
}

void set_alternate_index_output(const char *name)
{
alternate_index_output = name;
}

static int commit_locked_index(struct lock_file *lk)
{
if (alternate_index_output)
return commit_lock_file_to(lk, alternate_index_output);
else
return commit_lock_file(lk);
}

static int do_write_locked_index(struct index_state *istate, struct lock_file *lock,
unsigned flags)
{
int ret;





trace2_region_enter_printf("index", "do_write_index", the_repository,
"%s", lock->tempfile->filename.buf);
ret = do_write_index(istate, lock->tempfile, 0);
trace2_region_leave_printf("index", "do_write_index", the_repository,
"%s", lock->tempfile->filename.buf);

if (ret)
return ret;
if (flags & COMMIT_LOCK)
ret = commit_locked_index(lock);
else
ret = close_lock_file_gently(lock);

run_hook_le(NULL, "post-index-change",
istate->updated_workdir ? "1" : "0",
istate->updated_skipworktree ? "1" : "0", NULL);
istate->updated_workdir = 0;
istate->updated_skipworktree = 0;

return ret;
}

static int write_split_index(struct index_state *istate,
struct lock_file *lock,
unsigned flags)
{
int ret;
prepare_to_write_split_index(istate);
ret = do_write_locked_index(istate, lock, flags);
finish_writing_split_index(istate);
return ret;
}

static const char *shared_index_expire = "2.weeks.ago";

static unsigned long get_shared_index_expire_date(void)
{
static unsigned long shared_index_expire_date;
static int shared_index_expire_date_prepared;

if (!shared_index_expire_date_prepared) {
git_config_get_expiry("splitindex.sharedindexexpire",
&shared_index_expire);
shared_index_expire_date = approxidate(shared_index_expire);
shared_index_expire_date_prepared = 1;
}

return shared_index_expire_date;
}

static int should_delete_shared_index(const char *shared_index_path)
{
struct stat st;
unsigned long expiration;


expiration = get_shared_index_expire_date();
if (!expiration)
return 0;
if (stat(shared_index_path, &st))
return error_errno(_("could not stat '%s'"), shared_index_path);
if (st.st_mtime > expiration)
return 0;

return 1;
}

static int clean_shared_index_files(const char *current_hex)
{
struct dirent *de;
DIR *dir = opendir(get_git_dir());

if (!dir)
return error_errno(_("unable to open git dir: %s"), get_git_dir());

while ((de = readdir(dir)) != NULL) {
const char *sha1_hex;
const char *shared_index_path;
if (!skip_prefix(de->d_name, "sharedindex.", &sha1_hex))
continue;
if (!strcmp(sha1_hex, current_hex))
continue;
shared_index_path = git_path("%s", de->d_name);
if (should_delete_shared_index(shared_index_path) > 0 &&
unlink(shared_index_path))
warning_errno(_("unable to unlink: %s"), shared_index_path);
}
closedir(dir);

return 0;
}

static int write_shared_index(struct index_state *istate,
struct tempfile **temp)
{
struct split_index *si = istate->split_index;
int ret;

move_cache_to_base_index(istate);

trace2_region_enter_printf("index", "shared/do_write_index",
the_repository, "%s", (*temp)->filename.buf);
ret = do_write_index(si->base, *temp, 1);
trace2_region_leave_printf("index", "shared/do_write_index",
the_repository, "%s", (*temp)->filename.buf);

if (ret)
return ret;
ret = adjust_shared_perm(get_tempfile_path(*temp));
if (ret) {
error(_("cannot fix permission bits on '%s'"), get_tempfile_path(*temp));
return ret;
}
ret = rename_tempfile(temp,
git_path("sharedindex.%s", oid_to_hex(&si->base->oid)));
if (!ret) {
oidcpy(&si->base_oid, &si->base->oid);
clean_shared_index_files(oid_to_hex(&si->base->oid));
}

return ret;
}

static const int default_max_percent_split_change = 20;

static int too_many_not_shared_entries(struct index_state *istate)
{
int i, not_shared = 0;
int max_split = git_config_get_max_percent_split_change();

switch (max_split) {
case -1:

max_split = default_max_percent_split_change;
break;
case 0:
return 1; 
case 100:
return 0; 
default:
break; 
}


for (i = 0; i < istate->cache_nr; i++) {
struct cache_entry *ce = istate->cache[i];
if (!ce->index)
not_shared++;
}

return (int64_t)istate->cache_nr * max_split < (int64_t)not_shared * 100;
}

int write_locked_index(struct index_state *istate, struct lock_file *lock,
unsigned flags)
{
int new_shared_index, ret;
struct split_index *si = istate->split_index;

if (git_env_bool("GIT_TEST_CHECK_CACHE_TREE", 0))
cache_tree_verify(the_repository, istate);

if ((flags & SKIP_IF_UNCHANGED) && !istate->cache_changed) {
if (flags & COMMIT_LOCK)
rollback_lock_file(lock);
return 0;
}

if (istate->fsmonitor_last_update)
fill_fsmonitor_bitmap(istate);

if (!si || alternate_index_output ||
(istate->cache_changed & ~EXTMASK)) {
if (si)
oidclr(&si->base_oid);
ret = do_write_locked_index(istate, lock, flags);
goto out;
}

if (git_env_bool("GIT_TEST_SPLIT_INDEX", 0)) {
int v = si->base_oid.hash[0];
if ((v & 15) < 6)
istate->cache_changed |= SPLIT_INDEX_ORDERED;
}
if (too_many_not_shared_entries(istate))
istate->cache_changed |= SPLIT_INDEX_ORDERED;

new_shared_index = istate->cache_changed & SPLIT_INDEX_ORDERED;

if (new_shared_index) {
struct tempfile *temp;
int saved_errno;


temp = mks_tempfile_sm(git_path("sharedindex_XXXXXX"), 0, 0666);
if (!temp) {
oidclr(&si->base_oid);
ret = do_write_locked_index(istate, lock, flags);
goto out;
}
ret = write_shared_index(istate, &temp);

saved_errno = errno;
if (is_tempfile_active(temp))
delete_tempfile(&temp);
errno = saved_errno;

if (ret)
goto out;
}

ret = write_split_index(istate, lock, flags);


if (!ret && !new_shared_index && !is_null_oid(&si->base_oid)) {
const char *shared_index = git_path("sharedindex.%s",
oid_to_hex(&si->base_oid));
freshen_shared_index(shared_index, 1);
}

out:
if (flags & COMMIT_LOCK)
rollback_lock_file(lock);
return ret;
}











int repo_read_index_unmerged(struct repository *repo)
{
struct index_state *istate;
int i;
int unmerged = 0;

repo_read_index(repo);
istate = repo->index;
for (i = 0; i < istate->cache_nr; i++) {
struct cache_entry *ce = istate->cache[i];
struct cache_entry *new_ce;
int len;

if (!ce_stage(ce))
continue;
unmerged = 1;
len = ce_namelen(ce);
new_ce = make_empty_cache_entry(istate, len);
memcpy(new_ce->name, ce->name, len);
new_ce->ce_flags = create_ce_flags(0) | CE_CONFLICTED;
new_ce->ce_namelen = len;
new_ce->ce_mode = ce->ce_mode;
if (add_index_entry(istate, new_ce, ADD_CACHE_SKIP_DFCHECK))
return error(_("%s: cannot drop to stage #0"),
new_ce->name);
}
return unmerged;
}










int index_name_is_other(const struct index_state *istate, const char *name,
int namelen)
{
int pos;
if (namelen && name[namelen - 1] == '/')
namelen--;
pos = index_name_pos(istate, name, namelen);
if (0 <= pos)
return 0; 
pos = -pos - 1;
if (pos < istate->cache_nr) {
struct cache_entry *ce = istate->cache[pos];
if (ce_namelen(ce) == namelen &&
!memcmp(ce->name, name, namelen))
return 0; 
}
return 1;
}

void *read_blob_data_from_index(const struct index_state *istate,
const char *path, unsigned long *size)
{
int pos, len;
unsigned long sz;
enum object_type type;
void *data;

len = strlen(path);
pos = index_name_pos(istate, path, len);
if (pos < 0) {




int i;
for (i = -pos - 1;
(pos < 0 && i < istate->cache_nr &&
!strcmp(istate->cache[i]->name, path));
i++)
if (ce_stage(istate->cache[i]) == 2)
pos = i;
}
if (pos < 0)
return NULL;
data = read_object_file(&istate->cache[pos]->oid, &type, &sz);
if (!data || type != OBJ_BLOB) {
free(data);
return NULL;
}
if (size)
*size = sz;
return data;
}

void stat_validity_clear(struct stat_validity *sv)
{
FREE_AND_NULL(sv->sd);
}

int stat_validity_check(struct stat_validity *sv, const char *path)
{
struct stat st;

if (stat(path, &st) < 0)
return sv->sd == NULL;
if (!sv->sd)
return 0;
return S_ISREG(st.st_mode) && !match_stat_data(sv->sd, &st);
}

void stat_validity_update(struct stat_validity *sv, int fd)
{
struct stat st;

if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode))
stat_validity_clear(sv);
else {
if (!sv->sd)
sv->sd = xcalloc(1, sizeof(struct stat_data));
fill_stat_data(sv->sd, &st);
}
}

void move_index_extensions(struct index_state *dst, struct index_state *src)
{
dst->untracked = src->untracked;
src->untracked = NULL;
dst->cache_tree = src->cache_tree;
src->cache_tree = NULL;
}

struct cache_entry *dup_cache_entry(const struct cache_entry *ce,
struct index_state *istate)
{
unsigned int size = ce_size(ce);
int mem_pool_allocated;
struct cache_entry *new_entry = make_empty_cache_entry(istate, ce_namelen(ce));
mem_pool_allocated = new_entry->mem_pool_allocated;

memcpy(new_entry, ce, size);
new_entry->mem_pool_allocated = mem_pool_allocated;
return new_entry;
}

void discard_cache_entry(struct cache_entry *ce)
{
if (ce && should_validate_cache_entries())
memset(ce, 0xCD, cache_entry_size(ce->ce_namelen));

if (ce && ce->mem_pool_allocated)
return;

free(ce);
}

int should_validate_cache_entries(void)
{
static int validate_index_cache_entries = -1;

if (validate_index_cache_entries < 0) {
if (getenv("GIT_TEST_VALIDATE_INDEX_CACHE_ENTRIES"))
validate_index_cache_entries = 1;
else
validate_index_cache_entries = 0;
}

return validate_index_cache_entries;
}

#define EOIE_SIZE (4 + GIT_SHA1_RAWSZ) 
#define EOIE_SIZE_WITH_HEADER (4 + 4 + EOIE_SIZE) 

static size_t read_eoie_extension(const char *mmap, size_t mmap_size)
{









const char *index, *eoie;
uint32_t extsize;
size_t offset, src_offset;
unsigned char hash[GIT_MAX_RAWSZ];
git_hash_ctx c;


if (mmap_size < sizeof(struct cache_header) + EOIE_SIZE_WITH_HEADER + the_hash_algo->rawsz)
return 0;


index = eoie = mmap + mmap_size - EOIE_SIZE_WITH_HEADER - the_hash_algo->rawsz;
if (CACHE_EXT(index) != CACHE_EXT_ENDOFINDEXENTRIES)
return 0;
index += sizeof(uint32_t);


extsize = get_be32(index);
if (extsize != EOIE_SIZE)
return 0;
index += sizeof(uint32_t);





offset = get_be32(index);
if (mmap + offset < mmap + sizeof(struct cache_header))
return 0;
if (mmap + offset >= eoie)
return 0;
index += sizeof(uint32_t);










src_offset = offset;
the_hash_algo->init_fn(&c);
while (src_offset < mmap_size - the_hash_algo->rawsz - EOIE_SIZE_WITH_HEADER) {






uint32_t extsize;
memcpy(&extsize, mmap + src_offset + 4, 4);
extsize = ntohl(extsize);


if (src_offset + 8 + extsize < src_offset)
return 0;

the_hash_algo->update_fn(&c, mmap + src_offset, 8);

src_offset += 8;
src_offset += extsize;
}
the_hash_algo->final_fn(hash, &c);
if (!hasheq(hash, (const unsigned char *)index))
return 0;


if (src_offset != mmap_size - the_hash_algo->rawsz - EOIE_SIZE_WITH_HEADER)
return 0;

return offset;
}

static void write_eoie_extension(struct strbuf *sb, git_hash_ctx *eoie_context, size_t offset)
{
uint32_t buffer;
unsigned char hash[GIT_MAX_RAWSZ];


put_be32(&buffer, offset);
strbuf_add(sb, &buffer, sizeof(uint32_t));


the_hash_algo->final_fn(hash, eoie_context);
strbuf_add(sb, hash, the_hash_algo->rawsz);
}

#define IEOT_VERSION (1)

static struct index_entry_offset_table *read_ieot_extension(const char *mmap, size_t mmap_size, size_t offset)
{
const char *index = NULL;
uint32_t extsize, ext_version;
struct index_entry_offset_table *ieot;
int i, nr;


if (!offset)
return NULL;
while (offset <= mmap_size - the_hash_algo->rawsz - 8) {
extsize = get_be32(mmap + offset + 4);
if (CACHE_EXT((mmap + offset)) == CACHE_EXT_INDEXENTRYOFFSETTABLE) {
index = mmap + offset + 4 + 4;
break;
}
offset += 8;
offset += extsize;
}
if (!index)
return NULL;


ext_version = get_be32(index);
if (ext_version != IEOT_VERSION) {
error("invalid IEOT version %d", ext_version);
return NULL;
}
index += sizeof(uint32_t);


nr = (extsize - sizeof(uint32_t)) / (sizeof(uint32_t) + sizeof(uint32_t));
if (!nr) {
error("invalid number of IEOT entries %d", nr);
return NULL;
}
ieot = xmalloc(sizeof(struct index_entry_offset_table)
+ (nr * sizeof(struct index_entry_offset)));
ieot->nr = nr;
for (i = 0; i < nr; i++) {
ieot->entries[i].offset = get_be32(index);
index += sizeof(uint32_t);
ieot->entries[i].nr = get_be32(index);
index += sizeof(uint32_t);
}

return ieot;
}

static void write_ieot_extension(struct strbuf *sb, struct index_entry_offset_table *ieot)
{
uint32_t buffer;
int i;


put_be32(&buffer, IEOT_VERSION);
strbuf_add(sb, &buffer, sizeof(uint32_t));


for (i = 0; i < ieot->nr; i++) {


put_be32(&buffer, ieot->entries[i].offset);
strbuf_add(sb, &buffer, sizeof(uint32_t));


put_be32(&buffer, ieot->entries[i].nr);
strbuf_add(sb, &buffer, sizeof(uint32_t));
}
}
