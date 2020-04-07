



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

DIR *fdopendir (int fd)
{
#if defined(F_GETFL)

int mode = fcntl (fd, F_GETFL);
if (mode == -1 || (mode & O_ACCMODE) == O_WRONLY)
{
errno = EBADF;
return NULL;
}
#endif

struct stat st;
if (fstat (fd, &st))
return NULL;

if (!S_ISDIR (st.st_mode))
{
errno = ENOTDIR;
return NULL;
}



char path[sizeof ("/proc/self/fd/") + 3 * sizeof (int)];
sprintf (path, "/proc/self/fd/%u", fd);

DIR *dir = opendir (path);
if (dir != NULL)
{
close (fd);
return dir;
}


switch (errno)
{
case EACCES:
#if defined(ELOOP)
case ELOOP:
#endif
case ENAMETOOLONG:
case ENOENT:
case EMFILE:
case ENFILE:
errno = EIO;
}
return NULL;
}
