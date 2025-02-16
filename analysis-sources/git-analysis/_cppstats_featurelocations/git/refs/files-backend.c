#include "../cache.h"
#include "../config.h"
#include "../refs.h"
#include "refs-internal.h"
#include "ref-cache.h"
#include "packed-backend.h"
#include "../iterator.h"
#include "../dir-iterator.h"
#include "../lockfile.h"
#include "../object.h"
#include "../dir.h"
#include "../chdir-notify.h"
#include "worktree.h"













#define REF_IS_PRUNING (1 << 4)





#define REF_DELETING (1 << 5)





#define REF_NEEDS_COMMIT (1 << 6)






#define REF_LOG_ONLY (1 << 7)





#define REF_UPDATE_VIA_HEAD (1 << 8)





#define REF_DELETED_LOOSE (1 << 9)

struct ref_lock {
char *ref_name;
struct lock_file lk;
struct object_id old_oid;
};

struct files_ref_store {
struct ref_store base;
unsigned int store_flags;

char *gitdir;
char *gitcommondir;

struct ref_cache *loose;

struct ref_store *packed_ref_store;
};

static void clear_loose_ref_cache(struct files_ref_store *refs)
{
if (refs->loose) {
free_ref_cache(refs->loose);
refs->loose = NULL;
}
}





static struct ref_store *files_ref_store_create(const char *gitdir,
unsigned int flags)
{
struct files_ref_store *refs = xcalloc(1, sizeof(*refs));
struct ref_store *ref_store = (struct ref_store *)refs;
struct strbuf sb = STRBUF_INIT;

base_ref_store_init(ref_store, &refs_be_files);
refs->store_flags = flags;

refs->gitdir = xstrdup(gitdir);
get_common_dir_noenv(&sb, gitdir);
refs->gitcommondir = strbuf_detach(&sb, NULL);
strbuf_addf(&sb, "%s/packed-refs", refs->gitcommondir);
refs->packed_ref_store = packed_ref_store_create(sb.buf, flags);
strbuf_release(&sb);

chdir_notify_reparent("files-backend $GIT_DIR",
&refs->gitdir);
chdir_notify_reparent("files-backend $GIT_COMMONDIR",
&refs->gitcommondir);

return ref_store;
}





static void files_assert_main_repository(struct files_ref_store *refs,
const char *caller)
{
if (refs->store_flags & REF_STORE_MAIN)
return;

BUG("operation %s only allowed for main ref store", caller);
}







static struct files_ref_store *files_downcast(struct ref_store *ref_store,
unsigned int required_flags,
const char *caller)
{
struct files_ref_store *refs;

if (ref_store->be != &refs_be_files)
BUG("ref_store is type \"%s\" not \"files\" in %s",
ref_store->be->name, caller);

refs = (struct files_ref_store *)ref_store;

if ((refs->store_flags & required_flags) != required_flags)
BUG("operation %s requires abilities 0x%x, but only have 0x%x",
caller, required_flags, refs->store_flags);

return refs;
}

static void files_reflog_path_other_worktrees(struct files_ref_store *refs,
struct strbuf *sb,
const char *refname)
{
const char *real_ref;
const char *worktree_name;
int length;

if (parse_worktree_ref(refname, &worktree_name, &length, &real_ref))
BUG("refname %s is not a other-worktree ref", refname);

if (worktree_name)
strbuf_addf(sb, "%s/worktrees/%.*s/logs/%s", refs->gitcommondir,
length, worktree_name, real_ref);
else
strbuf_addf(sb, "%s/logs/%s", refs->gitcommondir,
real_ref);
}

static void files_reflog_path(struct files_ref_store *refs,
struct strbuf *sb,
const char *refname)
{
switch (ref_type(refname)) {
case REF_TYPE_PER_WORKTREE:
case REF_TYPE_PSEUDOREF:
strbuf_addf(sb, "%s/logs/%s", refs->gitdir, refname);
break;
case REF_TYPE_OTHER_PSEUDOREF:
case REF_TYPE_MAIN_PSEUDOREF:
files_reflog_path_other_worktrees(refs, sb, refname);
break;
case REF_TYPE_NORMAL:
strbuf_addf(sb, "%s/logs/%s", refs->gitcommondir, refname);
break;
default:
BUG("unknown ref type %d of ref %s",
ref_type(refname), refname);
}
}

static void files_ref_path(struct files_ref_store *refs,
struct strbuf *sb,
const char *refname)
{
switch (ref_type(refname)) {
case REF_TYPE_PER_WORKTREE:
case REF_TYPE_PSEUDOREF:
strbuf_addf(sb, "%s/%s", refs->gitdir, refname);
break;
case REF_TYPE_MAIN_PSEUDOREF:
if (!skip_prefix(refname, "main-worktree/", &refname))
BUG("ref %s is not a main pseudoref", refname);

case REF_TYPE_OTHER_PSEUDOREF:
case REF_TYPE_NORMAL:
strbuf_addf(sb, "%s/%s", refs->gitcommondir, refname);
break;
default:
BUG("unknown ref type %d of ref %s",
ref_type(refname), refname);
}
}






static void add_per_worktree_entries_to_dir(struct ref_dir *dir, const char *dirname)
{
const char *prefixes[] = { "refs/bisect/", "refs/worktree/", "refs/rewritten/" };
int ip;

if (strcmp(dirname, "refs/"))
return;

for (ip = 0; ip < ARRAY_SIZE(prefixes); ip++) {
const char *prefix = prefixes[ip];
int prefix_len = strlen(prefix);
struct ref_entry *child_entry;
int pos;

pos = search_ref_dir(dir, prefix, prefix_len);
if (pos >= 0)
continue;
child_entry = create_dir_entry(dir->cache, prefix, prefix_len, 1);
add_entry_to_dir(dir, child_entry);
}
}






static void loose_fill_ref_dir(struct ref_store *ref_store,
struct ref_dir *dir, const char *dirname)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_READ, "fill_ref_dir");
DIR *d;
struct dirent *de;
int dirnamelen = strlen(dirname);
struct strbuf refname;
struct strbuf path = STRBUF_INIT;
size_t path_baselen;

files_ref_path(refs, &path, dirname);
path_baselen = path.len;

d = opendir(path.buf);
if (!d) {
strbuf_release(&path);
return;
}

strbuf_init(&refname, dirnamelen + 257);
strbuf_add(&refname, dirname, dirnamelen);

while ((de = readdir(d)) != NULL) {
struct object_id oid;
struct stat st;
int flag;

if (de->d_name[0] == '.')
continue;
if (ends_with(de->d_name, ".lock"))
continue;
strbuf_addstr(&refname, de->d_name);
strbuf_addstr(&path, de->d_name);
if (stat(path.buf, &st) < 0) {
; 
} else if (S_ISDIR(st.st_mode)) {
strbuf_addch(&refname, '/');
add_entry_to_dir(dir,
create_dir_entry(dir->cache, refname.buf,
refname.len, 1));
} else {
if (!refs_resolve_ref_unsafe(&refs->base,
refname.buf,
RESOLVE_REF_READING,
&oid, &flag)) {
oidclr(&oid);
flag |= REF_ISBROKEN;
} else if (is_null_oid(&oid)) {








flag |= REF_ISBROKEN;
}

if (check_refname_format(refname.buf,
REFNAME_ALLOW_ONELEVEL)) {
if (!refname_is_safe(refname.buf))
die("loose refname is dangerous: %s", refname.buf);
oidclr(&oid);
flag |= REF_BAD_NAME | REF_ISBROKEN;
}
add_entry_to_dir(dir,
create_ref_entry(refname.buf, &oid, flag));
}
strbuf_setlen(&refname, dirnamelen);
strbuf_setlen(&path, path_baselen);
}
strbuf_release(&refname);
strbuf_release(&path);
closedir(d);

add_per_worktree_entries_to_dir(dir, dirname);
}

static struct ref_cache *get_loose_ref_cache(struct files_ref_store *refs)
{
if (!refs->loose) {





refs->loose = create_ref_cache(&refs->base, loose_fill_ref_dir);


refs->loose->root->flag &= ~REF_INCOMPLETE;





add_entry_to_dir(get_ref_dir(refs->loose->root),
create_dir_entry(refs->loose, "refs/", 5, 1));
}
return refs->loose;
}

static int files_read_raw_ref(struct ref_store *ref_store,
const char *refname, struct object_id *oid,
struct strbuf *referent, unsigned int *type)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_READ, "read_raw_ref");
struct strbuf sb_contents = STRBUF_INIT;
struct strbuf sb_path = STRBUF_INIT;
const char *path;
const char *buf;
const char *p;
struct stat st;
int fd;
int ret = -1;
int save_errno;
int remaining_retries = 3;

*type = 0;
strbuf_reset(&sb_path);

files_ref_path(refs, &sb_path, refname);

path = sb_path.buf;

stat_ref:













if (remaining_retries-- <= 0)
goto out;

