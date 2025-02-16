#include "cache.h"
#include "config.h"
#include "lockfile.h"
#include "string-list.h"
#include "rerere.h"
#include "xdiff-interface.h"
#include "dir.h"
#include "resolve-undo.h"
#include "ll-merge.h"
#include "attr.h"
#include "pathspec.h"
#include "object-store.h"
#include "sha1-lookup.h"
#define RESOLVED 0
#define PUNTED 1
#define THREE_STAGED 2
void *RERERE_RESOLVED = &RERERE_RESOLVED;
static int rerere_enabled = -1;
static int rerere_autoupdate;
static int rerere_dir_nr;
static int rerere_dir_alloc;
#define RR_HAS_POSTIMAGE 1
#define RR_HAS_PREIMAGE 2
static struct rerere_dir {
unsigned char hash[GIT_MAX_HEXSZ];
int status_alloc, status_nr;
unsigned char *status;
} **rerere_dir;
static void free_rerere_dirs(void)
{
int i;
for (i = 0; i < rerere_dir_nr; i++) {
free(rerere_dir[i]->status);
free(rerere_dir[i]);
}
FREE_AND_NULL(rerere_dir);
rerere_dir_nr = rerere_dir_alloc = 0;
}
static void free_rerere_id(struct string_list_item *item)
{
free(item->util);
}
static const char *rerere_id_hex(const struct rerere_id *id)
{
return hash_to_hex(id->collection->hash);
}
static void fit_variant(struct rerere_dir *rr_dir, int variant)
{
variant++;
ALLOC_GROW(rr_dir->status, variant, rr_dir->status_alloc);
if (rr_dir->status_nr < variant) {
memset(rr_dir->status + rr_dir->status_nr,
'\0', variant - rr_dir->status_nr);
rr_dir->status_nr = variant;
}
}
static void assign_variant(struct rerere_id *id)
{
int variant;
struct rerere_dir *rr_dir = id->collection;
variant = id->variant;
if (variant < 0) {
for (variant = 0; variant < rr_dir->status_nr; variant++)
if (!rr_dir->status[variant])
break;
}
fit_variant(rr_dir, variant);
id->variant = variant;
}
const char *rerere_path(const struct rerere_id *id, const char *file)
{
if (!file)
return git_path("rr-cache/%s", rerere_id_hex(id));
if (id->variant <= 0)
return git_path("rr-cache/%s/%s", rerere_id_hex(id), file);
return git_path("rr-cache/%s/%s.%d",
rerere_id_hex(id), file, id->variant);
}
static int is_rr_file(const char *name, const char *filename, int *variant)
{
const char *suffix;
char *ep;
if (!strcmp(name, filename)) {
*variant = 0;
return 1;
}
if (!skip_prefix(name, filename, &suffix) || *suffix != '.')
return 0;
errno = 0;
*variant = strtol(suffix + 1, &ep, 10);
if (errno || *ep)
return 0;
return 1;
}
static void scan_rerere_dir(struct rerere_dir *rr_dir)
{
struct dirent *de;
DIR *dir = opendir(git_path("rr-cache/%s", hash_to_hex(rr_dir->hash)));
if (!dir)
return;
while ((de = readdir(dir)) != NULL) {
int variant;
if (is_rr_file(de->d_name, "postimage", &variant)) {
fit_variant(rr_dir, variant);
rr_dir->status[variant] |= RR_HAS_POSTIMAGE;
} else if (is_rr_file(de->d_name, "preimage", &variant)) {
fit_variant(rr_dir, variant);
rr_dir->status[variant] |= RR_HAS_PREIMAGE;
}
}
closedir(dir);
}
static const unsigned char *rerere_dir_hash(size_t i, void *table)
{
struct rerere_dir **rr_dir = table;
return rr_dir[i]->hash;
}
static struct rerere_dir *find_rerere_dir(const char *hex)
{
unsigned char hash[GIT_MAX_RAWSZ];
struct rerere_dir *rr_dir;
int pos;
if (get_sha1_hex(hex, hash))
return NULL; 
pos = sha1_pos(hash, rerere_dir, rerere_dir_nr, rerere_dir_hash);
if (pos < 0) {
rr_dir = xmalloc(sizeof(*rr_dir));
hashcpy(rr_dir->hash, hash);
rr_dir->status = NULL;
rr_dir->status_nr = 0;
rr_dir->status_alloc = 0;
pos = -1 - pos;
ALLOC_GROW(rerere_dir, rerere_dir_nr + 1, rerere_dir_alloc);
rerere_dir_nr++;
MOVE_ARRAY(rerere_dir + pos + 1, rerere_dir + pos,
rerere_dir_nr - pos - 1);
rerere_dir[pos] = rr_dir;
scan_rerere_dir(rr_dir);
}
return rerere_dir[pos];
}
static int has_rerere_resolution(const struct rerere_id *id)
{
const int both = RR_HAS_POSTIMAGE|RR_HAS_PREIMAGE;
int variant = id->variant;
if (variant < 0)
return 0;
return ((id->collection->status[variant] & both) == both);
}
static struct rerere_id *new_rerere_id_hex(char *hex)
{
struct rerere_id *id = xmalloc(sizeof(*id));
id->collection = find_rerere_dir(hex);
id->variant = -1; 
return id;
}
static struct rerere_id *new_rerere_id(unsigned char *hash)
{
return new_rerere_id_hex(hash_to_hex(hash));
}
static void read_rr(struct repository *r, struct string_list *rr)
{
struct strbuf buf = STRBUF_INIT;
FILE *in = fopen_or_warn(git_path_merge_rr(r), "r");
if (!in)
return;
while (!strbuf_getwholeline(&buf, in, '\0')) {
char *path;
unsigned char hash[GIT_MAX_RAWSZ];
struct rerere_id *id;
int variant;
const unsigned hexsz = the_hash_algo->hexsz;
if (buf.len < hexsz + 2 || get_sha1_hex(buf.buf, hash))
die(_("corrupt MERGE_RR"));
if (buf.buf[hexsz] != '.') {
variant = 0;
path = buf.buf + hexsz;
} else {
errno = 0;
variant = strtol(buf.buf + hexsz + 1, &path, 10);
if (errno)
die(_("corrupt MERGE_RR"));
}
if (*(path++) != '\t')
die(_("corrupt MERGE_RR"));
buf.buf[hexsz] = '\0';
id = new_rerere_id_hex(buf.buf);
id->variant = variant;
string_list_insert(rr, path)->util = id;
}
strbuf_release(&buf);
fclose(in);
}
static struct lock_file write_lock;
static int write_rr(struct string_list *rr, int out_fd)
{
int i;
for (i = 0; i < rr->nr; i++) {
struct strbuf buf = STRBUF_INIT;
struct rerere_id *id;
assert(rr->items[i].util != RERERE_RESOLVED);
id = rr->items[i].util;
if (!id)
continue;
assert(id->variant >= 0);
if (0 < id->variant)
strbuf_addf(&buf, "%s.%d\t%s%c",
rerere_id_hex(id), id->variant,
rr->items[i].string, 0);
else
strbuf_addf(&buf, "%s\t%s%c",
rerere_id_hex(id),
rr->items[i].string, 0);
if (write_in_full(out_fd, buf.buf, buf.len) < 0)
die(_("unable to write rerere record"));
strbuf_release(&buf);
}
if (commit_lock_file(&write_lock) != 0)
die(_("unable to write rerere record"));
return 0;
}
struct rerere_io {
int (*getline)(struct strbuf *, struct rerere_io *);
FILE *output;
int wrerror;
};
static void ferr_write(const void *p, size_t count, FILE *fp, int *err)
{
if (!count || *err)
return;
if (fwrite(p, count, 1, fp) != 1)
*err = errno;
}
static inline void ferr_puts(const char *s, FILE *fp, int *err)
{
ferr_write(s, strlen(s), fp, err);
}
static void rerere_io_putstr(const char *str, struct rerere_io *io)
{
if (io->output)
ferr_puts(str, io->output, &io->wrerror);
}
static void rerere_io_putmem(const char *mem, size_t sz, struct rerere_io *io)
{
if (io->output)
ferr_write(mem, sz, io->output, &io->wrerror);
}
struct rerere_io_file {
struct rerere_io io;
FILE *input;
};
static int rerere_file_getline(struct strbuf *sb, struct rerere_io *io_)
{
struct rerere_io_file *io = (struct rerere_io_file *)io_;
return strbuf_getwholeline(sb, io->input, '\n');
}
static int is_cmarker(char *buf, int marker_char, int marker_size)
{
int want_sp;
want_sp = (marker_char == '<') || (marker_char == '>');
while (marker_size--)
if (*buf++ != marker_char)
return 0;
if (want_sp && *buf != ' ')
return 0;
return isspace(*buf);
}
static void rerere_strbuf_putconflict(struct strbuf *buf, int ch, size_t size)
{
strbuf_addchars(buf, ch, size);
strbuf_addch(buf, '\n');
}
static int handle_conflict(struct strbuf *out, struct rerere_io *io,
int marker_size, git_hash_ctx *ctx)
{
enum {
RR_SIDE_1 = 0, RR_SIDE_2, RR_ORIGINAL
} hunk = RR_SIDE_1;
struct strbuf one = STRBUF_INIT, two = STRBUF_INIT;
struct strbuf buf = STRBUF_INIT, conflict = STRBUF_INIT;
int has_conflicts = -1;
while (!io->getline(&buf, io)) {
if (is_cmarker(buf.buf, '<', marker_size)) {
if (handle_conflict(&conflict, io, marker_size, NULL) < 0)
break;
if (hunk == RR_SIDE_1)
strbuf_addbuf(&one, &conflict);
else
strbuf_addbuf(&two, &conflict);
strbuf_release(&conflict);
} else if (is_cmarker(buf.buf, '|', marker_size)) {
if (hunk != RR_SIDE_1)
break;
hunk = RR_ORIGINAL;
} else if (is_cmarker(buf.buf, '=', marker_size)) {
if (hunk != RR_SIDE_1 && hunk != RR_ORIGINAL)
break;
hunk = RR_SIDE_2;
} else if (is_cmarker(buf.buf, '>', marker_size)) {
if (hunk != RR_SIDE_2)
break;
if (strbuf_cmp(&one, &two) > 0)
strbuf_swap(&one, &two);
has_conflicts = 1;
rerere_strbuf_putconflict(out, '<', marker_size);
strbuf_addbuf(out, &one);
rerere_strbuf_putconflict(out, '=', marker_size);
strbuf_addbuf(out, &two);
rerere_strbuf_putconflict(out, '>', marker_size);
if (ctx) {
the_hash_algo->update_fn(ctx, one.buf ?
one.buf : "",
one.len + 1);
the_hash_algo->update_fn(ctx, two.buf ?
two.buf : "",
two.len + 1);
}
break;
} else if (hunk == RR_SIDE_1)
strbuf_addbuf(&one, &buf);
else if (hunk == RR_ORIGINAL)
; 
else if (hunk == RR_SIDE_2)
strbuf_addbuf(&two, &buf);
}
strbuf_release(&one);
strbuf_release(&two);
strbuf_release(&buf);
return has_conflicts;
}
static int handle_path(unsigned char *hash, struct rerere_io *io, int marker_size)
{
git_hash_ctx ctx;
struct strbuf buf = STRBUF_INIT, out = STRBUF_INIT;
int has_conflicts = 0;
if (hash)
the_hash_algo->init_fn(&ctx);
while (!io->getline(&buf, io)) {
if (is_cmarker(buf.buf, '<', marker_size)) {
has_conflicts = handle_conflict(&out, io, marker_size,
hash ? &ctx : NULL);
if (has_conflicts < 0)
break;
rerere_io_putmem(out.buf, out.len, io);
strbuf_reset(&out);
} else
rerere_io_putstr(buf.buf, io);
}
strbuf_release(&buf);
strbuf_release(&out);
if (hash)
the_hash_algo->final_fn(hash, &ctx);
return has_conflicts;
}
static int handle_file(struct index_state *istate,
const char *path, unsigned char *hash, const char *output)
{
int has_conflicts = 0;
struct rerere_io_file io;
int marker_size = ll_merge_marker_size(istate, path);
memset(&io, 0, sizeof(io));
io.io.getline = rerere_file_getline;
io.input = fopen(path, "r");
io.io.wrerror = 0;
if (!io.input)
return error_errno(_("could not open '%s'"), path);
if (output) {
io.io.output = fopen(output, "w");
if (!io.io.output) {
error_errno(_("could not write '%s'"), output);
fclose(io.input);
return -1;
}
}
has_conflicts = handle_path(hash, (struct rerere_io *)&io, marker_size);
fclose(io.input);
if (io.io.wrerror)
error(_("there were errors while writing '%s' (%s)"),
path, strerror(io.io.wrerror));
if (io.io.output && fclose(io.io.output))
io.io.wrerror = error_errno(_("failed to flush '%s'"), path);
if (has_conflicts < 0) {
if (output)
unlink_or_warn(output);
return error(_("could not parse conflict hunks in '%s'"), path);
}
if (io.io.wrerror)
return -1;
return has_conflicts;
}
static int check_one_conflict(struct index_state *istate, int i, int *type)
{
const struct cache_entry *e = istate->cache[i];
if (!ce_stage(e)) {
*type = RESOLVED;
return i + 1;
}
*type = PUNTED;
while (i < istate->cache_nr && ce_stage(istate->cache[i]) == 1)
i++;
if (i + 1 < istate->cache_nr) {
const struct cache_entry *e2 = istate->cache[i];
const struct cache_entry *e3 = istate->cache[i + 1];
if (ce_stage(e2) == 2 &&
ce_stage(e3) == 3 &&
ce_same_name(e, e3) &&
S_ISREG(e2->ce_mode) &&
S_ISREG(e3->ce_mode))
*type = THREE_STAGED;
}
while (i < istate->cache_nr && ce_same_name(e, istate->cache[i]))
i++;
return i;
}
static int find_conflict(struct repository *r, struct string_list *conflict)
{
int i;
if (repo_read_index(r) < 0)
return error(_("index file corrupt"));
for (i = 0; i < r->index->cache_nr;) {
int conflict_type;
const struct cache_entry *e = r->index->cache[i];
i = check_one_conflict(r->index, i, &conflict_type);
if (conflict_type == THREE_STAGED)
string_list_insert(conflict, (const char *)e->name);
}
return 0;
}
int rerere_remaining(struct repository *r, struct string_list *merge_rr)
{
int i;
if (setup_rerere(r, merge_rr, RERERE_READONLY))
return 0;
if (repo_read_index(r) < 0)
return error(_("index file corrupt"));
for (i = 0; i < r->index->cache_nr;) {
int conflict_type;
const struct cache_entry *e = r->index->cache[i];
i = check_one_conflict(r->index, i, &conflict_type);
if (conflict_type == PUNTED)
string_list_insert(merge_rr, (const char *)e->name);
else if (conflict_type == RESOLVED) {
struct string_list_item *it;
it = string_list_lookup(merge_rr, (const char *)e->name);
if (it != NULL) {
free_rerere_id(it);
it->util = RERERE_RESOLVED;
}
}
}
return 0;
}
static int try_merge(struct index_state *istate,
const struct rerere_id *id, const char *path,
mmfile_t *cur, mmbuffer_t *result)
{
int ret;
mmfile_t base = {NULL, 0}, other = {NULL, 0};
if (read_mmfile(&base, rerere_path(id, "preimage")) ||
read_mmfile(&other, rerere_path(id, "postimage")))
ret = 1;
else
ret = ll_merge(result, path, &base, NULL, cur, "", &other, "",
istate, NULL);
free(base.ptr);
free(other.ptr);
return ret;
}
static int merge(struct index_state *istate, const struct rerere_id *id, const char *path)
{
FILE *f;
int ret;
mmfile_t cur = {NULL, 0};
mmbuffer_t result = {NULL, 0};
if ((handle_file(istate, path, NULL, rerere_path(id, "thisimage")) < 0) ||
read_mmfile(&cur, rerere_path(id, "thisimage"))) {
ret = 1;
goto out;
}
ret = try_merge(istate, id, path, &cur, &result);
if (ret)
goto out;
if (utime(rerere_path(id, "postimage"), NULL) < 0)
warning_errno(_("failed utime() on '%s'"),
rerere_path(id, "postimage"));
f = fopen(path, "w");
if (!f)
return error_errno(_("could not open '%s'"), path);
if (fwrite(result.ptr, result.size, 1, f) != 1)
error_errno(_("could not write '%s'"), path);
if (fclose(f))
return error_errno(_("writing '%s' failed"), path);
out:
free(cur.ptr);
free(result.ptr);
return ret;
}
static void update_paths(struct repository *r, struct string_list *update)
{
struct lock_file index_lock = LOCK_INIT;
int i;
repo_hold_locked_index(r, &index_lock, LOCK_DIE_ON_ERROR);
for (i = 0; i < update->nr; i++) {
struct string_list_item *item = &update->items[i];
if (add_file_to_index(r->index, item->string, 0))
exit(128);
fprintf_ln(stderr, _("Staged '%s' using previous resolution."),
item->string);
}
if (write_locked_index(r->index, &index_lock,
COMMIT_LOCK | SKIP_IF_UNCHANGED))
die(_("unable to write new index file"));
}
static void remove_variant(struct rerere_id *id)
{
unlink_or_warn(rerere_path(id, "postimage"));
unlink_or_warn(rerere_path(id, "preimage"));
id->collection->status[id->variant] = 0;
}
static void do_rerere_one_path(struct index_state *istate,
struct string_list_item *rr_item,
struct string_list *update)
{
const char *path = rr_item->string;
struct rerere_id *id = rr_item->util;
struct rerere_dir *rr_dir = id->collection;
int variant;
variant = id->variant;
if (variant >= 0) {
if (!handle_file(istate, path, NULL, NULL)) {
copy_file(rerere_path(id, "postimage"), path, 0666);
id->collection->status[variant] |= RR_HAS_POSTIMAGE;
fprintf_ln(stderr, _("Recorded resolution for '%s'."), path);
free_rerere_id(rr_item);
rr_item->util = NULL;
return;
}
}
for (variant = 0; variant < rr_dir->status_nr; variant++) {
const int both = RR_HAS_PREIMAGE | RR_HAS_POSTIMAGE;
struct rerere_id vid = *id;
if ((rr_dir->status[variant] & both) != both)
continue;
vid.variant = variant;
if (merge(istate, &vid, path))
continue; 
if (0 <= id->variant && id->variant != variant)
remove_variant(id);
if (rerere_autoupdate)
string_list_insert(update, path);
else
fprintf_ln(stderr,
_("Resolved '%s' using previous resolution."),
path);
free_rerere_id(rr_item);
rr_item->util = NULL;
return;
}
assign_variant(id);
variant = id->variant;
handle_file(istate, path, NULL, rerere_path(id, "preimage"));
if (id->collection->status[variant] & RR_HAS_POSTIMAGE) {
const char *path = rerere_path(id, "postimage");
if (unlink(path))
die_errno(_("cannot unlink stray '%s'"), path);
id->collection->status[variant] &= ~RR_HAS_POSTIMAGE;
}
id->collection->status[variant] |= RR_HAS_PREIMAGE;
fprintf_ln(stderr, _("Recorded preimage for '%s'"), path);
}
static int do_plain_rerere(struct repository *r,
struct string_list *rr, int fd)
{
struct string_list conflict = STRING_LIST_INIT_DUP;
struct string_list update = STRING_LIST_INIT_DUP;
int i;
find_conflict(r, &conflict);
for (i = 0; i < conflict.nr; i++) {
struct rerere_id *id;
unsigned char hash[GIT_MAX_RAWSZ];
const char *path = conflict.items[i].string;
int ret;
ret = handle_file(r->index, path, hash, NULL);
if (ret != 0 && string_list_has_string(rr, path)) {
remove_variant(string_list_lookup(rr, path)->util);
string_list_remove(rr, path, 1);
}
if (ret < 1)
continue;
id = new_rerere_id(hash);
string_list_insert(rr, path)->util = id;
mkdir_in_gitdir(rerere_path(id, NULL));
}
for (i = 0; i < rr->nr; i++)
do_rerere_one_path(r->index, &rr->items[i], &update);
if (update.nr)
update_paths(r, &update);
return write_rr(rr, fd);
}
static void git_rerere_config(void)
{
git_config_get_bool("rerere.enabled", &rerere_enabled);
git_config_get_bool("rerere.autoupdate", &rerere_autoupdate);
git_config(git_default_config, NULL);
}
static GIT_PATH_FUNC(git_path_rr_cache, "rr-cache")
static int is_rerere_enabled(void)
{
int rr_cache_exists;
if (!rerere_enabled)
return 0;
rr_cache_exists = is_directory(git_path_rr_cache());
if (rerere_enabled < 0)
return rr_cache_exists;
if (!rr_cache_exists && mkdir_in_gitdir(git_path_rr_cache()))
die(_("could not create directory '%s'"), git_path_rr_cache());
return 1;
}
int setup_rerere(struct repository *r, struct string_list *merge_rr, int flags)
{
int fd;
git_rerere_config();
if (!is_rerere_enabled())
return -1;
if (flags & (RERERE_AUTOUPDATE|RERERE_NOAUTOUPDATE))
rerere_autoupdate = !!(flags & RERERE_AUTOUPDATE);
if (flags & RERERE_READONLY)
fd = 0;
else
fd = hold_lock_file_for_update(&write_lock,
git_path_merge_rr(r),
LOCK_DIE_ON_ERROR);
read_rr(r, merge_rr);
return fd;
}
int repo_rerere(struct repository *r, int flags)
{
struct string_list merge_rr = STRING_LIST_INIT_DUP;
int fd, status;
fd = setup_rerere(r, &merge_rr, flags);
if (fd < 0)
return 0;
status = do_plain_rerere(r, &merge_rr, fd);
free_rerere_dirs();
return status;
}
struct rerere_io_mem {
struct rerere_io io;
struct strbuf input;
};
static int rerere_mem_getline(struct strbuf *sb, struct rerere_io *io_)
{
struct rerere_io_mem *io = (struct rerere_io_mem *)io_;
char *ep;
size_t len;
strbuf_release(sb);
if (!io->input.len)
return -1;
ep = memchr(io->input.buf, '\n', io->input.len);
if (!ep)
ep = io->input.buf + io->input.len;
else if (*ep == '\n')
ep++;
len = ep - io->input.buf;
strbuf_add(sb, io->input.buf, len);
strbuf_remove(&io->input, 0, len);
return 0;
}
static int handle_cache(struct index_state *istate,
const char *path, unsigned char *hash, const char *output)
{
mmfile_t mmfile[3] = {{NULL}};
mmbuffer_t result = {NULL, 0};
const struct cache_entry *ce;
int pos, len, i, has_conflicts;
struct rerere_io_mem io;
int marker_size = ll_merge_marker_size(istate, path);
len = strlen(path);
pos = index_name_pos(istate, path, len);
if (0 <= pos)
return -1;
pos = -pos - 1;
while (pos < istate->cache_nr) {
enum object_type type;
unsigned long size;
ce = istate->cache[pos++];
if (ce_namelen(ce) != len || memcmp(ce->name, path, len))
break;
i = ce_stage(ce) - 1;
if (!mmfile[i].ptr) {
mmfile[i].ptr = read_object_file(&ce->oid, &type,
&size);
mmfile[i].size = size;
}
}
for (i = 0; i < 3; i++)
if (!mmfile[i].ptr && !mmfile[i].size)
mmfile[i].ptr = xstrdup("");
ll_merge(&result, path, &mmfile[0], NULL,
&mmfile[1], "ours",
&mmfile[2], "theirs",
istate, NULL);
for (i = 0; i < 3; i++)
free(mmfile[i].ptr);
memset(&io, 0, sizeof(io));
io.io.getline = rerere_mem_getline;
if (output)
io.io.output = fopen(output, "w");
else
io.io.output = NULL;
strbuf_init(&io.input, 0);
strbuf_attach(&io.input, result.ptr, result.size, result.size);
has_conflicts = handle_path(hash, (struct rerere_io *)&io, marker_size);
strbuf_release(&io.input);
if (io.io.output)
fclose(io.io.output);
return has_conflicts;
}
static int rerere_forget_one_path(struct index_state *istate,
const char *path,
struct string_list *rr)
{
const char *filename;
struct rerere_id *id;
unsigned char hash[GIT_MAX_RAWSZ];
int ret;
struct string_list_item *item;
ret = handle_cache(istate, path, hash, NULL);
if (ret < 1)
return error(_("could not parse conflict hunks in '%s'"), path);
id = new_rerere_id(hash);
for (id->variant = 0;
id->variant < id->collection->status_nr;
id->variant++) {
mmfile_t cur = { NULL, 0 };
mmbuffer_t result = {NULL, 0};
int cleanly_resolved;
if (!has_rerere_resolution(id))
continue;
handle_cache(istate, path, hash, rerere_path(id, "thisimage"));
if (read_mmfile(&cur, rerere_path(id, "thisimage"))) {
free(cur.ptr);
error(_("failed to update conflicted state in '%s'"), path);
goto fail_exit;
}
cleanly_resolved = !try_merge(istate, id, path, &cur, &result);
free(result.ptr);
free(cur.ptr);
if (cleanly_resolved)
break;
}
if (id->collection->status_nr <= id->variant) {
error(_("no remembered resolution for '%s'"), path);
goto fail_exit;
}
filename = rerere_path(id, "postimage");
if (unlink(filename)) {
if (errno == ENOENT)
error(_("no remembered resolution for '%s'"), path);
else
error_errno(_("cannot unlink '%s'"), filename);
goto fail_exit;
}
handle_cache(istate, path, hash, rerere_path(id, "preimage"));
fprintf_ln(stderr, _("Updated preimage for '%s'"), path);
item = string_list_insert(rr, path);
free_rerere_id(item);
item->util = id;
fprintf(stderr, _("Forgot resolution for '%s'\n"), path);
return 0;
fail_exit:
free(id);
return -1;
}
int rerere_forget(struct repository *r, struct pathspec *pathspec)
{
int i, fd;
struct string_list conflict = STRING_LIST_INIT_DUP;
struct string_list merge_rr = STRING_LIST_INIT_DUP;
if (repo_read_index(r) < 0)
return error(_("index file corrupt"));
fd = setup_rerere(r, &merge_rr, RERERE_NOAUTOUPDATE);
if (fd < 0)
return 0;
unmerge_index(r->index, pathspec);
find_conflict(r, &conflict);
for (i = 0; i < conflict.nr; i++) {
struct string_list_item *it = &conflict.items[i];
if (!match_pathspec(r->index, pathspec, it->string,
strlen(it->string), 0, NULL, 0))
continue;
rerere_forget_one_path(r->index, it->string, &merge_rr);
}
return write_rr(&merge_rr, fd);
}
static timestamp_t rerere_created_at(struct rerere_id *id)
{
struct stat st;
return stat(rerere_path(id, "preimage"), &st) ? (time_t) 0 : st.st_mtime;
}
static timestamp_t rerere_last_used_at(struct rerere_id *id)
{
struct stat st;
return stat(rerere_path(id, "postimage"), &st) ? (time_t) 0 : st.st_mtime;
}
static void unlink_rr_item(struct rerere_id *id)
{
unlink_or_warn(rerere_path(id, "thisimage"));
remove_variant(id);
id->collection->status[id->variant] = 0;
}
static void prune_one(struct rerere_id *id,
timestamp_t cutoff_resolve, timestamp_t cutoff_noresolve)
{
timestamp_t then;
timestamp_t cutoff;
then = rerere_last_used_at(id);
if (then)
cutoff = cutoff_resolve;
else {
then = rerere_created_at(id);
if (!then)
return;
cutoff = cutoff_noresolve;
}
if (then < cutoff)
unlink_rr_item(id);
}
void rerere_gc(struct repository *r, struct string_list *rr)
{
struct string_list to_remove = STRING_LIST_INIT_DUP;
DIR *dir;
struct dirent *e;
int i;
timestamp_t now = time(NULL);
timestamp_t cutoff_noresolve = now - 15 * 86400;
timestamp_t cutoff_resolve = now - 60 * 86400;
if (setup_rerere(r, rr, 0) < 0)
return;
git_config_get_expiry_in_days("gc.rerereresolved", &cutoff_resolve, now);
git_config_get_expiry_in_days("gc.rerereunresolved", &cutoff_noresolve, now);
git_config(git_default_config, NULL);
dir = opendir(git_path("rr-cache"));
if (!dir)
die_errno(_("unable to open rr-cache directory"));
while ((e = readdir(dir))) {
struct rerere_dir *rr_dir;
struct rerere_id id;
int now_empty;
if (is_dot_or_dotdot(e->d_name))
continue;
rr_dir = find_rerere_dir(e->d_name);
if (!rr_dir)
continue; 
now_empty = 1;
for (id.variant = 0, id.collection = rr_dir;
id.variant < id.collection->status_nr;
id.variant++) {
prune_one(&id, cutoff_resolve, cutoff_noresolve);
if (id.collection->status[id.variant])
now_empty = 0;
}
if (now_empty)
string_list_append(&to_remove, e->d_name);
}
closedir(dir);
for (i = 0; i < to_remove.nr; i++)
rmdir(git_path("rr-cache/%s", to_remove.items[i].string));
string_list_clear(&to_remove, 0);
rollback_lock_file(&write_lock);
}
void rerere_clear(struct repository *r, struct string_list *merge_rr)
{
int i;
if (setup_rerere(r, merge_rr, 0) < 0)
return;
for (i = 0; i < merge_rr->nr; i++) {
struct rerere_id *id = merge_rr->items[i].util;
if (!has_rerere_resolution(id)) {
unlink_rr_item(id);
rmdir(rerere_path(id, NULL));
}
}
unlink_or_warn(git_path_merge_rr(r));
rollback_lock_file(&write_lock);
}
