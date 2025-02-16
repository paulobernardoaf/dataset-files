#include "../cache.h"
#include "../config.h"
#include "../refs.h"
#include "refs-internal.h"
#include "packed-backend.h"
#include "../iterator.h"
#include "../lockfile.h"
#include "../chdir-notify.h"
enum mmap_strategy {
MMAP_NONE,
MMAP_TEMPORARY,
MMAP_OK
};
#if defined(NO_MMAP)
static enum mmap_strategy mmap_strategy = MMAP_NONE;
#elif defined(MMAP_PREVENTS_DELETE)
static enum mmap_strategy mmap_strategy = MMAP_TEMPORARY;
#else
static enum mmap_strategy mmap_strategy = MMAP_OK;
#endif
struct packed_ref_store;
struct snapshot {
struct packed_ref_store *refs;
int mmapped;
char *buf, *start, *eof;
enum { PEELED_NONE, PEELED_TAGS, PEELED_FULLY } peeled;
unsigned int referrers;
struct stat_validity validity;
};
struct packed_ref_store {
struct ref_store base;
unsigned int store_flags;
char *path;
struct snapshot *snapshot;
struct lock_file lock;
struct tempfile *tempfile;
};
static void acquire_snapshot(struct snapshot *snapshot)
{
snapshot->referrers++;
}
static void clear_snapshot_buffer(struct snapshot *snapshot)
{
if (snapshot->mmapped) {
if (munmap(snapshot->buf, snapshot->eof - snapshot->buf))
die_errno("error ummapping packed-refs file %s",
snapshot->refs->path);
snapshot->mmapped = 0;
} else {
free(snapshot->buf);
}
snapshot->buf = snapshot->start = snapshot->eof = NULL;
}
static int release_snapshot(struct snapshot *snapshot)
{
if (!--snapshot->referrers) {
stat_validity_clear(&snapshot->validity);
clear_snapshot_buffer(snapshot);
free(snapshot);
return 1;
} else {
return 0;
}
}
struct ref_store *packed_ref_store_create(const char *path,
unsigned int store_flags)
{
struct packed_ref_store *refs = xcalloc(1, sizeof(*refs));
struct ref_store *ref_store = (struct ref_store *)refs;
base_ref_store_init(ref_store, &refs_be_packed);
refs->store_flags = store_flags;
refs->path = xstrdup(path);
chdir_notify_reparent("packed-refs", &refs->path);
return ref_store;
}
static struct packed_ref_store *packed_downcast(struct ref_store *ref_store,
unsigned int required_flags,
const char *caller)
{
struct packed_ref_store *refs;
if (ref_store->be != &refs_be_packed)
BUG("ref_store is type \"%s\" not \"packed\" in %s",
ref_store->be->name, caller);
refs = (struct packed_ref_store *)ref_store;
if ((refs->store_flags & required_flags) != required_flags)
BUG("unallowed operation (%s), requires %x, has %x\n",
caller, required_flags, refs->store_flags);
return refs;
}
static void clear_snapshot(struct packed_ref_store *refs)
{
if (refs->snapshot) {
struct snapshot *snapshot = refs->snapshot;
refs->snapshot = NULL;
release_snapshot(snapshot);
}
}
static NORETURN void die_unterminated_line(const char *path,
const char *p, size_t len)
{
if (len < 80)
die("unterminated line in %s: %.*s", path, (int)len, p);
else
die("unterminated line in %s: %.75s...", path, p);
}
static NORETURN void die_invalid_line(const char *path,
const char *p, size_t len)
{
const char *eol = memchr(p, '\n', len);
if (!eol)
die_unterminated_line(path, p, len);
else if (eol - p < 80)
die("unexpected line in %s: %.*s", path, (int)(eol - p), p);
else
die("unexpected line in %s: %.75s...", path, p);
}
struct snapshot_record {
const char *start;
size_t len;
};
static int cmp_packed_ref_records(const void *v1, const void *v2)
{
const struct snapshot_record *e1 = v1, *e2 = v2;
const char *r1 = e1->start + the_hash_algo->hexsz + 1;
const char *r2 = e2->start + the_hash_algo->hexsz + 1;
while (1) {
if (*r1 == '\n')
return *r2 == '\n' ? 0 : -1;
if (*r1 != *r2) {
if (*r2 == '\n')
return 1;
else
return (unsigned char)*r1 < (unsigned char)*r2 ? -1 : +1;
}
r1++;
r2++;
}
}
static int cmp_record_to_refname(const char *rec, const char *refname)
{
const char *r1 = rec + the_hash_algo->hexsz + 1;
const char *r2 = refname;
while (1) {
if (*r1 == '\n')
return *r2 ? -1 : 0;
if (!*r2)
return 1;
if (*r1 != *r2)
return (unsigned char)*r1 < (unsigned char)*r2 ? -1 : +1;
r1++;
r2++;
}
}
static void sort_snapshot(struct snapshot *snapshot)
{
struct snapshot_record *records = NULL;
size_t alloc = 0, nr = 0;
int sorted = 1;
const char *pos, *eof, *eol;
size_t len, i;
char *new_buffer, *dst;
pos = snapshot->start;
eof = snapshot->eof;
if (pos == eof)
return;
len = eof - pos;
ALLOC_GROW(records, len / 80 + 20, alloc);
while (pos < eof) {
eol = memchr(pos, '\n', eof - pos);
if (!eol)
BUG("unterminated line found in packed-refs");
if (eol - pos < the_hash_algo->hexsz + 2)
die_invalid_line(snapshot->refs->path,
pos, eof - pos);
eol++;
if (eol < eof && *eol == '^') {
const char *peeled_start = eol;
eol = memchr(peeled_start, '\n', eof - peeled_start);
if (!eol)
BUG("unterminated peeled line found in packed-refs");
eol++;
}
ALLOC_GROW(records, nr + 1, alloc);
records[nr].start = pos;
records[nr].len = eol - pos;
nr++;
if (sorted &&
nr > 1 &&
cmp_packed_ref_records(&records[nr - 2],
&records[nr - 1]) >= 0)
sorted = 0;
pos = eol;
}
if (sorted)
goto cleanup;
QSORT(records, nr, cmp_packed_ref_records);
new_buffer = xmalloc(len);
for (dst = new_buffer, i = 0; i < nr; i++) {
memcpy(dst, records[i].start, records[i].len);
dst += records[i].len;
}
clear_snapshot_buffer(snapshot);
snapshot->buf = snapshot->start = new_buffer;
snapshot->eof = new_buffer + len;
cleanup:
free(records);
}
static const char *find_start_of_record(const char *buf, const char *p)
{
while (p > buf && (p[-1] != '\n' || p[0] == '^'))
p--;
return p;
}
static const char *find_end_of_record(const char *p, const char *end)
{
while (++p < end && (p[-1] != '\n' || p[0] == '^'))
;
return p;
}
static void verify_buffer_safe(struct snapshot *snapshot)
{
const char *start = snapshot->start;
const char *eof = snapshot->eof;
const char *last_line;
if (start == eof)
return;
last_line = find_start_of_record(start, eof - 1);
if (*(eof - 1) != '\n' || eof - last_line < the_hash_algo->hexsz + 2)
die_invalid_line(snapshot->refs->path,
last_line, eof - last_line);
}
#define SMALL_FILE_SIZE (32*1024)
static int load_contents(struct snapshot *snapshot)
{
int fd;
struct stat st;
size_t size;
ssize_t bytes_read;
fd = open(snapshot->refs->path, O_RDONLY);
if (fd < 0) {
if (errno == ENOENT) {
return 0;
} else {
die_errno("couldn't read %s", snapshot->refs->path);
}
}
stat_validity_update(&snapshot->validity, fd);
if (fstat(fd, &st) < 0)
die_errno("couldn't stat %s", snapshot->refs->path);
size = xsize_t(st.st_size);
if (!size) {
close(fd);
return 0;
} else if (mmap_strategy == MMAP_NONE || size <= SMALL_FILE_SIZE) {
snapshot->buf = xmalloc(size);
bytes_read = read_in_full(fd, snapshot->buf, size);
if (bytes_read < 0 || bytes_read != size)
die_errno("couldn't read %s", snapshot->refs->path);
snapshot->mmapped = 0;
} else {
snapshot->buf = xmmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
snapshot->mmapped = 1;
}
close(fd);
snapshot->start = snapshot->buf;
snapshot->eof = snapshot->buf + size;
return 1;
}
static const char *find_reference_location(struct snapshot *snapshot,
const char *refname, int mustexist)
{
const char *lo = snapshot->start;
const char *hi = snapshot->eof;
while (lo != hi) {
const char *mid, *rec;
int cmp;
mid = lo + (hi - lo) / 2;
rec = find_start_of_record(lo, mid);
cmp = cmp_record_to_refname(rec, refname);
if (cmp < 0) {
lo = find_end_of_record(mid, hi);
} else if (cmp > 0) {
hi = rec;
} else {
return rec;
}
}
if (mustexist)
return NULL;
else
return lo;
}
static struct snapshot *create_snapshot(struct packed_ref_store *refs)
{
struct snapshot *snapshot = xcalloc(1, sizeof(*snapshot));
int sorted = 0;
snapshot->refs = refs;
acquire_snapshot(snapshot);
snapshot->peeled = PEELED_NONE;
if (!load_contents(snapshot))
return snapshot;
if (snapshot->buf < snapshot->eof && *snapshot->buf == '#') {
char *tmp, *p, *eol;
struct string_list traits = STRING_LIST_INIT_NODUP;
eol = memchr(snapshot->buf, '\n',
snapshot->eof - snapshot->buf);
if (!eol)
die_unterminated_line(refs->path,
snapshot->buf,
snapshot->eof - snapshot->buf);
tmp = xmemdupz(snapshot->buf, eol - snapshot->buf);
if (!skip_prefix(tmp, "#pack-refs with:", (const char **)&p))
die_invalid_line(refs->path,
snapshot->buf,
snapshot->eof - snapshot->buf);
string_list_split_in_place(&traits, p, ' ', -1);
if (unsorted_string_list_has_string(&traits, "fully-peeled"))
snapshot->peeled = PEELED_FULLY;
else if (unsorted_string_list_has_string(&traits, "peeled"))
snapshot->peeled = PEELED_TAGS;
sorted = unsorted_string_list_has_string(&traits, "sorted");
snapshot->start = eol + 1;
string_list_clear(&traits, 0);
free(tmp);
}
verify_buffer_safe(snapshot);
if (!sorted) {
sort_snapshot(snapshot);
verify_buffer_safe(snapshot);
}
if (mmap_strategy != MMAP_OK && snapshot->mmapped) {
size_t size = snapshot->eof - snapshot->start;
char *buf_copy = xmalloc(size);
memcpy(buf_copy, snapshot->start, size);
clear_snapshot_buffer(snapshot);
snapshot->buf = snapshot->start = buf_copy;
snapshot->eof = buf_copy + size;
}
return snapshot;
}
static void validate_snapshot(struct packed_ref_store *refs)
{
if (refs->snapshot &&
!stat_validity_check(&refs->snapshot->validity, refs->path))
clear_snapshot(refs);
}
static struct snapshot *get_snapshot(struct packed_ref_store *refs)
{
if (!is_lock_file_locked(&refs->lock))
validate_snapshot(refs);
if (!refs->snapshot)
refs->snapshot = create_snapshot(refs);
return refs->snapshot;
}
static int packed_read_raw_ref(struct ref_store *ref_store,
const char *refname, struct object_id *oid,
struct strbuf *referent, unsigned int *type)
{
struct packed_ref_store *refs =
packed_downcast(ref_store, REF_STORE_READ, "read_raw_ref");
struct snapshot *snapshot = get_snapshot(refs);
const char *rec;
*type = 0;
rec = find_reference_location(snapshot, refname, 1);
if (!rec) {
errno = ENOENT;
return -1;
}
if (get_oid_hex(rec, oid))
die_invalid_line(refs->path, rec, snapshot->eof - rec);
*type = REF_ISPACKED;
return 0;
}
#define REF_KNOWS_PEELED 0x40
struct packed_ref_iterator {
struct ref_iterator base;
struct snapshot *snapshot;
const char *pos;
const char *eof;
struct object_id oid, peeled;
struct strbuf refname_buf;
unsigned int flags;
};
static int next_record(struct packed_ref_iterator *iter)
{
const char *p = iter->pos, *eol;
strbuf_reset(&iter->refname_buf);
if (iter->pos == iter->eof)
return ITER_DONE;
iter->base.flags = REF_ISPACKED;
if (iter->eof - p < the_hash_algo->hexsz + 2 ||
parse_oid_hex(p, &iter->oid, &p) ||
!isspace(*p++))
die_invalid_line(iter->snapshot->refs->path,
iter->pos, iter->eof - iter->pos);
eol = memchr(p, '\n', iter->eof - p);
if (!eol)
die_unterminated_line(iter->snapshot->refs->path,
iter->pos, iter->eof - iter->pos);
strbuf_add(&iter->refname_buf, p, eol - p);
iter->base.refname = iter->refname_buf.buf;
if (check_refname_format(iter->base.refname, REFNAME_ALLOW_ONELEVEL)) {
if (!refname_is_safe(iter->base.refname))
die("packed refname is dangerous: %s",
iter->base.refname);
oidclr(&iter->oid);
iter->base.flags |= REF_BAD_NAME | REF_ISBROKEN;
}
if (iter->snapshot->peeled == PEELED_FULLY ||
(iter->snapshot->peeled == PEELED_TAGS &&
starts_with(iter->base.refname, "refs/tags/")))
iter->base.flags |= REF_KNOWS_PEELED;
iter->pos = eol + 1;
if (iter->pos < iter->eof && *iter->pos == '^') {
p = iter->pos + 1;
if (iter->eof - p < the_hash_algo->hexsz + 1 ||
parse_oid_hex(p, &iter->peeled, &p) ||
*p++ != '\n')
die_invalid_line(iter->snapshot->refs->path,
iter->pos, iter->eof - iter->pos);
iter->pos = p;
if ((iter->base.flags & REF_ISBROKEN)) {
oidclr(&iter->peeled);
iter->base.flags &= ~REF_KNOWS_PEELED;
} else {
iter->base.flags |= REF_KNOWS_PEELED;
}
} else {
oidclr(&iter->peeled);
}
return ITER_OK;
}
static int packed_ref_iterator_advance(struct ref_iterator *ref_iterator)
{
struct packed_ref_iterator *iter =
(struct packed_ref_iterator *)ref_iterator;
int ok;
while ((ok = next_record(iter)) == ITER_OK) {
if (iter->flags & DO_FOR_EACH_PER_WORKTREE_ONLY &&
ref_type(iter->base.refname) != REF_TYPE_PER_WORKTREE)
continue;
if (!(iter->flags & DO_FOR_EACH_INCLUDE_BROKEN) &&
!ref_resolves_to_object(iter->base.refname, &iter->oid,
iter->flags))
continue;
return ITER_OK;
}
if (ref_iterator_abort(ref_iterator) != ITER_DONE)
ok = ITER_ERROR;
return ok;
}
static int packed_ref_iterator_peel(struct ref_iterator *ref_iterator,
struct object_id *peeled)
{
struct packed_ref_iterator *iter =
(struct packed_ref_iterator *)ref_iterator;
if ((iter->base.flags & REF_KNOWS_PEELED)) {
oidcpy(peeled, &iter->peeled);
return is_null_oid(&iter->peeled) ? -1 : 0;
} else if ((iter->base.flags & (REF_ISBROKEN | REF_ISSYMREF))) {
return -1;
} else {
return !!peel_object(&iter->oid, peeled);
}
}
static int packed_ref_iterator_abort(struct ref_iterator *ref_iterator)
{
struct packed_ref_iterator *iter =
(struct packed_ref_iterator *)ref_iterator;
int ok = ITER_DONE;
strbuf_release(&iter->refname_buf);
release_snapshot(iter->snapshot);
base_ref_iterator_free(ref_iterator);
return ok;
}
static struct ref_iterator_vtable packed_ref_iterator_vtable = {
packed_ref_iterator_advance,
packed_ref_iterator_peel,
packed_ref_iterator_abort
};
static struct ref_iterator *packed_ref_iterator_begin(
struct ref_store *ref_store,
const char *prefix, unsigned int flags)
{
struct packed_ref_store *refs;
struct snapshot *snapshot;
const char *start;
struct packed_ref_iterator *iter;
struct ref_iterator *ref_iterator;
unsigned int required_flags = REF_STORE_READ;
if (!(flags & DO_FOR_EACH_INCLUDE_BROKEN))
required_flags |= REF_STORE_ODB;
refs = packed_downcast(ref_store, required_flags, "ref_iterator_begin");
snapshot = get_snapshot(refs);
if (prefix && *prefix)
start = find_reference_location(snapshot, prefix, 0);
else
start = snapshot->start;
if (start == snapshot->eof)
return empty_ref_iterator_begin();
iter = xcalloc(1, sizeof(*iter));
ref_iterator = &iter->base;
base_ref_iterator_init(ref_iterator, &packed_ref_iterator_vtable, 1);
iter->snapshot = snapshot;
acquire_snapshot(snapshot);
iter->pos = start;
iter->eof = snapshot->eof;
strbuf_init(&iter->refname_buf, 0);
iter->base.oid = &iter->oid;
iter->flags = flags;
if (prefix && *prefix)
ref_iterator = prefix_ref_iterator_begin(ref_iterator, prefix, 0);
return ref_iterator;
}
static int write_packed_entry(FILE *fh, const char *refname,
const struct object_id *oid,
const struct object_id *peeled)
{
if (fprintf(fh, "%s %s\n", oid_to_hex(oid), refname) < 0 ||
(peeled && fprintf(fh, "^%s\n", oid_to_hex(peeled)) < 0))
return -1;
return 0;
}
int packed_refs_lock(struct ref_store *ref_store, int flags, struct strbuf *err)
{
struct packed_ref_store *refs =
packed_downcast(ref_store, REF_STORE_WRITE | REF_STORE_MAIN,
"packed_refs_lock");
static int timeout_configured = 0;
static int timeout_value = 1000;
if (!timeout_configured) {
git_config_get_int("core.packedrefstimeout", &timeout_value);
timeout_configured = 1;
}
if (hold_lock_file_for_update_timeout(
&refs->lock,
refs->path,
flags, timeout_value) < 0) {
unable_to_lock_message(refs->path, errno, err);
return -1;
}
if (close_lock_file_gently(&refs->lock)) {
strbuf_addf(err, "unable to close %s: %s", refs->path, strerror(errno));
rollback_lock_file(&refs->lock);
return -1;
}
clear_snapshot(refs);
get_snapshot(refs);
return 0;
}
void packed_refs_unlock(struct ref_store *ref_store)
{
struct packed_ref_store *refs = packed_downcast(
ref_store,
REF_STORE_READ | REF_STORE_WRITE,
"packed_refs_unlock");
if (!is_lock_file_locked(&refs->lock))
BUG("packed_refs_unlock() called when not locked");
rollback_lock_file(&refs->lock);
}
int packed_refs_is_locked(struct ref_store *ref_store)
{
struct packed_ref_store *refs = packed_downcast(
ref_store,
REF_STORE_READ | REF_STORE_WRITE,
"packed_refs_is_locked");
return is_lock_file_locked(&refs->lock);
}
static const char PACKED_REFS_HEADER[] =
"#pack-refs with: peeled fully-peeled sorted \n";
static int packed_init_db(struct ref_store *ref_store, struct strbuf *err)
{
return 0;
}
static int write_with_updates(struct packed_ref_store *refs,
struct string_list *updates,
struct strbuf *err)
{
struct ref_iterator *iter = NULL;
size_t i;
int ok;
FILE *out;
struct strbuf sb = STRBUF_INIT;
char *packed_refs_path;
if (!is_lock_file_locked(&refs->lock))
BUG("write_with_updates() called while unlocked");
packed_refs_path = get_locked_file_path(&refs->lock);
strbuf_addf(&sb, "%s.new", packed_refs_path);
free(packed_refs_path);
refs->tempfile = create_tempfile(sb.buf);
if (!refs->tempfile) {
strbuf_addf(err, "unable to create file %s: %s",
sb.buf, strerror(errno));
strbuf_release(&sb);
return -1;
}
strbuf_release(&sb);
out = fdopen_tempfile(refs->tempfile, "w");
if (!out) {
strbuf_addf(err, "unable to fdopen packed-refs tempfile: %s",
strerror(errno));
goto error;
}
if (fprintf(out, "%s", PACKED_REFS_HEADER) < 0)
goto write_error;
iter = packed_ref_iterator_begin(&refs->base, "",
DO_FOR_EACH_INCLUDE_BROKEN);
if ((ok = ref_iterator_advance(iter)) != ITER_OK)
iter = NULL;
i = 0;
while (iter || i < updates->nr) {
struct ref_update *update = NULL;
int cmp;
if (i >= updates->nr) {
cmp = -1;
} else {
update = updates->items[i].util;
if (!iter)
cmp = +1;
else
cmp = strcmp(iter->refname, update->refname);
}
if (!cmp) {
if ((update->flags & REF_HAVE_OLD)) {
if (is_null_oid(&update->old_oid)) {
strbuf_addf(err, "cannot update ref '%s': "
"reference already exists",
update->refname);
goto error;
} else if (!oideq(&update->old_oid, iter->oid)) {
strbuf_addf(err, "cannot update ref '%s': "
"is at %s but expected %s",
update->refname,
oid_to_hex(iter->oid),
oid_to_hex(&update->old_oid));
goto error;
}
}
if ((update->flags & REF_HAVE_NEW)) {
if ((ok = ref_iterator_advance(iter)) != ITER_OK)
iter = NULL;
cmp = +1;
} else {
i++;
cmp = -1;
}
} else if (cmp > 0) {
if ((update->flags & REF_HAVE_OLD) &&
!is_null_oid(&update->old_oid)) {
strbuf_addf(err, "cannot update ref '%s': "
"reference is missing but expected %s",
update->refname,
oid_to_hex(&update->old_oid));
goto error;
}
}
if (cmp < 0) {
struct object_id peeled;
int peel_error = ref_iterator_peel(iter, &peeled);
if (write_packed_entry(out, iter->refname,
iter->oid,
peel_error ? NULL : &peeled))
goto write_error;
if ((ok = ref_iterator_advance(iter)) != ITER_OK)
iter = NULL;
} else if (is_null_oid(&update->new_oid)) {
i++;
} else {
struct object_id peeled;
int peel_error = peel_object(&update->new_oid,
&peeled);
if (write_packed_entry(out, update->refname,
&update->new_oid,
peel_error ? NULL : &peeled))
goto write_error;
i++;
}
}
if (ok != ITER_DONE) {
strbuf_addstr(err, "unable to write packed-refs file: "
"error iterating over old contents");
goto error;
}
if (close_tempfile_gently(refs->tempfile)) {
strbuf_addf(err, "error closing file %s: %s",
get_tempfile_path(refs->tempfile),
strerror(errno));
strbuf_release(&sb);
delete_tempfile(&refs->tempfile);
return -1;
}
return 0;
write_error:
strbuf_addf(err, "error writing to %s: %s",
get_tempfile_path(refs->tempfile), strerror(errno));
error:
if (iter)
ref_iterator_abort(iter);
delete_tempfile(&refs->tempfile);
return -1;
}
int is_packed_transaction_needed(struct ref_store *ref_store,
struct ref_transaction *transaction)
{
struct packed_ref_store *refs = packed_downcast(
ref_store,
REF_STORE_READ,
"is_packed_transaction_needed");
struct strbuf referent = STRBUF_INIT;
size_t i;
int ret;
if (!is_lock_file_locked(&refs->lock))
BUG("is_packed_transaction_needed() called while unlocked");
for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];
if (update->flags & REF_HAVE_OLD)
return 1;
if ((update->flags & REF_HAVE_NEW) && !is_null_oid(&update->new_oid))
return 1;
}
ret = 0;
for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];
unsigned int type;
struct object_id oid;
if (!(update->flags & REF_HAVE_NEW))
continue;
if (!refs_read_raw_ref(ref_store, update->refname,
&oid, &referent, &type) ||
errno != ENOENT) {
ret = 1;
break;
}
}
strbuf_release(&referent);
return ret;
}
struct packed_transaction_backend_data {
int own_lock;
struct string_list updates;
};
static void packed_transaction_cleanup(struct packed_ref_store *refs,
struct ref_transaction *transaction)
{
struct packed_transaction_backend_data *data = transaction->backend_data;
if (data) {
string_list_clear(&data->updates, 0);
if (is_tempfile_active(refs->tempfile))
delete_tempfile(&refs->tempfile);
if (data->own_lock && is_lock_file_locked(&refs->lock)) {
packed_refs_unlock(&refs->base);
data->own_lock = 0;
}
free(data);
transaction->backend_data = NULL;
}
transaction->state = REF_TRANSACTION_CLOSED;
}
static int packed_transaction_prepare(struct ref_store *ref_store,
struct ref_transaction *transaction,
struct strbuf *err)
{
struct packed_ref_store *refs = packed_downcast(
ref_store,
REF_STORE_READ | REF_STORE_WRITE | REF_STORE_ODB,
"ref_transaction_prepare");
struct packed_transaction_backend_data *data;
size_t i;
int ret = TRANSACTION_GENERIC_ERROR;
data = xcalloc(1, sizeof(*data));
string_list_init(&data->updates, 0);
transaction->backend_data = data;
for (i = 0; i < transaction->nr; i++) {
struct ref_update *update = transaction->updates[i];
struct string_list_item *item =
string_list_append(&data->updates, update->refname);
item->util = update;
}
string_list_sort(&data->updates);
if (ref_update_reject_duplicates(&data->updates, err))
goto failure;
if (!is_lock_file_locked(&refs->lock)) {
if (packed_refs_lock(ref_store, 0, err))
goto failure;
data->own_lock = 1;
}
if (write_with_updates(refs, &data->updates, err))
goto failure;
transaction->state = REF_TRANSACTION_PREPARED;
return 0;
failure:
packed_transaction_cleanup(refs, transaction);
return ret;
}
static int packed_transaction_abort(struct ref_store *ref_store,
struct ref_transaction *transaction,
struct strbuf *err)
{
struct packed_ref_store *refs = packed_downcast(
ref_store,
REF_STORE_READ | REF_STORE_WRITE | REF_STORE_ODB,
"ref_transaction_abort");
packed_transaction_cleanup(refs, transaction);
return 0;
}
static int packed_transaction_finish(struct ref_store *ref_store,
struct ref_transaction *transaction,
struct strbuf *err)
{
struct packed_ref_store *refs = packed_downcast(
ref_store,
REF_STORE_READ | REF_STORE_WRITE | REF_STORE_ODB,
"ref_transaction_finish");
int ret = TRANSACTION_GENERIC_ERROR;
char *packed_refs_path;
clear_snapshot(refs);
packed_refs_path = get_locked_file_path(&refs->lock);
if (rename_tempfile(&refs->tempfile, packed_refs_path)) {
strbuf_addf(err, "error replacing %s: %s",
refs->path, strerror(errno));
goto cleanup;
}
ret = 0;
cleanup:
free(packed_refs_path);
packed_transaction_cleanup(refs, transaction);
return ret;
}
static int packed_initial_transaction_commit(struct ref_store *ref_store,
struct ref_transaction *transaction,
struct strbuf *err)
{
return ref_transaction_commit(transaction, err);
}
static int packed_delete_refs(struct ref_store *ref_store, const char *msg,
struct string_list *refnames, unsigned int flags)
{
struct packed_ref_store *refs =
packed_downcast(ref_store, REF_STORE_WRITE, "delete_refs");
struct strbuf err = STRBUF_INIT;
struct ref_transaction *transaction;
struct string_list_item *item;
int ret;
(void)refs; 
if (!refnames->nr)
return 0;
transaction = ref_store_transaction_begin(ref_store, &err);
if (!transaction)
return -1;
for_each_string_list_item(item, refnames) {
if (ref_transaction_delete(transaction, item->string, NULL,
flags, msg, &err)) {
warning(_("could not delete reference %s: %s"),
item->string, err.buf);
strbuf_reset(&err);
}
}
ret = ref_transaction_commit(transaction, &err);
if (ret) {
if (refnames->nr == 1)
error(_("could not delete reference %s: %s"),
refnames->items[0].string, err.buf);
else
error(_("could not delete references: %s"), err.buf);
}
ref_transaction_free(transaction);
strbuf_release(&err);
return ret;
}
static int packed_pack_refs(struct ref_store *ref_store, unsigned int flags)
{
return 0;
}
static int packed_create_symref(struct ref_store *ref_store,
const char *refname, const char *target,
const char *logmsg)
{
BUG("packed reference store does not support symrefs");
}
static int packed_rename_ref(struct ref_store *ref_store,
const char *oldrefname, const char *newrefname,
const char *logmsg)
{
BUG("packed reference store does not support renaming references");
}
static int packed_copy_ref(struct ref_store *ref_store,
const char *oldrefname, const char *newrefname,
const char *logmsg)
{
BUG("packed reference store does not support copying references");
}
static struct ref_iterator *packed_reflog_iterator_begin(struct ref_store *ref_store)
{
return empty_ref_iterator_begin();
}
static int packed_for_each_reflog_ent(struct ref_store *ref_store,
const char *refname,
each_reflog_ent_fn fn, void *cb_data)
{
return 0;
}
static int packed_for_each_reflog_ent_reverse(struct ref_store *ref_store,
const char *refname,
each_reflog_ent_fn fn,
void *cb_data)
{
return 0;
}
static int packed_reflog_exists(struct ref_store *ref_store,
const char *refname)
{
return 0;
}
static int packed_create_reflog(struct ref_store *ref_store,
const char *refname, int force_create,
struct strbuf *err)
{
BUG("packed reference store does not support reflogs");
}
static int packed_delete_reflog(struct ref_store *ref_store,
const char *refname)
{
return 0;
}
static int packed_reflog_expire(struct ref_store *ref_store,
const char *refname, const struct object_id *oid,
unsigned int flags,
reflog_expiry_prepare_fn prepare_fn,
reflog_expiry_should_prune_fn should_prune_fn,
reflog_expiry_cleanup_fn cleanup_fn,
void *policy_cb_data)
{
return 0;
}
struct ref_storage_be refs_be_packed = {
NULL,
"packed",
packed_ref_store_create,
packed_init_db,
packed_transaction_prepare,
packed_transaction_finish,
packed_transaction_abort,
packed_initial_transaction_commit,
packed_pack_refs,
packed_create_symref,
packed_delete_refs,
packed_rename_ref,
packed_copy_ref,
packed_ref_iterator_begin,
packed_read_raw_ref,
packed_reflog_iterator_begin,
packed_for_each_reflog_ent,
packed_for_each_reflog_ent_reverse,
packed_reflog_exists,
packed_create_reflog,
packed_delete_reflog,
packed_reflog_expire
};