if (lstat(path, &st) < 0) {
if (errno != ENOENT)
goto out;
if (refs_read_raw_ref(refs->packed_ref_store, refname,
oid, referent, type)) {
errno = ENOENT;
goto out;
}
ret = 0;
goto out;
}


if (S_ISLNK(st.st_mode)) {
strbuf_reset(&sb_contents);
if (strbuf_readlink(&sb_contents, path, st.st_size) < 0) {
if (errno == ENOENT || errno == EINVAL)

goto stat_ref;
else
goto out;
}
if (starts_with(sb_contents.buf, "refs/") &&
!check_refname_format(sb_contents.buf, 0)) {
strbuf_swap(&sb_contents, referent);
*type |= REF_ISSYMREF;
ret = 0;
goto out;
}





}


if (S_ISDIR(st.st_mode)) {





if (refs_read_raw_ref(refs->packed_ref_store, refname,
oid, referent, type)) {
errno = EISDIR;
goto out;
}
ret = 0;
goto out;
}





fd = open(path, O_RDONLY);
if (fd < 0) {
if (errno == ENOENT && !S_ISLNK(st.st_mode))

goto stat_ref;
else
goto out;
}
strbuf_reset(&sb_contents);
if (strbuf_read(&sb_contents, fd, 256) < 0) {
int save_errno = errno;
close(fd);
errno = save_errno;
goto out;
}
close(fd);
strbuf_rtrim(&sb_contents);
buf = sb_contents.buf;
if (skip_prefix(buf, "ref:", &buf)) {
while (isspace(*buf))
buf++;

strbuf_reset(referent);
strbuf_addstr(referent, buf);
*type |= REF_ISSYMREF;
ret = 0;
goto out;
}





if (parse_oid_hex(buf, oid, &p) ||
(*p != '\0' && !isspace(*p))) {
*type |= REF_ISBROKEN;
errno = EINVAL;
goto out;
}

ret = 0;

out:
save_errno = errno;
strbuf_release(&sb_path);
strbuf_release(&sb_contents);
errno = save_errno;
return ret;
}

static void unlock_ref(struct ref_lock *lock)
{
rollback_lock_file(&lock->lk);
free(lock->ref_name);
free(lock);
}






























static int lock_raw_ref(struct files_ref_store *refs,
const char *refname, int mustexist,
const struct string_list *extras,
const struct string_list *skip,
struct ref_lock **lock_p,
struct strbuf *referent,
unsigned int *type,
struct strbuf *err)
{
struct ref_lock *lock;
struct strbuf ref_file = STRBUF_INIT;
int attempts_remaining = 3;
int ret = TRANSACTION_GENERIC_ERROR;

assert(err);
files_assert_main_repository(refs, "lock_raw_ref");

*type = 0;



*lock_p = lock = xcalloc(1, sizeof(*lock));

lock->ref_name = xstrdup(refname);
files_ref_path(refs, &ref_file, refname);

retry:
switch (safe_create_leading_directories(ref_file.buf)) {
case SCLD_OK:
break; 
case SCLD_EXISTS:








if (refs_verify_refname_available(&refs->base, refname,
extras, skip, err)) {
if (mustexist) {





strbuf_reset(err);
strbuf_addf(err, "unable to resolve reference '%s'",
refname);
} else {





ret = TRANSACTION_NAME_CONFLICT;
}
} else {





strbuf_addf(err, "unable to create lock file %s.lock; "
"non-directory in the way",
ref_file.buf);
}
goto error_return;
case SCLD_VANISHED:

if (--attempts_remaining > 0)
goto retry;

default:
strbuf_addf(err, "unable to create directory for %s",
ref_file.buf);
goto error_return;
}

if (hold_lock_file_for_update_timeout(
&lock->lk, ref_file.buf, LOCK_NO_DEREF,
get_files_ref_lock_timeout_ms()) < 0) {
if (errno == ENOENT && --attempts_remaining > 0) {





goto retry;
} else {
unable_to_lock_message(ref_file.buf, errno, err);
goto error_return;
}
}






if (files_read_raw_ref(&refs->base, refname,
&lock->old_oid, referent, type)) {
if (errno == ENOENT) {
if (mustexist) {

strbuf_addf(err, "unable to resolve reference '%s'",
refname);
goto error_return;
} else {
















}
} else if (errno == EISDIR) {








if (mustexist) {

strbuf_addf(err, "unable to resolve reference '%s'",
refname);
goto error_return;
} else if (remove_dir_recursively(&ref_file,
REMOVE_DIR_EMPTY_ONLY)) {
if (refs_verify_refname_available(
&refs->base, refname,
extras, skip, err)) {




ret = TRANSACTION_NAME_CONFLICT;
goto error_return;
} else {






strbuf_addf(err, "there is a non-empty directory '%s' "
"blocking reference '%s'",
ref_file.buf, refname);
goto error_return;
}
}
} else if (errno == EINVAL && (*type & REF_ISBROKEN)) {
strbuf_addf(err, "unable to resolve reference '%s': "
"reference broken", refname);
goto error_return;
} else {
strbuf_addf(err, "unable to resolve reference '%s': %s",
refname, strerror(errno));
goto error_return;
}






if (refs_verify_refname_available(
refs->packed_ref_store, refname,
extras, skip, err))
goto error_return;
}

ret = 0;
goto out;

error_return:
unlock_ref(lock);
*lock_p = NULL;

out:
strbuf_release(&ref_file);
return ret;
}

struct files_ref_iterator {
struct ref_iterator base;

struct ref_iterator *iter0;
unsigned int flags;
};

static int files_ref_iterator_advance(struct ref_iterator *ref_iterator)
{
struct files_ref_iterator *iter =
(struct files_ref_iterator *)ref_iterator;
int ok;

while ((ok = ref_iterator_advance(iter->iter0)) == ITER_OK) {
if (iter->flags & DO_FOR_EACH_PER_WORKTREE_ONLY &&
ref_type(iter->iter0->refname) != REF_TYPE_PER_WORKTREE)
continue;

if (!(iter->flags & DO_FOR_EACH_INCLUDE_BROKEN) &&
!ref_resolves_to_object(iter->iter0->refname,
iter->iter0->oid,
iter->iter0->flags))
continue;

iter->base.refname = iter->iter0->refname;
iter->base.oid = iter->iter0->oid;
iter->base.flags = iter->iter0->flags;
return ITER_OK;
}

iter->iter0 = NULL;
if (ref_iterator_abort(ref_iterator) != ITER_DONE)
ok = ITER_ERROR;

return ok;
}

static int files_ref_iterator_peel(struct ref_iterator *ref_iterator,
struct object_id *peeled)
{
struct files_ref_iterator *iter =
(struct files_ref_iterator *)ref_iterator;

return ref_iterator_peel(iter->iter0, peeled);
}

static int files_ref_iterator_abort(struct ref_iterator *ref_iterator)
{
struct files_ref_iterator *iter =
(struct files_ref_iterator *)ref_iterator;
int ok = ITER_DONE;

if (iter->iter0)
ok = ref_iterator_abort(iter->iter0);

base_ref_iterator_free(ref_iterator);
return ok;
}

static struct ref_iterator_vtable files_ref_iterator_vtable = {
files_ref_iterator_advance,
files_ref_iterator_peel,
files_ref_iterator_abort
};

static struct ref_iterator *files_ref_iterator_begin(
struct ref_store *ref_store,
const char *prefix, unsigned int flags)
{
struct files_ref_store *refs;
struct ref_iterator *loose_iter, *packed_iter, *overlay_iter;
struct files_ref_iterator *iter;
struct ref_iterator *ref_iterator;
unsigned int required_flags = REF_STORE_READ;

if (!(flags & DO_FOR_EACH_INCLUDE_BROKEN))
required_flags |= REF_STORE_ODB;

refs = files_downcast(ref_store, required_flags, "ref_iterator_begin");


















loose_iter = cache_ref_iterator_begin(get_loose_ref_cache(refs),
prefix, 1);












packed_iter = refs_ref_iterator_begin(
refs->packed_ref_store, prefix, 0,
DO_FOR_EACH_INCLUDE_BROKEN);

overlay_iter = overlay_ref_iterator_begin(loose_iter, packed_iter);

iter = xcalloc(1, sizeof(*iter));
ref_iterator = &iter->base;
base_ref_iterator_init(ref_iterator, &files_ref_iterator_vtable,
overlay_iter->ordered);
iter->iter0 = overlay_iter;
iter->flags = flags;

return ref_iterator;
}







static int verify_lock(struct ref_store *ref_store, struct ref_lock *lock,
const struct object_id *old_oid, int mustexist,
struct strbuf *err)
{
assert(err);

if (refs_read_ref_full(ref_store, lock->ref_name,
mustexist ? RESOLVE_REF_READING : 0,
&lock->old_oid, NULL)) {
if (old_oid) {
int save_errno = errno;
strbuf_addf(err, "can't verify ref '%s'", lock->ref_name);
errno = save_errno;
return -1;
} else {
oidclr(&lock->old_oid);
return 0;
}
}
if (old_oid && !oideq(&lock->old_oid, old_oid)) {
strbuf_addf(err, "ref '%s' is at %s but expected %s",
lock->ref_name,
oid_to_hex(&lock->old_oid),
oid_to_hex(old_oid));
errno = EBUSY;
return -1;
}
return 0;
}

