#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if defined(HAVE_FSTATVFS)
#include <sys/statvfs.h>
#if defined (HAVE_SYS_MOUNT_H)
#include <sys/param.h>
#include <sys/mount.h>
#endif
#endif
#if defined(HAVE_LINUX_MAGIC_H)
#include <sys/vfs.h>
#include <linux/magic.h>
#endif
#if defined( _WIN32 )
#include <io.h>
#include <ctype.h>
#include <shlwapi.h>
#else
#include <unistd.h>
#endif
#include <dirent.h>
#include <vlc_common.h>
#include "fs.h"
#include <vlc_access.h>
#if defined(_WIN32)
#include <vlc_charset.h>
#endif
#include <vlc_fs.h>
#include <vlc_url.h>
#include <vlc_interrupt.h>
typedef struct
{
int fd;
bool b_pace_control;
} access_sys_t;
#if !defined (_WIN32) && !defined (__OS2__)
static bool IsRemote (int fd)
{
#if defined (HAVE_FSTATVFS) && defined (MNT_LOCAL)
struct statvfs stf;
if (fstatvfs (fd, &stf))
return false;
return !(stf.f_flag & MNT_LOCAL);
#elif defined (HAVE_LINUX_MAGIC_H)
struct statfs stf;
if (fstatfs (fd, &stf))
return false;
switch ((unsigned long)stf.f_type)
{
case AFS_SUPER_MAGIC:
case CODA_SUPER_MAGIC:
case NCP_SUPER_MAGIC:
case NFS_SUPER_MAGIC:
case SMB_SUPER_MAGIC:
case 0xFF534D42 :
return true;
}
return false;
#else
(void)fd;
return false;
#endif
}
#define IsRemote(fd,path) IsRemote(fd)
#else 
static bool IsRemote (const char *path)
{
#if !defined(__OS2__) && !VLC_WINSTORE_APP
wchar_t *wpath = ToWide (path);
bool is_remote = (wpath != NULL && PathIsNetworkPathW (wpath));
free (wpath);
return is_remote;
#else
return (! strncmp(path, "\\\\", 2));
#endif
}
#define IsRemote(fd,path) IsRemote(path)
#endif
#if !defined(HAVE_POSIX_FADVISE)
#define posix_fadvise(fd, off, len, adv)
#endif
static ssize_t Read (stream_t *, void *, size_t);
static int FileSeek (stream_t *, uint64_t);
static int FileControl (stream_t *, int, va_list);
int FileOpen( vlc_object_t *p_this )
{
stream_t *p_access = (stream_t*)p_this;
int fd = -1;
if (!strcasecmp (p_access->psz_name, "fd"))
{
char *end;
int oldfd = strtol (p_access->psz_location, &end, 10);
if (*end == '\0')
fd = vlc_dup (oldfd);
else if (*end == '/' && end > p_access->psz_location)
{
char *name = vlc_uri_decode_duplicate (end - 1);
if (name != NULL)
{
name[0] = '.';
fd = vlc_openat (oldfd, name, O_RDONLY | O_NONBLOCK);
free (name);
}
}
}
else
{
if (unlikely(p_access->psz_filepath == NULL))
return VLC_EGENERIC;
fd = vlc_open (p_access->psz_filepath, O_RDONLY | O_NONBLOCK);
}
if (fd == -1)
{
msg_Err (p_access, "cannot open file %s (%s)",
p_access->psz_filepath ? p_access->psz_filepath
: p_access->psz_location,
vlc_strerror_c(errno));
return VLC_EGENERIC;
}
struct stat st;
if (fstat (fd, &st))
{
msg_Err (p_access, "read error: %s", vlc_strerror_c(errno));
goto error;
}
#if O_NONBLOCK
fcntl (fd, F_SETFL, fcntl (fd, F_GETFL) & ~O_NONBLOCK);
#endif
if (S_ISDIR (st.st_mode))
{
#if defined(HAVE_FDOPENDIR)
DIR *p_dir = fdopendir(fd);
if (!p_dir) {
msg_Err (p_access, "fdopendir error: %s", vlc_strerror_c(errno));
goto error;
}
return DirInit (p_access, p_dir);
#else
msg_Dbg (p_access, "ignoring directory");
goto error;
#endif
}
access_sys_t *p_sys = vlc_obj_malloc(p_this, sizeof (*p_sys));
if (unlikely(p_sys == NULL))
goto error;
p_access->pf_read = Read;
p_access->pf_block = NULL;
p_access->pf_control = FileControl;
p_access->p_sys = p_sys;
p_sys->fd = fd;
if (S_ISREG (st.st_mode) || S_ISBLK (st.st_mode))
{
p_access->pf_seek = FileSeek;
p_sys->b_pace_control = true;
posix_fadvise (fd, 0, 4096, POSIX_FADV_WILLNEED);
posix_fadvise (fd, 0, 0, POSIX_FADV_NOREUSE);
#if defined(F_NOCACHE)
fcntl (fd, F_NOCACHE, 0);
#endif
#if defined(F_RDAHEAD)
if (IsRemote(fd, p_access->psz_filepath))
fcntl (fd, F_RDAHEAD, 0);
else
fcntl (fd, F_RDAHEAD, 1);
#endif
}
else
{
p_access->pf_seek = NULL;
p_sys->b_pace_control = strcasecmp (p_access->psz_name, "stream");
}
return VLC_SUCCESS;
error:
vlc_close (fd);
return VLC_EGENERIC;
}
void FileClose (vlc_object_t * p_this)
{
stream_t *p_access = (stream_t*)p_this;
if (p_access->pf_read == NULL)
{
DirClose (p_this);
return;
}
access_sys_t *p_sys = p_access->p_sys;
vlc_close (p_sys->fd);
}
static ssize_t Read (stream_t *p_access, void *p_buffer, size_t i_len)
{
access_sys_t *p_sys = p_access->p_sys;
int fd = p_sys->fd;
ssize_t val = vlc_read_i11e (fd, p_buffer, i_len);
if (val < 0)
{
switch (errno)
{
case EINTR:
case EAGAIN:
return -1;
}
msg_Err (p_access, "read error: %s", vlc_strerror_c(errno));
val = 0;
}
return val;
}
static int FileSeek (stream_t *p_access, uint64_t i_pos)
{
access_sys_t *sys = p_access->p_sys;
if (lseek(sys->fd, i_pos, SEEK_SET) == (off_t)-1)
return VLC_EGENERIC;
return VLC_SUCCESS;
}
static int FileControl( stream_t *p_access, int i_query, va_list args )
{
access_sys_t *p_sys = p_access->p_sys;
bool *pb_bool;
vlc_tick_t *pi_64;
switch( i_query )
{
case STREAM_CAN_SEEK:
case STREAM_CAN_FASTSEEK:
pb_bool = va_arg( args, bool * );
*pb_bool = (p_access->pf_seek != NULL);
break;
case STREAM_CAN_PAUSE:
case STREAM_CAN_CONTROL_PACE:
pb_bool = va_arg( args, bool * );
*pb_bool = p_sys->b_pace_control;
break;
case STREAM_GET_SIZE:
{
struct stat st;
if (fstat (p_sys->fd, &st) || !S_ISREG(st.st_mode))
return VLC_EGENERIC;
*va_arg( args, uint64_t * ) = st.st_size;
break;
}
case STREAM_GET_PTS_DELAY:
pi_64 = va_arg( args, vlc_tick_t * );
if (IsRemote (p_sys->fd, p_access->psz_filepath))
*pi_64 = VLC_TICK_FROM_MS(
var_InheritInteger (p_access, "network-caching") );
else
*pi_64 = VLC_TICK_FROM_MS(
var_InheritInteger (p_access, "file-caching") );
break;
case STREAM_SET_PAUSE_STATE:
break;
default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
