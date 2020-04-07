#include "notes-utils.h"
#include "strbuf.h"
struct commit;
struct object_id;
struct repository;
#define NOTES_MERGE_WORKTREE "NOTES_MERGE_WORKTREE"
enum notes_merge_verbosity {
NOTES_MERGE_VERBOSITY_DEFAULT = 2,
NOTES_MERGE_VERBOSITY_MAX = 5
};
struct notes_merge_options {
struct repository *repo;
const char *local_ref;
const char *remote_ref;
struct strbuf commit_msg;
int verbosity;
enum notes_merge_strategy strategy;
unsigned has_worktree:1;
};
void init_notes_merge_options(struct repository *r,
struct notes_merge_options *o);
int notes_merge(struct notes_merge_options *o,
struct notes_tree *local_tree,
struct object_id *result_oid);
int notes_merge_commit(struct notes_merge_options *o,
struct notes_tree *partial_tree,
struct commit *partial_commit,
struct object_id *result_oid);
int notes_merge_abort(struct notes_merge_options *o);