static int remove_empty_directories(struct strbuf *path)
{





return remove_dir_recursively(path, REMOVE_DIR_EMPTY_ONLY);
}

static int create_reflock(const char *path, void *cb)
{
struct lock_file *lk = cb;

return hold_lock_file_for_update_timeout(
lk, path, LOCK_NO_DEREF,
get_files_ref_lock_timeout_ms()) < 0 ? -1 : 0;
}





static struct ref_lock *lock_ref_oid_basic(struct files_ref_store *refs,
const char *refname,
const struct object_id *old_oid,
const struct string_list *extras,
const struct string_list *skip,
unsigned int flags, int *type,
struct strbuf *err)
{
struct strbuf ref_file = STRBUF_INIT;
struct ref_lock *lock;
int last_errno = 0;
int mustexist = (old_oid && !is_null_oid(old_oid));
int resolve_flags = RESOLVE_REF_NO_RECURSE;
int resolved;

files_assert_main_repository(refs, "lock_ref_oid_basic");
assert(err);

lock = xcalloc(1, sizeof(struct ref_lock));

if (mustexist)
resolve_flags |= RESOLVE_REF_READING;
if (flags & REF_DELETING)
resolve_flags |= RESOLVE_REF_ALLOW_BAD_NAME;

files_ref_path(refs, &ref_file, refname);
resolved = !!refs_resolve_ref_unsafe(&refs->base,
refname, resolve_flags,
&lock->old_oid, type);
if (!resolved && errno == EISDIR) {






if (remove_empty_directories(&ref_file)) {
last_errno = errno;
if (!refs_verify_refname_available(
&refs->base,
refname, extras, skip, err))
strbuf_addf(err, "there are still refs under '%s'",
refname);
goto error_return;
}
resolved = !!refs_resolve_ref_unsafe(&refs->base,
refname, resolve_flags,
&lock->old_oid, type);
}
if (!resolved) {
last_errno = errno;
if (last_errno != ENOTDIR ||
!refs_verify_refname_available(&refs->base, refname,
extras, skip, err))
strbuf_addf(err, "unable to resolve reference '%s': %s",
refname, strerror(last_errno));

goto error_return;
}







if (is_null_oid(&lock->old_oid) &&
refs_verify_refname_available(refs->packed_ref_store, refname,
extras, skip, err)) {
last_errno = ENOTDIR;
goto error_return;
}

lock->ref_name = xstrdup(refname);

if (raceproof_create_file(ref_file.buf, create_reflock, &lock->lk)) {
last_errno = errno;
unable_to_lock_message(ref_file.buf, errno, err);
goto error_return;
}

if (verify_lock(&refs->base, lock, old_oid, mustexist, err)) {
last_errno = errno;
goto error_return;
}
goto out;

error_return:
unlock_ref(lock);
lock = NULL;

out:
strbuf_release(&ref_file);
errno = last_errno;
return lock;
}

struct ref_to_prune {
struct ref_to_prune *next;
struct object_id oid;
char name[FLEX_ARRAY];
};

enum {
REMOVE_EMPTY_PARENTS_REF = 0x01,
REMOVE_EMPTY_PARENTS_REFLOG = 0x02
};







static void try_remove_empty_parents(struct files_ref_store *refs,
const char *refname,
unsigned int flags)
{
struct strbuf buf = STRBUF_INIT;
struct strbuf sb = STRBUF_INIT;
char *p, *q;
int i;

strbuf_addstr(&buf, refname);
p = buf.buf;
for (i = 0; i < 2; i++) { 
while (*p && *p != '/')
p++;

while (*p == '/')
p++;
}
q = buf.buf + buf.len;
while (flags & (REMOVE_EMPTY_PARENTS_REF | REMOVE_EMPTY_PARENTS_REFLOG)) {
while (q > p && *q != '/')
q--;
while (q > p && *(q-1) == '/')
q--;
if (q == p)
break;
strbuf_setlen(&buf, q - buf.buf);

strbuf_reset(&sb);
files_ref_path(refs, &sb, buf.buf);
if ((flags & REMOVE_EMPTY_PARENTS_REF) && rmdir(sb.buf))
flags &= ~REMOVE_EMPTY_PARENTS_REF;

strbuf_reset(&sb);
files_reflog_path(refs, &sb, buf.buf);
if ((flags & REMOVE_EMPTY_PARENTS_REFLOG) && rmdir(sb.buf))
flags &= ~REMOVE_EMPTY_PARENTS_REFLOG;
}
strbuf_release(&buf);
strbuf_release(&sb);
}


static void prune_ref(struct files_ref_store *refs, struct ref_to_prune *r)
{
struct ref_transaction *transaction;
struct strbuf err = STRBUF_INIT;
int ret = -1;

if (check_refname_format(r->name, 0))
return;

transaction = ref_store_transaction_begin(&refs->base, &err);
if (!transaction)
goto cleanup;
ref_transaction_add_update(
transaction, r->name,
REF_NO_DEREF | REF_HAVE_NEW | REF_HAVE_OLD | REF_IS_PRUNING,
&null_oid, &r->oid, NULL);
if (ref_transaction_commit(transaction, &err))
goto cleanup;

ret = 0;

cleanup:
if (ret)
error("%s", err.buf);
strbuf_release(&err);
ref_transaction_free(transaction);
return;
}





static void prune_refs(struct files_ref_store *refs, struct ref_to_prune **refs_to_prune)
{
while (*refs_to_prune) {
struct ref_to_prune *r = *refs_to_prune;
*refs_to_prune = r->next;
prune_ref(refs, r);
free(r);
}
}




static int should_pack_ref(const char *refname,
const struct object_id *oid, unsigned int ref_flags,
unsigned int pack_flags)
{

if (ref_type(refname) != REF_TYPE_NORMAL)
return 0;


if (!(pack_flags & PACK_REFS_ALL) && !starts_with(refname, "refs/tags/"))
return 0;


if (ref_flags & REF_ISSYMREF)
return 0;


if (!ref_resolves_to_object(refname, oid, ref_flags))
return 0;

return 1;
}

static int files_pack_refs(struct ref_store *ref_store, unsigned int flags)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE | REF_STORE_ODB,
"pack_refs");
struct ref_iterator *iter;
int ok;
struct ref_to_prune *refs_to_prune = NULL;
struct strbuf err = STRBUF_INIT;
struct ref_transaction *transaction;

transaction = ref_store_transaction_begin(refs->packed_ref_store, &err);
if (!transaction)
return -1;

packed_refs_lock(refs->packed_ref_store, LOCK_DIE_ON_ERROR, &err);

iter = cache_ref_iterator_begin(get_loose_ref_cache(refs), NULL, 0);
while ((ok = ref_iterator_advance(iter)) == ITER_OK) {





if (!should_pack_ref(iter->refname, iter->oid, iter->flags,
flags))
continue;





if (ref_transaction_update(transaction, iter->refname,
iter->oid, NULL,
REF_NO_DEREF, NULL, &err))
die("failure preparing to create packed reference %s: %s",
iter->refname, err.buf);


if ((flags & PACK_REFS_PRUNE)) {
struct ref_to_prune *n;
FLEX_ALLOC_STR(n, name, iter->refname);
oidcpy(&n->oid, iter->oid);
n->next = refs_to_prune;
refs_to_prune = n;
}
}
if (ok != ITER_DONE)
die("error while iterating over references");

if (ref_transaction_commit(transaction, &err))
die("unable to write new packed-refs: %s", err.buf);

ref_transaction_free(transaction);

packed_refs_unlock(refs->packed_ref_store);

prune_refs(refs, &refs_to_prune);
strbuf_release(&err);
return 0;
}

static int files_delete_refs(struct ref_store *ref_store, const char *msg,
struct string_list *refnames, unsigned int flags)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE, "delete_refs");
struct strbuf err = STRBUF_INIT;
int i, result = 0;

if (!refnames->nr)
return 0;

if (packed_refs_lock(refs->packed_ref_store, 0, &err))
goto error;

if (refs_delete_refs(refs->packed_ref_store, msg, refnames, flags)) {
packed_refs_unlock(refs->packed_ref_store);
goto error;
}

packed_refs_unlock(refs->packed_ref_store);

for (i = 0; i < refnames->nr; i++) {
const char *refname = refnames->items[i].string;

if (refs_delete_ref(&refs->base, msg, refname, NULL, flags))
result |= error(_("could not remove reference %s"), refname);
}

strbuf_release(&err);
return result;

error:






if (refnames->nr == 1)
error(_("could not delete reference %s: %s"),
refnames->items[0].string, err.buf);
else
error(_("could not delete references: %s"), err.buf);

strbuf_release(&err);
return -1;
}








#define TMP_RENAMED_LOG "refs/.tmp-renamed-log"

struct rename_cb {
const char *tmp_renamed_log;
int true_errno;
};

