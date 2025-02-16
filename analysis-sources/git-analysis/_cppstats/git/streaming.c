#include "cache.h"
#include "streaming.h"
#include "repository.h"
#include "object-store.h"
#include "replace-object.h"
#include "packfile.h"
enum input_source {
stream_error = -1,
incore = 0,
loose = 1,
pack_non_delta = 2
};
typedef int (*open_istream_fn)(struct git_istream *,
struct repository *,
struct object_info *,
const struct object_id *,
enum object_type *);
typedef int (*close_istream_fn)(struct git_istream *);
typedef ssize_t (*read_istream_fn)(struct git_istream *, char *, size_t);
struct stream_vtbl {
close_istream_fn close;
read_istream_fn read;
};
#define open_method_decl(name) int open_istream_ ##name (struct git_istream *st, struct repository *r, struct object_info *oi, const struct object_id *oid, enum object_type *type)
#define close_method_decl(name) int close_istream_ ##name (struct git_istream *st)
#define read_method_decl(name) ssize_t read_istream_ ##name (struct git_istream *st, char *buf, size_t sz)
static open_method_decl(incore);
static open_method_decl(loose);
static open_method_decl(pack_non_delta);
static struct git_istream *attach_stream_filter(struct git_istream *st,
struct stream_filter *filter);
static open_istream_fn open_istream_tbl[] = {
open_istream_incore,
open_istream_loose,
open_istream_pack_non_delta,
};
#define FILTER_BUFFER (1024*16)
struct filtered_istream {
struct git_istream *upstream;
struct stream_filter *filter;
char ibuf[FILTER_BUFFER];
char obuf[FILTER_BUFFER];
int i_end, i_ptr;
int o_end, o_ptr;
int input_finished;
};
struct git_istream {
const struct stream_vtbl *vtbl;
unsigned long size; 
git_zstream z;
enum { z_unused, z_used, z_done, z_error } z_state;
union {
struct {
char *buf; 
unsigned long read_ptr;
} incore;
struct {
void *mapped;
unsigned long mapsize;
char hdr[32];
int hdr_avail;
int hdr_used;
} loose;
struct {
struct packed_git *pack;
off_t pos;
} in_pack;
struct filtered_istream filtered;
} u;
};
int close_istream(struct git_istream *st)
{
int r = st->vtbl->close(st);
free(st);
return r;
}
ssize_t read_istream(struct git_istream *st, void *buf, size_t sz)
{
return st->vtbl->read(st, buf, sz);
}
static enum input_source istream_source(struct repository *r,
const struct object_id *oid,
enum object_type *type,
struct object_info *oi)
{
unsigned long size;
int status;
oi->typep = type;
oi->sizep = &size;
status = oid_object_info_extended(r, oid, oi, 0);
if (status < 0)
return stream_error;
switch (oi->whence) {
case OI_LOOSE:
return loose;
case OI_PACKED:
if (!oi->u.packed.is_delta && big_file_threshold < size)
return pack_non_delta;
default:
return incore;
}
}
struct git_istream *open_istream(struct repository *r,
const struct object_id *oid,
enum object_type *type,
unsigned long *size,
struct stream_filter *filter)
{
struct git_istream *st;
struct object_info oi = OBJECT_INFO_INIT;
const struct object_id *real = lookup_replace_object(r, oid);
enum input_source src = istream_source(r, real, type, &oi);
if (src < 0)
return NULL;
st = xmalloc(sizeof(*st));
if (open_istream_tbl[src](st, r, &oi, real, type)) {
if (open_istream_incore(st, r, &oi, real, type)) {
free(st);
return NULL;
}
}
if (filter) {
struct git_istream *nst = attach_stream_filter(st, filter);
if (!nst) {
close_istream(st);
return NULL;
}
st = nst;
}
*size = st->size;
return st;
}
static void close_deflated_stream(struct git_istream *st)
{
if (st->z_state == z_used)
git_inflate_end(&st->z);
}
static close_method_decl(filtered)
{
free_stream_filter(st->u.filtered.filter);
return close_istream(st->u.filtered.upstream);
}
static read_method_decl(filtered)
{
struct filtered_istream *fs = &(st->u.filtered);
size_t filled = 0;
while (sz) {
if (fs->o_ptr < fs->o_end) {
size_t to_move = fs->o_end - fs->o_ptr;
if (sz < to_move)
to_move = sz;
memcpy(buf + filled, fs->obuf + fs->o_ptr, to_move);
fs->o_ptr += to_move;
sz -= to_move;
filled += to_move;
continue;
}
fs->o_end = fs->o_ptr = 0;
if (fs->i_ptr < fs->i_end) {
size_t to_feed = fs->i_end - fs->i_ptr;
size_t to_receive = FILTER_BUFFER;
if (stream_filter(fs->filter,
fs->ibuf + fs->i_ptr, &to_feed,
fs->obuf, &to_receive))
return -1;
fs->i_ptr = fs->i_end - to_feed;
fs->o_end = FILTER_BUFFER - to_receive;
continue;
}
if (fs->input_finished) {
size_t to_receive = FILTER_BUFFER;
if (stream_filter(fs->filter,
NULL, NULL,
fs->obuf, &to_receive))
return -1;
fs->o_end = FILTER_BUFFER - to_receive;
if (!fs->o_end)
break;
continue;
}
fs->i_end = fs->i_ptr = 0;
if (!fs->input_finished) {
fs->i_end = read_istream(fs->upstream, fs->ibuf, FILTER_BUFFER);
if (fs->i_end < 0)
return -1;
if (fs->i_end)
continue;
}
fs->input_finished = 1;
}
return filled;
}
static struct stream_vtbl filtered_vtbl = {
close_istream_filtered,
read_istream_filtered,
};
static struct git_istream *attach_stream_filter(struct git_istream *st,
struct stream_filter *filter)
{
struct git_istream *ifs = xmalloc(sizeof(*ifs));
struct filtered_istream *fs = &(ifs->u.filtered);
ifs->vtbl = &filtered_vtbl;
fs->upstream = st;
fs->filter = filter;
fs->i_end = fs->i_ptr = 0;
fs->o_end = fs->o_ptr = 0;
fs->input_finished = 0;
ifs->size = -1; 
return ifs;
}
static read_method_decl(loose)
{
size_t total_read = 0;
switch (st->z_state) {
case z_done:
return 0;
case z_error:
return -1;
default:
break;
}
if (st->u.loose.hdr_used < st->u.loose.hdr_avail) {
size_t to_copy = st->u.loose.hdr_avail - st->u.loose.hdr_used;
if (sz < to_copy)
to_copy = sz;
memcpy(buf, st->u.loose.hdr + st->u.loose.hdr_used, to_copy);
st->u.loose.hdr_used += to_copy;
total_read += to_copy;
}
while (total_read < sz) {
int status;
st->z.next_out = (unsigned char *)buf + total_read;
st->z.avail_out = sz - total_read;
status = git_inflate(&st->z, Z_FINISH);
total_read = st->z.next_out - (unsigned char *)buf;
if (status == Z_STREAM_END) {
git_inflate_end(&st->z);
st->z_state = z_done;
break;
}
if (status != Z_OK && (status != Z_BUF_ERROR || total_read < sz)) {
git_inflate_end(&st->z);
st->z_state = z_error;
return -1;
}
}
return total_read;
}
static close_method_decl(loose)
{
close_deflated_stream(st);
munmap(st->u.loose.mapped, st->u.loose.mapsize);
return 0;
}
static struct stream_vtbl loose_vtbl = {
close_istream_loose,
read_istream_loose,
};
static open_method_decl(loose)
{
st->u.loose.mapped = map_loose_object(r, oid, &st->u.loose.mapsize);
if (!st->u.loose.mapped)
return -1;
if ((unpack_loose_header(&st->z,
st->u.loose.mapped,
st->u.loose.mapsize,
st->u.loose.hdr,
sizeof(st->u.loose.hdr)) < 0) ||
(parse_loose_header(st->u.loose.hdr, &st->size) < 0)) {
git_inflate_end(&st->z);
munmap(st->u.loose.mapped, st->u.loose.mapsize);
return -1;
}
st->u.loose.hdr_used = strlen(st->u.loose.hdr) + 1;
st->u.loose.hdr_avail = st->z.total_out;
st->z_state = z_used;
st->vtbl = &loose_vtbl;
return 0;
}
static read_method_decl(pack_non_delta)
{
size_t total_read = 0;
switch (st->z_state) {
case z_unused:
memset(&st->z, 0, sizeof(st->z));
git_inflate_init(&st->z);
st->z_state = z_used;
break;
case z_done:
return 0;
case z_error:
return -1;
case z_used:
break;
}
while (total_read < sz) {
int status;
struct pack_window *window = NULL;
unsigned char *mapped;
mapped = use_pack(st->u.in_pack.pack, &window,
st->u.in_pack.pos, &st->z.avail_in);
st->z.next_out = (unsigned char *)buf + total_read;
st->z.avail_out = sz - total_read;
st->z.next_in = mapped;
status = git_inflate(&st->z, Z_FINISH);
st->u.in_pack.pos += st->z.next_in - mapped;
total_read = st->z.next_out - (unsigned char *)buf;
unuse_pack(&window);
if (status == Z_STREAM_END) {
git_inflate_end(&st->z);
st->z_state = z_done;
break;
}
if (status != Z_OK && status != Z_BUF_ERROR) {
git_inflate_end(&st->z);
st->z_state = z_error;
return -1;
}
}
return total_read;
}
static close_method_decl(pack_non_delta)
{
close_deflated_stream(st);
return 0;
}
static struct stream_vtbl pack_non_delta_vtbl = {
close_istream_pack_non_delta,
read_istream_pack_non_delta,
};
static open_method_decl(pack_non_delta)
{
struct pack_window *window;
enum object_type in_pack_type;
st->u.in_pack.pack = oi->u.packed.pack;
st->u.in_pack.pos = oi->u.packed.offset;
window = NULL;
in_pack_type = unpack_object_header(st->u.in_pack.pack,
&window,
&st->u.in_pack.pos,
&st->size);
unuse_pack(&window);
switch (in_pack_type) {
default:
return -1; 
case OBJ_COMMIT:
case OBJ_TREE:
case OBJ_BLOB:
case OBJ_TAG:
break;
}
st->z_state = z_unused;
st->vtbl = &pack_non_delta_vtbl;
return 0;
}
static close_method_decl(incore)
{
free(st->u.incore.buf);
return 0;
}
static read_method_decl(incore)
{
size_t read_size = sz;
size_t remainder = st->size - st->u.incore.read_ptr;
if (remainder <= read_size)
read_size = remainder;
if (read_size) {
memcpy(buf, st->u.incore.buf + st->u.incore.read_ptr, read_size);
st->u.incore.read_ptr += read_size;
}
return read_size;
}
static struct stream_vtbl incore_vtbl = {
close_istream_incore,
read_istream_incore,
};
static open_method_decl(incore)
{
st->u.incore.buf = read_object_file_extended(r, oid, type, &st->size, 0);
st->u.incore.read_ptr = 0;
st->vtbl = &incore_vtbl;
return st->u.incore.buf ? 0 : -1;
}
int stream_blob_to_fd(int fd, const struct object_id *oid, struct stream_filter *filter,
int can_seek)
{
struct git_istream *st;
enum object_type type;
unsigned long sz;
ssize_t kept = 0;
int result = -1;
st = open_istream(the_repository, oid, &type, &sz, filter);
if (!st) {
if (filter)
free_stream_filter(filter);
return result;
}
if (type != OBJ_BLOB)
goto close_and_exit;
for (;;) {
char buf[1024 * 16];
ssize_t wrote, holeto;
ssize_t readlen = read_istream(st, buf, sizeof(buf));
if (readlen < 0)
goto close_and_exit;
if (!readlen)
break;
if (can_seek && sizeof(buf) == readlen) {
for (holeto = 0; holeto < readlen; holeto++)
if (buf[holeto])
break;
if (readlen == holeto) {
kept += holeto;
continue;
}
}
if (kept && lseek(fd, kept, SEEK_CUR) == (off_t) -1)
goto close_and_exit;
else
kept = 0;
wrote = write_in_full(fd, buf, readlen);
if (wrote < 0)
goto close_and_exit;
}
if (kept && (lseek(fd, kept - 1, SEEK_CUR) == (off_t) -1 ||
xwrite(fd, "", 1) != 1))
goto close_and_exit;
result = 0;
close_and_exit:
close_istream(st);
return result;
}
