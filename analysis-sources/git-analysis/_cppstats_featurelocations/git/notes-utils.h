#if !defined(NOTES_UTILS_H)
#define NOTES_UTILS_H

#include "notes.h"

struct commit_list;
struct object_id;
struct repository;












void create_notes_commit(struct repository *r,
struct notes_tree *t,
struct commit_list *parents,
const char *msg, size_t msg_len,
struct object_id *result_oid);

void commit_notes(struct repository *r, struct notes_tree *t, const char *msg);

enum notes_merge_strategy {
NOTES_MERGE_RESOLVE_MANUAL = 0,
NOTES_MERGE_RESOLVE_OURS,
NOTES_MERGE_RESOLVE_THEIRS,
NOTES_MERGE_RESOLVE_UNION,
NOTES_MERGE_RESOLVE_CAT_SORT_UNIQ
};

struct notes_rewrite_cfg {
struct notes_tree **trees;
const char *cmd;
int enabled;
combine_notes_fn combine;
struct string_list *refs;
int refs_from_env;
int mode_from_env;
};

int parse_notes_merge_strategy(const char *v, enum notes_merge_strategy *s);
struct notes_rewrite_cfg *init_copy_notes_for_rewrite(const char *cmd);
int copy_note_for_rewrite(struct notes_rewrite_cfg *c,
const struct object_id *from_obj, const struct object_id *to_obj);
void finish_copy_notes_for_rewrite(struct repository *r,
struct notes_rewrite_cfg *c,
const char *msg);

#endif