static int rename_tmp_log_callback(const char *path, void *cb_data)
{
struct rename_cb *cb = cb_data;

if (rename(cb->tmp_renamed_log, path)) {







cb->true_errno = errno;
if (errno == ENOTDIR)
errno = EISDIR;
return -1;
} else {
return 0;
}
}

static int rename_tmp_log(struct files_ref_store *refs, const char *newrefname)
{
struct strbuf path = STRBUF_INIT;
struct strbuf tmp = STRBUF_INIT;
struct rename_cb cb;
int ret;

files_reflog_path(refs, &path, newrefname);
files_reflog_path(refs, &tmp, TMP_RENAMED_LOG);
cb.tmp_renamed_log = tmp.buf;
ret = raceproof_create_file(path.buf, rename_tmp_log_callback, &cb);
if (ret) {
if (errno == EISDIR)
error("directory not empty: %s", path.buf);
else
error("unable to move logfile %s to %s: %s",
tmp.buf, path.buf,
strerror(cb.true_errno));
}

strbuf_release(&path);
strbuf_release(&tmp);
return ret;
}

static int write_ref_to_lockfile(struct ref_lock *lock,
const struct object_id *oid, struct strbuf *err);
static int commit_ref_update(struct files_ref_store *refs,
struct ref_lock *lock,
const struct object_id *oid, const char *logmsg,
struct strbuf *err);

static int files_copy_or_rename_ref(struct ref_store *ref_store,
const char *oldrefname, const char *newrefname,
const char *logmsg, int copy)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE, "rename_ref");
struct object_id orig_oid;
int flag = 0, logmoved = 0;
struct ref_lock *lock;
struct stat loginfo;
struct strbuf sb_oldref = STRBUF_INIT;
struct strbuf sb_newref = STRBUF_INIT;
struct strbuf tmp_renamed_log = STRBUF_INIT;
int log, ret;
struct strbuf err = STRBUF_INIT;

files_reflog_path(refs, &sb_oldref, oldrefname);
files_reflog_path(refs, &sb_newref, newrefname);
files_reflog_path(refs, &tmp_renamed_log, TMP_RENAMED_LOG);

log = !lstat(sb_oldref.buf, &loginfo);
if (log && S_ISLNK(loginfo.st_mode)) {
ret = error("reflog for %s is a symlink", oldrefname);
goto out;
}

if (!refs_resolve_ref_unsafe(&refs->base, oldrefname,
RESOLVE_REF_READING | RESOLVE_REF_NO_RECURSE,
&orig_oid, &flag)) {
ret = error("refname %s not found", oldrefname);
goto out;
}

if (flag & REF_ISSYMREF) {
if (copy)
ret = error("refname %s is a symbolic ref, copying it is not supported",
oldrefname);
else
ret = error("refname %s is a symbolic ref, renaming it is not supported",
oldrefname);
goto out;
}
if (!refs_rename_ref_available(&refs->base, oldrefname, newrefname)) {
ret = 1;
goto out;
}

if (!copy && log && rename(sb_oldref.buf, tmp_renamed_log.buf)) {
ret = error("unable to move logfile logs/%s to logs/"TMP_RENAMED_LOG": %s",
oldrefname, strerror(errno));
goto out;
}

if (copy && log && copy_file(tmp_renamed_log.buf, sb_oldref.buf, 0644)) {
ret = error("unable to copy logfile logs/%s to logs/"TMP_RENAMED_LOG": %s",
oldrefname, strerror(errno));
goto out;
}

if (!copy && refs_delete_ref(&refs->base, logmsg, oldrefname,
&orig_oid, REF_NO_DEREF)) {
error("unable to delete old %s", oldrefname);
goto rollback;
}








if (!copy && !refs_read_ref_full(&refs->base, newrefname,
RESOLVE_REF_READING | RESOLVE_REF_NO_RECURSE,
NULL, NULL) &&
refs_delete_ref(&refs->base, NULL, newrefname,
NULL, REF_NO_DEREF)) {
if (errno == EISDIR) {
struct strbuf path = STRBUF_INIT;
int result;

files_ref_path(refs, &path, newrefname);
result = remove_empty_directories(&path);
strbuf_release(&path);

if (result) {
error("Directory not empty: %s", newrefname);
goto rollback;
}
} else {
error("unable to delete existing %s", newrefname);
goto rollback;
}
}

if (log && rename_tmp_log(refs, newrefname))
goto rollback;

logmoved = log;

lock = lock_ref_oid_basic(refs, newrefname, NULL, NULL, NULL,
REF_NO_DEREF, NULL, &err);
if (!lock) {
if (copy)
error("unable to copy '%s' to '%s': %s", oldrefname, newrefname, err.buf);
else
error("unable to rename '%s' to '%s': %s", oldrefname, newrefname, err.buf);
strbuf_release(&err);
goto rollback;
}
oidcpy(&lock->old_oid, &orig_oid);

if (write_ref_to_lockfile(lock, &orig_oid, &err) ||
commit_ref_update(refs, lock, &orig_oid, logmsg, &err)) {
error("unable to write current sha1 into %s: %s", newrefname, err.buf);
strbuf_release(&err);
goto rollback;
}

ret = 0;
goto out;

rollback:
lock = lock_ref_oid_basic(refs, oldrefname, NULL, NULL, NULL,
REF_NO_DEREF, NULL, &err);
if (!lock) {
error("unable to lock %s for rollback: %s", oldrefname, err.buf);
strbuf_release(&err);
goto rollbacklog;
}

flag = log_all_ref_updates;
log_all_ref_updates = LOG_REFS_NONE;
if (write_ref_to_lockfile(lock, &orig_oid, &err) ||
commit_ref_update(refs, lock, &orig_oid, NULL, &err)) {
error("unable to write current sha1 into %s: %s", oldrefname, err.buf);
strbuf_release(&err);
}
log_all_ref_updates = flag;

rollbacklog:
if (logmoved && rename(sb_newref.buf, sb_oldref.buf))
error("unable to restore logfile %s from %s: %s",
oldrefname, newrefname, strerror(errno));
if (!logmoved && log &&
rename(tmp_renamed_log.buf, sb_oldref.buf))
error("unable to restore logfile %s from logs/"TMP_RENAMED_LOG": %s",
oldrefname, strerror(errno));
ret = 1;
out:
strbuf_release(&sb_newref);
strbuf_release(&sb_oldref);
strbuf_release(&tmp_renamed_log);

return ret;
}

static int files_rename_ref(struct ref_store *ref_store,
const char *oldrefname, const char *newrefname,
const char *logmsg)
{
return files_copy_or_rename_ref(ref_store, oldrefname,
newrefname, logmsg, 0);
}

static int files_copy_ref(struct ref_store *ref_store,
const char *oldrefname, const char *newrefname,
const char *logmsg)
{
return files_copy_or_rename_ref(ref_store, oldrefname,
newrefname, logmsg, 1);
}

static int close_ref_gently(struct ref_lock *lock)
{
if (close_lock_file_gently(&lock->lk))
return -1;
return 0;
}

static int commit_ref(struct ref_lock *lock)
{
char *path = get_locked_file_path(&lock->lk);
struct stat st;

if (!lstat(path, &st) && S_ISDIR(st.st_mode)) {





size_t len = strlen(path);
struct strbuf sb_path = STRBUF_INIT;

strbuf_attach(&sb_path, path, len, len);





remove_empty_directories(&sb_path);
strbuf_release(&sb_path);
} else {
free(path);
}

if (commit_lock_file(&lock->lk))
return -1;
return 0;
}

static int open_or_create_logfile(const char *path, void *cb)
{
int *fd = cb;

*fd = open(path, O_APPEND | O_WRONLY | O_CREAT, 0666);
return (*fd < 0) ? -1 : 0;
}











static int log_ref_setup(struct files_ref_store *refs,
const char *refname, int force_create,
int *logfd, struct strbuf *err)
{
struct strbuf logfile_sb = STRBUF_INIT;
char *logfile;

files_reflog_path(refs, &logfile_sb, refname);
logfile = strbuf_detach(&logfile_sb, NULL);

if (force_create || should_autocreate_reflog(refname)) {
if (raceproof_create_file(logfile, open_or_create_logfile, logfd)) {
if (errno == ENOENT)
strbuf_addf(err, "unable to create directory for '%s': "
"%s", logfile, strerror(errno));
else if (errno == EISDIR)
strbuf_addf(err, "there are still logs under '%s'",
logfile);
else
strbuf_addf(err, "unable to append to '%s': %s",
logfile, strerror(errno));

goto error;
}
} else {
*logfd = open(logfile, O_APPEND | O_WRONLY, 0666);
if (*logfd < 0) {
if (errno == ENOENT || errno == EISDIR) {






;
} else {
strbuf_addf(err, "unable to append to '%s': %s",
logfile, strerror(errno));
goto error;
}
}
}

if (*logfd >= 0)
adjust_shared_perm(logfile);

free(logfile);
return 0;

error:
free(logfile);
return -1;
}

