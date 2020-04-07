#include "config.h"
#include <sys/stat.h>
#if defined(_WIN32)
#if HAVE_DIRECT_H
#include <direct.h>
#endif
#if HAVE_IO_H
#include <io.h>
#endif
#endif
#if defined(_WIN32)
#include <fcntl.h>
#if defined(lseek)
#undef lseek
#endif
#define lseek(f,p,w) _lseeki64((f), (p), (w))
#if defined(stat)
#undef stat
#endif
#define stat _stati64
#if defined(fstat)
#undef fstat
#endif
#define fstat(f,s) _fstati64((f), (s))
#endif 
#if defined(__ANDROID__)
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if defined(lseek)
#undef lseek
#endif
#define lseek(f,p,w) lseek64((f), (p), (w))
#endif
static inline int is_dos_path(const char *path)
{
#if HAVE_DOS_PATHS
if (path[0] && path[1] == ':')
return 1;
#endif
return 0;
}
#if defined(_WIN32)
#if !defined(S_IRUSR)
#define S_IRUSR S_IREAD
#endif
#if !defined(S_IWUSR)
#define S_IWUSR S_IWRITE
#endif
#endif
#if CONFIG_NETWORK
#if defined(_WIN32)
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
#else
#include <sys/socket.h>
#if !defined(SHUT_RD) 
#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2
#endif
#endif
#if !HAVE_SOCKLEN_T
typedef int socklen_t;
#endif
#if !HAVE_CLOSESOCKET
#define closesocket close
#endif
#if !HAVE_POLL_H
typedef unsigned long nfds_t;
#if HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#if !HAVE_STRUCT_POLLFD
struct pollfd {
int fd;
short events; 
short revents; 
};
#define POLLIN 0x0001 
#define POLLOUT 0x0002 
#define POLLRDNORM POLLIN
#define POLLWRNORM POLLOUT
#define POLLRDBAND 0x0008 
#define POLLWRBAND 0x0010 
#define POLLPRI 0x0020 
#define POLLERR 0x0004 
#define POLLHUP 0x0080 
#define POLLNVAL 0x1000 
#endif
int ff_poll(struct pollfd *fds, nfds_t numfds, int timeout);
#define poll ff_poll
#endif 
#endif 
#if defined(_WIN32)
#include <stdio.h>
#include <windows.h>
#include "libavutil/wchar_filename.h"
#define DEF_FS_FUNCTION(name, wfunc, afunc) static inline int win32_##name(const char *filename_utf8) { wchar_t *filename_w; int ret; if (utf8towchar(filename_utf8, &filename_w)) return -1; if (!filename_w) goto fallback; ret = wfunc(filename_w); av_free(filename_w); return ret; fallback: return afunc(filename_utf8); }
DEF_FS_FUNCTION(unlink, _wunlink, _unlink)
DEF_FS_FUNCTION(mkdir, _wmkdir, _mkdir)
DEF_FS_FUNCTION(rmdir, _wrmdir , _rmdir)
#define DEF_FS_FUNCTION2(name, wfunc, afunc, partype) static inline int win32_##name(const char *filename_utf8, partype par) { wchar_t *filename_w; int ret; if (utf8towchar(filename_utf8, &filename_w)) return -1; if (!filename_w) goto fallback; ret = wfunc(filename_w, par); av_free(filename_w); return ret; fallback: return afunc(filename_utf8, par); }
DEF_FS_FUNCTION2(access, _waccess, _access, int)
DEF_FS_FUNCTION2(stat, _wstati64, _stati64, struct stat*)
static inline int win32_rename(const char *src_utf8, const char *dest_utf8)
{
wchar_t *src_w, *dest_w;
int ret;
if (utf8towchar(src_utf8, &src_w))
return -1;
if (utf8towchar(dest_utf8, &dest_w)) {
av_free(src_w);
return -1;
}
if (!src_w || !dest_w) {
av_free(src_w);
av_free(dest_w);
goto fallback;
}
ret = MoveFileExW(src_w, dest_w, MOVEFILE_REPLACE_EXISTING);
av_free(src_w);
av_free(dest_w);
if (ret)
errno = EPERM;
return ret;
fallback:
#if !HAVE_UWP
ret = MoveFileExA(src_utf8, dest_utf8, MOVEFILE_REPLACE_EXISTING);
if (ret)
errno = EPERM;
#else
ret = rename(src_utf8, dest_utf8);
#endif
return ret;
}
#define mkdir(a, b) win32_mkdir(a)
#define rename win32_rename
#define rmdir win32_rmdir
#define unlink win32_unlink
#define access win32_access
#endif
