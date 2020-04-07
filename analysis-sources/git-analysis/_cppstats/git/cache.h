#include "git-compat-util.h"
#include "strbuf.h"
#include "hashmap.h"
#include "list.h"
#include "advice.h"
#include "gettext.h"
#include "convert.h"
#include "trace.h"
#include "trace2.h"
#include "string-list.h"
#include "pack-revindex.h"
#include "hash.h"
#include "path.h"
#include "sha1-array.h"
#include "repository.h"
#include "mem-pool.h"
#include <zlib.h>
typedef struct git_zstream {
z_stream z;
unsigned long avail_in;
unsigned long avail_out;
unsigned long total_in;
unsigned long total_out;
unsigned char *next_in;
unsigned char *next_out;
} git_zstream;
void git_inflate_init(git_zstream *);
void git_inflate_init_gzip_only(git_zstream *);
void git_inflate_end(git_zstream *);
int git_inflate(git_zstream *, int flush);
void git_deflate_init(git_zstream *, int level);
void git_deflate_init_gzip(git_zstream *, int level);
void git_deflate_init_raw(git_zstream *, int level);
void git_deflate_end(git_zstream *);
int git_deflate_abort(git_zstream *);
int git_deflate_end_gently(git_zstream *);
int git_deflate(git_zstream *, int flush);
unsigned long git_deflate_bound(git_zstream *, unsigned long);
#if defined(DT_UNKNOWN) && !defined(NO_D_TYPE_IN_DIRENT)
#define DTYPE(de) ((de)->d_type)
#else
#undef DT_UNKNOWN
#undef DT_DIR
#undef DT_REG
#undef DT_LNK
#define DT_UNKNOWN 0
#define DT_DIR 1
#define DT_REG 2
#define DT_LNK 3
#define DTYPE(de) DT_UNKNOWN
#endif
#define S_IFINVALID 0030000
#define S_IFGITLINK 0160000
#define S_ISGITLINK(m) (((m) & S_IFMT) == S_IFGITLINK)
#define S_DIFFTREE_IFXMIN_NEQ 0x80000000
#define DEFAULT_GIT_PORT 9418
#define CACHE_SIGNATURE 0x44495243 
struct cache_header {
uint32_t hdr_signature;
uint32_t hdr_version;
uint32_t hdr_entries;
};
#define INDEX_FORMAT_LB 2
#define INDEX_FORMAT_UB 4
struct cache_time {
uint32_t sec;
uint32_t nsec;
};
struct stat_data {
struct cache_time sd_ctime;
struct cache_time sd_mtime;
unsigned int sd_dev;
unsigned int sd_ino;
unsigned int sd_uid;
unsigned int sd_gid;
unsigned int sd_size;
};
struct cache_entry {
struct hashmap_entry ent;
struct stat_data ce_stat_data;
unsigned int ce_mode;
unsigned int ce_flags;
unsigned int mem_pool_allocated;
unsigned int ce_namelen;
unsigned int index; 
struct object_id oid;
char name[FLEX_ARRAY]; 
};
#define CE_STAGEMASK (0x3000)
#define CE_EXTENDED (0x4000)
#define CE_VALID (0x8000)
#define CE_STAGESHIFT 12
#define CE_UPDATE (1 << 16)
#define CE_REMOVE (1 << 17)
#define CE_UPTODATE (1 << 18)
#define CE_ADDED (1 << 19)
#define CE_HASHED (1 << 20)
#define CE_FSMONITOR_VALID (1 << 21)
#define CE_WT_REMOVE (1 << 22) 
#define CE_CONFLICTED (1 << 23)
#define CE_UNPACKED (1 << 24)
#define CE_NEW_SKIP_WORKTREE (1 << 25)
#define CE_MATCHED (1 << 26)
#define CE_UPDATE_IN_BASE (1 << 27)
#define CE_STRIP_NAME (1 << 28)
#define CE_INTENT_TO_ADD (1 << 29)
#define CE_SKIP_WORKTREE (1 << 30)
#define CE_EXTENDED2 (1U << 31)
#define CE_EXTENDED_FLAGS (CE_INTENT_TO_ADD | CE_SKIP_WORKTREE)
#if CE_EXTENDED_FLAGS & 0x803FFFFF
#error "CE_EXTENDED_FLAGS out of range"
#endif
struct pathspec;
struct child_process;
struct tree;
static inline void copy_cache_entry(struct cache_entry *dst,
const struct cache_entry *src)
{
unsigned int state = dst->ce_flags & CE_HASHED;
int mem_pool_allocated = dst->mem_pool_allocated;
memcpy(&dst->ce_stat_data, &src->ce_stat_data,
offsetof(struct cache_entry, name) -
offsetof(struct cache_entry, ce_stat_data));
dst->ce_flags = (dst->ce_flags & ~CE_HASHED) | state;
dst->mem_pool_allocated = mem_pool_allocated;
}
static inline unsigned create_ce_flags(unsigned stage)
{
return (stage << CE_STAGESHIFT);
}
#define ce_namelen(ce) ((ce)->ce_namelen)
#define ce_size(ce) cache_entry_size(ce_namelen(ce))
#define ce_stage(ce) ((CE_STAGEMASK & (ce)->ce_flags) >> CE_STAGESHIFT)
#define ce_uptodate(ce) ((ce)->ce_flags & CE_UPTODATE)
#define ce_skip_worktree(ce) ((ce)->ce_flags & CE_SKIP_WORKTREE)
#define ce_mark_uptodate(ce) ((ce)->ce_flags |= CE_UPTODATE)
#define ce_intent_to_add(ce) ((ce)->ce_flags & CE_INTENT_TO_ADD)
#define ce_permissions(mode) (((mode) & 0100) ? 0755 : 0644)
static inline unsigned int create_ce_mode(unsigned int mode)
{
if (S_ISLNK(mode))
return S_IFLNK;
if (S_ISDIR(mode) || S_ISGITLINK(mode))
return S_IFGITLINK;
return S_IFREG | ce_permissions(mode);
}
static inline unsigned int ce_mode_from_stat(const struct cache_entry *ce,
unsigned int mode)
{
extern int trust_executable_bit, has_symlinks;
if (!has_symlinks && S_ISREG(mode) &&
ce && S_ISLNK(ce->ce_mode))
return ce->ce_mode;
if (!trust_executable_bit && S_ISREG(mode)) {
if (ce && S_ISREG(ce->ce_mode))
return ce->ce_mode;
return create_ce_mode(0666);
}
return create_ce_mode(mode);
}
static inline int ce_to_dtype(const struct cache_entry *ce)
{
unsigned ce_mode = ntohl(ce->ce_mode);
if (S_ISREG(ce_mode))
return DT_REG;
else if (S_ISDIR(ce_mode) || S_ISGITLINK(ce_mode))
return DT_DIR;
else if (S_ISLNK(ce_mode))
return DT_LNK;
else
return DT_UNKNOWN;
}
static inline unsigned int canon_mode(unsigned int mode)
{
if (S_ISREG(mode))
return S_IFREG | ce_permissions(mode);
if (S_ISLNK(mode))
return S_IFLNK;
if (S_ISDIR(mode))
return S_IFDIR;
return S_IFGITLINK;
}
#define cache_entry_size(len) (offsetof(struct cache_entry,name) + (len) + 1)
#define SOMETHING_CHANGED (1 << 0) 
#define CE_ENTRY_CHANGED (1 << 1)
#define CE_ENTRY_REMOVED (1 << 2)
#define CE_ENTRY_ADDED (1 << 3)
#define RESOLVE_UNDO_CHANGED (1 << 4)
#define CACHE_TREE_CHANGED (1 << 5)
#define SPLIT_INDEX_ORDERED (1 << 6)
#define UNTRACKED_CHANGED (1 << 7)
#define FSMONITOR_CHANGED (1 << 8)
struct split_index;
struct untracked_cache;
struct progress;
struct index_state {
struct cache_entry **cache;
unsigned int version;
unsigned int cache_nr, cache_alloc, cache_changed;
struct string_list *resolve_undo;
struct cache_tree *cache_tree;
struct split_index *split_index;
struct cache_time timestamp;
unsigned name_hash_initialized : 1,
initialized : 1,
drop_cache_tree : 1,
updated_workdir : 1,
updated_skipworktree : 1,
fsmonitor_has_run_once : 1;
struct hashmap name_hash;
struct hashmap dir_hash;
struct object_id oid;
struct untracked_cache *untracked;
char *fsmonitor_last_update;
struct ewah_bitmap *fsmonitor_dirty;
struct mem_pool *ce_mem_pool;
struct progress *progress;
};
int test_lazy_init_name_hash(struct index_state *istate, int try_threaded);
void add_name_hash(struct index_state *istate, struct cache_entry *ce);
void remove_name_hash(struct index_state *istate, struct cache_entry *ce);
void free_name_hash(struct index_state *istate);
struct cache_entry *make_cache_entry(struct index_state *istate,
unsigned int mode,
const struct object_id *oid,
const char *path,
int stage,
unsigned int refresh_options);
struct cache_entry *make_empty_cache_entry(struct index_state *istate,
size_t name_len);
struct cache_entry *make_transient_cache_entry(unsigned int mode,
const struct object_id *oid,
const char *path,
int stage);
struct cache_entry *make_empty_transient_cache_entry(size_t name_len);
void discard_cache_entry(struct cache_entry *ce);
int should_validate_cache_entries(void);
struct cache_entry *dup_cache_entry(const struct cache_entry *ce, struct index_state *istate);
void validate_cache_entries(const struct index_state *istate);
#if defined(USE_THE_INDEX_COMPATIBILITY_MACROS)
extern struct index_state the_index;
#define active_cache (the_index.cache)
#define active_nr (the_index.cache_nr)
#define active_alloc (the_index.cache_alloc)
#define active_cache_changed (the_index.cache_changed)
#define active_cache_tree (the_index.cache_tree)
#define read_cache() repo_read_index(the_repository)
#define read_cache_from(path) read_index_from(&the_index, (path), (get_git_dir()))
#define read_cache_preload(pathspec) repo_read_index_preload(the_repository, (pathspec), 0)
#define is_cache_unborn() is_index_unborn(&the_index)
#define read_cache_unmerged() repo_read_index_unmerged(the_repository)
#define discard_cache() discard_index(&the_index)
#define unmerged_cache() unmerged_index(&the_index)
#define cache_name_pos(name, namelen) index_name_pos(&the_index,(name),(namelen))
#define add_cache_entry(ce, option) add_index_entry(&the_index, (ce), (option))
#define rename_cache_entry_at(pos, new_name) rename_index_entry_at(&the_index, (pos), (new_name))
#define remove_cache_entry_at(pos) remove_index_entry_at(&the_index, (pos))
#define remove_file_from_cache(path) remove_file_from_index(&the_index, (path))
#define add_to_cache(path, st, flags) add_to_index(&the_index, (path), (st), (flags))
#define add_file_to_cache(path, flags) add_file_to_index(&the_index, (path), (flags))
#define chmod_cache_entry(ce, flip) chmod_index_entry(&the_index, (ce), (flip))
#define refresh_cache(flags) refresh_index(&the_index, (flags), NULL, NULL, NULL)
#define refresh_and_write_cache(refresh_flags, write_flags, gentle) repo_refresh_and_write_index(the_repository, (refresh_flags), (write_flags), (gentle), NULL, NULL, NULL)
#define ce_match_stat(ce, st, options) ie_match_stat(&the_index, (ce), (st), (options))
#define ce_modified(ce, st, options) ie_modified(&the_index, (ce), (st), (options))
#define cache_dir_exists(name, namelen) index_dir_exists(&the_index, (name), (namelen))
#define cache_file_exists(name, namelen, igncase) index_file_exists(&the_index, (name), (namelen), (igncase))
#define cache_name_is_other(name, namelen) index_name_is_other(&the_index, (name), (namelen))
#define resolve_undo_clear() resolve_undo_clear_index(&the_index)
#define unmerge_cache_entry_at(at) unmerge_index_entry_at(&the_index, at)
#define unmerge_cache(pathspec) unmerge_index(&the_index, pathspec)
#define read_blob_data_from_cache(path, sz) read_blob_data_from_index(&the_index, (path), (sz))
#define hold_locked_index(lock_file, flags) repo_hold_locked_index(the_repository, (lock_file), (flags))
#endif
#define TYPE_BITS 3
enum object_type {
OBJ_BAD = -1,
OBJ_NONE = 0,
OBJ_COMMIT = 1,
OBJ_TREE = 2,
OBJ_BLOB = 3,
OBJ_TAG = 4,
OBJ_OFS_DELTA = 6,
OBJ_REF_DELTA = 7,
OBJ_ANY,
OBJ_MAX
};
static inline enum object_type object_type(unsigned int mode)
{
return S_ISDIR(mode) ? OBJ_TREE :
S_ISGITLINK(mode) ? OBJ_COMMIT :
OBJ_BLOB;
}
#define GIT_DIR_ENVIRONMENT "GIT_DIR"
#define GIT_COMMON_DIR_ENVIRONMENT "GIT_COMMON_DIR"
#define GIT_NAMESPACE_ENVIRONMENT "GIT_NAMESPACE"
#define GIT_WORK_TREE_ENVIRONMENT "GIT_WORK_TREE"
#define GIT_PREFIX_ENVIRONMENT "GIT_PREFIX"
#define GIT_SUPER_PREFIX_ENVIRONMENT "GIT_INTERNAL_SUPER_PREFIX"
#define DEFAULT_GIT_DIR_ENVIRONMENT ".git"
#define DB_ENVIRONMENT "GIT_OBJECT_DIRECTORY"
#define INDEX_ENVIRONMENT "GIT_INDEX_FILE"
#define GRAFT_ENVIRONMENT "GIT_GRAFT_FILE"
#define GIT_SHALLOW_FILE_ENVIRONMENT "GIT_SHALLOW_FILE"
#define TEMPLATE_DIR_ENVIRONMENT "GIT_TEMPLATE_DIR"
#define CONFIG_ENVIRONMENT "GIT_CONFIG"
#define CONFIG_DATA_ENVIRONMENT "GIT_CONFIG_PARAMETERS"
#define EXEC_PATH_ENVIRONMENT "GIT_EXEC_PATH"
#define CEILING_DIRECTORIES_ENVIRONMENT "GIT_CEILING_DIRECTORIES"
#define NO_REPLACE_OBJECTS_ENVIRONMENT "GIT_NO_REPLACE_OBJECTS"
#define GIT_REPLACE_REF_BASE_ENVIRONMENT "GIT_REPLACE_REF_BASE"
#define GITATTRIBUTES_FILE ".gitattributes"
#define INFOATTRIBUTES_FILE "info/attributes"
#define ATTRIBUTE_MACRO_PREFIX "[attr]"
#define GITMODULES_FILE ".gitmodules"
#define GITMODULES_INDEX ":.gitmodules"
#define GITMODULES_HEAD "HEAD:.gitmodules"
#define GIT_NOTES_REF_ENVIRONMENT "GIT_NOTES_REF"
#define GIT_NOTES_DEFAULT_REF "refs/notes/commits"
#define GIT_NOTES_DISPLAY_REF_ENVIRONMENT "GIT_NOTES_DISPLAY_REF"
#define GIT_NOTES_REWRITE_REF_ENVIRONMENT "GIT_NOTES_REWRITE_REF"
#define GIT_NOTES_REWRITE_MODE_ENVIRONMENT "GIT_NOTES_REWRITE_MODE"
#define GIT_LITERAL_PATHSPECS_ENVIRONMENT "GIT_LITERAL_PATHSPECS"
#define GIT_GLOB_PATHSPECS_ENVIRONMENT "GIT_GLOB_PATHSPECS"
#define GIT_NOGLOB_PATHSPECS_ENVIRONMENT "GIT_NOGLOB_PATHSPECS"
#define GIT_ICASE_PATHSPECS_ENVIRONMENT "GIT_ICASE_PATHSPECS"
#define GIT_QUARANTINE_ENVIRONMENT "GIT_QUARANTINE_PATH"
#define GIT_OPTIONAL_LOCKS_ENVIRONMENT "GIT_OPTIONAL_LOCKS"
#define GIT_TEXT_DOMAIN_DIR_ENVIRONMENT "GIT_TEXTDOMAINDIR"
#define GIT_PROTOCOL_ENVIRONMENT "GIT_PROTOCOL"
#define GIT_PROTOCOL_HEADER "Git-Protocol"
#define GIT_IMPLICIT_WORK_TREE_ENVIRONMENT "GIT_IMPLICIT_WORK_TREE"
extern const char * const local_repo_env[];
void setup_git_env(const char *git_dir);
int have_git_dir(void);
extern int is_bare_repository_cfg;
int is_bare_repository(void);
int is_inside_git_dir(void);
extern char *git_work_tree_cfg;
int is_inside_work_tree(void);
const char *get_git_dir(void);
const char *get_git_common_dir(void);
char *get_object_directory(void);
char *get_index_file(void);
char *get_graft_file(struct repository *r);
void set_git_dir(const char *path, int make_realpath);
int get_common_dir_noenv(struct strbuf *sb, const char *gitdir);
int get_common_dir(struct strbuf *sb, const char *gitdir);
const char *get_git_namespace(void);
const char *strip_namespace(const char *namespaced_ref);
const char *get_super_prefix(void);
const char *get_git_work_tree(void);
int is_git_directory(const char *path);
int is_nonbare_repository_dir(struct strbuf *path);
#define READ_GITFILE_ERR_STAT_FAILED 1
#define READ_GITFILE_ERR_NOT_A_FILE 2
#define READ_GITFILE_ERR_OPEN_FAILED 3
#define READ_GITFILE_ERR_READ_FAILED 4
#define READ_GITFILE_ERR_INVALID_FORMAT 5
#define READ_GITFILE_ERR_NO_PATH 6
#define READ_GITFILE_ERR_NOT_A_REPO 7
#define READ_GITFILE_ERR_TOO_LARGE 8
void read_gitfile_error_die(int error_code, const char *path, const char *dir);
const char *read_gitfile_gently(const char *path, int *return_error_code);
#define read_gitfile(path) read_gitfile_gently((path), NULL)
const char *resolve_gitdir_gently(const char *suspect, int *return_error_code);
#define resolve_gitdir(path) resolve_gitdir_gently((path), NULL)
void set_git_work_tree(const char *tree);
#define ALTERNATE_DB_ENVIRONMENT "GIT_ALTERNATE_OBJECT_DIRECTORIES"
void setup_work_tree(void);
int discover_git_directory(struct strbuf *commondir,
struct strbuf *gitdir);
const char *setup_git_directory_gently(int *);
const char *setup_git_directory(void);
char *prefix_path(const char *prefix, int len, const char *path);
char *prefix_path_gently(const char *prefix, int len, int *remaining, const char *path);
char *prefix_filename(const char *prefix, const char *path);
int check_filename(const char *prefix, const char *name);
void verify_filename(const char *prefix,
const char *name,
int diagnose_misspelt_rev);
void verify_non_filename(const char *prefix, const char *name);
int path_inside_repo(const char *prefix, const char *path);
#define INIT_DB_QUIET 0x0001
#define INIT_DB_EXIST_OK 0x0002
int init_db(const char *git_dir, const char *real_git_dir,
const char *template_dir, int hash_algo,
unsigned int flags);
void initialize_repository_version(int hash_algo);
void sanitize_stdfds(void);
int daemonize(void);
#define alloc_nr(x) (((x)+16)*3/2)
#define ALLOC_GROW(x, nr, alloc) do { if ((nr) > alloc) { if (alloc_nr(alloc) < (nr)) alloc = (nr); else alloc = alloc_nr(alloc); REALLOC_ARRAY(x, alloc); } } while (0)
#define ALLOC_GROW_BY(x, nr, increase, alloc) do { if (increase) { size_t new_nr = nr + (increase); if (new_nr < nr) BUG("negative growth in ALLOC_GROW_BY"); ALLOC_GROW(x, new_nr, alloc); memset((x) + nr, 0, sizeof(*(x)) * (increase)); nr = new_nr; } } while (0)
struct lock_file;
void preload_index(struct index_state *index,
const struct pathspec *pathspec,
unsigned int refresh_flags);
int do_read_index(struct index_state *istate, const char *path,
int must_exist); 
int read_index_from(struct index_state *, const char *path,
const char *gitdir);
int is_index_unborn(struct index_state *);
#define COMMIT_LOCK (1 << 0)
#define SKIP_IF_UNCHANGED (1 << 1)
int write_locked_index(struct index_state *, struct lock_file *lock, unsigned flags);
int discard_index(struct index_state *);
void move_index_extensions(struct index_state *dst, struct index_state *src);
int unmerged_index(const struct index_state *);
int repo_index_has_changes(struct repository *repo,
struct tree *tree,
struct strbuf *sb);
int verify_path(const char *path, unsigned mode);
int strcmp_offset(const char *s1, const char *s2, size_t *first_change);
int index_dir_exists(struct index_state *istate, const char *name, int namelen);
void adjust_dirname_case(struct index_state *istate, char *name);
struct cache_entry *index_file_exists(struct index_state *istate, const char *name, int namelen, int igncase);
int index_name_pos(const struct index_state *, const char *name, int namelen);
static inline int index_pos_to_insert_pos(uintmax_t pos)
{
if (pos > INT_MAX)
die("overflow: -1 - %"PRIuMAX, pos);
return -1 - (int)pos;
}
#define ADD_CACHE_OK_TO_ADD 1 
#define ADD_CACHE_OK_TO_REPLACE 2 
#define ADD_CACHE_SKIP_DFCHECK 4 
#define ADD_CACHE_JUST_APPEND 8 
#define ADD_CACHE_NEW_ONLY 16 
#define ADD_CACHE_KEEP_CACHE_TREE 32 
#define ADD_CACHE_RENORMALIZE 64 
int add_index_entry(struct index_state *, struct cache_entry *ce, int option);
void rename_index_entry_at(struct index_state *, int pos, const char *new_name);
int remove_index_entry_at(struct index_state *, int pos);
void remove_marked_cache_entries(struct index_state *istate, int invalidate);
int remove_file_from_index(struct index_state *, const char *path);
#define ADD_CACHE_VERBOSE 1
#define ADD_CACHE_PRETEND 2
#define ADD_CACHE_IGNORE_ERRORS 4
#define ADD_CACHE_IGNORE_REMOVAL 8
#define ADD_CACHE_INTENT 16
int add_to_index(struct index_state *, const char *path, struct stat *, int flags);
int add_file_to_index(struct index_state *, const char *path, int flags);
int chmod_index_entry(struct index_state *, struct cache_entry *ce, char flip);
int ce_same_name(const struct cache_entry *a, const struct cache_entry *b);
void set_object_name_for_intent_to_add_entry(struct cache_entry *ce);
int index_name_is_other(const struct index_state *, const char *, int);
void *read_blob_data_from_index(const struct index_state *, const char *, unsigned long *);
#define CE_MATCH_IGNORE_VALID 01
#define CE_MATCH_RACY_IS_DIRTY 02
#define CE_MATCH_IGNORE_SKIP_WORKTREE 04
#define CE_MATCH_IGNORE_MISSING 0x08
#define CE_MATCH_REFRESH 0x10
#define CE_MATCH_IGNORE_FSMONITOR 0X20
int is_racy_timestamp(const struct index_state *istate,
const struct cache_entry *ce);
int ie_match_stat(struct index_state *, const struct cache_entry *, struct stat *, unsigned int);
int ie_modified(struct index_state *, const struct cache_entry *, struct stat *, unsigned int);
#define HASH_WRITE_OBJECT 1
#define HASH_FORMAT_CHECK 2
#define HASH_RENORMALIZE 4
int index_fd(struct index_state *istate, struct object_id *oid, int fd, struct stat *st, enum object_type type, const char *path, unsigned flags);
int index_path(struct index_state *istate, struct object_id *oid, const char *path, struct stat *st, unsigned flags);
void fill_stat_data(struct stat_data *sd, struct stat *st);
int match_stat_data(const struct stat_data *sd, struct stat *st);
int match_stat_data_racy(const struct index_state *istate,
const struct stat_data *sd, struct stat *st);
void fill_stat_cache_info(struct index_state *istate, struct cache_entry *ce, struct stat *st);
#define REFRESH_REALLY 0x0001 
#define REFRESH_UNMERGED 0x0002 
#define REFRESH_QUIET 0x0004 
#define REFRESH_IGNORE_MISSING 0x0008 
#define REFRESH_IGNORE_SUBMODULES 0x0010 
#define REFRESH_IN_PORCELAIN 0x0020 
#define REFRESH_PROGRESS 0x0040 
int refresh_index(struct index_state *, unsigned int flags, const struct pathspec *pathspec, char *seen, const char *header_msg);
int repo_refresh_and_write_index(struct repository*, unsigned int refresh_flags, unsigned int write_flags, int gentle, const struct pathspec *, char *seen, const char *header_msg);
struct cache_entry *refresh_cache_entry(struct index_state *, struct cache_entry *, unsigned int);
void set_alternate_index_output(const char *);
extern int verify_index_checksum;
extern int verify_ce_order;
extern int trust_executable_bit;
extern int trust_ctime;
extern int check_stat;
extern int quote_path_fully;
extern int has_symlinks;
extern int minimum_abbrev, default_abbrev;
extern int ignore_case;
extern int assume_unchanged;
extern int prefer_symlink_refs;
extern int warn_ambiguous_refs;
extern int warn_on_object_refname_ambiguity;
extern const char *apply_default_whitespace;
extern const char *apply_default_ignorewhitespace;
extern const char *git_attributes_file;
extern const char *git_hooks_path;
extern int zlib_compression_level;
extern int core_compression_level;
extern int pack_compression_level;
extern size_t packed_git_window_size;
extern size_t packed_git_limit;
extern size_t delta_base_cache_limit;
extern unsigned long big_file_threshold;
extern unsigned long pack_size_limit_cfg;
void set_shared_repository(int value);
int get_shared_repository(void);
void reset_shared_repository(void);
extern int read_replace_refs;
extern char *git_replace_ref_base;
extern int fsync_object_files;
extern int core_preload_index;
extern int precomposed_unicode;
extern int protect_hfs;
extern int protect_ntfs;
extern const char *core_fsmonitor;
extern int core_apply_sparse_checkout;
extern int core_sparse_checkout_cone;
extern int ref_paranoia;
int use_optional_locks(void);
extern char comment_line_char;
extern int auto_comment_line_char;
enum log_refs_config {
LOG_REFS_UNSET = -1,
LOG_REFS_NONE = 0,
LOG_REFS_NORMAL,
LOG_REFS_ALWAYS
};
extern enum log_refs_config log_all_ref_updates;
enum rebase_setup_type {
AUTOREBASE_NEVER = 0,
AUTOREBASE_LOCAL,
AUTOREBASE_REMOTE,
AUTOREBASE_ALWAYS
};
enum push_default_type {
PUSH_DEFAULT_NOTHING = 0,
PUSH_DEFAULT_MATCHING,
PUSH_DEFAULT_SIMPLE,
PUSH_DEFAULT_UPSTREAM,
PUSH_DEFAULT_CURRENT,
PUSH_DEFAULT_UNSPECIFIED
};
extern enum rebase_setup_type autorebase;
extern enum push_default_type push_default;
enum object_creation_mode {
OBJECT_CREATION_USES_HARDLINKS = 0,
OBJECT_CREATION_USES_RENAMES = 1
};
extern enum object_creation_mode object_creation_mode;
extern char *notes_ref_name;
extern int grafts_replace_parents;
#define GIT_REPO_VERSION 0
#define GIT_REPO_VERSION_READ 1
extern int repository_format_precious_objects;
extern int repository_format_worktree_config;
struct repository_format {
int version;
int precious_objects;
char *partial_clone; 
int worktree_config;
int is_bare;
int hash_algo;
char *work_tree;
struct string_list unknown_extensions;
};
#define REPOSITORY_FORMAT_INIT { .version = -1, .is_bare = -1, .hash_algo = GIT_HASH_SHA1, .unknown_extensions = STRING_LIST_INIT_DUP, }
int read_repository_format(struct repository_format *format, const char *path);
void clear_repository_format(struct repository_format *format);
int verify_repository_format(const struct repository_format *format,
struct strbuf *err);
void check_repository_format(struct repository_format *fmt);
#define MTIME_CHANGED 0x0001
#define CTIME_CHANGED 0x0002
#define OWNER_CHANGED 0x0004
#define MODE_CHANGED 0x0008
#define INODE_CHANGED 0x0010
#define DATA_CHANGED 0x0020
#define TYPE_CHANGED 0x0040
const char *repo_find_unique_abbrev(struct repository *r, const struct object_id *oid, int len);
#define find_unique_abbrev(oid, len) repo_find_unique_abbrev(the_repository, oid, len)
int repo_find_unique_abbrev_r(struct repository *r, char *hex, const struct object_id *oid, int len);
#define find_unique_abbrev_r(hex, oid, len) repo_find_unique_abbrev_r(the_repository, hex, oid, len)
extern const struct object_id null_oid;
static inline int hashcmp(const unsigned char *sha1, const unsigned char *sha2)
{
if (the_hash_algo->rawsz == GIT_MAX_RAWSZ)
return memcmp(sha1, sha2, GIT_MAX_RAWSZ);
return memcmp(sha1, sha2, GIT_SHA1_RAWSZ);
}
static inline int oidcmp(const struct object_id *oid1, const struct object_id *oid2)
{
return hashcmp(oid1->hash, oid2->hash);
}
static inline int hasheq(const unsigned char *sha1, const unsigned char *sha2)
{
if (the_hash_algo->rawsz == GIT_MAX_RAWSZ)
return !memcmp(sha1, sha2, GIT_MAX_RAWSZ);
return !memcmp(sha1, sha2, GIT_SHA1_RAWSZ);
}
static inline int oideq(const struct object_id *oid1, const struct object_id *oid2)
{
return hasheq(oid1->hash, oid2->hash);
}
static inline int is_null_oid(const struct object_id *oid)
{
return oideq(oid, &null_oid);
}
static inline void hashcpy(unsigned char *sha_dst, const unsigned char *sha_src)
{
memcpy(sha_dst, sha_src, the_hash_algo->rawsz);
}
static inline void oidcpy(struct object_id *dst, const struct object_id *src)
{
memcpy(dst->hash, src->hash, GIT_MAX_RAWSZ);
}
static inline struct object_id *oiddup(const struct object_id *src)
{
struct object_id *dst = xmalloc(sizeof(struct object_id));
oidcpy(dst, src);
return dst;
}
static inline void hashclr(unsigned char *hash)
{
memset(hash, 0, the_hash_algo->rawsz);
}
static inline void oidclr(struct object_id *oid)
{
memset(oid->hash, 0, GIT_MAX_RAWSZ);
}
static inline void oidread(struct object_id *oid, const unsigned char *hash)
{
memcpy(oid->hash, hash, the_hash_algo->rawsz);
}
static inline int is_empty_blob_sha1(const unsigned char *sha1)
{
return hasheq(sha1, the_hash_algo->empty_blob->hash);
}
static inline int is_empty_blob_oid(const struct object_id *oid)
{
return oideq(oid, the_hash_algo->empty_blob);
}
static inline int is_empty_tree_sha1(const unsigned char *sha1)
{
return hasheq(sha1, the_hash_algo->empty_tree->hash);
}
static inline int is_empty_tree_oid(const struct object_id *oid)
{
return oideq(oid, the_hash_algo->empty_tree);
}
const char *empty_tree_oid_hex(void);
const char *empty_blob_oid_hex(void);
int git_mkstemps_mode(char *pattern, int suffix_len, int mode);
int git_mkstemp_mode(char *pattern, int mode);
enum sharedrepo {
PERM_UMASK = 0,
OLD_PERM_GROUP = 1,
OLD_PERM_EVERYBODY = 2,
PERM_GROUP = 0660,
PERM_EVERYBODY = 0664
};
int git_config_perm(const char *var, const char *value);
int adjust_shared_perm(const char *path);
enum scld_error {
SCLD_OK = 0,
SCLD_FAILED = -1,
SCLD_PERMS = -2,
SCLD_EXISTS = -3,
SCLD_VANISHED = -4
};
enum scld_error safe_create_leading_directories(char *path);
enum scld_error safe_create_leading_directories_const(const char *path);
typedef int create_file_fn(const char *path, void *cb);
int raceproof_create_file(const char *path, create_file_fn fn, void *cb);
int mkdir_in_gitdir(const char *path);
char *expand_user_path(const char *path, int real_home);
const char *enter_repo(const char *path, int strict);
static inline int is_absolute_path(const char *path)
{
return is_dir_sep(path[0]) || has_dos_drive_prefix(path);
}
int is_directory(const char *);
char *strbuf_realpath(struct strbuf *resolved, const char *path,
int die_on_error);
char *real_pathdup(const char *path, int die_on_error);
const char *absolute_path(const char *path);
char *absolute_pathdup(const char *path);
const char *remove_leading_path(const char *in, const char *prefix);
const char *relative_path(const char *in, const char *prefix, struct strbuf *sb);
int normalize_path_copy_len(char *dst, const char *src, int *prefix_len);
int normalize_path_copy(char *dst, const char *src);
int longest_ancestor_length(const char *path, struct string_list *prefixes);
char *strip_path_suffix(const char *path, const char *suffix);
int daemon_avoid_alias(const char *path);
int is_ntfs_dotgit(const char *name);
int is_ntfs_dotgitmodules(const char *name);
int is_ntfs_dotgitignore(const char *name);
int is_ntfs_dotgitattributes(const char *name);
int looks_like_command_line_option(const char *str);
char *xdg_config_home(const char *filename);
char *xdg_cache_home(const char *filename);
int git_open_cloexec(const char *name, int flags);
#define git_open(name) git_open_cloexec(name, O_RDONLY)
int unpack_loose_header(git_zstream *stream, unsigned char *map, unsigned long mapsize, void *buffer, unsigned long bufsiz);
int parse_loose_header(const char *hdr, unsigned long *sizep);
int check_object_signature(struct repository *r, const struct object_id *oid,
void *buf, unsigned long size, const char *type);
int finalize_object_file(const char *tmpfile, const char *filename);
int check_and_freshen_file(const char *fn, int freshen);
extern const signed char hexval_table[256];
static inline unsigned int hexval(unsigned char c)
{
return hexval_table[c];
}
static inline int hex2chr(const char *s)
{
unsigned int val = hexval(s[0]);
return (val & ~0xf) ? val : (val << 4) | hexval(s[1]);
}
#define MINIMUM_ABBREV minimum_abbrev
#define DEFAULT_ABBREV default_abbrev
#define FALLBACK_DEFAULT_ABBREV 7
struct object_context {
unsigned short mode;
struct strbuf symlink_path;
char *path;
};
#define GET_OID_QUIETLY 01
#define GET_OID_COMMIT 02
#define GET_OID_COMMITTISH 04
#define GET_OID_TREE 010
#define GET_OID_TREEISH 020
#define GET_OID_BLOB 040
#define GET_OID_FOLLOW_SYMLINKS 0100
#define GET_OID_RECORD_PATH 0200
#define GET_OID_ONLY_TO_DIE 04000
#define GET_OID_DISAMBIGUATORS (GET_OID_COMMIT | GET_OID_COMMITTISH | GET_OID_TREE | GET_OID_TREEISH | GET_OID_BLOB)
enum get_oid_result {
FOUND = 0,
MISSING_OBJECT = -1, 
SHORT_NAME_AMBIGUOUS = -2,
DANGLING_SYMLINK = -4, 
SYMLINK_LOOP = -5,
NOT_DIR = -6, 
};
int repo_get_oid(struct repository *r, const char *str, struct object_id *oid);
int get_oidf(struct object_id *oid, const char *fmt, ...);
int repo_get_oid_commit(struct repository *r, const char *str, struct object_id *oid);
int repo_get_oid_committish(struct repository *r, const char *str, struct object_id *oid);
int repo_get_oid_tree(struct repository *r, const char *str, struct object_id *oid);
int repo_get_oid_treeish(struct repository *r, const char *str, struct object_id *oid);
int repo_get_oid_blob(struct repository *r, const char *str, struct object_id *oid);
int repo_get_oid_mb(struct repository *r, const char *str, struct object_id *oid);
void maybe_die_on_misspelt_object_name(struct repository *repo,
const char *name,
const char *prefix);
enum get_oid_result get_oid_with_context(struct repository *repo, const char *str,
unsigned flags, struct object_id *oid,
struct object_context *oc);
#define get_oid(str, oid) repo_get_oid(the_repository, str, oid)
#define get_oid_commit(str, oid) repo_get_oid_commit(the_repository, str, oid)
#define get_oid_committish(str, oid) repo_get_oid_committish(the_repository, str, oid)
#define get_oid_tree(str, oid) repo_get_oid_tree(the_repository, str, oid)
#define get_oid_treeish(str, oid) repo_get_oid_treeish(the_repository, str, oid)
#define get_oid_blob(str, oid) repo_get_oid_blob(the_repository, str, oid)
#define get_oid_mb(str, oid) repo_get_oid_mb(the_repository, str, oid)
typedef int each_abbrev_fn(const struct object_id *oid, void *);
int repo_for_each_abbrev(struct repository *r, const char *prefix, each_abbrev_fn, void *);
#define for_each_abbrev(prefix, fn, data) repo_for_each_abbrev(the_repository, prefix, fn, data)
int set_disambiguate_hint_config(const char *var, const char *value);
int get_sha1_hex(const char *hex, unsigned char *sha1);
int get_oid_hex(const char *hex, struct object_id *sha1);
int get_oid_hex_algop(const char *hex, struct object_id *oid, const struct git_hash_algo *algop);
int hex_to_bytes(unsigned char *binary, const char *hex, size_t len);
char *hash_to_hex_algop_r(char *buffer, const unsigned char *hash, const struct git_hash_algo *);
char *oid_to_hex_r(char *out, const struct object_id *oid);
char *hash_to_hex_algop(const unsigned char *hash, const struct git_hash_algo *); 
char *hash_to_hex(const unsigned char *hash); 
char *oid_to_hex(const struct object_id *oid); 
int parse_oid_hex(const char *hex, struct object_id *oid, const char **end);
int parse_oid_hex_algop(const char *hex, struct object_id *oid, const char **end,
const struct git_hash_algo *algo);
int get_oid_hex_any(const char *hex, struct object_id *oid);
int parse_oid_hex_any(const char *hex, struct object_id *oid, const char **end);
#define INTERPRET_BRANCH_LOCAL (1<<0)
#define INTERPRET_BRANCH_REMOTE (1<<1)
#define INTERPRET_BRANCH_HEAD (1<<2)
int repo_interpret_branch_name(struct repository *r,
const char *str, int len,
struct strbuf *buf,
unsigned allowed);
#define interpret_branch_name(str, len, buf, allowed) repo_interpret_branch_name(the_repository, str, len, buf, allowed)
int validate_headref(const char *ref);
int base_name_compare(const char *name1, int len1, int mode1, const char *name2, int len2, int mode2);
int df_name_compare(const char *name1, int len1, int mode1, const char *name2, int len2, int mode2);
int name_compare(const char *name1, size_t len1, const char *name2, size_t len2);
int cache_name_stage_compare(const char *name1, int len1, int stage1, const char *name2, int len2, int stage2);
void *read_object_with_reference(struct repository *r,
const struct object_id *oid,
const char *required_type,
unsigned long *size,
struct object_id *oid_ret);
struct object *repo_peel_to_type(struct repository *r,
const char *name, int namelen,
struct object *o, enum object_type);
#define peel_to_type(name, namelen, obj, type) repo_peel_to_type(the_repository, name, namelen, obj, type)
enum date_mode_type {
DATE_NORMAL = 0,
DATE_HUMAN,
DATE_RELATIVE,
DATE_SHORT,
DATE_ISO8601,
DATE_ISO8601_STRICT,
DATE_RFC2822,
DATE_STRFTIME,
DATE_RAW,
DATE_UNIX
};
struct date_mode {
enum date_mode_type type;
const char *strftime_fmt;
int local;
};
#define DATE_MODE(t) date_mode_from_type(DATE_##t)
struct date_mode *date_mode_from_type(enum date_mode_type type);
const char *show_date(timestamp_t time, int timezone, const struct date_mode *mode);
void show_date_relative(timestamp_t time, struct strbuf *timebuf);
void show_date_human(timestamp_t time, int tz, const struct timeval *now,
struct strbuf *timebuf);
int parse_date(const char *date, struct strbuf *out);
int parse_date_basic(const char *date, timestamp_t *timestamp, int *offset);
int parse_expiry_date(const char *date, timestamp_t *timestamp);
void datestamp(struct strbuf *out);
#define approxidate(s) approxidate_careful((s), NULL)
timestamp_t approxidate_careful(const char *, int *);
timestamp_t approxidate_relative(const char *date);
void parse_date_format(const char *format, struct date_mode *mode);
int date_overflows(timestamp_t date);
#define IDENT_STRICT 1
#define IDENT_NO_DATE 2
#define IDENT_NO_NAME 4
enum want_ident {
WANT_BLANK_IDENT,
WANT_AUTHOR_IDENT,
WANT_COMMITTER_IDENT
};
const char *git_author_info(int);
const char *git_committer_info(int);
const char *fmt_ident(const char *name, const char *email,
enum want_ident whose_ident,
const char *date_str, int);
const char *fmt_name(enum want_ident);
const char *ident_default_name(void);
const char *ident_default_email(void);
const char *git_editor(void);
const char *git_sequence_editor(void);
const char *git_pager(int stdout_is_tty);
int is_terminal_dumb(void);
int git_ident_config(const char *, const char *, void *);
void prepare_fallback_ident(const char *name, const char *email);
void reset_ident_date(void);
struct ident_split {
const char *name_begin;
const char *name_end;
const char *mail_begin;
const char *mail_end;
const char *date_begin;
const char *date_end;
const char *tz_begin;
const char *tz_end;
};
int split_ident_line(struct ident_split *, const char *, int);
const char *show_ident_date(const struct ident_split *id,
const struct date_mode *mode);
int ident_cmp(const struct ident_split *, const struct ident_split *);
struct checkout {
struct index_state *istate;
const char *base_dir;
int base_dir_len;
struct delayed_checkout *delayed_checkout;
struct checkout_metadata meta;
unsigned force:1,
quiet:1,
not_new:1,
clone:1,
refresh_cache:1;
};
#define CHECKOUT_INIT { NULL, "" }
#define TEMPORARY_FILENAME_LENGTH 25
int checkout_entry(struct cache_entry *ce, const struct checkout *state, char *topath, int *nr_checkouts);
void enable_delayed_checkout(struct checkout *state);
int finish_delayed_checkout(struct checkout *state, int *nr_checkouts);
void unlink_entry(const struct cache_entry *ce);
struct cache_def {
struct strbuf path;
int flags;
int track_flags;
int prefix_len_stat_func;
};
#define CACHE_DEF_INIT { STRBUF_INIT, 0, 0, 0 }
static inline void cache_def_clear(struct cache_def *cache)
{
strbuf_release(&cache->path);
}
int has_symlink_leading_path(const char *name, int len);
int threaded_has_symlink_leading_path(struct cache_def *, const char *, int);
int check_leading_path(const char *name, int len);
int has_dirs_only_path(const char *name, int len, int prefix_len);
void schedule_dir_for_removal(const char *name, int len);
void remove_scheduled_dirs(void);
struct pack_window {
struct pack_window *next;
unsigned char *base;
off_t offset;
size_t len;
unsigned int last_used;
unsigned int inuse_cnt;
};
struct pack_entry {
off_t offset;
struct packed_git *p;
};
int odb_mkstemp(struct strbuf *temp_filename, const char *pattern);
int odb_pack_keep(const char *name);
extern int fetch_if_missing;
int update_server_info(int);
const char *get_log_output_encoding(void);
const char *get_commit_output_encoding(void);
extern int ignore_untracked_cache_config;
int committer_ident_sufficiently_given(void);
int author_ident_sufficiently_given(void);
extern const char *git_commit_encoding;
extern const char *git_log_output_encoding;
extern const char *git_mailmap_file;
extern const char *git_mailmap_blob;
void maybe_flush_or_die(FILE *, const char *);
__attribute__((format (printf, 2, 3)))
void fprintf_or_die(FILE *, const char *fmt, ...);
#define COPY_READ_ERROR (-2)
#define COPY_WRITE_ERROR (-3)
int copy_fd(int ifd, int ofd);
int copy_file(const char *dst, const char *src, int mode);
int copy_file_with_time(const char *dst, const char *src, int mode);
void write_or_die(int fd, const void *buf, size_t count);
void fsync_or_die(int fd, const char *);
ssize_t read_in_full(int fd, void *buf, size_t count);
ssize_t write_in_full(int fd, const void *buf, size_t count);
ssize_t pread_in_full(int fd, void *buf, size_t count, off_t offset);
static inline ssize_t write_str_in_full(int fd, const char *str)
{
return write_in_full(fd, str, strlen(str));
}
void write_file_buf(const char *path, const char *buf, size_t len);
__attribute__((format (printf, 2, 3)))
void write_file(const char *path, const char *fmt, ...);
void setup_pager(void);
int pager_in_use(void);
extern int pager_use_color;
int term_columns(void);
void term_clear_line(void);
int decimal_width(uintmax_t);
int check_pager_config(const char *cmd);
void prepare_pager_args(struct child_process *, const char *pager);
extern const char *editor_program;
extern const char *askpass_program;
extern const char *excludes_file;
int decode_85(char *dst, const char *line, int linelen);
void encode_85(char *buf, const unsigned char *data, int bytes);
void packet_trace_identity(const char *prog);
int add_files_to_cache(const char *prefix, const struct pathspec *pathspec, int flags);
extern int diff_auto_refresh_index;
void shift_tree(struct repository *, const struct object_id *, const struct object_id *, struct object_id *, int);
void shift_tree_by(struct repository *, const struct object_id *, const struct object_id *, struct object_id *, const char *);
#define WS_BLANK_AT_EOL 0100
#define WS_SPACE_BEFORE_TAB 0200
#define WS_INDENT_WITH_NON_TAB 0400
#define WS_CR_AT_EOL 01000
#define WS_BLANK_AT_EOF 02000
#define WS_TAB_IN_INDENT 04000
#define WS_TRAILING_SPACE (WS_BLANK_AT_EOL|WS_BLANK_AT_EOF)
#define WS_DEFAULT_RULE (WS_TRAILING_SPACE|WS_SPACE_BEFORE_TAB|8)
#define WS_TAB_WIDTH_MASK 077
#define WS_RULE_MASK 07777
extern unsigned whitespace_rule_cfg;
unsigned whitespace_rule(struct index_state *, const char *);
unsigned parse_whitespace_rule(const char *);
unsigned ws_check(const char *line, int len, unsigned ws_rule);
void ws_check_emit(const char *line, int len, unsigned ws_rule, FILE *stream, const char *set, const char *reset, const char *ws);
char *whitespace_error_string(unsigned ws);
void ws_fix_copy(struct strbuf *, const char *, int, unsigned, int *);
int ws_blank_line(const char *line, int len, unsigned ws_rule);
#define ws_tab_width(rule) ((rule) & WS_TAB_WIDTH_MASK)
void overlay_tree_on_index(struct index_state *istate,
const char *tree_name, const char *prefix);
struct startup_info {
int have_repository;
const char *prefix;
};
extern struct startup_info *startup_info;
struct commit_list;
int try_merge_command(struct repository *r,
const char *strategy, size_t xopts_nr,
const char **xopts, struct commit_list *common,
const char *head_arg, struct commit_list *remotes);
int checkout_fast_forward(struct repository *r,
const struct object_id *from,
const struct object_id *to,
int overwrite_ignore);
int sane_execvp(const char *file, char *const argv[]);
struct stat_validity {
struct stat_data *sd;
};
void stat_validity_clear(struct stat_validity *sv);
int stat_validity_check(struct stat_validity *sv, const char *path);
void stat_validity_update(struct stat_validity *sv, int fd);
int versioncmp(const char *s1, const char *s2);
void sleep_millisec(int millisec);
void safe_create_dir(const char *dir, int share);
int print_sha1_ellipsis(void);
int is_empty_or_missing_file(const char *filename);