static int files_create_reflog(struct ref_store *ref_store,
const char *refname, int force_create,
struct strbuf *err)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE, "create_reflog");
int fd;

if (log_ref_setup(refs, refname, force_create, &fd, err))
return -1;

if (fd >= 0)
close(fd);

return 0;
}

static int log_ref_write_fd(int fd, const struct object_id *old_oid,
const struct object_id *new_oid,
const char *committer, const char *msg)
{
struct strbuf sb = STRBUF_INIT;
int ret = 0;

strbuf_addf(&sb, "%s %s %s", oid_to_hex(old_oid), oid_to_hex(new_oid), committer);
if (msg && *msg)
copy_reflog_msg(&sb, msg);
strbuf_addch(&sb, '\n');
if (write_in_full(fd, sb.buf, sb.len) < 0)
ret = -1;
strbuf_release(&sb);
return ret;
}

static int files_log_ref_write(struct files_ref_store *refs,
const char *refname, const struct object_id *old_oid,
const struct object_id *new_oid, const char *msg,
int flags, struct strbuf *err)
{
int logfd, result;

if (log_all_ref_updates == LOG_REFS_UNSET)
log_all_ref_updates = is_bare_repository() ? LOG_REFS_NONE : LOG_REFS_NORMAL;

result = log_ref_setup(refs, refname,
flags & REF_FORCE_CREATE_REFLOG,
&logfd, err);

if (result)
return result;

if (logfd < 0)
return 0;
result = log_ref_write_fd(logfd, old_oid, new_oid,
git_committer_info(0), msg);
if (result) {
struct strbuf sb = STRBUF_INIT;
int save_errno = errno;

files_reflog_path(refs, &sb, refname);
strbuf_addf(err, "unable to append to '%s': %s",
sb.buf, strerror(save_errno));
strbuf_release(&sb);
close(logfd);
return -1;
}
if (close(logfd)) {
struct strbuf sb = STRBUF_INIT;
int save_errno = errno;

files_reflog_path(refs, &sb, refname);
strbuf_addf(err, "unable to append to '%s': %s",
sb.buf, strerror(save_errno));
strbuf_release(&sb);
return -1;
}
return 0;
}





static int write_ref_to_lockfile(struct ref_lock *lock,
const struct object_id *oid, struct strbuf *err)
{
static char term = '\n';
struct object *o;
int fd;

o = parse_object(the_repository, oid);
if (!o) {
strbuf_addf(err,
"trying to write ref '%s' with nonexistent object %s",
lock->ref_name, oid_to_hex(oid));
unlock_ref(lock);
return -1;
}
if (o->type != OBJ_COMMIT && is_branch(lock->ref_name)) {
strbuf_addf(err,
"trying to write non-commit object %s to branch '%s'",
oid_to_hex(oid), lock->ref_name);
unlock_ref(lock);
return -1;
}
fd = get_lock_file_fd(&lock->lk);
if (write_in_full(fd, oid_to_hex(oid), the_hash_algo->hexsz) < 0 ||
write_in_full(fd, &term, 1) < 0 ||
close_ref_gently(lock) < 0) {
strbuf_addf(err,
"couldn't write '%s'", get_lock_file_path(&lock->lk));
unlock_ref(lock);
return -1;
}
return 0;
}






static int commit_ref_update(struct files_ref_store *refs,
struct ref_lock *lock,
const struct object_id *oid, const char *logmsg,
struct strbuf *err)
{
files_assert_main_repository(refs, "commit_ref_update");

clear_loose_ref_cache(refs);
if (files_log_ref_write(refs, lock->ref_name,
&lock->old_oid, oid,
logmsg, 0, err)) {
char *old_msg = strbuf_detach(err, NULL);
strbuf_addf(err, "cannot update the ref '%s': %s",
lock->ref_name, old_msg);
free(old_msg);
unlock_ref(lock);
return -1;
}

if (strcmp(lock->ref_name, "HEAD") != 0) {












int head_flag;
const char *head_ref;

head_ref = refs_resolve_ref_unsafe(&refs->base, "HEAD",
RESOLVE_REF_READING,
NULL, &head_flag);
if (head_ref && (head_flag & REF_ISSYMREF) &&
!strcmp(head_ref, lock->ref_name)) {
struct strbuf log_err = STRBUF_INIT;
if (files_log_ref_write(refs, "HEAD",
&lock->old_oid, oid,
logmsg, 0, &log_err)) {
error("%s", log_err.buf);
strbuf_release(&log_err);
}
}
}

if (commit_ref(lock)) {
strbuf_addf(err, "couldn't set '%s'", lock->ref_name);
unlock_ref(lock);
return -1;
}

unlock_ref(lock);
return 0;
}

static int create_ref_symlink(struct ref_lock *lock, const char *target)
{
int ret = -1;
#if !defined(NO_SYMLINK_HEAD)
char *ref_path = get_locked_file_path(&lock->lk);
unlink(ref_path);
ret = symlink(target, ref_path);
free(ref_path);

if (ret)
fprintf(stderr, "no symlink - falling back to symbolic ref\n");
#endif
return ret;
}

static void update_symref_reflog(struct files_ref_store *refs,
struct ref_lock *lock, const char *refname,
const char *target, const char *logmsg)
{
struct strbuf err = STRBUF_INIT;
struct object_id new_oid;
if (logmsg &&
!refs_read_ref_full(&refs->base, target,
RESOLVE_REF_READING, &new_oid, NULL) &&
files_log_ref_write(refs, refname, &lock->old_oid,
&new_oid, logmsg, 0, &err)) {
error("%s", err.buf);
strbuf_release(&err);
}
}

static int create_symref_locked(struct files_ref_store *refs,
struct ref_lock *lock, const char *refname,
const char *target, const char *logmsg)
{
if (prefer_symlink_refs && !create_ref_symlink(lock, target)) {
update_symref_reflog(refs, lock, refname, target, logmsg);
return 0;
}

if (!fdopen_lock_file(&lock->lk, "w"))
return error("unable to fdopen %s: %s",
lock->lk.tempfile->filename.buf, strerror(errno));

update_symref_reflog(refs, lock, refname, target, logmsg);


fprintf(lock->lk.tempfile->fp, "ref: %s\n", target);
if (commit_ref(lock) < 0)
return error("unable to write symref for %s: %s", refname,
strerror(errno));
return 0;
}

static int files_create_symref(struct ref_store *ref_store,
const char *refname, const char *target,
const char *logmsg)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE, "create_symref");
struct strbuf err = STRBUF_INIT;
struct ref_lock *lock;
int ret;

lock = lock_ref_oid_basic(refs, refname, NULL,
NULL, NULL, REF_NO_DEREF, NULL,
&err);
if (!lock) {
error("%s", err.buf);
strbuf_release(&err);
return -1;
}

ret = create_symref_locked(refs, lock, refname, target, logmsg);
unlock_ref(lock);
return ret;
}

static int files_reflog_exists(struct ref_store *ref_store,
const char *refname)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_READ, "reflog_exists");
struct strbuf sb = STRBUF_INIT;
struct stat st;
int ret;

files_reflog_path(refs, &sb, refname);
ret = !lstat(sb.buf, &st) && S_ISREG(st.st_mode);
strbuf_release(&sb);
return ret;
}

static int files_delete_reflog(struct ref_store *ref_store,
const char *refname)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE, "delete_reflog");
struct strbuf sb = STRBUF_INIT;
int ret;

files_reflog_path(refs, &sb, refname);
ret = remove_path(sb.buf);
strbuf_release(&sb);
return ret;
}

static int show_one_reflog_ent(struct strbuf *sb, each_reflog_ent_fn fn, void *cb_data)
{
struct object_id ooid, noid;
char *email_end, *message;
timestamp_t timestamp;
int tz;
const char *p = sb->buf;


if (!sb->len || sb->buf[sb->len - 1] != '\n' ||
parse_oid_hex(p, &ooid, &p) || *p++ != ' ' ||
parse_oid_hex(p, &noid, &p) || *p++ != ' ' ||
!(email_end = strchr(p, '>')) ||
email_end[1] != ' ' ||
!(timestamp = parse_timestamp(email_end + 2, &message, 10)) ||
!message || message[0] != ' ' ||
(message[1] != '+' && message[1] != '-') ||
!isdigit(message[2]) || !isdigit(message[3]) ||
!isdigit(message[4]) || !isdigit(message[5]))
return 0; 
email_end[1] = '\0';
tz = strtol(message + 1, NULL, 10);
if (message[6] != '\t')
message += 6;
else
message += 7;
return fn(&ooid, &noid, p, timestamp, tz, message, cb_data);
}

static char *find_beginning_of_line(char *bob, char *scan)
{
while (bob < scan && *(--scan) != '\n')
; 




return scan;
}

static int files_for_each_reflog_ent_reverse(struct ref_store *ref_store,
const char *refname,
each_reflog_ent_fn fn,
void *cb_data)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_READ,
"for_each_reflog_ent_reverse");
struct strbuf sb = STRBUF_INIT;
FILE *logfp;
long pos;
int ret = 0, at_tail = 1;

