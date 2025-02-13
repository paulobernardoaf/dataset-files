#include "cache.h"
#include "unix-socket.h"
static int unix_stream_socket(void)
{
int fd = socket(AF_UNIX, SOCK_STREAM, 0);
if (fd < 0)
die_errno("unable to create socket");
return fd;
}
static int chdir_len(const char *orig, int len)
{
char *path = xmemdupz(orig, len);
int r = chdir(path);
free(path);
return r;
}
struct unix_sockaddr_context {
char *orig_dir;
};
static void unix_sockaddr_cleanup(struct unix_sockaddr_context *ctx)
{
if (!ctx->orig_dir)
return;
if (chdir(ctx->orig_dir) < 0)
die("unable to restore original working directory");
free(ctx->orig_dir);
}
static int unix_sockaddr_init(struct sockaddr_un *sa, const char *path,
struct unix_sockaddr_context *ctx)
{
int size = strlen(path) + 1;
ctx->orig_dir = NULL;
if (size > sizeof(sa->sun_path)) {
const char *slash = find_last_dir_sep(path);
const char *dir;
struct strbuf cwd = STRBUF_INIT;
if (!slash) {
errno = ENAMETOOLONG;
return -1;
}
dir = path;
path = slash + 1;
size = strlen(path) + 1;
if (size > sizeof(sa->sun_path)) {
errno = ENAMETOOLONG;
return -1;
}
if (strbuf_getcwd(&cwd))
return -1;
ctx->orig_dir = strbuf_detach(&cwd, NULL);
if (chdir_len(dir, slash - dir) < 0)
return -1;
}
memset(sa, 0, sizeof(*sa));
sa->sun_family = AF_UNIX;
memcpy(sa->sun_path, path, size);
return 0;
}
int unix_stream_connect(const char *path)
{
int fd, saved_errno;
struct sockaddr_un sa;
struct unix_sockaddr_context ctx;
if (unix_sockaddr_init(&sa, path, &ctx) < 0)
return -1;
fd = unix_stream_socket();
if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
goto fail;
unix_sockaddr_cleanup(&ctx);
return fd;
fail:
saved_errno = errno;
unix_sockaddr_cleanup(&ctx);
close(fd);
errno = saved_errno;
return -1;
}
int unix_stream_listen(const char *path)
{
int fd, saved_errno;
struct sockaddr_un sa;
struct unix_sockaddr_context ctx;
unlink(path);
if (unix_sockaddr_init(&sa, path, &ctx) < 0)
return -1;
fd = unix_stream_socket();
if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
goto fail;
if (listen(fd, 5) < 0)
goto fail;
unix_sockaddr_cleanup(&ctx);
return fd;
fail:
saved_errno = errno;
unix_sockaddr_cleanup(&ctx);
close(fd);
errno = saved_errno;
return -1;
}
