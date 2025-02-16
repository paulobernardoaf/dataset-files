#include "cache.h"
#include "list.h"
#include "pack.h"
#include "repository.h"
#include "dir.h"
#include "mergesort.h"
#include "packfile.h"
#include "delta.h"
#include "streaming.h"
#include "sha1-lookup.h"
#include "commit.h"
#include "object.h"
#include "tag.h"
#include "tree-walk.h"
#include "tree.h"
#include "object-store.h"
#include "midx.h"
#include "commit-graph.h"
#include "promisor-remote.h"
char *odb_pack_name(struct strbuf *buf,
const unsigned char *hash,
const char *ext)
{
strbuf_reset(buf);
strbuf_addf(buf, "%s/pack/pack-%s.%s", get_object_directory(),
hash_to_hex(hash), ext);
return buf->buf;
}
char *sha1_pack_name(const unsigned char *sha1)
{
static struct strbuf buf = STRBUF_INIT;
return odb_pack_name(&buf, sha1, "pack");
}
char *sha1_pack_index_name(const unsigned char *sha1)
{
static struct strbuf buf = STRBUF_INIT;
return odb_pack_name(&buf, sha1, "idx");
}
static unsigned int pack_used_ctr;
static unsigned int pack_mmap_calls;
static unsigned int peak_pack_open_windows;
static unsigned int pack_open_windows;
static unsigned int pack_open_fds;
static unsigned int pack_max_fds;
static size_t peak_pack_mapped;
static size_t pack_mapped;
#define SZ_FMT PRIuMAX
static inline uintmax_t sz_fmt(size_t s) { return s; }
void pack_report(void)
{
fprintf(stderr,
"pack_report: getpagesize() = %10" SZ_FMT "\n"
"pack_report: core.packedGitWindowSize = %10" SZ_FMT "\n"
"pack_report: core.packedGitLimit = %10" SZ_FMT "\n",
sz_fmt(getpagesize()),
sz_fmt(packed_git_window_size),
sz_fmt(packed_git_limit));
fprintf(stderr,
"pack_report: pack_used_ctr = %10u\n"
"pack_report: pack_mmap_calls = %10u\n"
"pack_report: pack_open_windows = %10u / %10u\n"
"pack_report: pack_mapped = "
"%10" SZ_FMT " / %10" SZ_FMT "\n",
pack_used_ctr,
pack_mmap_calls,
pack_open_windows, peak_pack_open_windows,
sz_fmt(pack_mapped), sz_fmt(peak_pack_mapped));
}
static int check_packed_git_idx(const char *path, struct packed_git *p)
{
void *idx_map;
size_t idx_size;
int fd = git_open(path), ret;
struct stat st;
const unsigned int hashsz = the_hash_algo->rawsz;
if (fd < 0)
return -1;
if (fstat(fd, &st)) {
close(fd);
return -1;
}
idx_size = xsize_t(st.st_size);
if (idx_size < 4 * 256 + hashsz + hashsz) {
close(fd);
return error("index file %s is too small", path);
}
idx_map = xmmap(NULL, idx_size, PROT_READ, MAP_PRIVATE, fd, 0);
close(fd);
ret = load_idx(path, hashsz, idx_map, idx_size, p);
if (ret)
munmap(idx_map, idx_size);
return ret;
}
int load_idx(const char *path, const unsigned int hashsz, void *idx_map,
size_t idx_size, struct packed_git *p)
{
struct pack_idx_header *hdr = idx_map;
uint32_t version, nr, i, *index;
if (idx_size < 4 * 256 + hashsz + hashsz)
return error("index file %s is too small", path);
if (idx_map == NULL)
return error("empty data");
if (hdr->idx_signature == htonl(PACK_IDX_SIGNATURE)) {
version = ntohl(hdr->idx_version);
if (version < 2 || version > 2)
return error("index file %s is version %"PRIu32
" and is not supported by this binary"
" (try upgrading GIT to a newer version)",
path, version);
} else
version = 1;
nr = 0;
index = idx_map;
if (version > 1)
index += 2; 
for (i = 0; i < 256; i++) {
uint32_t n = ntohl(index[i]);
if (n < nr)
return error("non-monotonic index %s", path);
nr = n;
}
if (version == 1) {
if (idx_size != 4 * 256 + nr * (hashsz + 4) + hashsz + hashsz)
return error("wrong index v1 file size in %s", path);
} else if (version == 2) {
unsigned long min_size = 8 + 4*256 + nr*(hashsz + 4 + 4) + hashsz + hashsz;
unsigned long max_size = min_size;
if (nr)
max_size += (nr - 1)*8;
if (idx_size < min_size || idx_size > max_size)
return error("wrong index v2 file size in %s", path);
if (idx_size != min_size &&
(sizeof(off_t) <= 4))
return error("pack too large for current definition of off_t in %s", path);
}
p->index_version = version;
p->index_data = idx_map;
p->index_size = idx_size;
p->num_objects = nr;
return 0;
}
int open_pack_index(struct packed_git *p)
{
char *idx_name;
size_t len;
int ret;
if (p->index_data)
return 0;
if (!strip_suffix(p->pack_name, ".pack", &len))
BUG("pack_name does not end in .pack");
idx_name = xstrfmt("%.*s.idx", (int)len, p->pack_name);
ret = check_packed_git_idx(idx_name, p);
free(idx_name);
return ret;
}
uint32_t get_pack_fanout(struct packed_git *p, uint32_t value)
{
const uint32_t *level1_ofs = p->index_data;
if (!level1_ofs) {
if (open_pack_index(p))
return 0;
level1_ofs = p->index_data;
}
if (p->index_version > 1) {
level1_ofs += 2;
}
return ntohl(level1_ofs[value]);
}
static struct packed_git *alloc_packed_git(int extra)
{
struct packed_git *p = xmalloc(st_add(sizeof(*p), extra));
memset(p, 0, sizeof(*p));
p->pack_fd = -1;
return p;
}
struct packed_git *parse_pack_index(unsigned char *sha1, const char *idx_path)
{
const char *path = sha1_pack_name(sha1);
size_t alloc = st_add(strlen(path), 1);
struct packed_git *p = alloc_packed_git(alloc);
memcpy(p->pack_name, path, alloc); 
hashcpy(p->hash, sha1);
if (check_packed_git_idx(idx_path, p)) {
free(p);
return NULL;
}
return p;
}
static void scan_windows(struct packed_git *p,
struct packed_git **lru_p,
struct pack_window **lru_w,
struct pack_window **lru_l)
{
struct pack_window *w, *w_l;
for (w_l = NULL, w = p->windows; w; w = w->next) {
if (!w->inuse_cnt) {
if (!*lru_w || w->last_used < (*lru_w)->last_used) {
*lru_p = p;
*lru_w = w;
*lru_l = w_l;
}
}
w_l = w;
}
}
static int unuse_one_window(struct packed_git *current)
{
struct packed_git *p, *lru_p = NULL;
struct pack_window *lru_w = NULL, *lru_l = NULL;
if (current)
scan_windows(current, &lru_p, &lru_w, &lru_l);
for (p = the_repository->objects->packed_git; p; p = p->next)
scan_windows(p, &lru_p, &lru_w, &lru_l);
if (lru_p) {
munmap(lru_w->base, lru_w->len);
pack_mapped -= lru_w->len;
if (lru_l)
lru_l->next = lru_w->next;
else
lru_p->windows = lru_w->next;
free(lru_w);
pack_open_windows--;
return 1;
}
return 0;
}
void close_pack_windows(struct packed_git *p)
{
while (p->windows) {
struct pack_window *w = p->windows;
if (w->inuse_cnt)
die("pack '%s' still has open windows to it",
p->pack_name);
munmap(w->base, w->len);
pack_mapped -= w->len;
pack_open_windows--;
p->windows = w->next;
free(w);
}
}
int close_pack_fd(struct packed_git *p)
{
if (p->pack_fd < 0)
return 0;
close(p->pack_fd);
pack_open_fds--;
p->pack_fd = -1;
return 1;
}
void close_pack_index(struct packed_git *p)
{
if (p->index_data) {
munmap((void *)p->index_data, p->index_size);
p->index_data = NULL;
}
}
void close_pack(struct packed_git *p)
{
close_pack_windows(p);
close_pack_fd(p);
close_pack_index(p);
}
void close_object_store(struct raw_object_store *o)
{
struct packed_git *p;
for (p = o->packed_git; p; p = p->next)
if (p->do_not_close)
BUG("want to close pack marked 'do-not-close'");
else
close_pack(p);
if (o->multi_pack_index) {
close_midx(o->multi_pack_index);
o->multi_pack_index = NULL;
}
close_commit_graph(o);
}
void unlink_pack_path(const char *pack_name, int force_delete)
{
static const char *exts[] = {".pack", ".idx", ".keep", ".bitmap", ".promisor"};
int i;
struct strbuf buf = STRBUF_INIT;
size_t plen;
strbuf_addstr(&buf, pack_name);
strip_suffix_mem(buf.buf, &buf.len, ".pack");
plen = buf.len;
if (!force_delete) {
strbuf_addstr(&buf, ".keep");
if (!access(buf.buf, F_OK)) {
strbuf_release(&buf);
return;
}
}
for (i = 0; i < ARRAY_SIZE(exts); i++) {
strbuf_setlen(&buf, plen);
strbuf_addstr(&buf, exts[i]);
unlink(buf.buf);
}
strbuf_release(&buf);
}
static void find_lru_pack(struct packed_git *p, struct packed_git **lru_p, struct pack_window **mru_w, int *accept_windows_inuse)
{
struct pack_window *w, *this_mru_w;
int has_windows_inuse = 0;
if (*lru_p && !*mru_w && (p->windows || p->mtime > (*lru_p)->mtime))
return;
for (w = this_mru_w = p->windows; w; w = w->next) {
if (w->inuse_cnt) {
if (*accept_windows_inuse)
has_windows_inuse = 1;
else
return;
}
if (w->last_used > this_mru_w->last_used)
this_mru_w = w;
if (*mru_w && *accept_windows_inuse == has_windows_inuse &&
this_mru_w->last_used > (*mru_w)->last_used)
return;
}
*mru_w = this_mru_w;
*lru_p = p;
*accept_windows_inuse = has_windows_inuse;
}
static int close_one_pack(void)
{
struct packed_git *p, *lru_p = NULL;
struct pack_window *mru_w = NULL;
int accept_windows_inuse = 1;
for (p = the_repository->objects->packed_git; p; p = p->next) {
if (p->pack_fd == -1)
continue;
find_lru_pack(p, &lru_p, &mru_w, &accept_windows_inuse);
}
if (lru_p)
return close_pack_fd(lru_p);
return 0;
}
static unsigned int get_max_fd_limit(void)
{
#if defined(RLIMIT_NOFILE)
{
struct rlimit lim;
if (!getrlimit(RLIMIT_NOFILE, &lim))
return lim.rlim_cur;
}
#endif
#if defined(_SC_OPEN_MAX)
{
long open_max = sysconf(_SC_OPEN_MAX);
if (0 < open_max)
return open_max;
}
#endif
#if defined(OPEN_MAX)
return OPEN_MAX;
#else
return 1; 
#endif
}
const char *pack_basename(struct packed_git *p)
{
const char *ret = strrchr(p->pack_name, '/');
if (ret)
ret = ret + 1; 
else
ret = p->pack_name; 
return ret;
}
static int open_packed_git_1(struct packed_git *p)
{
struct stat st;
struct pack_header hdr;
unsigned char hash[GIT_MAX_RAWSZ];
unsigned char *idx_hash;
ssize_t read_result;
const unsigned hashsz = the_hash_algo->rawsz;
if (!p->index_data) {
struct multi_pack_index *m;
const char *pack_name = pack_basename(p);
for (m = the_repository->objects->multi_pack_index;
m; m = m->next) {
if (midx_contains_pack(m, pack_name))
break;
}
if (!m && open_pack_index(p))
return error("packfile %s index unavailable", p->pack_name);
}
if (!pack_max_fds) {
unsigned int max_fds = get_max_fd_limit();
if (25 < max_fds)
pack_max_fds = max_fds - 25;
else
pack_max_fds = 1;
}
while (pack_max_fds <= pack_open_fds && close_one_pack())
; 
p->pack_fd = git_open(p->pack_name);
if (p->pack_fd < 0 || fstat(p->pack_fd, &st))
return -1;
pack_open_fds++;
if (!p->pack_size) {
if (!S_ISREG(st.st_mode))
return error("packfile %s not a regular file", p->pack_name);
p->pack_size = st.st_size;
} else if (p->pack_size != st.st_size)
return error("packfile %s size changed", p->pack_name);
read_result = read_in_full(p->pack_fd, &hdr, sizeof(hdr));
if (read_result < 0)
return error_errno("error reading from %s", p->pack_name);
if (read_result != sizeof(hdr))
return error("file %s is far too short to be a packfile", p->pack_name);
if (hdr.hdr_signature != htonl(PACK_SIGNATURE))
return error("file %s is not a GIT packfile", p->pack_name);
if (!pack_version_ok(hdr.hdr_version))
return error("packfile %s is version %"PRIu32" and not"
" supported (try upgrading GIT to a newer version)",
p->pack_name, ntohl(hdr.hdr_version));
if (!p->index_data)
return 0;
if (p->num_objects != ntohl(hdr.hdr_entries))
return error("packfile %s claims to have %"PRIu32" objects"
" while index indicates %"PRIu32" objects",
p->pack_name, ntohl(hdr.hdr_entries),
p->num_objects);
read_result = pread_in_full(p->pack_fd, hash, hashsz,
p->pack_size - hashsz);
if (read_result < 0)
return error_errno("error reading from %s", p->pack_name);
if (read_result != hashsz)
return error("packfile %s signature is unavailable", p->pack_name);
idx_hash = ((unsigned char *)p->index_data) + p->index_size - hashsz * 2;
if (!hasheq(hash, idx_hash))
return error("packfile %s does not match index", p->pack_name);
return 0;
}
static int open_packed_git(struct packed_git *p)
{
if (!open_packed_git_1(p))
return 0;
close_pack_fd(p);
return -1;
}
static int in_window(struct pack_window *win, off_t offset)
{
off_t win_off = win->offset;
return win_off <= offset
&& (offset + the_hash_algo->rawsz) <= (win_off + win->len);
}
unsigned char *use_pack(struct packed_git *p,
struct pack_window **w_cursor,
off_t offset,
unsigned long *left)
{
struct pack_window *win = *w_cursor;
if (!p->pack_size && p->pack_fd == -1 && open_packed_git(p))
die("packfile %s cannot be accessed", p->pack_name);
if (offset > (p->pack_size - the_hash_algo->rawsz))
die("offset beyond end of packfile (truncated pack?)");
if (offset < 0)
die(_("offset before end of packfile (broken .idx?)"));
if (!win || !in_window(win, offset)) {
if (win)
win->inuse_cnt--;
for (win = p->windows; win; win = win->next) {
if (in_window(win, offset))
break;
}
if (!win) {
size_t window_align = packed_git_window_size / 2;
off_t len;
if (p->pack_fd == -1 && open_packed_git(p))
die("packfile %s cannot be accessed", p->pack_name);
win = xcalloc(1, sizeof(*win));
win->offset = (offset / window_align) * window_align;
len = p->pack_size - win->offset;
if (len > packed_git_window_size)
len = packed_git_window_size;
win->len = (size_t)len;
pack_mapped += win->len;
while (packed_git_limit < pack_mapped
&& unuse_one_window(p))
; 
win->base = xmmap_gently(NULL, win->len,
PROT_READ, MAP_PRIVATE,
p->pack_fd, win->offset);
if (win->base == MAP_FAILED)
die_errno("packfile %s cannot be mapped",
p->pack_name);
if (!win->offset && win->len == p->pack_size
&& !p->do_not_close)
close_pack_fd(p);
pack_mmap_calls++;
pack_open_windows++;
if (pack_mapped > peak_pack_mapped)
peak_pack_mapped = pack_mapped;
if (pack_open_windows > peak_pack_open_windows)
peak_pack_open_windows = pack_open_windows;
win->next = p->windows;
p->windows = win;
}
}
if (win != *w_cursor) {
win->last_used = pack_used_ctr++;
win->inuse_cnt++;
*w_cursor = win;
}
offset -= win->offset;
if (left)
*left = win->len - xsize_t(offset);
return win->base + offset;
}
void unuse_pack(struct pack_window **w_cursor)
{
struct pack_window *w = *w_cursor;
if (w) {
w->inuse_cnt--;
*w_cursor = NULL;
}
}
struct packed_git *add_packed_git(const char *path, size_t path_len, int local)
{
struct stat st;
size_t alloc;
struct packed_git *p;
if (!strip_suffix_mem(path, &path_len, ".idx"))
return NULL;
alloc = st_add3(path_len, strlen(".promisor"), 1);
p = alloc_packed_git(alloc);
memcpy(p->pack_name, path, path_len);
xsnprintf(p->pack_name + path_len, alloc - path_len, ".keep");
if (!access(p->pack_name, F_OK))
p->pack_keep = 1;
xsnprintf(p->pack_name + path_len, alloc - path_len, ".promisor");
if (!access(p->pack_name, F_OK))
p->pack_promisor = 1;
xsnprintf(p->pack_name + path_len, alloc - path_len, ".pack");
if (stat(p->pack_name, &st) || !S_ISREG(st.st_mode)) {
free(p);
return NULL;
}
p->pack_size = st.st_size;
p->pack_local = local;
p->mtime = st.st_mtime;
if (path_len < the_hash_algo->hexsz ||
get_sha1_hex(path + path_len - the_hash_algo->hexsz, p->hash))
hashclr(p->hash);
return p;
}
void install_packed_git(struct repository *r, struct packed_git *pack)
{
if (pack->pack_fd != -1)
pack_open_fds++;
pack->next = r->objects->packed_git;
r->objects->packed_git = pack;
hashmap_entry_init(&pack->packmap_ent, strhash(pack->pack_name));
hashmap_add(&r->objects->pack_map, &pack->packmap_ent);
}
void (*report_garbage)(unsigned seen_bits, const char *path);
static void report_helper(const struct string_list *list,
int seen_bits, int first, int last)
{
if (seen_bits == (PACKDIR_FILE_PACK|PACKDIR_FILE_IDX))
return;
for (; first < last; first++)
report_garbage(seen_bits, list->items[first].string);
}
static void report_pack_garbage(struct string_list *list)
{
int i, baselen = -1, first = 0, seen_bits = 0;
if (!report_garbage)
return;
string_list_sort(list);
for (i = 0; i < list->nr; i++) {
const char *path = list->items[i].string;
if (baselen != -1 &&
strncmp(path, list->items[first].string, baselen)) {
report_helper(list, seen_bits, first, i);
baselen = -1;
seen_bits = 0;
}
if (baselen == -1) {
const char *dot = strrchr(path, '.');
if (!dot) {
report_garbage(PACKDIR_FILE_GARBAGE, path);
continue;
}
baselen = dot - path + 1;
first = i;
}
if (!strcmp(path + baselen, "pack"))
seen_bits |= 1;
else if (!strcmp(path + baselen, "idx"))
seen_bits |= 2;
}
report_helper(list, seen_bits, first, list->nr);
}
void for_each_file_in_pack_dir(const char *objdir,
each_file_in_pack_dir_fn fn,
void *data)
{
struct strbuf path = STRBUF_INIT;
size_t dirnamelen;
DIR *dir;
struct dirent *de;
strbuf_addstr(&path, objdir);
strbuf_addstr(&path, "/pack");
dir = opendir(path.buf);
if (!dir) {
if (errno != ENOENT)
error_errno("unable to open object pack directory: %s",
path.buf);
strbuf_release(&path);
return;
}
strbuf_addch(&path, '/');
dirnamelen = path.len;
while ((de = readdir(dir)) != NULL) {
if (is_dot_or_dotdot(de->d_name))
continue;
strbuf_setlen(&path, dirnamelen);
strbuf_addstr(&path, de->d_name);
fn(path.buf, path.len, de->d_name, data);
}
closedir(dir);
strbuf_release(&path);
}
struct prepare_pack_data {
struct repository *r;
struct string_list *garbage;
int local;
struct multi_pack_index *m;
};
static void prepare_pack(const char *full_name, size_t full_name_len,
const char *file_name, void *_data)
{
struct prepare_pack_data *data = (struct prepare_pack_data *)_data;
struct packed_git *p;
size_t base_len = full_name_len;
if (strip_suffix_mem(full_name, &base_len, ".idx") &&
!(data->m && midx_contains_pack(data->m, file_name))) {
struct hashmap_entry hent;
char *pack_name = xstrfmt("%.*s.pack", (int)base_len, full_name);
unsigned int hash = strhash(pack_name);
hashmap_entry_init(&hent, hash);
if (!hashmap_get(&data->r->objects->pack_map, &hent, pack_name)) {
p = add_packed_git(full_name, full_name_len, data->local);
if (p)
install_packed_git(data->r, p);
}
free(pack_name);
}
if (!report_garbage)
return;
if (!strcmp(file_name, "multi-pack-index"))
return;
if (ends_with(file_name, ".idx") ||
ends_with(file_name, ".pack") ||
ends_with(file_name, ".bitmap") ||
ends_with(file_name, ".keep") ||
ends_with(file_name, ".promisor"))
string_list_append(data->garbage, full_name);
else
report_garbage(PACKDIR_FILE_GARBAGE, full_name);
}
static void prepare_packed_git_one(struct repository *r, char *objdir, int local)
{
struct prepare_pack_data data;
struct string_list garbage = STRING_LIST_INIT_DUP;
data.m = r->objects->multi_pack_index;
while (data.m && strcmp(data.m->object_dir, objdir))
data.m = data.m->next;
data.r = r;
data.garbage = &garbage;
data.local = local;
for_each_file_in_pack_dir(objdir, prepare_pack, &data);
report_pack_garbage(data.garbage);
string_list_clear(data.garbage, 0);
}
static void prepare_packed_git(struct repository *r);
unsigned long repo_approximate_object_count(struct repository *r)
{
if (!r->objects->approximate_object_count_valid) {
unsigned long count;
struct multi_pack_index *m;
struct packed_git *p;
prepare_packed_git(r);
count = 0;
for (m = get_multi_pack_index(r); m; m = m->next)
count += m->num_objects;
for (p = r->objects->packed_git; p; p = p->next) {
if (open_pack_index(p))
continue;
count += p->num_objects;
}
r->objects->approximate_object_count = count;
}
return r->objects->approximate_object_count;
}
static void *get_next_packed_git(const void *p)
{
return ((const struct packed_git *)p)->next;
}
static void set_next_packed_git(void *p, void *next)
{
((struct packed_git *)p)->next = next;
}
static int sort_pack(const void *a_, const void *b_)
{
const struct packed_git *a = a_;
const struct packed_git *b = b_;
int st;
st = a->pack_local - b->pack_local;
if (st)
return -st;
if (a->mtime < b->mtime)
return 1;
else if (a->mtime == b->mtime)
return 0;
return -1;
}
static void rearrange_packed_git(struct repository *r)
{
r->objects->packed_git = llist_mergesort(
r->objects->packed_git, get_next_packed_git,
set_next_packed_git, sort_pack);
}
static void prepare_packed_git_mru(struct repository *r)
{
struct packed_git *p;
INIT_LIST_HEAD(&r->objects->packed_git_mru);
for (p = r->objects->packed_git; p; p = p->next)
list_add_tail(&p->mru, &r->objects->packed_git_mru);
}
static void prepare_packed_git(struct repository *r)
{
struct object_directory *odb;
if (r->objects->packed_git_initialized)
return;
prepare_alt_odb(r);
for (odb = r->objects->odb; odb; odb = odb->next) {
int local = (odb == r->objects->odb);
prepare_multi_pack_index_one(r, odb->path, local);
prepare_packed_git_one(r, odb->path, local);
}
rearrange_packed_git(r);
prepare_packed_git_mru(r);
r->objects->packed_git_initialized = 1;
}
void reprepare_packed_git(struct repository *r)
{
struct object_directory *odb;
obj_read_lock();
for (odb = r->objects->odb; odb; odb = odb->next)
odb_clear_loose_cache(odb);
r->objects->approximate_object_count_valid = 0;
r->objects->packed_git_initialized = 0;
prepare_packed_git(r);
obj_read_unlock();
}
struct packed_git *get_packed_git(struct repository *r)
{
prepare_packed_git(r);
return r->objects->packed_git;
}
struct multi_pack_index *get_multi_pack_index(struct repository *r)
{
prepare_packed_git(r);
return r->objects->multi_pack_index;
}
struct packed_git *get_all_packs(struct repository *r)
{
struct multi_pack_index *m;
prepare_packed_git(r);
for (m = r->objects->multi_pack_index; m; m = m->next) {
uint32_t i;
for (i = 0; i < m->num_packs; i++)
prepare_midx_pack(r, m, i);
}
return r->objects->packed_git;
}
struct list_head *get_packed_git_mru(struct repository *r)
{
prepare_packed_git(r);
return &r->objects->packed_git_mru;
}
unsigned long unpack_object_header_buffer(const unsigned char *buf,
unsigned long len, enum object_type *type, unsigned long *sizep)
{
unsigned shift;
unsigned long size, c;
unsigned long used = 0;
c = buf[used++];
*type = (c >> 4) & 7;
size = c & 15;
shift = 4;
while (c & 0x80) {
if (len <= used || bitsizeof(long) <= shift) {
error("bad object header");
size = used = 0;
break;
}
c = buf[used++];
size += (c & 0x7f) << shift;
shift += 7;
}
*sizep = size;
return used;
}
unsigned long get_size_from_delta(struct packed_git *p,
struct pack_window **w_curs,
off_t curpos)
{
const unsigned char *data;
unsigned char delta_head[20], *in;
git_zstream stream;
int st;
memset(&stream, 0, sizeof(stream));
stream.next_out = delta_head;
stream.avail_out = sizeof(delta_head);
git_inflate_init(&stream);
do {
in = use_pack(p, w_curs, curpos, &stream.avail_in);
stream.next_in = in;
obj_read_unlock();
st = git_inflate(&stream, Z_FINISH);
obj_read_lock();
curpos += stream.next_in - in;
} while ((st == Z_OK || st == Z_BUF_ERROR) &&
stream.total_out < sizeof(delta_head));
git_inflate_end(&stream);
if ((st != Z_STREAM_END) && stream.total_out != sizeof(delta_head)) {
error("delta data unpack-initial failed");
return 0;
}
data = delta_head;
get_delta_hdr_size(&data, delta_head+sizeof(delta_head));
return get_delta_hdr_size(&data, delta_head+sizeof(delta_head));
}
int unpack_object_header(struct packed_git *p,
struct pack_window **w_curs,
off_t *curpos,
unsigned long *sizep)
{
unsigned char *base;
unsigned long left;
unsigned long used;
enum object_type type;
base = use_pack(p, w_curs, *curpos, &left);
used = unpack_object_header_buffer(base, left, &type, sizep);
if (!used) {
type = OBJ_BAD;
} else
*curpos += used;
return type;
}
void mark_bad_packed_object(struct packed_git *p, const unsigned char *sha1)
{
unsigned i;
const unsigned hashsz = the_hash_algo->rawsz;
for (i = 0; i < p->num_bad_objects; i++)
if (hasheq(sha1, p->bad_object_sha1 + hashsz * i))
return;
p->bad_object_sha1 = xrealloc(p->bad_object_sha1,
st_mult(GIT_MAX_RAWSZ,
st_add(p->num_bad_objects, 1)));
hashcpy(p->bad_object_sha1 + hashsz * p->num_bad_objects, sha1);
p->num_bad_objects++;
}
const struct packed_git *has_packed_and_bad(struct repository *r,
const unsigned char *sha1)
{
struct packed_git *p;
unsigned i;
for (p = r->objects->packed_git; p; p = p->next)
for (i = 0; i < p->num_bad_objects; i++)
if (hasheq(sha1,
p->bad_object_sha1 + the_hash_algo->rawsz * i))
return p;
return NULL;
}
off_t get_delta_base(struct packed_git *p,
struct pack_window **w_curs,
off_t *curpos,
enum object_type type,
off_t delta_obj_offset)
{
unsigned char *base_info = use_pack(p, w_curs, *curpos, NULL);
off_t base_offset;
if (type == OBJ_OFS_DELTA) {
unsigned used = 0;
unsigned char c = base_info[used++];
base_offset = c & 127;
while (c & 128) {
base_offset += 1;
if (!base_offset || MSB(base_offset, 7))
return 0; 
c = base_info[used++];
base_offset = (base_offset << 7) + (c & 127);
}
base_offset = delta_obj_offset - base_offset;
if (base_offset <= 0 || base_offset >= delta_obj_offset)
return 0; 
*curpos += used;
} else if (type == OBJ_REF_DELTA) {
base_offset = find_pack_entry_one(base_info, p);
*curpos += the_hash_algo->rawsz;
} else
die("I am totally screwed");
return base_offset;
}
static int get_delta_base_oid(struct packed_git *p,
struct pack_window **w_curs,
off_t curpos,
struct object_id *oid,
enum object_type type,
off_t delta_obj_offset)
{
if (type == OBJ_REF_DELTA) {
unsigned char *base = use_pack(p, w_curs, curpos, NULL);
oidread(oid, base);
return 0;
} else if (type == OBJ_OFS_DELTA) {
struct revindex_entry *revidx;
off_t base_offset = get_delta_base(p, w_curs, &curpos,
type, delta_obj_offset);
if (!base_offset)
return -1;
revidx = find_pack_revindex(p, base_offset);
if (!revidx)
return -1;
return nth_packed_object_id(oid, p, revidx->nr);
} else
return -1;
}
static int retry_bad_packed_offset(struct repository *r,
struct packed_git *p,
off_t obj_offset)
{
int type;
struct revindex_entry *revidx;
struct object_id oid;
revidx = find_pack_revindex(p, obj_offset);
if (!revidx)
return OBJ_BAD;
nth_packed_object_id(&oid, p, revidx->nr);
mark_bad_packed_object(p, oid.hash);
type = oid_object_info(r, &oid, NULL);
if (type <= OBJ_NONE)
return OBJ_BAD;
return type;
}
#define POI_STACK_PREALLOC 64
static enum object_type packed_to_object_type(struct repository *r,
struct packed_git *p,
off_t obj_offset,
enum object_type type,
struct pack_window **w_curs,
off_t curpos)
{
off_t small_poi_stack[POI_STACK_PREALLOC];
off_t *poi_stack = small_poi_stack;
int poi_stack_nr = 0, poi_stack_alloc = POI_STACK_PREALLOC;
while (type == OBJ_OFS_DELTA || type == OBJ_REF_DELTA) {
off_t base_offset;
unsigned long size;
if (poi_stack_nr >= poi_stack_alloc && poi_stack == small_poi_stack) {
poi_stack_alloc = alloc_nr(poi_stack_nr);
ALLOC_ARRAY(poi_stack, poi_stack_alloc);
COPY_ARRAY(poi_stack, small_poi_stack, poi_stack_nr);
} else {
ALLOC_GROW(poi_stack, poi_stack_nr+1, poi_stack_alloc);
}
poi_stack[poi_stack_nr++] = obj_offset;
base_offset = get_delta_base(p, w_curs, &curpos, type, obj_offset);
if (!base_offset)
goto unwind;
curpos = obj_offset = base_offset;
type = unpack_object_header(p, w_curs, &curpos, &size);
if (type <= OBJ_NONE) {
type = retry_bad_packed_offset(r, p, base_offset);
if (type > OBJ_NONE)
goto out;
goto unwind;
}
}
switch (type) {
case OBJ_BAD:
case OBJ_COMMIT:
case OBJ_TREE:
case OBJ_BLOB:
case OBJ_TAG:
break;
default:
error("unknown object type %i at offset %"PRIuMAX" in %s",
type, (uintmax_t)obj_offset, p->pack_name);
type = OBJ_BAD;
}
out:
if (poi_stack != small_poi_stack)
free(poi_stack);
return type;
unwind:
while (poi_stack_nr) {
obj_offset = poi_stack[--poi_stack_nr];
type = retry_bad_packed_offset(r, p, obj_offset);
if (type > OBJ_NONE)
goto out;
}
type = OBJ_BAD;
goto out;
}
static struct hashmap delta_base_cache;
static size_t delta_base_cached;
static LIST_HEAD(delta_base_cache_lru);
struct delta_base_cache_key {
struct packed_git *p;
off_t base_offset;
};
struct delta_base_cache_entry {
struct hashmap_entry ent;
struct delta_base_cache_key key;
struct list_head lru;
void *data;
unsigned long size;
enum object_type type;
};
static unsigned int pack_entry_hash(struct packed_git *p, off_t base_offset)
{
unsigned int hash;
hash = (unsigned int)(intptr_t)p + (unsigned int)base_offset;
hash += (hash >> 8) + (hash >> 16);
return hash;
}
static struct delta_base_cache_entry *
get_delta_base_cache_entry(struct packed_git *p, off_t base_offset)
{
struct hashmap_entry entry, *e;
struct delta_base_cache_key key;
if (!delta_base_cache.cmpfn)
return NULL;
hashmap_entry_init(&entry, pack_entry_hash(p, base_offset));
key.p = p;
key.base_offset = base_offset;
e = hashmap_get(&delta_base_cache, &entry, &key);
return e ? container_of(e, struct delta_base_cache_entry, ent) : NULL;
}
static int delta_base_cache_key_eq(const struct delta_base_cache_key *a,
const struct delta_base_cache_key *b)
{
return a->p == b->p && a->base_offset == b->base_offset;
}
static int delta_base_cache_hash_cmp(const void *unused_cmp_data,
const struct hashmap_entry *va,
const struct hashmap_entry *vb,
const void *vkey)
{
const struct delta_base_cache_entry *a, *b;
const struct delta_base_cache_key *key = vkey;
a = container_of(va, const struct delta_base_cache_entry, ent);
b = container_of(vb, const struct delta_base_cache_entry, ent);
if (key)
return !delta_base_cache_key_eq(&a->key, key);
else
return !delta_base_cache_key_eq(&a->key, &b->key);
}
static int in_delta_base_cache(struct packed_git *p, off_t base_offset)
{
return !!get_delta_base_cache_entry(p, base_offset);
}
static void detach_delta_base_cache_entry(struct delta_base_cache_entry *ent)
{
hashmap_remove(&delta_base_cache, &ent->ent, &ent->key);
list_del(&ent->lru);
delta_base_cached -= ent->size;
free(ent);
}
static void *cache_or_unpack_entry(struct repository *r, struct packed_git *p,
off_t base_offset, unsigned long *base_size,
enum object_type *type)
{
struct delta_base_cache_entry *ent;
ent = get_delta_base_cache_entry(p, base_offset);
if (!ent)
return unpack_entry(r, p, base_offset, type, base_size);
if (type)
*type = ent->type;
if (base_size)
*base_size = ent->size;
return xmemdupz(ent->data, ent->size);
}
static inline void release_delta_base_cache(struct delta_base_cache_entry *ent)
{
free(ent->data);
detach_delta_base_cache_entry(ent);
}
void clear_delta_base_cache(void)
{
struct list_head *lru, *tmp;
list_for_each_safe(lru, tmp, &delta_base_cache_lru) {
struct delta_base_cache_entry *entry =
list_entry(lru, struct delta_base_cache_entry, lru);
release_delta_base_cache(entry);
}
}
static void add_delta_base_cache(struct packed_git *p, off_t base_offset,
void *base, unsigned long base_size, enum object_type type)
{
struct delta_base_cache_entry *ent = xmalloc(sizeof(*ent));
struct list_head *lru, *tmp;
if (in_delta_base_cache(p, base_offset))
return;
delta_base_cached += base_size;
list_for_each_safe(lru, tmp, &delta_base_cache_lru) {
struct delta_base_cache_entry *f =
list_entry(lru, struct delta_base_cache_entry, lru);
if (delta_base_cached <= delta_base_cache_limit)
break;
release_delta_base_cache(f);
}
ent->key.p = p;
ent->key.base_offset = base_offset;
ent->type = type;
ent->data = base;
ent->size = base_size;
list_add_tail(&ent->lru, &delta_base_cache_lru);
if (!delta_base_cache.cmpfn)
hashmap_init(&delta_base_cache, delta_base_cache_hash_cmp, NULL, 0);
hashmap_entry_init(&ent->ent, pack_entry_hash(p, base_offset));
hashmap_add(&delta_base_cache, &ent->ent);
}
int packed_object_info(struct repository *r, struct packed_git *p,
off_t obj_offset, struct object_info *oi)
{
struct pack_window *w_curs = NULL;
unsigned long size;
off_t curpos = obj_offset;
enum object_type type;
if (oi->contentp) {
*oi->contentp = cache_or_unpack_entry(r, p, obj_offset, oi->sizep,
&type);
if (!*oi->contentp)
type = OBJ_BAD;
} else {
type = unpack_object_header(p, &w_curs, &curpos, &size);
}
if (!oi->contentp && oi->sizep) {
if (type == OBJ_OFS_DELTA || type == OBJ_REF_DELTA) {
off_t tmp_pos = curpos;
off_t base_offset = get_delta_base(p, &w_curs, &tmp_pos,
type, obj_offset);
if (!base_offset) {
type = OBJ_BAD;
goto out;
}
*oi->sizep = get_size_from_delta(p, &w_curs, tmp_pos);
if (*oi->sizep == 0) {
type = OBJ_BAD;
goto out;
}
} else {
*oi->sizep = size;
}
}
if (oi->disk_sizep) {
struct revindex_entry *revidx = find_pack_revindex(p, obj_offset);
*oi->disk_sizep = revidx[1].offset - obj_offset;
}
if (oi->typep || oi->type_name) {
enum object_type ptot;
ptot = packed_to_object_type(r, p, obj_offset,
type, &w_curs, curpos);
if (oi->typep)
*oi->typep = ptot;
if (oi->type_name) {
const char *tn = type_name(ptot);
if (tn)
strbuf_addstr(oi->type_name, tn);
}
if (ptot < 0) {
type = OBJ_BAD;
goto out;
}
}
if (oi->delta_base_oid) {
if (type == OBJ_OFS_DELTA || type == OBJ_REF_DELTA) {
if (get_delta_base_oid(p, &w_curs, curpos,
oi->delta_base_oid,
type, obj_offset) < 0) {
type = OBJ_BAD;
goto out;
}
} else
oidclr(oi->delta_base_oid);
}
oi->whence = in_delta_base_cache(p, obj_offset) ? OI_DBCACHED :
OI_PACKED;
out:
unuse_pack(&w_curs);
return type;
}
static void *unpack_compressed_entry(struct packed_git *p,
struct pack_window **w_curs,
off_t curpos,
unsigned long size)
{
int st;
git_zstream stream;
unsigned char *buffer, *in;
buffer = xmallocz_gently(size);
if (!buffer)
return NULL;
memset(&stream, 0, sizeof(stream));
stream.next_out = buffer;
stream.avail_out = size + 1;
git_inflate_init(&stream);
do {
in = use_pack(p, w_curs, curpos, &stream.avail_in);
stream.next_in = in;
obj_read_unlock();
st = git_inflate(&stream, Z_FINISH);
obj_read_lock();
if (!stream.avail_out)
break; 
curpos += stream.next_in - in;
} while (st == Z_OK || st == Z_BUF_ERROR);
git_inflate_end(&stream);
if ((st != Z_STREAM_END) || stream.total_out != size) {
free(buffer);
return NULL;
}
buffer[size] = '\0';
return buffer;
}
static void write_pack_access_log(struct packed_git *p, off_t obj_offset)
{
static struct trace_key pack_access = TRACE_KEY_INIT(PACK_ACCESS);
trace_printf_key(&pack_access, "%s %"PRIuMAX"\n",
p->pack_name, (uintmax_t)obj_offset);
}
int do_check_packed_object_crc;
#define UNPACK_ENTRY_STACK_PREALLOC 64
struct unpack_entry_stack_ent {
off_t obj_offset;
off_t curpos;
unsigned long size;
};
static void *read_object(struct repository *r,
const struct object_id *oid,
enum object_type *type,
unsigned long *size)
{
struct object_info oi = OBJECT_INFO_INIT;
void *content;
oi.typep = type;
oi.sizep = size;
oi.contentp = &content;
if (oid_object_info_extended(r, oid, &oi, 0) < 0)
return NULL;
return content;
}
void *unpack_entry(struct repository *r, struct packed_git *p, off_t obj_offset,
enum object_type *final_type, unsigned long *final_size)
{
struct pack_window *w_curs = NULL;
off_t curpos = obj_offset;
void *data = NULL;
unsigned long size;
enum object_type type;
struct unpack_entry_stack_ent small_delta_stack[UNPACK_ENTRY_STACK_PREALLOC];
struct unpack_entry_stack_ent *delta_stack = small_delta_stack;
int delta_stack_nr = 0, delta_stack_alloc = UNPACK_ENTRY_STACK_PREALLOC;
int base_from_cache = 0;
write_pack_access_log(p, obj_offset);
for (;;) {
off_t base_offset;
int i;
struct delta_base_cache_entry *ent;
ent = get_delta_base_cache_entry(p, curpos);
if (ent) {
type = ent->type;
data = ent->data;
size = ent->size;
detach_delta_base_cache_entry(ent);
base_from_cache = 1;
break;
}
if (do_check_packed_object_crc && p->index_version > 1) {
struct revindex_entry *revidx = find_pack_revindex(p, obj_offset);
off_t len = revidx[1].offset - obj_offset;
if (check_pack_crc(p, &w_curs, obj_offset, len, revidx->nr)) {
struct object_id oid;
nth_packed_object_id(&oid, p, revidx->nr);
error("bad packed object CRC for %s",
oid_to_hex(&oid));
mark_bad_packed_object(p, oid.hash);
data = NULL;
goto out;
}
}
type = unpack_object_header(p, &w_curs, &curpos, &size);
if (type != OBJ_OFS_DELTA && type != OBJ_REF_DELTA)
break;
base_offset = get_delta_base(p, &w_curs, &curpos, type, obj_offset);
if (!base_offset) {
error("failed to validate delta base reference "
"at offset %"PRIuMAX" from %s",
(uintmax_t)curpos, p->pack_name);
data = NULL;
break;
}
if (delta_stack_nr >= delta_stack_alloc
&& delta_stack == small_delta_stack) {
delta_stack_alloc = alloc_nr(delta_stack_nr);
ALLOC_ARRAY(delta_stack, delta_stack_alloc);
COPY_ARRAY(delta_stack, small_delta_stack,
delta_stack_nr);
} else {
ALLOC_GROW(delta_stack, delta_stack_nr+1, delta_stack_alloc);
}
i = delta_stack_nr++;
delta_stack[i].obj_offset = obj_offset;
delta_stack[i].curpos = curpos;
delta_stack[i].size = size;
curpos = obj_offset = base_offset;
}
switch (type) {
case OBJ_OFS_DELTA:
case OBJ_REF_DELTA:
if (data)
BUG("unpack_entry: left loop at a valid delta");
break;
case OBJ_COMMIT:
case OBJ_TREE:
case OBJ_BLOB:
case OBJ_TAG:
if (!base_from_cache)
data = unpack_compressed_entry(p, &w_curs, curpos, size);
break;
default:
data = NULL;
error("unknown object type %i at offset %"PRIuMAX" in %s",
type, (uintmax_t)obj_offset, p->pack_name);
}
while (delta_stack_nr) {
void *delta_data;
void *base = data;
void *external_base = NULL;
unsigned long delta_size, base_size = size;
int i;
data = NULL;
if (base)
add_delta_base_cache(p, obj_offset, base, base_size, type);
if (!base) {
struct revindex_entry *revidx;
struct object_id base_oid;
revidx = find_pack_revindex(p, obj_offset);
if (revidx) {
nth_packed_object_id(&base_oid, p, revidx->nr);
error("failed to read delta base object %s"
" at offset %"PRIuMAX" from %s",
oid_to_hex(&base_oid), (uintmax_t)obj_offset,
p->pack_name);
mark_bad_packed_object(p, base_oid.hash);
base = read_object(r, &base_oid, &type, &base_size);
external_base = base;
}
}
i = --delta_stack_nr;
obj_offset = delta_stack[i].obj_offset;
curpos = delta_stack[i].curpos;
delta_size = delta_stack[i].size;
if (!base)
continue;
delta_data = unpack_compressed_entry(p, &w_curs, curpos, delta_size);
if (!delta_data) {
error("failed to unpack compressed delta "
"at offset %"PRIuMAX" from %s",
(uintmax_t)curpos, p->pack_name);
data = NULL;
free(external_base);
continue;
}
data = patch_delta(base, base_size,
delta_data, delta_size,
&size);
if (!data)
error("failed to apply delta");
free(delta_data);
free(external_base);
}
if (final_type)
*final_type = type;
if (final_size)
*final_size = size;
out:
unuse_pack(&w_curs);
if (delta_stack != small_delta_stack)
free(delta_stack);
return data;
}
int bsearch_pack(const struct object_id *oid, const struct packed_git *p, uint32_t *result)
{
const unsigned char *index_fanout = p->index_data;
const unsigned char *index_lookup;
const unsigned int hashsz = the_hash_algo->rawsz;
int index_lookup_width;
if (!index_fanout)
BUG("bsearch_pack called without a valid pack-index");
index_lookup = index_fanout + 4 * 256;
if (p->index_version == 1) {
index_lookup_width = hashsz + 4;
index_lookup += 4;
} else {
index_lookup_width = hashsz;
index_fanout += 8;
index_lookup += 8;
}
return bsearch_hash(oid->hash, (const uint32_t*)index_fanout,
index_lookup, index_lookup_width, result);
}
int nth_packed_object_id(struct object_id *oid,
struct packed_git *p,
uint32_t n)
{
const unsigned char *index = p->index_data;
const unsigned int hashsz = the_hash_algo->rawsz;
if (!index) {
if (open_pack_index(p))
return -1;
index = p->index_data;
}
if (n >= p->num_objects)
return -1;
index += 4 * 256;
if (p->index_version == 1) {
oidread(oid, index + (hashsz + 4) * n + 4);
} else {
index += 8;
oidread(oid, index + hashsz * n);
}
return 0;
}
void check_pack_index_ptr(const struct packed_git *p, const void *vptr)
{
const unsigned char *ptr = vptr;
const unsigned char *start = p->index_data;
const unsigned char *end = start + p->index_size;
if (ptr < start)
die(_("offset before start of pack index for %s (corrupt index?)"),
p->pack_name);
if (ptr >= end - 8)
die(_("offset beyond end of pack index for %s (truncated index?)"),
p->pack_name);
}
off_t nth_packed_object_offset(const struct packed_git *p, uint32_t n)
{
const unsigned char *index = p->index_data;
const unsigned int hashsz = the_hash_algo->rawsz;
index += 4 * 256;
if (p->index_version == 1) {
return ntohl(*((uint32_t *)(index + (hashsz + 4) * n)));
} else {
uint32_t off;
index += 8 + p->num_objects * (hashsz + 4);
off = ntohl(*((uint32_t *)(index + 4 * n)));
if (!(off & 0x80000000))
return off;
index += p->num_objects * 4 + (off & 0x7fffffff) * 8;
check_pack_index_ptr(p, index);
return get_be64(index);
}
}
off_t find_pack_entry_one(const unsigned char *sha1,
struct packed_git *p)
{
const unsigned char *index = p->index_data;
struct object_id oid;
uint32_t result;
if (!index) {
if (open_pack_index(p))
return 0;
}
hashcpy(oid.hash, sha1);
if (bsearch_pack(&oid, p, &result))
return nth_packed_object_offset(p, result);
return 0;
}
int is_pack_valid(struct packed_git *p)
{
if (p->pack_fd != -1)
return 1;
if (p->windows) {
struct pack_window *w = p->windows;
if (!w->offset && w->len == p->pack_size)
return 1;
}
return !open_packed_git(p);
}
struct packed_git *find_sha1_pack(const unsigned char *sha1,
struct packed_git *packs)
{
struct packed_git *p;
for (p = packs; p; p = p->next) {
if (find_pack_entry_one(sha1, p))
return p;
}
return NULL;
}
static int fill_pack_entry(const struct object_id *oid,
struct pack_entry *e,
struct packed_git *p)
{
off_t offset;
if (p->num_bad_objects) {
unsigned i;
for (i = 0; i < p->num_bad_objects; i++)
if (hasheq(oid->hash,
p->bad_object_sha1 + the_hash_algo->rawsz * i))
return 0;
}
offset = find_pack_entry_one(oid->hash, p);
if (!offset)
return 0;
if (!is_pack_valid(p))
return 0;
e->offset = offset;
e->p = p;
return 1;
}
int find_pack_entry(struct repository *r, const struct object_id *oid, struct pack_entry *e)
{
struct list_head *pos;
struct multi_pack_index *m;
prepare_packed_git(r);
if (!r->objects->packed_git && !r->objects->multi_pack_index)
return 0;
for (m = r->objects->multi_pack_index; m; m = m->next) {
if (fill_midx_entry(r, oid, e, m))
return 1;
}
list_for_each(pos, &r->objects->packed_git_mru) {
struct packed_git *p = list_entry(pos, struct packed_git, mru);
if (!p->multi_pack_index && fill_pack_entry(oid, e, p)) {
list_move(&p->mru, &r->objects->packed_git_mru);
return 1;
}
}
return 0;
}
int has_object_pack(const struct object_id *oid)
{
struct pack_entry e;
return find_pack_entry(the_repository, oid, &e);
}
int has_pack_index(const unsigned char *sha1)
{
struct stat st;
if (stat(sha1_pack_index_name(sha1), &st))
return 0;
return 1;
}
int for_each_object_in_pack(struct packed_git *p,
each_packed_object_fn cb, void *data,
enum for_each_object_flags flags)
{
uint32_t i;
int r = 0;
if (flags & FOR_EACH_OBJECT_PACK_ORDER) {
if (load_pack_revindex(p))
return -1;
}
for (i = 0; i < p->num_objects; i++) {
uint32_t pos;
struct object_id oid;
if (flags & FOR_EACH_OBJECT_PACK_ORDER)
pos = p->revindex[i].nr;
else
pos = i;
if (nth_packed_object_id(&oid, p, pos) < 0)
return error("unable to get sha1 of object %u in %s",
pos, p->pack_name);
r = cb(&oid, p, pos, data);
if (r)
break;
}
return r;
}
int for_each_packed_object(each_packed_object_fn cb, void *data,
enum for_each_object_flags flags)
{
struct packed_git *p;
int r = 0;
int pack_errors = 0;
prepare_packed_git(the_repository);
for (p = get_all_packs(the_repository); p; p = p->next) {
if ((flags & FOR_EACH_OBJECT_LOCAL_ONLY) && !p->pack_local)
continue;
if ((flags & FOR_EACH_OBJECT_PROMISOR_ONLY) &&
!p->pack_promisor)
continue;
if (open_pack_index(p)) {
pack_errors = 1;
continue;
}
r = for_each_object_in_pack(p, cb, data, flags);
if (r)
break;
}
return r ? r : pack_errors;
}
static int add_promisor_object(const struct object_id *oid,
struct packed_git *pack,
uint32_t pos,
void *set_)
{
struct oidset *set = set_;
struct object *obj = parse_object(the_repository, oid);
if (!obj)
return 1;
oidset_insert(set, oid);
if (obj->type == OBJ_TREE) {
struct tree *tree = (struct tree *)obj;
struct tree_desc desc;
struct name_entry entry;
if (init_tree_desc_gently(&desc, tree->buffer, tree->size))
return 0;
while (tree_entry_gently(&desc, &entry))
oidset_insert(set, &entry.oid);
} else if (obj->type == OBJ_COMMIT) {
struct commit *commit = (struct commit *) obj;
struct commit_list *parents = commit->parents;
oidset_insert(set, get_commit_tree_oid(commit));
for (; parents; parents = parents->next)
oidset_insert(set, &parents->item->object.oid);
} else if (obj->type == OBJ_TAG) {
struct tag *tag = (struct tag *) obj;
oidset_insert(set, get_tagged_oid(tag));
}
return 0;
}
int is_promisor_object(const struct object_id *oid)
{
static struct oidset promisor_objects;
static int promisor_objects_prepared;
if (!promisor_objects_prepared) {
if (has_promisor_remote()) {
for_each_packed_object(add_promisor_object,
&promisor_objects,
FOR_EACH_OBJECT_PROMISOR_ONLY);
}
promisor_objects_prepared = 1;
}
return oidset_contains(&promisor_objects, oid);
}