files_reflog_path(refs, &sb, refname);
logfp = fopen(sb.buf, "r");
strbuf_release(&sb);
if (!logfp)
return -1;


if (fseek(logfp, 0, SEEK_END) < 0)
ret = error("cannot seek back reflog for %s: %s",
refname, strerror(errno));
pos = ftell(logfp);
while (!ret && 0 < pos) {
int cnt;
size_t nread;
char buf[BUFSIZ];
char *endp, *scanp;


cnt = (sizeof(buf) < pos) ? sizeof(buf) : pos;
if (fseek(logfp, pos - cnt, SEEK_SET)) {
ret = error("cannot seek back reflog for %s: %s",
refname, strerror(errno));
break;
}
nread = fread(buf, cnt, 1, logfp);
if (nread != 1) {
ret = error("cannot read %d bytes from reflog for %s: %s",
cnt, refname, strerror(errno));
break;
}
pos -= cnt;

scanp = endp = buf + cnt;
if (at_tail && scanp[-1] == '\n')

scanp--;
at_tail = 0;

while (buf < scanp) {




char *bp;

bp = find_beginning_of_line(buf, scanp);

if (*bp == '\n') {






strbuf_splice(&sb, 0, 0, bp + 1, endp - (bp + 1));
scanp = bp;
endp = bp + 1;
ret = show_one_reflog_ent(&sb, fn, cb_data);
strbuf_reset(&sb);
if (ret)
break;
} else if (!pos) {






strbuf_splice(&sb, 0, 0, buf, endp - buf);
ret = show_one_reflog_ent(&sb, fn, cb_data);
strbuf_reset(&sb);
break;
}

if (bp == buf) {












strbuf_splice(&sb, 0, 0, buf, endp - buf);
break;
}
}

}
if (!ret && sb.len)
BUG("reverse reflog parser had leftover data");

fclose(logfp);
strbuf_release(&sb);
return ret;
}

static int files_for_each_reflog_ent(struct ref_store *ref_store,
const char *refname,
each_reflog_ent_fn fn, void *cb_data)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_READ,
"for_each_reflog_ent");
FILE *logfp;
struct strbuf sb = STRBUF_INIT;
int ret = 0;

files_reflog_path(refs, &sb, refname);
logfp = fopen(sb.buf, "r");
strbuf_release(&sb);
if (!logfp)
return -1;

while (!ret && !strbuf_getwholeline(&sb, logfp, '\n'))
ret = show_one_reflog_ent(&sb, fn, cb_data);
fclose(logfp);
strbuf_release(&sb);
return ret;
}

struct files_reflog_iterator {
struct ref_iterator base;

struct ref_store *ref_store;
struct dir_iterator *dir_iterator;
struct object_id oid;
};

static int files_reflog_iterator_advance(struct ref_iterator *ref_iterator)
{
struct files_reflog_iterator *iter =
(struct files_reflog_iterator *)ref_iterator;
struct dir_iterator *diter = iter->dir_iterator;
int ok;

while ((ok = dir_iterator_advance(diter)) == ITER_OK) {
int flags;

if (!S_ISREG(diter->st.st_mode))
continue;
if (diter->basename[0] == '.')
continue;
if (ends_with(diter->basename, ".lock"))
continue;

if (refs_read_ref_full(iter->ref_store,
diter->relative_path, 0,
&iter->oid, &flags)) {
error("bad ref for %s", diter->path.buf);
continue;
}

iter->base.refname = diter->relative_path;
iter->base.oid = &iter->oid;
iter->base.flags = flags;
return ITER_OK;
}

iter->dir_iterator = NULL;
if (ref_iterator_abort(ref_iterator) == ITER_ERROR)
ok = ITER_ERROR;
return ok;
}

static int files_reflog_iterator_peel(struct ref_iterator *ref_iterator,
struct object_id *peeled)
{
BUG("ref_iterator_peel() called for reflog_iterator");
}

static int files_reflog_iterator_abort(struct ref_iterator *ref_iterator)
{
struct files_reflog_iterator *iter =
(struct files_reflog_iterator *)ref_iterator;
int ok = ITER_DONE;

if (iter->dir_iterator)
ok = dir_iterator_abort(iter->dir_iterator);

base_ref_iterator_free(ref_iterator);
return ok;
}

static struct ref_iterator_vtable files_reflog_iterator_vtable = {
files_reflog_iterator_advance,
files_reflog_iterator_peel,
files_reflog_iterator_abort
};

static struct ref_iterator *reflog_iterator_begin(struct ref_store *ref_store,
const char *gitdir)
{
struct dir_iterator *diter;
struct files_reflog_iterator *iter;
struct ref_iterator *ref_iterator;
struct strbuf sb = STRBUF_INIT;

strbuf_addf(&sb, "%s/logs", gitdir);

diter = dir_iterator_begin(sb.buf, 0);
if (!diter) {
strbuf_release(&sb);
return empty_ref_iterator_begin();
}

iter = xcalloc(1, sizeof(*iter));
ref_iterator = &iter->base;

base_ref_iterator_init(ref_iterator, &files_reflog_iterator_vtable, 0);
iter->dir_iterator = diter;
iter->ref_store = ref_store;
strbuf_release(&sb);

return ref_iterator;
}

static enum iterator_selection reflog_iterator_select(
struct ref_iterator *iter_worktree,
struct ref_iterator *iter_common,
void *cb_data)
{
if (iter_worktree) {





return ITER_SELECT_0;
} else if (iter_common) {
if (ref_type(iter_common->refname) == REF_TYPE_NORMAL)
return ITER_SELECT_1;





return ITER_SKIP_1;
} else
return ITER_DONE;
}

static struct ref_iterator *files_reflog_iterator_begin(struct ref_store *ref_store)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_READ,
"reflog_iterator_begin");

if (!strcmp(refs->gitdir, refs->gitcommondir)) {
return reflog_iterator_begin(ref_store, refs->gitcommondir);
} else {
return merge_ref_iterator_begin(
0,
reflog_iterator_begin(ref_store, refs->gitdir),
reflog_iterator_begin(ref_store, refs->gitcommondir),
reflog_iterator_select, refs);
}
}





static int split_head_update(struct ref_update *update,
struct ref_transaction *transaction,
const char *head_ref,
struct string_list *affected_refnames,
struct strbuf *err)
{
struct string_list_item *item;
struct ref_update *new_update;

if ((update->flags & REF_LOG_ONLY) ||
(update->flags & REF_IS_PRUNING) ||
(update->flags & REF_UPDATE_VIA_HEAD))
return 0;

if (strcmp(update->refname, head_ref))
return 0;






if (string_list_has_string(affected_refnames, "HEAD")) {

strbuf_addf(err,
"multiple updates for 'HEAD' (including one "
"via its referent '%s') are not allowed",
update->refname);
return TRANSACTION_NAME_CONFLICT;
}

new_update = ref_transaction_add_update(
transaction, "HEAD",
update->flags | REF_LOG_ONLY | REF_NO_DEREF,
&update->new_oid, &update->old_oid,
update->msg);






if (strcmp(new_update->refname, "HEAD"))
BUG("%s unexpectedly not 'HEAD'", new_update->refname);
item = string_list_insert(affected_refnames, new_update->refname);
item->util = new_update;

return 0;
}









static int split_symref_update(struct ref_update *update,
const char *referent,
struct ref_transaction *transaction,
struct string_list *affected_refnames,
struct strbuf *err)
{
struct string_list_item *item;
struct ref_update *new_update;
unsigned int new_flags;







if (string_list_has_string(affected_refnames, referent)) {

strbuf_addf(err,
"multiple updates for '%s' (including one "
"via symref '%s') are not allowed",
referent, update->refname);
return TRANSACTION_NAME_CONFLICT;
}

new_flags = update->flags;
if (!strcmp(update->refname, "HEAD")) {







new_flags |= REF_UPDATE_VIA_HEAD;
}

new_update = ref_transaction_add_update(
transaction, referent, new_flags,
&update->new_oid, &update->old_oid,
update->msg);

new_update->parent_update = update;






update->flags |= REF_LOG_ONLY | REF_NO_DEREF;
update->flags &= ~REF_HAVE_OLD;








item = string_list_insert(affected_refnames, new_update->refname);
if (item->util)
BUG("%s unexpectedly found in affected_refnames",
new_update->refname);
item->util = new_update;

return 0;
}




static const char *original_update_refname(struct ref_update *update)
{
while (update->parent_update)
update = update->parent_update;

return update->refname;
}







static int check_old_oid(struct ref_update *update, struct object_id *oid,
struct strbuf *err)
{
if (!(update->flags & REF_HAVE_OLD) ||
oideq(oid, &update->old_oid))
return 0;

if (is_null_oid(&update->old_oid))
strbuf_addf(err, "cannot lock ref '%s': "
"reference already exists",
original_update_refname(update));
else if (is_null_oid(oid))
strbuf_addf(err, "cannot lock ref '%s': "
"reference is missing but expected %s",
original_update_refname(update),
oid_to_hex(&update->old_oid));
else
strbuf_addf(err, "cannot lock ref '%s': "
"is at %s but expected %s",
original_update_refname(update),
oid_to_hex(oid),
oid_to_hex(&update->old_oid));

return -1;
}














