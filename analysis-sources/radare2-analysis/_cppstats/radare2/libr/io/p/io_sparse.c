#include "r_io.h"
#include "r_lib.h"
#include "r_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
typedef struct {
int fd;
RBuffer *buf;
ut64 offset;
} RIOSparse;
#define RIOSPARSE_FD(x) (((RIOSparse*)(x)->data)->fd)
#define RIOSPARSE_BUF(x) (((RIOSparse*)(x)->data)->buf)
#define RIOSPARSE_OFF(x) (((RIOSparse*)(x)->data)->offset)
static int __write(RIO *io, RIODesc *fd, const ut8 *buf, int count) {
ut64 o;
RBuffer *b;
if (!fd || !fd->data) {
return -1;
}
b = RIOSPARSE_BUF(fd);
o = RIOSPARSE_OFF(fd);
int r = r_buf_write_at (b, o, buf, count);
if (r >= 0) {
r_buf_seek (b, r, R_BUF_CUR);
}
return r;
}
static int __read(RIO *io, RIODesc *fd, ut8 *buf, int count) {
ut64 o;
RBuffer *b;
if (!fd || !fd->data) {
return -1;
}
b = RIOSPARSE_BUF(fd);
o = RIOSPARSE_OFF(fd);
int r = r_buf_read_at (b, o, buf, count);
if (r >= 0) {
r_buf_seek (b, count, R_BUF_CUR);
}
return count;
}
static int __close(RIODesc *fd) {
RIOSparse *riom;
if (!fd || !fd->data) {
return -1;
}
riom = fd->data;
R_FREE (riom->buf);
R_FREE (fd->data);
return 0;
}
static ut64 __lseek(RIO* io, RIODesc *fd, ut64 offset, int whence) {
RBuffer *b;
ut64 r_offset = offset;
if (!fd->data) {
return offset;
}
b = RIOSPARSE_BUF(fd);
r_offset = r_buf_seek (b, offset, whence);
RIOSPARSE_OFF (fd) = r_offset;
return r_offset;
}
static bool __plugin_open(struct r_io_t *io, const char *pathname, bool many) {
return (!strncmp (pathname, "sparse://", 9));
}
static RIODesc *__open(RIO *io, const char *pathname, int rw, int mode) {
if (__plugin_open (io, pathname,0)) {
RIOSparse *mal = R_NEW0 (RIOSparse);
int size = (int)r_num_math (NULL, pathname + 9);
mal->buf = r_buf_new_sparse (io->Oxff);
if (!mal->buf) {
free (mal);
return NULL;
}
if (size > 0) {
ut8 *data = malloc (size);
if (!data) {
eprintf ("Cannot allocate (%s) %d byte(s)\n",
pathname+9, size);
mal->offset = 0;
} else {
memset (data, 0x00, size);
r_buf_write_at (mal->buf, 0, data, size);
free (data);
}
}
if (mal->buf) {
return r_io_desc_new (io, &r_io_plugin_sparse,
pathname, rw, mode, mal);
}
r_buf_free (mal->buf);
free (mal);
}
return NULL;
}
RIOPlugin r_io_plugin_sparse = {
.name = "sparse",
.desc = "Sparse buffer allocation plugin",
.uris = "sparse://",
.license = "LGPL3",
.open = __open,
.close = __close,
.read = __read,
.check = __plugin_open,
.lseek = __lseek,
.write = __write,
.resize = NULL,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_IO,
.data = &r_io_plugin_sparse,
.version = R2_VERSION
};
#endif
