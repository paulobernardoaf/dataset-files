#if !defined(REPOSITORY_H)
#define REPOSITORY_H

#include "path.h"

struct config_set;
struct git_hash_algo;
struct index_state;
struct lock_file;
struct pathspec;
struct raw_object_store;
struct submodule_cache;

enum untracked_cache_setting {
UNTRACKED_CACHE_UNSET = -1,
UNTRACKED_CACHE_REMOVE = 0,
UNTRACKED_CACHE_KEEP = 1,
UNTRACKED_CACHE_WRITE = 2
};

enum fetch_negotiation_setting {
FETCH_NEGOTIATION_UNSET = -1,
FETCH_NEGOTIATION_NONE = 0,
FETCH_NEGOTIATION_DEFAULT = 1,
FETCH_NEGOTIATION_SKIPPING = 2,
};

struct repo_settings {
int initialized;

int core_commit_graph;
int gc_write_commit_graph;
int fetch_write_commit_graph;

int index_version;
enum untracked_cache_setting core_untracked_cache;

int pack_use_sparse;
enum fetch_negotiation_setting fetch_negotiation_algorithm;
};

struct repository {





char *gitdir;





char *commondir;




struct raw_object_store *objects;








struct parsed_object_pool *parsed_objects;


struct ref_store *refs;




struct path_cache cached_paths;





char *graft_file;





char *index_file;





char *worktree;






char *submodule_prefix;

struct repo_settings settings;







struct config_set *config;


struct submodule_cache *submodule_cache;





struct index_state *index;


const struct git_hash_algo *hash_algo;


int trace2_repo_id;


int commit_graph_disabled;




unsigned different_commondir:1;
};

extern struct repository *the_repository;





struct set_gitdir_args {
const char *commondir;
const char *object_dir;
const char *graft_file;
const char *index_file;
const char *alternate_db;
};

void repo_set_gitdir(struct repository *repo, const char *root,
const struct set_gitdir_args *extra_args);
void repo_set_worktree(struct repository *repo, const char *path);
void repo_set_hash_algo(struct repository *repo, int algo);
void initialize_the_repository(void);
int repo_init(struct repository *r, const char *gitdir, const char *worktree);







struct submodule;
int repo_submodule_init(struct repository *subrepo,
struct repository *superproject,
const struct submodule *sub);
void repo_clear(struct repository *repo);









int repo_read_index(struct repository *repo);
int repo_hold_locked_index(struct repository *repo,
struct lock_file *lf,
int flags);

int repo_read_index_preload(struct repository *,
const struct pathspec *pathspec,
unsigned refresh_flags);
int repo_read_index_unmerged(struct repository *);




void repo_update_index_if_able(struct repository *, struct lock_file *);

void prepare_repo_settings(struct repository *r);

#endif 
