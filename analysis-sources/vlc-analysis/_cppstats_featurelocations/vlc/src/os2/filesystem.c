
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>

#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#if defined(HAVE_SYS_SOCKET_H)
#include <sys/socket.h>
#endif
#include <signal.h>

#include <vlc_common.h>
#include <vlc_charset.h>
#include <vlc_fs.h>
#include <vlc_network.h>
#include "libvlc.h" 

int vlc_open (const char *filename, int flags, ...)
{
unsigned int mode = 0;
va_list ap;

va_start (ap, flags);
if (flags & O_CREAT)
mode = va_arg (ap, unsigned int);
va_end (ap);

const char *local_name = ToLocaleDup (filename);

if (local_name == NULL)
{
errno = ENOENT;
return -1;
}

int fd = open (local_name, flags, mode);
if (fd != -1)
fcntl (fd, F_SETFD, FD_CLOEXEC);

free (local_name);
return fd;
}

int vlc_openat (int dir, const char *filename, int flags, ...)
{
errno = ENOSYS;

return -1;
}

int vlc_memfd (void)
{
errno = ENOSYS;
return -1;
}

int vlc_close (int fd)
{
return close (fd);
}

int vlc_mkdir (const char *dirname, mode_t mode)
{
char *locname = ToLocaleDup (dirname);
if (unlikely(locname == NULL))
{
errno = ENOENT;
return -1;
}

int res = mkdir (locname, mode);
free (locname);
return res;
}

DIR *vlc_opendir (const char *dirname)
{
const char *locname = ToLocaleDup (dirname);
if (unlikely(locname == NULL))
{
errno = ENOENT;
return NULL;
}

DIR *dir = opendir (locname);

free (locname);

return dir;
}

const char *vlc_readdir(DIR *dir)
{



struct dirent *ent;
char *path = NULL;





#if 1
long len = sizeof (struct dirent);
#else
long len = fpathconf (dirfd (dir), _PC_NAME_MAX);
len += offsetof (struct dirent, d_name) + 1;
#endif

struct dirent *buf = malloc (len);
if (unlikely(buf == NULL))
return NULL;

int val = readdir_r (dir, buf, &ent);
if (val != 0)
errno = val;
else if (ent != NULL)
path = FromCharset ("", ent->d_name, strlen(ent->d_name));
free (buf);
return path;
}

static int vlc_statEx (const char *filename, struct stat *buf, bool deref)
{
const char *local_name = ToLocaleDup (filename);
if (unlikely(local_name == NULL))
{
errno = ENOENT;
return -1;
}

int res = deref ? stat (local_name, buf)
: lstat (local_name, buf);
free (local_name);
return res;
}

int vlc_stat (const char *filename, struct stat *buf)
{
return vlc_statEx (filename, buf, true);
}

int vlc_lstat (const char *filename, struct stat *buf)
{
return vlc_statEx (filename, buf, false);
}

int vlc_unlink (const char *filename)
{
const char *local_name = ToLocaleDup (filename);
if (unlikely(local_name == NULL))
{
errno = ENOENT;
return -1;
}

int ret = unlink (local_name);
free (local_name);
return ret;
}

int vlc_rename (const char *oldpath, const char *newpath)
{
const char *lo = ToLocaleDup (oldpath);
if (lo == NULL)
goto error;

const char *ln = ToLocaleDup (newpath);
if (ln == NULL)
{
free (lo);
error:
errno = ENOENT;
return -1;
}

int ret = rename (lo, ln);
free (lo);
free (ln);
return ret;
}

char *vlc_getcwd (void)
{

const char *pwd = getenv ("PWD");
if (pwd != NULL)
{
struct stat s1, s2;

if (stat (pwd, &s1) == 0 && stat (".", &s2) == 0
&& s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino)
return ToLocaleDup (pwd);
}


long path_max = pathconf (".", _PC_PATH_MAX);
size_t size = (path_max == -1 || path_max > 4096) ? 4096 : path_max;

for (;; size *= 2)
{
char *buf = malloc (size);
if (unlikely(buf == NULL))
break;

if (getcwd (buf, size) != NULL)
{
char *ret = ToLocaleDup (buf);
free (buf);
return ret; 
}
free (buf);

if (errno != ERANGE)
break;
}
return NULL;
}

int vlc_dup (int oldfd)
{
int newfd;

newfd = dup (oldfd);
if (likely(newfd != -1))
fcntl (newfd, F_SETFD, FD_CLOEXEC);

return newfd;
}

int vlc_pipe (int fds[2])
{
if (vlc_socketpair (AF_LOCAL, SOCK_STREAM, 0, fds, false))
return -1;

shutdown (fds[0], SHUT_WR);
shutdown (fds[1], SHUT_RD);

setmode (fds[0], O_BINARY);
setmode (fds[1], O_BINARY);

return 0;
}

ssize_t vlc_write(int fd, const void *buf, size_t len)
{
struct iovec iov = { .iov_base = (void *)buf, .iov_len = len };

return vlc_writev(fd, &iov, 1);
}

ssize_t vlc_writev(int fd, const struct iovec *iov, int count)
{
sigset_t set, oset;

sigemptyset(&set);
sigaddset(&set, SIGPIPE);
pthread_sigmask(SIG_BLOCK, &set, &oset);

ssize_t val = writev(fd, iov, count);
if (val < 0 && errno == EPIPE)
{
siginfo_t info;
struct timespec ts = { 0, 0 };

while (sigtimedwait(&set, &info, &ts) >= 0 || errno != EAGAIN);
}

if (!sigismember(&oset, SIGPIPE)) 
pthread_sigmask(SIG_SETMASK, &oset, NULL);

return val;
}

static void vlc_socket_setup(int fd, bool nonblock)
{
fcntl(fd, F_SETFD, FD_CLOEXEC);

if (nonblock)
fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

int vlc_socket (int pf, int type, int proto, bool nonblock)
{
int fd = socket(pf, type, proto);
if (fd != -1)
vlc_socket_setup(fd, nonblock);
return fd;
}

int vlc_socketpair(int pf, int type, int proto, int fds[2], bool nonblock)
{
if (socketpair(pf, type, proto, fds))
return -1;

vlc_socket_setup(fds[0], nonblock);
vlc_socket_setup(fds[1], nonblock);
return 0;
}

int vlc_accept (int lfd, struct sockaddr *addr, socklen_t *alen, bool nonblock)
{
do
{
int fd = accept (lfd, addr, alen);
if (fd != -1)
{
fcntl (fd, F_SETFD, FD_CLOEXEC);
if (nonblock)
fcntl (fd, F_SETFL, fcntl (fd, F_GETFL, 0) | O_NONBLOCK);
return fd;
}
}
while (errno == EINTR);

return -1;
}
