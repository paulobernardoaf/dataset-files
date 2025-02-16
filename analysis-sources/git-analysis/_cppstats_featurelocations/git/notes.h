#if !defined(NOTES_H)
#define NOTES_H

#include "string-list.h"

struct object_id;
struct strbuf;





















typedef int (*combine_notes_fn)(struct object_id *cur_oid,
const struct object_id *new_oid);


int combine_notes_concatenate(struct object_id *cur_oid,
const struct object_id *new_oid);
int combine_notes_overwrite(struct object_id *cur_oid,
const struct object_id *new_oid);
int combine_notes_ignore(struct object_id *cur_oid,
const struct object_id *new_oid);
int combine_notes_cat_sort_uniq(struct object_id *cur_oid,
const struct object_id *new_oid);










extern struct notes_tree {
struct int_node *root;
struct non_note *first_non_note, *prev_non_note;
char *ref;
char *update_ref;
combine_notes_fn combine_notes;
int initialized;
int dirty;
} default_notes_tree;













const char *default_notes_ref(void);







#define NOTES_INIT_EMPTY 1






#define NOTES_INIT_WRITABLE 2




















void init_notes(struct notes_tree *t, const char *notes_ref,
combine_notes_fn combine_notes, int flags);





















int add_note(struct notes_tree *t, const struct object_id *object_oid,
const struct object_id *note_oid, combine_notes_fn combine_notes);










int remove_note(struct notes_tree *t, const unsigned char *object_sha1);






const struct object_id *get_note(struct notes_tree *t,
const struct object_id *object_oid);













int copy_note(struct notes_tree *t,
const struct object_id *from_obj, const struct object_id *to_obj,
int force, combine_notes_fn combine_notes);

























#define FOR_EACH_NOTE_DONT_UNPACK_SUBTREES 1
#define FOR_EACH_NOTE_YIELD_SUBTREES 2

















typedef int each_note_fn(const struct object_id *object_oid,
const struct object_id *note_oid, char *note_path,
void *cb_data);
int for_each_note(struct notes_tree *t, int flags, each_note_fn fn,
void *cb_data);













int write_notes_tree(struct notes_tree *t, struct object_id *result);


#define NOTES_PRUNE_VERBOSE 1
#define NOTES_PRUNE_DRYRUN 2










void prune_notes(struct notes_tree *t, int flags);







void free_notes(struct notes_tree *t);

struct string_list;

struct display_notes_opt {
int use_default_notes;
struct string_list extra_notes_refs;
};




void init_display_notes(struct display_notes_opt *opt);











void enable_default_display_notes(struct display_notes_opt *opt, int *show_notes);
void enable_ref_display_notes(struct display_notes_opt *opt, int *show_notes,
const char *ref);
void disable_display_notes(struct display_notes_opt *opt, int *show_notes);













void load_display_notes(struct display_notes_opt *opt);










void format_display_notes(const struct object_id *object_oid,
struct strbuf *sb, const char *output_encoding, int raw);





struct notes_tree **load_notes_trees(struct string_list *refs, int flags);




void string_list_add_refs_by_glob(struct string_list *list, const char *glob);






void string_list_add_refs_from_colon_sep(struct string_list *list,
const char *globs);


void expand_notes_ref(struct strbuf *sb);






void expand_loose_notes_ref(struct strbuf *sb);

#endif
