#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <dirent.h>
#include <errno.h>
int (dirfd) (DIR *dir)
{
#if defined(dirfd)
return dirfd (dir);
#else
(void) dir;
#if defined(ENOTSUP)
errno = ENOTSUP;
#endif
return -1;
#endif
}