static int lock_ref_for_update(struct files_ref_store *refs,
struct ref_update *update,
struct ref_transaction *transaction,
const char *head_ref,
struct string_list *affected_refnames,
struct strbuf *err)
{
struct strbuf referent = STRBUF_INIT;
int mustexist = (update->flags & REF_HAVE_OLD) &&
!is_null_oid(&update->old_oid);
int ret = 0;
struct ref_lock *lock;

files_assert_main_repository(refs, "lock_ref_for_update");

if ((update->flags & REF_HAVE_NEW) && is_null_oid(&update->new_oid))
update->flags |= REF_DELETING;

if (head_ref) {
ret = split_head_update(update, transaction, head_ref,
affected_refnames, err);
if (ret)
goto out;
}

ret = lock_raw_ref(refs, update->refname, mustexist,
affected_refnames, NULL,
&lock, &referent,
&update->type, err);
if (ret) {
char *reason;

reason = strbuf_detach(err, NULL);
strbuf_addf(err, "cannot lock ref '%s': %s",
original_update_refname(update), reason);
free(reason);
goto out;
}

update->backend_data = lock;

if (update->type & REF_ISSYMREF) {
if (update->flags & REF_NO_DEREF) {





if (refs_read_ref_full(&refs->base,
referent.buf, 0,
&lock->old_oid, NULL)) {
if (update->flags & REF_HAVE_OLD) {
strbuf_addf(err, "cannot lock ref '%s': "
"error reading reference",
original_update_refname(update));
ret = TRANSACTION_GENERIC_ERROR;
goto out;
}
} else if (check_old_oid(update, &lock->old_oid, err)) {
ret = TRANSACTION_GENERIC_ERROR;
goto out;
}
} else {







ret = split_symref_update(update,
referent.buf, transaction,
affected_refnames, err);
if (ret)
goto out;
}
} else {
struct ref_update *parent_update;

if (check_old_oid(update, &lock->old_oid, err)) {
ret = TRANSACTION_GENERIC_ERROR;
goto out;
}






for (parent_update = update->parent_update;
parent_update;
parent_update = parent_update->parent_update) {
struct ref_lock *parent_lock = parent_update->backend_data;
oidcpy(&parent_lock->old_oid, &lock->old_oid);
}
}

if ((update->flags & REF_HAVE_NEW) &&
!(update->flags & REF_DELETING) &&
!(update->flags & REF_LOG_ONLY)) {
if (!(update->type & REF_ISSYMREF) &&
oideq(&lock->old_oid, &update->new_oid)) {




} else if (write_ref_to_lockfile(lock, &update->new_oid,
err)) {
char *write_err = strbuf_detach(err, NULL);





update->backend_data = NULL;
strbuf_addf(err,
"cannot update ref '%s': %s",
update->refname, write_err);
free(write_err);
ret = TRANSACTION_GENERIC_ERROR;
goto out;
} else {
update->flags |= REF_NEEDS_COMMIT;
}
}
if (!(update->flags & REF_NEEDS_COMMIT)) {





if (close_ref_gently(lock)) {
strbuf_addf(err, "couldn't close '%s.lock'",
update->refname);
ret = TRANSACTION_GENERIC_ERROR;
goto out;
}
}

out:
strbuf_release(&referent);
return ret;
}

struct files_transaction_backend_data {
struct ref_transaction *packed_transaction;
int packed_refs_locked;
};





static void files_transaction_cleanup(struct files_ref_store *refs,
struct ref_transaction *transaction)
{
size_t i;
struct files_transaction_backend_data *backend_data =
transaction->backend_data;
struct strbuf err = STRBUF_INIT;

for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];
struct ref_lock *lock = update->backend_data;

if (lock) {
unlock_ref(lock);
update->backend_data = NULL;
}
}

if (backend_data->packed_transaction &&
ref_transaction_abort(backend_data->packed_transaction, &err)) {
error("error aborting transaction: %s", err.buf);
strbuf_release(&err);
}

if (backend_data->packed_refs_locked)
packed_refs_unlock(refs->packed_ref_store);

free(backend_data);

transaction->state = REF_TRANSACTION_CLOSED;
}

static int files_transaction_prepare(struct ref_store *ref_store,
struct ref_transaction *transaction,
struct strbuf *err)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE,
"ref_transaction_prepare");
size_t i;
int ret = 0;
struct string_list affected_refnames = STRING_LIST_INIT_NODUP;
char *head_ref = NULL;
int head_type;
struct files_transaction_backend_data *backend_data;
struct ref_transaction *packed_transaction = NULL;

assert(err);

if (!transaction->nr)
goto cleanup;

backend_data = xcalloc(1, sizeof(*backend_data));
transaction->backend_data = backend_data;









for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];
struct string_list_item *item =
string_list_append(&affected_refnames, update->refname);

if ((update->flags & REF_IS_PRUNING) &&
!(update->flags & REF_NO_DEREF))
BUG("REF_IS_PRUNING set without REF_NO_DEREF");






item->util = update;
}
string_list_sort(&affected_refnames);
if (ref_update_reject_duplicates(&affected_refnames, err)) {
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}



















head_ref = refs_resolve_refdup(ref_store, "HEAD",
RESOLVE_REF_NO_RECURSE,
NULL, &head_type);

if (head_ref && !(head_type & REF_ISSYMREF)) {
FREE_AND_NULL(head_ref);
}









for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];

ret = lock_ref_for_update(refs, update, transaction,
head_ref, &affected_refnames, err);
if (ret)
goto cleanup;

if (update->flags & REF_DELETING &&
!(update->flags & REF_LOG_ONLY) &&
!(update->flags & REF_IS_PRUNING)) {




if (!packed_transaction) {
packed_transaction = ref_store_transaction_begin(
refs->packed_ref_store, err);
if (!packed_transaction) {
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}

backend_data->packed_transaction =
packed_transaction;
}

ref_transaction_add_update(
packed_transaction, update->refname,
REF_HAVE_NEW | REF_NO_DEREF,
&update->new_oid, NULL,
NULL);
}
}

if (packed_transaction) {
if (packed_refs_lock(refs->packed_ref_store, 0, err)) {
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}
backend_data->packed_refs_locked = 1;

if (is_packed_transaction_needed(refs->packed_ref_store,
packed_transaction)) {
ret = ref_transaction_prepare(packed_transaction, err);






if (ret) {
ref_transaction_free(packed_transaction);
backend_data->packed_transaction = NULL;
}
} else {










backend_data->packed_transaction = NULL;
if (ref_transaction_abort(packed_transaction, err)) {
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}
}
}

cleanup:
free(head_ref);
string_list_clear(&affected_refnames, 0);

if (ret)
files_transaction_cleanup(refs, transaction);
else
transaction->state = REF_TRANSACTION_PREPARED;

return ret;
}

static int files_transaction_finish(struct ref_store *ref_store,
struct ref_transaction *transaction,
struct strbuf *err)
{
struct files_ref_store *refs =
files_downcast(ref_store, 0, "ref_transaction_finish");
size_t i;
int ret = 0;
struct strbuf sb = STRBUF_INIT;
struct files_transaction_backend_data *backend_data;
struct ref_transaction *packed_transaction;


assert(err);

if (!transaction->nr) {
transaction->state = REF_TRANSACTION_CLOSED;
return 0;
}

backend_data = transaction->backend_data;
packed_transaction = backend_data->packed_transaction;


for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];
struct ref_lock *lock = update->backend_data;

if (update->flags & REF_NEEDS_COMMIT ||
update->flags & REF_LOG_ONLY) {
if (files_log_ref_write(refs,
lock->ref_name,
&lock->old_oid,
&update->new_oid,
update->msg, update->flags,
err)) {
char *old_msg = strbuf_detach(err, NULL);

strbuf_addf(err, "cannot update the ref '%s': %s",
lock->ref_name, old_msg);
free(old_msg);
unlock_ref(lock);
update->backend_data = NULL;
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}
}
if (update->flags & REF_NEEDS_COMMIT) {
clear_loose_ref_cache(refs);
if (commit_ref(lock)) {
strbuf_addf(err, "couldn't set '%s'", lock->ref_name);
unlock_ref(lock);
update->backend_data = NULL;
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}
}
}









for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];
if (update->flags & REF_DELETING &&
!(update->flags & REF_LOG_ONLY) &&
!(update->flags & REF_IS_PRUNING)) {
strbuf_reset(&sb);
files_reflog_path(refs, &sb, update->refname);
if (!unlink_or_warn(sb.buf))
try_remove_empty_parents(refs, update->refname,
REMOVE_EMPTY_PARENTS_REFLOG);
}
}







if (packed_transaction) {
ret = ref_transaction_commit(packed_transaction, err);
ref_transaction_free(packed_transaction);
packed_transaction = NULL;
backend_data->packed_transaction = NULL;
if (ret)
goto cleanup;
}


