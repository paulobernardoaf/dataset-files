#include "libavutil/avstring.h"
#include "libavutil/internal.h"
#include "libavutil/opt.h"
#include "avformat.h"
#if HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <fcntl.h>
#if HAVE_IO_H
#include <io.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <stdlib.h>
#include "os_support.h"
#include "url.h"
#if !defined(S_ISFIFO)
#if defined(S_IFIFO)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#else
#define S_ISFIFO(m) 0
#endif
#endif
#if !defined(S_ISLNK)
#if defined(S_IFLNK)
#define S_ISLNK(m) (((m) & S_IFLNK) == S_IFLNK)
#else
#define S_ISLNK(m) 0
#endif
#endif
#if !defined(S_ISSOCK)
#if defined(S_IFSOCK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#else
#define S_ISSOCK(m) 0
#endif
#endif
typedef struct FileContext {
const AVClass *class;
int fd;
int trunc;
int blocksize;
int follow;
int seekable;
#if HAVE_DIRENT_H
DIR *dir;
#endif
} FileContext;
static const AVOption file_options[] = {
{ "truncate", "truncate existing files on write", offsetof(FileContext, trunc), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, AV_OPT_FLAG_ENCODING_PARAM },
{ "blocksize", "set I/O operation maximum block size", offsetof(FileContext, blocksize), AV_OPT_TYPE_INT, { .i64 = INT_MAX }, 1, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM },
{ "follow", "Follow a file as it is being written", offsetof(FileContext, follow), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, AV_OPT_FLAG_DECODING_PARAM },
{ "seekable", "Sets if the file is seekable", offsetof(FileContext, seekable), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, 0, AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_ENCODING_PARAM },
{ NULL }
};
static const AVOption pipe_options[] = {
{ "blocksize", "set I/O operation maximum block size", offsetof(FileContext, blocksize), AV_OPT_TYPE_INT, { .i64 = INT_MAX }, 1, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM },
{ NULL }
};
static const AVClass file_class = {
.class_name = "file",
.item_name = av_default_item_name,
.option = file_options,
.version = LIBAVUTIL_VERSION_INT,
};
static const AVClass pipe_class = {
.class_name = "pipe",
.item_name = av_default_item_name,
.option = pipe_options,
.version = LIBAVUTIL_VERSION_INT,
};
static int file_read(URLContext *h, unsigned char *buf, int size)
{
FileContext *c = h->priv_data;
int ret;
size = FFMIN(size, c->blocksize);
ret = read(c->fd, buf, size);
if (ret == 0 && c->follow)
return AVERROR(EAGAIN);
if (ret == 0)
return AVERROR_EOF;
return (ret == -1) ? AVERROR(errno) : ret;
}
static int file_write(URLContext *h, const unsigned char *buf, int size)
{
FileContext *c = h->priv_data;
int ret;
size = FFMIN(size, c->blocksize);
ret = write(c->fd, buf, size);
return (ret == -1) ? AVERROR(errno) : ret;
}
static int file_get_handle(URLContext *h)
{
FileContext *c = h->priv_data;
return c->fd;
}
static int file_check(URLContext *h, int mask)
{
int ret = 0;
const char *filename = h->filename;
av_strstart(filename, "file:", &filename);
{
#if HAVE_ACCESS && defined(R_OK)
if (access(filename, F_OK) < 0)
return AVERROR(errno);
if (mask&AVIO_FLAG_READ)
if (access(filename, R_OK) >= 0)
ret |= AVIO_FLAG_READ;
if (mask&AVIO_FLAG_WRITE)
if (access(filename, W_OK) >= 0)
ret |= AVIO_FLAG_WRITE;
#else
struct stat st;
#if !defined(_WIN32)
ret = stat(filename, &st);
#else
ret = win32_stat(filename, &st);
#endif
if (ret < 0)
return AVERROR(errno);
ret |= st.st_mode&S_IRUSR ? mask&AVIO_FLAG_READ : 0;
ret |= st.st_mode&S_IWUSR ? mask&AVIO_FLAG_WRITE : 0;
#endif
}
return ret;
}
static int file_delete(URLContext *h)
{
#if HAVE_UNISTD_H
int ret;
const char *filename = h->filename;
av_strstart(filename, "file:", &filename);
ret = rmdir(filename);
if (ret < 0 && (errno == ENOTDIR
#if defined(_WIN32)
|| errno == EINVAL
#endif
))
ret = unlink(filename);
if (ret < 0)
return AVERROR(errno);
return ret;
#else
return AVERROR(ENOSYS);
#endif 
}
static int file_move(URLContext *h_src, URLContext *h_dst)
{
const char *filename_src = h_src->filename;
const char *filename_dst = h_dst->filename;
av_strstart(filename_src, "file:", &filename_src);
av_strstart(filename_dst, "file:", &filename_dst);
if (rename(filename_src, filename_dst) < 0)
return AVERROR(errno);
return 0;
}
#if CONFIG_FILE_PROTOCOL
static int file_open(URLContext *h, const char *filename, int flags)
{
FileContext *c = h->priv_data;
int access;
int fd;
struct stat st;
av_strstart(filename, "file:", &filename);
if (flags & AVIO_FLAG_WRITE && flags & AVIO_FLAG_READ) {
access = O_CREAT | O_RDWR;
if (c->trunc)
access |= O_TRUNC;
} else if (flags & AVIO_FLAG_WRITE) {
access = O_CREAT | O_WRONLY;
if (c->trunc)
access |= O_TRUNC;
} else {
access = O_RDONLY;
}
#if defined(O_BINARY)
access |= O_BINARY;
#endif
fd = avpriv_open(filename, access, 0666);
if (fd == -1)
return AVERROR(errno);
c->fd = fd;
h->is_streamed = !fstat(fd, &st) && S_ISFIFO(st.st_mode);
if (!h->is_streamed && flags & AVIO_FLAG_WRITE)
h->min_packet_size = h->max_packet_size = 262144;
if (c->seekable >= 0)
h->is_streamed = !c->seekable;
return 0;
}
static int64_t file_seek(URLContext *h, int64_t pos, int whence)
{
FileContext *c = h->priv_data;
int64_t ret;
if (whence == AVSEEK_SIZE) {
struct stat st;
ret = fstat(c->fd, &st);
return ret < 0 ? AVERROR(errno) : (S_ISFIFO(st.st_mode) ? 0 : st.st_size);
}
ret = lseek(c->fd, pos, whence);
return ret < 0 ? AVERROR(errno) : ret;
}
static int file_close(URLContext *h)
{
FileContext *c = h->priv_data;
return close(c->fd);
}
static int file_open_dir(URLContext *h)
{
#if HAVE_LSTAT
FileContext *c = h->priv_data;
c->dir = opendir(h->filename);
if (!c->dir)
return AVERROR(errno);
return 0;
#else
return AVERROR(ENOSYS);
#endif 
}
static int file_read_dir(URLContext *h, AVIODirEntry **next)
{
#if HAVE_LSTAT
FileContext *c = h->priv_data;
struct dirent *dir;
char *fullpath = NULL;
*next = ff_alloc_dir_entry();
if (!*next)
return AVERROR(ENOMEM);
do {
errno = 0;
dir = readdir(c->dir);
if (!dir) {
av_freep(next);
return AVERROR(errno);
}
} while (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."));
fullpath = av_append_path_component(h->filename, dir->d_name);
if (fullpath) {
struct stat st;
if (!lstat(fullpath, &st)) {
if (S_ISDIR(st.st_mode))
(*next)->type = AVIO_ENTRY_DIRECTORY;
else if (S_ISFIFO(st.st_mode))
(*next)->type = AVIO_ENTRY_NAMED_PIPE;
else if (S_ISCHR(st.st_mode))
(*next)->type = AVIO_ENTRY_CHARACTER_DEVICE;
else if (S_ISBLK(st.st_mode))
(*next)->type = AVIO_ENTRY_BLOCK_DEVICE;
else if (S_ISLNK(st.st_mode))
(*next)->type = AVIO_ENTRY_SYMBOLIC_LINK;
else if (S_ISSOCK(st.st_mode))
(*next)->type = AVIO_ENTRY_SOCKET;
else if (S_ISREG(st.st_mode))
(*next)->type = AVIO_ENTRY_FILE;
else
(*next)->type = AVIO_ENTRY_UNKNOWN;
(*next)->group_id = st.st_gid;
(*next)->user_id = st.st_uid;
(*next)->size = st.st_size;
(*next)->filemode = st.st_mode & 0777;
(*next)->modification_timestamp = INT64_C(1000000) * st.st_mtime;
(*next)->access_timestamp = INT64_C(1000000) * st.st_atime;
(*next)->status_change_timestamp = INT64_C(1000000) * st.st_ctime;
}
av_free(fullpath);
}
(*next)->name = av_strdup(dir->d_name);
return 0;
#else
return AVERROR(ENOSYS);
#endif 
}
static int file_close_dir(URLContext *h)
{
#if HAVE_LSTAT
FileContext *c = h->priv_data;
closedir(c->dir);
return 0;
#else
return AVERROR(ENOSYS);
#endif 
}
const URLProtocol ff_file_protocol = {
.name = "file",
.url_open = file_open,
.url_read = file_read,
.url_write = file_write,
.url_seek = file_seek,
.url_close = file_close,
.url_get_file_handle = file_get_handle,
.url_check = file_check,
.url_delete = file_delete,
.url_move = file_move,
.priv_data_size = sizeof(FileContext),
.priv_data_class = &file_class,
.url_open_dir = file_open_dir,
.url_read_dir = file_read_dir,
.url_close_dir = file_close_dir,
.default_whitelist = "file,crypto,data"
};
#endif 
#if CONFIG_PIPE_PROTOCOL
static int pipe_open(URLContext *h, const char *filename, int flags)
{
FileContext *c = h->priv_data;
int fd;
char *final;
av_strstart(filename, "pipe:", &filename);
fd = strtol(filename, &final, 10);
if((filename == final) || *final ) {
if (flags & AVIO_FLAG_WRITE) {
fd = 1;
} else {
fd = 0;
}
}
#if HAVE_SETMODE
setmode(fd, O_BINARY);
#endif
c->fd = fd;
h->is_streamed = 1;
return 0;
}
const URLProtocol ff_pipe_protocol = {
.name = "pipe",
.url_open = pipe_open,
.url_read = file_read,
.url_write = file_write,
.url_get_file_handle = file_get_handle,
.url_check = file_check,
.priv_data_size = sizeof(FileContext),
.priv_data_class = &pipe_class,
.default_whitelist = "crypto,data"
};
#endif 
