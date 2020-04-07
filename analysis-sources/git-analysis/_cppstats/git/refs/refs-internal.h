#include "cache.h"
#include "refs.h"
#include "iterator.h"
struct ref_transaction;
#define REF_HAVE_NEW (1 << 2)
#define REF_HAVE_OLD (1 << 3)
long get_files_ref_lock_timeout_ms(void);
int refname_is_safe(const char *refname);
int ref_resolves_to_object(const char *refname,
const struct object_id *oid,
unsigned int flags);
enum peel_status {
PEEL_PEELED = 0,
PEEL_INVALID = -1,
PEEL_NON_TAG = -2,
PEEL_IS_SYMREF = -3,
PEEL_BROKEN = -4
};
enum peel_status peel_object(const struct object_id *name, struct object_id *oid);
void copy_reflog_msg(struct strbuf *sb, const char *msg);
struct ref_update {
struct object_id new_oid;
struct object_id old_oid;
unsigned int flags;
void *backend_data;
unsigned int type;
char *msg;
struct ref_update *parent_update;
const char refname[FLEX_ARRAY];
};
int refs_read_raw_ref(struct ref_store *ref_store,
const char *refname, struct object_id *oid,
struct strbuf *referent, unsigned int *type);
int ref_update_reject_duplicates(struct string_list *refnames,
struct strbuf *err);
struct ref_update *ref_transaction_add_update(
struct ref_transaction *transaction,
const char *refname, unsigned int flags,
const struct object_id *new_oid,
const struct object_id *old_oid,
const char *msg);
enum ref_transaction_state {
REF_TRANSACTION_OPEN = 0,
REF_TRANSACTION_PREPARED = 1,
REF_TRANSACTION_CLOSED = 2
};
struct ref_transaction {
struct ref_store *ref_store;
struct ref_update **updates;
size_t alloc;
size_t nr;
enum ref_transaction_state state;
void *backend_data;
};
const char *find_descendant_ref(const char *dirname,
const struct string_list *extras,
const struct string_list *skip);
int refs_rename_ref_available(struct ref_store *refs,
const char *old_refname,
const char *new_refname);
#define SYMREF_MAXDEPTH 5
#define DO_FOR_EACH_INCLUDE_BROKEN 0x01
struct ref_iterator {
struct ref_iterator_vtable *vtable;
unsigned int ordered : 1;
const char *refname;
const struct object_id *oid;
unsigned int flags;
};
int ref_iterator_advance(struct ref_iterator *ref_iterator);
int ref_iterator_peel(struct ref_iterator *ref_iterator,
struct object_id *peeled);
int ref_iterator_abort(struct ref_iterator *ref_iterator);
struct ref_iterator *empty_ref_iterator_begin(void);
int is_empty_ref_iterator(struct ref_iterator *ref_iterator);
struct ref_iterator *refs_ref_iterator_begin(
struct ref_store *refs,
const char *prefix, int trim, int flags);
typedef enum iterator_selection ref_iterator_select_fn(
struct ref_iterator *iter0, struct ref_iterator *iter1,
void *cb_data);
struct ref_iterator *merge_ref_iterator_begin(
int ordered,
struct ref_iterator *iter0, struct ref_iterator *iter1,
ref_iterator_select_fn *select, void *cb_data);
struct ref_iterator *overlay_ref_iterator_begin(
struct ref_iterator *front, struct ref_iterator *back);
struct ref_iterator *prefix_ref_iterator_begin(struct ref_iterator *iter0,
const char *prefix,
int trim);
void base_ref_iterator_init(struct ref_iterator *iter,
struct ref_iterator_vtable *vtable,
int ordered);
void base_ref_iterator_free(struct ref_iterator *iter);
typedef int ref_iterator_advance_fn(struct ref_iterator *ref_iterator);
typedef int ref_iterator_peel_fn(struct ref_iterator *ref_iterator,
struct object_id *peeled);
typedef int ref_iterator_abort_fn(struct ref_iterator *ref_iterator);
struct ref_iterator_vtable {
ref_iterator_advance_fn *advance;
ref_iterator_peel_fn *peel;
ref_iterator_abort_fn *abort;
};
extern struct ref_iterator *current_ref_iter;
int do_for_each_repo_ref_iterator(struct repository *r,
struct ref_iterator *iter,
each_repo_ref_fn fn, void *cb_data);
#define DO_FOR_EACH_PER_WORKTREE_ONLY 0x02
struct ref_store;
#define REF_STORE_READ (1 << 0)
#define REF_STORE_WRITE (1 << 1) 
#define REF_STORE_ODB (1 << 2) 
#define REF_STORE_MAIN (1 << 3)
#define REF_STORE_ALL_CAPS (REF_STORE_READ | REF_STORE_WRITE | REF_STORE_ODB | REF_STORE_MAIN)
typedef struct ref_store *ref_store_init_fn(const char *gitdir,
unsigned int flags);
typedef int ref_init_db_fn(struct ref_store *refs, struct strbuf *err);
typedef int ref_transaction_prepare_fn(struct ref_store *refs,
struct ref_transaction *transaction,
struct strbuf *err);
typedef int ref_transaction_finish_fn(struct ref_store *refs,
struct ref_transaction *transaction,
struct strbuf *err);
typedef int ref_transaction_abort_fn(struct ref_store *refs,
struct ref_transaction *transaction,
struct strbuf *err);
typedef int ref_transaction_commit_fn(struct ref_store *refs,
struct ref_transaction *transaction,
struct strbuf *err);
typedef int pack_refs_fn(struct ref_store *ref_store, unsigned int flags);
typedef int create_symref_fn(struct ref_store *ref_store,
const char *ref_target,
const char *refs_heads_master,
const char *logmsg);
typedef int delete_refs_fn(struct ref_store *ref_store, const char *msg,
struct string_list *refnames, unsigned int flags);
typedef int rename_ref_fn(struct ref_store *ref_store,
const char *oldref, const char *newref,
const char *logmsg);
typedef int copy_ref_fn(struct ref_store *ref_store,
const char *oldref, const char *newref,
const char *logmsg);
typedef struct ref_iterator *ref_iterator_begin_fn(
struct ref_store *ref_store,
const char *prefix, unsigned int flags);
typedef struct ref_iterator *reflog_iterator_begin_fn(
struct ref_store *ref_store);
typedef int for_each_reflog_ent_fn(struct ref_store *ref_store,
const char *refname,
each_reflog_ent_fn fn,
void *cb_data);
typedef int for_each_reflog_ent_reverse_fn(struct ref_store *ref_store,
const char *refname,
each_reflog_ent_fn fn,
void *cb_data);
typedef int reflog_exists_fn(struct ref_store *ref_store, const char *refname);
typedef int create_reflog_fn(struct ref_store *ref_store, const char *refname,
int force_create, struct strbuf *err);
typedef int delete_reflog_fn(struct ref_store *ref_store, const char *refname);
typedef int reflog_expire_fn(struct ref_store *ref_store,
const char *refname, const struct object_id *oid,
unsigned int flags,
reflog_expiry_prepare_fn prepare_fn,
reflog_expiry_should_prune_fn should_prune_fn,
reflog_expiry_cleanup_fn cleanup_fn,
void *policy_cb_data);
typedef int read_raw_ref_fn(struct ref_store *ref_store,
const char *refname, struct object_id *oid,
struct strbuf *referent, unsigned int *type);
struct ref_storage_be {
struct ref_storage_be *next;
const char *name;
ref_store_init_fn *init;
ref_init_db_fn *init_db;
ref_transaction_prepare_fn *transaction_prepare;
ref_transaction_finish_fn *transaction_finish;
ref_transaction_abort_fn *transaction_abort;
ref_transaction_commit_fn *initial_transaction_commit;
pack_refs_fn *pack_refs;
create_symref_fn *create_symref;
delete_refs_fn *delete_refs;
rename_ref_fn *rename_ref;
copy_ref_fn *copy_ref;
ref_iterator_begin_fn *iterator_begin;
read_raw_ref_fn *read_raw_ref;
reflog_iterator_begin_fn *reflog_iterator_begin;
for_each_reflog_ent_fn *for_each_reflog_ent;
for_each_reflog_ent_reverse_fn *for_each_reflog_ent_reverse;
reflog_exists_fn *reflog_exists;
create_reflog_fn *create_reflog;
delete_reflog_fn *delete_reflog;
reflog_expire_fn *reflog_expire;
};
extern struct ref_storage_be refs_be_files;
extern struct ref_storage_be refs_be_packed;
struct ref_store {
const struct ref_storage_be *be;
};
void base_ref_store_init(struct ref_store *refs,
const struct ref_storage_be *be);
