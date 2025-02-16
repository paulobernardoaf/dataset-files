#if !defined(DIR_H)
#define DIR_H

#include "cache.h"
#include "hashmap.h"
#include "strbuf.h"




































struct dir_entry {
unsigned int len;
char name[FLEX_ARRAY]; 
};

#define PATTERN_FLAG_NODIR 1
#define PATTERN_FLAG_ENDSWITH 4
#define PATTERN_FLAG_MUSTBEDIR 8
#define PATTERN_FLAG_NEGATIVE 16

struct path_pattern {




struct pattern_list *pl;

const char *pattern;
int patternlen;
int nowildcardlen;
const char *base;
int baselen;
unsigned flags; 





int srcpos;
};


struct pattern_entry {
struct hashmap_entry ent;
char *pattern;
size_t patternlen;
};








struct pattern_list {
int nr;
int alloc;


char *filebuf;


const char *src;

struct path_pattern **patterns;







unsigned use_cone_patterns;
unsigned full_cone;





struct hashmap recursive_hashmap;




struct hashmap parent_hashmap;
};







struct exclude_stack {
struct exclude_stack *prev; 
int baselen;
int exclude_ix; 
struct untracked_cache_dir *ucd;
};

struct exclude_list_group {
int nr, alloc;
struct pattern_list *pl;
};

struct oid_stat {
struct stat_data stat;
struct object_id oid;
int valid;
};





























struct untracked_cache_dir {
struct untracked_cache_dir **dirs;
char **untracked;
struct stat_data stat_data;
unsigned int untracked_alloc, dirs_nr, dirs_alloc;
unsigned int untracked_nr;
unsigned int check_only : 1;

unsigned int valid : 1;
unsigned int recurse : 1;

struct object_id exclude_oid;
char name[FLEX_ARRAY];
};

struct untracked_cache {
struct oid_stat ss_info_exclude;
struct oid_stat ss_excludes_file;
const char *exclude_per_dir;
struct strbuf ident;




unsigned dir_flags;
struct untracked_cache_dir *root;

int dir_created;
int gitignore_invalidated;
int dir_invalidated;
int dir_opened;

unsigned int use_fsmonitor : 1;
};






struct dir_struct {


int nr;


int alloc;


int ignored_nr;

int ignored_alloc;


enum {





DIR_SHOW_IGNORED = 1<<0,


DIR_SHOW_OTHER_DIRECTORIES = 1<<1,


DIR_HIDE_EMPTY_DIRECTORIES = 1<<2,





DIR_NO_GITLINKS = 1<<3,







DIR_COLLECT_IGNORED = 1<<4,






DIR_SHOW_IGNORED_TOO = 1<<5,

DIR_COLLECT_KILLED_ONLY = 1<<6,






DIR_KEEP_UNTRACKED_CONTENTS = 1<<7,















DIR_SHOW_IGNORED_TOO_MODE_MATCHING = 1<<8,

DIR_SKIP_NESTED_GIT = 1<<9
} flags;


struct dir_entry **entries;





struct dir_entry **ignored;





const char *exclude_per_dir;













#define EXC_CMDL 0
#define EXC_DIRS 1
#define EXC_FILE 2
struct exclude_list_group exclude_list_group[3];










struct exclude_stack *exclude_stack;
struct path_pattern *pattern;
struct strbuf basebuf;


struct untracked_cache *untracked;
struct oid_stat ss_info_exclude;
struct oid_stat ss_excludes_file;
unsigned unmanaged_exclude_files;
};


int count_slashes(const char *s);







#define MATCHED_RECURSIVELY 1
#define MATCHED_RECURSIVELY_LEADING_PATHSPEC 2
#define MATCHED_FNMATCH 3
#define MATCHED_EXACTLY 4
int simple_length(const char *match);
int no_wildcard(const char *string);
char *common_prefix(const struct pathspec *pathspec);
int match_pathspec(const struct index_state *istate,
const struct pathspec *pathspec,
const char *name, int namelen,
int prefix, char *seen, int is_dir);
int report_path_error(const char *ps_matched, const struct pathspec *pathspec);
int within_depth(const char *name, int namelen, int depth, int max_depth);

int fill_directory(struct dir_struct *dir,
struct index_state *istate,
const struct pathspec *pathspec);
int read_directory(struct dir_struct *, struct index_state *istate,
const char *path, int len,
const struct pathspec *pathspec);

enum pattern_match_result {
UNDECIDED = -1,
NOT_MATCHED = 0,
MATCHED = 1,
MATCHED_RECURSIVE = 2,
};