for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];
struct ref_lock *lock = update->backend_data;

if (update->flags & REF_DELETING &&
!(update->flags & REF_LOG_ONLY)) {
if (!(update->type & REF_ISPACKED) ||
update->type & REF_ISSYMREF) {

strbuf_reset(&sb);
files_ref_path(refs, &sb, lock->ref_name);
if (unlink_or_msg(sb.buf, err)) {
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}
update->flags |= REF_DELETED_LOOSE;
}
}
}

clear_loose_ref_cache(refs);

cleanup:
files_transaction_cleanup(refs, transaction);

for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];

if (update->flags & REF_DELETED_LOOSE) {






try_remove_empty_parents(refs, update->refname,
REMOVE_EMPTY_PARENTS_REF);
}
}

strbuf_release(&sb);
return ret;
}

static int files_transaction_abort(struct ref_store *ref_store,
struct ref_transaction *transaction,
struct strbuf *err)
{
struct files_ref_store *refs =
files_downcast(ref_store, 0, "ref_transaction_abort");

files_transaction_cleanup(refs, transaction);
return 0;
}

static int ref_present(const char *refname,
const struct object_id *oid, int flags, void *cb_data)
{
struct string_list *affected_refnames = cb_data;

return string_list_has_string(affected_refnames, refname);
}

static int files_initial_transaction_commit(struct ref_store *ref_store,
struct ref_transaction *transaction,
struct strbuf *err)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE,
"initial_ref_transaction_commit");
size_t i;
int ret = 0;
struct string_list affected_refnames = STRING_LIST_INIT_NODUP;
struct ref_transaction *packed_transaction = NULL;

assert(err);

if (transaction->state != REF_TRANSACTION_OPEN)
BUG("commit called for transaction that is not open");


for (i = 0; i < transaction->nr; i++)
string_list_append(&affected_refnames,
transaction->updates[i]->refname);
string_list_sort(&affected_refnames);
if (ref_update_reject_duplicates(&affected_refnames, err)) {
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}













if (refs_for_each_rawref(&refs->base, ref_present,
&affected_refnames))
BUG("initial ref transaction called with existing refs");

packed_transaction = ref_store_transaction_begin(refs->packed_ref_store, err);
if (!packed_transaction) {
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}

for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];

if ((update->flags & REF_HAVE_OLD) &&
!is_null_oid(&update->old_oid))
BUG("initial ref transaction with old_sha1 set");
if (refs_verify_refname_available(&refs->base, update->refname,
&affected_refnames, NULL,
err)) {
ret = TRANSACTION_NAME_CONFLICT;
goto cleanup;
}





ref_transaction_add_update(packed_transaction, update->refname,
update->flags & ~REF_HAVE_OLD,
&update->new_oid, &update->old_oid,
NULL);
}

if (packed_refs_lock(refs->packed_ref_store, 0, err)) {
ret = TRANSACTION_GENERIC_ERROR;
goto cleanup;
}

if (initial_ref_transaction_commit(packed_transaction, err)) {
ret = TRANSACTION_GENERIC_ERROR;
}

packed_refs_unlock(refs->packed_ref_store);
cleanup:
if (packed_transaction)
ref_transaction_free(packed_transaction);
transaction->state = REF_TRANSACTION_CLOSED;
string_list_clear(&affected_refnames, 0);
return ret;
}

struct expire_reflog_cb {
unsigned int flags;
reflog_expiry_should_prune_fn *should_prune_fn;
void *policy_cb;
FILE *newlog;
struct object_id last_kept_oid;
};

static int expire_reflog_ent(struct object_id *ooid, struct object_id *noid,
const char *email, timestamp_t timestamp, int tz,
const char *message, void *cb_data)
{
struct expire_reflog_cb *cb = cb_data;
struct expire_reflog_policy_cb *policy_cb = cb->policy_cb;

if (cb->flags & EXPIRE_REFLOGS_REWRITE)
ooid = &cb->last_kept_oid;

if ((*cb->should_prune_fn)(ooid, noid, email, timestamp, tz,
message, policy_cb)) {
if (!cb->newlog)
printf("would prune %s", message);
else if (cb->flags & EXPIRE_REFLOGS_VERBOSE)
printf("prune %s", message);
} else {
if (cb->newlog) {
fprintf(cb->newlog, "%s %s %s %"PRItime" %+05d\t%s",
oid_to_hex(ooid), oid_to_hex(noid),
email, timestamp, tz, message);
oidcpy(&cb->last_kept_oid, noid);
}
if (cb->flags & EXPIRE_REFLOGS_VERBOSE)
printf("keep %s", message);
}
return 0;
}

static int files_reflog_expire(struct ref_store *ref_store,
const char *refname, const struct object_id *oid,
unsigned int flags,
reflog_expiry_prepare_fn prepare_fn,
reflog_expiry_should_prune_fn should_prune_fn,
reflog_expiry_cleanup_fn cleanup_fn,
void *policy_cb_data)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE, "reflog_expire");
struct lock_file reflog_lock = LOCK_INIT;
struct expire_reflog_cb cb;
struct ref_lock *lock;
struct strbuf log_file_sb = STRBUF_INIT;
char *log_file;
int status = 0;
int type;
struct strbuf err = STRBUF_INIT;

memset(&cb, 0, sizeof(cb));
cb.flags = flags;
cb.policy_cb = policy_cb_data;
cb.should_prune_fn = should_prune_fn;






lock = lock_ref_oid_basic(refs, refname, oid,
NULL, NULL, REF_NO_DEREF,
&type, &err);
if (!lock) {
error("cannot lock ref '%s': %s", refname, err.buf);
strbuf_release(&err);
return -1;
}
if (!refs_reflog_exists(ref_store, refname)) {
unlock_ref(lock);
return 0;
}

files_reflog_path(refs, &log_file_sb, refname);
log_file = strbuf_detach(&log_file_sb, NULL);
if (!(flags & EXPIRE_REFLOGS_DRY_RUN)) {







if (hold_lock_file_for_update(&reflog_lock, log_file, 0) < 0) {
struct strbuf err = STRBUF_INIT;
unable_to_lock_message(log_file, errno, &err);
error("%s", err.buf);
strbuf_release(&err);
goto failure;
}
cb.newlog = fdopen_lock_file(&reflog_lock, "w");
if (!cb.newlog) {
error("cannot fdopen %s (%s)",
get_lock_file_path(&reflog_lock), strerror(errno));
goto failure;
}
}

(*prepare_fn)(refname, oid, cb.policy_cb);
refs_for_each_reflog_ent(ref_store, refname, expire_reflog_ent, &cb);
(*cleanup_fn)(cb.policy_cb);

if (!(flags & EXPIRE_REFLOGS_DRY_RUN)) {







int update = (flags & EXPIRE_REFLOGS_UPDATE_REF) &&
!(type & REF_ISSYMREF) &&
!is_null_oid(&cb.last_kept_oid);

if (close_lock_file_gently(&reflog_lock)) {
status |= error("couldn't write %s: %s", log_file,
strerror(errno));
rollback_lock_file(&reflog_lock);
} else if (update &&
(write_in_full(get_lock_file_fd(&lock->lk),
oid_to_hex(&cb.last_kept_oid), the_hash_algo->hexsz) < 0 ||
write_str_in_full(get_lock_file_fd(&lock->lk), "\n") < 0 ||
close_ref_gently(lock) < 0)) {
status |= error("couldn't write %s",
get_lock_file_path(&lock->lk));
rollback_lock_file(&reflog_lock);
} else if (commit_lock_file(&reflog_lock)) {
status |= error("unable to write reflog '%s' (%s)",
log_file, strerror(errno));
} else if (update && commit_ref(lock)) {
status |= error("couldn't set %s", lock->ref_name);
}
}
free(log_file);
unlock_ref(lock);
return status;

failure:
rollback_lock_file(&reflog_lock);
free(log_file);
unlock_ref(lock);
return -1;
}

static int files_init_db(struct ref_store *ref_store, struct strbuf *err)
{
struct files_ref_store *refs =
files_downcast(ref_store, REF_STORE_WRITE, "init_db");
struct strbuf sb = STRBUF_INIT;




files_ref_path(refs, &sb, "refs/heads");
safe_create_dir(sb.buf, 1);

strbuf_reset(&sb);
files_ref_path(refs, &sb, "refs/tags");
safe_create_dir(sb.buf, 1);

strbuf_release(&sb);
return 0;
}

struct ref_storage_be refs_be_files = {
NULL,
"files",
files_ref_store_create,
files_init_db,
files_transaction_prepare,
files_transaction_finish,
files_transaction_abort,
files_initial_transaction_commit,

files_pack_refs,
files_create_symref,
files_delete_refs,
files_rename_ref,
files_copy_ref,

files_ref_iterator_begin,
files_read_raw_ref,

files_reflog_iterator_begin,
files_for_each_reflog_ent,
files_for_each_reflog_ent_reverse,
files_reflog_exists,
files_create_reflog,
files_delete_reflog,
files_reflog_expire
};
