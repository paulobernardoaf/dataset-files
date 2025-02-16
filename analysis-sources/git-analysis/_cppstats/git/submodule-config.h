#include "cache.h"
#include "config.h"
#include "hashmap.h"
#include "submodule.h"
#include "strbuf.h"
struct submodule {
const char *path;
const char *name;
const char *url;
int fetch_recurse;
const char *ignore;
const char *branch;
struct submodule_update_strategy update_strategy;
struct object_id gitmodules_oid;
int recommend_shallow;
};
#define SUBMODULE_INIT { NULL, NULL, NULL, RECURSE_SUBMODULES_NONE, NULL, NULL, SUBMODULE_UPDATE_STRATEGY_INIT, { { 0 } }, -1 };
struct submodule_cache;
struct repository;
void submodule_cache_free(struct submodule_cache *cache);
int parse_submodule_fetchjobs(const char *var, const char *value);
int parse_fetch_recurse_submodules_arg(const char *opt, const char *arg);
struct option;
int option_fetch_parse_recurse_submodules(const struct option *opt,
const char *arg, int unset);
int parse_update_recurse_submodules_arg(const char *opt, const char *arg);
int parse_push_recurse_submodules_arg(const char *opt, const char *arg);
void repo_read_gitmodules(struct repository *repo, int skip_if_read);
void gitmodules_config_oid(const struct object_id *commit_oid);
const struct submodule *submodule_from_name(struct repository *r,
const struct object_id *commit_or_tree,
const char *name);
const struct submodule *submodule_from_path(struct repository *r,
const struct object_id *commit_or_tree,
const char *path);
void submodule_free(struct repository *r);
int print_config_from_gitmodules(struct repository *repo, const char *key);
int config_set_in_gitmodules_file_gently(const char *key, const char *value);
int check_submodule_name(const char *name);
void fetch_config_from_gitmodules(int *max_children, int *recurse_submodules);
void update_clone_config_from_gitmodules(int *max_jobs);
