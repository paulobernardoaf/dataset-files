#include "strbuf.h"
struct commit;
struct commit_list;
struct object_id;
struct repository;
struct tree;
struct merge_options_internal;
struct merge_options {
struct repository *repo;
const char *ancestor;
const char *branch1;
const char *branch2;
int detect_renames;
enum {
MERGE_DIRECTORY_RENAMES_NONE = 0,
MERGE_DIRECTORY_RENAMES_CONFLICT = 1,
MERGE_DIRECTORY_RENAMES_TRUE = 2
} detect_directory_renames;
int rename_limit;
int rename_score;
int show_rename_progress;
long xdl_opts;
enum {
MERGE_VARIANT_NORMAL = 0,
MERGE_VARIANT_OURS,
MERGE_VARIANT_THEIRS
} recursive_variant;
int verbosity;
unsigned buffer_output; 
struct strbuf obuf; 
const char *subtree_shift;
unsigned renormalize : 1;
struct merge_options_internal *priv;
};
void init_merge_options(struct merge_options *opt, struct repository *repo);
int parse_merge_opt(struct merge_options *opt, const char *s);
int merge_trees(struct merge_options *opt,
struct tree *head,
struct tree *merge,
struct tree *merge_base);
int merge_recursive(struct merge_options *opt,
struct commit *h1,
struct commit *h2,
struct commit_list *merge_bases,
struct commit **result);
int merge_recursive_generic(struct merge_options *opt,
const struct object_id *head,
const struct object_id *merge,
int num_merge_bases,
const struct object_id **merge_bases,
struct commit **result);