enum pattern_match_result path_matches_pattern_list(const char *pathname,
int pathlen,
const char *basename, int *dtype,
struct pattern_list *pl,
struct index_state *istate);
struct dir_entry *dir_add_ignored(struct dir_struct *dir,
struct index_state *istate,
const char *pathname, int len);





int match_basename(const char *, int,
const char *, int, int, unsigned);
int match_pathname(const char *, int,
const char *, int,
const char *, int, int, unsigned);

struct path_pattern *last_matching_pattern(struct dir_struct *dir,
struct index_state *istate,
const char *name, int *dtype);

int is_excluded(struct dir_struct *dir,
struct index_state *istate,
const char *name, int *dtype);

int pl_hashmap_cmp(const void *unused_cmp_data,
const struct hashmap_entry *a,
const struct hashmap_entry *b,
const void *key);
int hashmap_contains_parent(struct hashmap *map,
const char *path,
struct strbuf *buffer);
struct pattern_list *add_pattern_list(struct dir_struct *dir,
int group_type, const char *src);
int add_patterns_from_file_to_list(const char *fname, const char *base, int baselen,
struct pattern_list *pl, struct index_state *istate);
void add_patterns_from_file(struct dir_struct *, const char *fname);
int add_patterns_from_blob_to_list(struct object_id *oid,
const char *base, int baselen,
struct pattern_list *pl);
void parse_path_pattern(const char **string, int *patternlen, unsigned *flags, int *nowildcardlen);
void add_pattern(const char *string, const char *base,
int baselen, struct pattern_list *pl, int srcpos);
void clear_pattern_list(struct pattern_list *pl);
void clear_directory(struct dir_struct *dir);

int repo_file_exists(struct repository *repo, const char *path);
int file_exists(const char *);

int is_inside_dir(const char *dir);
int dir_inside_of(const char *subdir, const char *dir);

static inline int is_dot_or_dotdot(const char *name)
{
return (name[0] == '.' &&
(name[1] == '\0' ||
(name[1] == '.' && name[2] == '\0')));
}

int is_empty_dir(const char *dir);

void setup_standard_excludes(struct dir_struct *dir);









#define REMOVE_DIR_EMPTY_ONLY 01





#define REMOVE_DIR_KEEP_NESTED_GIT 02


#define REMOVE_DIR_KEEP_TOPLEVEL 04








int remove_dir_recursively(struct strbuf *path, int flag);


int remove_path(const char *path);

int fspathcmp(const char *a, const char *b);
int fspathncmp(const char *a, const char *b, size_t count);




struct pathspec_item;
int git_fnmatch(const struct pathspec_item *item,
const char *pattern, const char *string,
int prefix);

int submodule_path_match(const struct index_state *istate,
const struct pathspec *ps,
const char *submodule_name,
char *seen);

static inline int ce_path_match(const struct index_state *istate,
const struct cache_entry *ce,
const struct pathspec *pathspec,
char *seen)
{
return match_pathspec(istate, pathspec, ce->name, ce_namelen(ce), 0, seen,
S_ISDIR(ce->ce_mode) || S_ISGITLINK(ce->ce_mode));
}

static inline int dir_path_match(const struct index_state *istate,
const struct dir_entry *ent,
const struct pathspec *pathspec,
int prefix, char *seen)
{
int has_trailing_dir = ent->len && ent->name[ent->len - 1] == '/';
int len = has_trailing_dir ? ent->len - 1 : ent->len;
return match_pathspec(istate, pathspec, ent->name, len, prefix, seen,
has_trailing_dir);
}

int cmp_dir_entry(const void *p1, const void *p2);
int check_dir_entry_contains(const struct dir_entry *out, const struct dir_entry *in);

void untracked_cache_invalidate_path(struct index_state *, const char *, int safe_path);
void untracked_cache_remove_from_index(struct index_state *, const char *);
void untracked_cache_add_to_index(struct index_state *, const char *);

void free_untracked_cache(struct untracked_cache *);
struct untracked_cache *read_untracked_extension(const void *data, unsigned long sz);
void write_untracked_extension(struct strbuf *out, struct untracked_cache *untracked);
void add_untracked_cache(struct index_state *istate);
void remove_untracked_cache(struct index_state *istate);








void connect_work_tree_and_git_dir(const char *work_tree,
const char *git_dir,
int recurse_into_nested);
void relocate_gitdir(const char *path,
const char *old_git_dir,
const char *new_git_dir);
#endif
