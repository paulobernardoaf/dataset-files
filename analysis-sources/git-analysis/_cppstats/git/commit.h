#include "object.h"
#include "tree.h"
#include "strbuf.h"
#include "decorate.h"
#include "gpg-interface.h"
#include "string-list.h"
#include "pretty.h"
#include "commit-slab.h"
#define COMMIT_NOT_FROM_GRAPH 0xFFFFFFFF
#define GENERATION_NUMBER_INFINITY 0xFFFFFFFF
#define GENERATION_NUMBER_MAX 0x3FFFFFFF
#define GENERATION_NUMBER_ZERO 0
struct commit_list {
struct commit *item;
struct commit_list *next;
};
struct commit {
struct object object;
timestamp_t date;
struct commit_list *parents;
struct tree *maybe_tree;
uint32_t graph_pos;
uint32_t generation;
unsigned int index;
};
extern int save_commit_buffer;
extern const char *commit_type;
struct name_decoration {
struct name_decoration *next;
int type;
char name[FLEX_ARRAY];
};
enum decoration_type {
DECORATION_NONE = 0,
DECORATION_REF_LOCAL,
DECORATION_REF_REMOTE,
DECORATION_REF_TAG,
DECORATION_REF_STASH,
DECORATION_REF_HEAD,
DECORATION_GRAFTED,
};
void add_name_decoration(enum decoration_type type, const char *name, struct object *obj);
const struct name_decoration *get_name_decoration(const struct object *obj);
struct commit *lookup_commit(struct repository *r, const struct object_id *oid);
struct commit *lookup_commit_reference(struct repository *r,
const struct object_id *oid);
struct commit *lookup_commit_reference_gently(struct repository *r,
const struct object_id *oid,
int quiet);
struct commit *lookup_commit_reference_by_name(const char *name);
struct commit *lookup_commit_or_die(const struct object_id *oid, const char *ref_name);
int parse_commit_buffer(struct repository *r, struct commit *item, const void *buffer, unsigned long size, int check_graph);
int repo_parse_commit_internal(struct repository *r, struct commit *item,
int quiet_on_missing, int use_commit_graph);
int repo_parse_commit_gently(struct repository *r,
struct commit *item,
int quiet_on_missing);
static inline int repo_parse_commit(struct repository *r, struct commit *item)
{
return repo_parse_commit_gently(r, item, 0);
}
static inline int parse_commit_no_graph(struct commit *commit)
{
return repo_parse_commit_internal(the_repository, commit, 0, 0);
}
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define parse_commit_internal(item, quiet, use) repo_parse_commit_internal(the_repository, item, quiet, use)
#define parse_commit_gently(item, quiet) repo_parse_commit_gently(the_repository, item, quiet)
#define parse_commit(item) repo_parse_commit(the_repository, item)
#endif
void parse_commit_or_die(struct commit *item);
struct buffer_slab;
struct buffer_slab *allocate_commit_buffer_slab(void);
void free_commit_buffer_slab(struct buffer_slab *bs);
void set_commit_buffer(struct repository *r, struct commit *, void *buffer, unsigned long size);
const void *get_cached_commit_buffer(struct repository *, const struct commit *, unsigned long *size);
const void *repo_get_commit_buffer(struct repository *r,
const struct commit *,
unsigned long *size);
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define get_commit_buffer(c, s) repo_get_commit_buffer(the_repository, c, s)
#endif
void repo_unuse_commit_buffer(struct repository *r,
const struct commit *,
const void *buffer);
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define unuse_commit_buffer(c, b) repo_unuse_commit_buffer(the_repository, c, b)
#endif
void free_commit_buffer(struct parsed_object_pool *pool, struct commit *);
struct tree *repo_get_commit_tree(struct repository *, const struct commit *);
#define get_commit_tree(c) repo_get_commit_tree(the_repository, c)
struct object_id *get_commit_tree_oid(const struct commit *);
void release_commit_memory(struct parsed_object_pool *pool, struct commit *c);
const void *detach_commit_buffer(struct commit *, unsigned long *sizep);
int find_commit_subject(const char *commit_buffer, const char **subject);
struct commit_list *commit_list_insert(struct commit *item,
struct commit_list **list);
struct commit_list **commit_list_append(struct commit *commit,
struct commit_list **next);
unsigned commit_list_count(const struct commit_list *l);
struct commit_list *commit_list_insert_by_date(struct commit *item,
struct commit_list **list);
void commit_list_sort_by_date(struct commit_list **list);
struct commit_list *copy_commit_list(struct commit_list *list);
void free_commit_list(struct commit_list *list);
struct rev_info; 
int has_non_ascii(const char *text);
const char *logmsg_reencode(const struct commit *commit,
char **commit_encoding,
const char *output_encoding);
const char *repo_logmsg_reencode(struct repository *r,
const struct commit *commit,
char **commit_encoding,
const char *output_encoding);
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define logmsg_reencode(c, enc, out) repo_logmsg_reencode(the_repository, c, enc, out)
#endif
const char *skip_blank_lines(const char *msg);
struct commit *pop_most_recent_commit(struct commit_list **list,
unsigned int mark);
struct commit *pop_commit(struct commit_list **stack);
void clear_commit_marks(struct commit *commit, unsigned int mark);
void clear_commit_marks_many(int nr, struct commit **commit, unsigned int mark);
enum rev_sort_order {
REV_SORT_IN_GRAPH_ORDER = 0,
REV_SORT_BY_COMMIT_DATE,
REV_SORT_BY_AUTHOR_DATE
};
void sort_in_topological_order(struct commit_list **, enum rev_sort_order);
struct commit_graft {
struct object_id oid;
int nr_parent; 
struct object_id parent[FLEX_ARRAY]; 
};
typedef int (*each_commit_graft_fn)(const struct commit_graft *, void *);
struct commit_graft *read_graft_line(struct strbuf *line);
int register_commit_graft(struct repository *r, struct commit_graft *, int);
void prepare_commit_graft(struct repository *r);
struct commit_graft *lookup_commit_graft(struct repository *r, const struct object_id *oid);
struct commit *get_fork_point(const char *refname, struct commit *commit);
#define INFINITE_DEPTH 0x7fffffff
struct oid_array;
struct ref;
int register_shallow(struct repository *r, const struct object_id *oid);
int unregister_shallow(const struct object_id *oid);
int for_each_commit_graft(each_commit_graft_fn, void *);
int is_repository_shallow(struct repository *r);
struct commit_list *get_shallow_commits(struct object_array *heads,
int depth, int shallow_flag, int not_shallow_flag);
struct commit_list *get_shallow_commits_by_rev_list(
int ac, const char **av, int shallow_flag, int not_shallow_flag);
void set_alternate_shallow_file(struct repository *r, const char *path, int override);
int write_shallow_commits(struct strbuf *out, int use_pack_protocol,
const struct oid_array *extra);
void setup_alternate_shallow(struct lock_file *shallow_lock,
const char **alternate_shallow_file,
const struct oid_array *extra);
const char *setup_temporary_shallow(const struct oid_array *extra);
void advertise_shallow_grafts(int);
struct shallow_info {
struct oid_array *shallow;
int *ours, nr_ours;
int *theirs, nr_theirs;
struct oid_array *ref;
uint32_t **used_shallow;
int *need_reachability_test;
int *reachable;
int *shallow_ref;
struct commit **commits;
int nr_commits;
};
void prepare_shallow_info(struct shallow_info *, struct oid_array *);
void clear_shallow_info(struct shallow_info *);
void remove_nonexistent_theirs_shallow(struct shallow_info *);
void assign_shallow_commits_to_refs(struct shallow_info *info,
uint32_t **used,
int *ref_status);
int delayed_reachability_test(struct shallow_info *si, int c);
#define PRUNE_SHOW_ONLY 1
#define PRUNE_QUICK 2
void prune_shallow(unsigned options);
extern struct trace_key trace_shallow;
int interactive_add(int argc, const char **argv, const char *prefix, int patch);
int run_add_interactive(const char *revision, const char *patch_mode,
const struct pathspec *pathspec);
struct commit_extra_header {
struct commit_extra_header *next;
char *key;
char *value;
size_t len;
};
void append_merge_tag_headers(struct commit_list *parents,
struct commit_extra_header ***tail);
int commit_tree(const char *msg, size_t msg_len,
const struct object_id *tree,
struct commit_list *parents, struct object_id *ret,
const char *author, const char *sign_commit);
int commit_tree_extended(const char *msg, size_t msg_len,
const struct object_id *tree,
struct commit_list *parents,
struct object_id *ret, const char *author,
const char *sign_commit,
struct commit_extra_header *);
struct commit_extra_header *read_commit_extra_headers(struct commit *, const char **);
void free_commit_extra_headers(struct commit_extra_header *extra);
const char *find_commit_header(const char *msg, const char *key,
size_t *out_len);
size_t ignore_non_trailer(const char *buf, size_t len);
typedef int (*each_mergetag_fn)(struct commit *commit, struct commit_extra_header *extra,
void *cb_data);
int for_each_mergetag(each_mergetag_fn fn, struct commit *commit, void *data);
struct merge_remote_desc {
struct object *obj; 
char name[FLEX_ARRAY];
};
struct merge_remote_desc *merge_remote_util(struct commit *);
void set_merge_remote_desc(struct commit *commit,
const char *name, struct object *obj);
struct commit *get_merge_parent(const char *name);
int parse_signed_commit(const struct commit *commit,
struct strbuf *message, struct strbuf *signature);
int remove_signature(struct strbuf *buf);
int check_commit_signature(const struct commit *commit, struct signature_check *sigc);
struct author_date_slab;
void record_author_date(struct author_date_slab *author_date,
struct commit *commit);
int compare_commits_by_author_date(const void *a_, const void *b_, void *unused);
void verify_merge_signature(struct commit *commit, int verbose,
int check_trust);
int compare_commits_by_commit_date(const void *a_, const void *b_, void *unused);
int compare_commits_by_gen_then_commit_date(const void *a_, const void *b_, void *unused);
LAST_ARG_MUST_BE_NULL
int run_commit_hook(int editor_is_used, const char *index_file, const char *name, ...);
