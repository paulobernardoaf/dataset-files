#include "ruby/config.h"
#include "ruby/ruby.h"
#if defined _WIN32
#elif defined HAVE_FCNTL && defined HAVE_FCNTL_H
#if !defined(LOCK_SH)
#define LOCK_SH 1
#endif
#if !defined(LOCK_EX)
#define LOCK_EX 2
#endif
#if !defined(LOCK_NB)
#define LOCK_NB 4
#endif
#if !defined(LOCK_UN)
#define LOCK_UN 8
#endif
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
int
flock(int fd, int operation)
{
struct flock lock;
switch (operation & ~LOCK_NB) {
case LOCK_SH:
lock.l_type = F_RDLCK;
break;
case LOCK_EX:
lock.l_type = F_WRLCK;
break;
case LOCK_UN:
lock.l_type = F_UNLCK;
break;
default:
errno = EINVAL;
return -1;
}
lock.l_whence = SEEK_SET;
lock.l_start = lock.l_len = 0L;
return fcntl(fd, (operation & LOCK_NB) ? F_SETLK : F_SETLKW, &lock);
}
#elif defined(HAVE_LOCKF)
#include <unistd.h>
#include <errno.h>
#if !defined(F_ULOCK)
#define F_ULOCK 0 
#endif
#if !defined(F_LOCK)
#define F_LOCK 1 
#endif
#if !defined(F_TLOCK)
#define F_TLOCK 2 
#endif
#if !defined(F_TEST)
#define F_TEST 3 
#endif
#if !defined(LOCK_SH)
#define LOCK_SH 1
#endif
#if !defined(LOCK_EX)
#define LOCK_EX 2
#endif
#if !defined(LOCK_NB)
#define LOCK_NB 4
#endif
#if !defined(LOCK_UN)
#define LOCK_UN 8
#endif
int
flock(int fd, int operation)
{
switch (operation) {
case LOCK_SH:
rb_notimplement();
return -1;
case LOCK_EX:
return lockf (fd, F_LOCK, 0);
case LOCK_SH|LOCK_NB:
rb_notimplement();
return -1;
case LOCK_EX|LOCK_NB:
return lockf (fd, F_TLOCK, 0);
case LOCK_UN:
return lockf (fd, F_ULOCK, 0);
default:
errno = EINVAL;
return -1;
}
}
#else
int
flock(int fd, int operation)
{
rb_notimplement();
return -1;
}
#endif
