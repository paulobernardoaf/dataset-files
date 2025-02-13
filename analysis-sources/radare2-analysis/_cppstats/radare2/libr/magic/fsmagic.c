#include <r_userconf.h>
#if !USE_LIB_MAGIC
#include <r_magic.h>
#include "file.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#if defined(MAJOR_IN_MKDEV)
#include <sys/mkdev.h>
#define HAVE_MAJOR
#endif
#if defined(MAJOR_IN_SYSMACROS)
#include <sys/sysmacros.h>
#define HAVE_MAJOR
#endif
#if defined(major)
#define HAVE_MAJOR
#endif
#if !defined(HAVE_MAJOR)
#define major(dev) (((dev) >> 8) & 0xff)
#define minor(dev) ((dev) & 0xff)
#endif
#undef HAVE_MAJOR
static int bad_link(RMagic *ms, int err, char *buf) {
#if defined(ELOOP)
const char *errfmt = (err == ELOOP)?
"symbolic link in a loop":
"broken symbolic link to `%s'";
#else
const char *errfmt = "broken symbolic link to `%s'";
#endif
if (ms->flags & R_MAGIC_ERROR) {
file_error (ms, err, errfmt, buf);
return -1;
} 
if (file_printf (ms, errfmt, buf) == -1)
return -1;
return 1;
}
int file_fsmagic(struct r_magic_set *ms, const char *fn, struct stat *sb) {
int ret = 0;
int mime = ms->flags & R_MAGIC_MIME;
#if defined(S_IFLNK)
char buf[BUFSIZ+4];
int nch;
struct stat tstatbuf;
#endif
if (!fn)
return 0;
#if defined(S_IFLNK)
if ((ms->flags & R_MAGIC_SYMLINK) == 0)
ret = lstat (fn, sb);
else
#endif
ret = stat (fn, sb); 
if (ret) {
if (ms->flags & R_MAGIC_ERROR) {
file_error (ms, errno, "cannot stat `%s'", fn);
return -1;
}
if (file_printf (ms, "cannot open `%s' (%s)",
fn, strerror (errno)) == -1)
return -1;
return 1;
}
if (mime) {
if ((sb->st_mode & S_IFMT) != S_IFREG) {
if ((mime & R_MAGIC_MIME_TYPE) &&
file_printf (ms, "application/x-not-regular-file")
== -1)
return -1;
return 1;
}
} else {
#if defined(S_ISUID)
if (sb->st_mode & S_ISUID) 
if (file_printf(ms, "setuid ") == -1)
return -1;
#endif
#if defined(S_ISGID)
if (sb->st_mode & S_ISGID) 
if (file_printf(ms, "setgid ") == -1)
return -1;
#endif
#if defined(S_ISVTX)
if (sb->st_mode & S_ISVTX) 
if (file_printf(ms, "sticky ") == -1)
return -1;
#endif
}
switch (sb->st_mode & S_IFMT) {
case S_IFDIR:
if (file_printf (ms, "directory") == -1)
return -1;
return 1;
#if defined(S_IFCHR)
case S_IFCHR:
if ((ms->flags & R_MAGIC_DEVICES) != 0)
break;
#if defined(HAVE_STAT_ST_RDEV)
#if defined(dv_unit)
if (file_printf (ms, "character special (%d/%d/%d)",
major (sb->st_rdev), dv_unit(sb->st_rdev),
dv_subunit (sb->st_rdev)) == -1)
return -1;
#else
if (file_printf (ms, "character special (%ld/%ld)",
(long) major (sb->st_rdev), (long) minor(sb->st_rdev)) == -1)
return -1;
#endif
#else
if (file_printf (ms, "character special") == -1)
return -1;
#endif
return 1;
#endif
#if defined(S_IFBLK)
case S_IFBLK:
if ((ms->flags & R_MAGIC_DEVICES) != 0)
break;
#if defined(HAVE_STAT_ST_RDEV)
#if defined(dv_unit)
if (file_printf(ms, "block special (%d/%d/%d)",
major(sb->st_rdev), dv_unit(sb->st_rdev),
dv_subunit(sb->st_rdev)) == -1)
return -1;
#else
if (file_printf(ms, "block special (%ld/%ld)",
(long)major(sb->st_rdev), (long)minor(sb->st_rdev)) == -1)
return -1;
#endif
#else
if (file_printf(ms, "block special") == -1)
return -1;
#endif
return 1;
#endif
#if defined(S_IFIFO)
case S_IFIFO:
if((ms->flags & R_MAGIC_DEVICES) != 0)
break;
if (file_printf(ms, "fifo (named pipe)") == -1)
return -1;
return 1;
#endif
#if defined(S_IFDOOR)
case S_IFDOOR:
return (file_printf (ms, "door") == -1)? -1:1;
#endif
#if defined(S_IFLNK)
case S_IFLNK:
if ((nch = readlink (fn, buf, BUFSIZ-1)) <= 0) {
if (ms->flags & R_MAGIC_ERROR) {
file_error (ms, errno, "unreadable symlink `%s'", fn);
return -1;
}
if (file_printf(ms,
"unreadable symlink `%s' (%s)", fn,
strerror(errno)) == -1)
return -1;
return 1;
}
buf[nch] = '\0'; 
if (*buf == '/') {
if (stat(buf, &tstatbuf) < 0)
return bad_link(ms, errno, buf);
} else {
char *tmp;
char buf2[BUFSIZ+BUFSIZ+4];
if (!(tmp = strrchr(fn, '/'))) {
tmp = buf; 
} else {
if (tmp - fn + 1 > BUFSIZ) {
if (ms->flags & R_MAGIC_ERROR) {
file_error (ms, 0, "path too long: `%s'", buf);
return -1;
}
if (file_printf (ms, "path too long: `%s'", fn) == -1)
return -1;
return 1;
}
snprintf (buf2, sizeof (buf2), "%s%s", fn, buf);
tmp = buf2;
}
if (stat (tmp, &tstatbuf) < 0)
return bad_link(ms, errno, buf);
}
if ((ms->flags & R_MAGIC_SYMLINK) != 0) {
const char *p;
ms->flags &= R_MAGIC_SYMLINK;
p = r_magic_file(ms, buf);
ms->flags |= R_MAGIC_SYMLINK;
return p != NULL ? 1 : -1;
} else { 
if (file_printf (ms, "symbolic link to `%s'", buf) == -1)
return -1;
}
return 1;
#endif
#if defined(S_IFSOCK)
case S_IFSOCK:
if (file_printf(ms, "socket") == -1)
return -1;
return 1;
#endif
case S_IFREG:
break;
default:
file_error (ms, 0, "invalid mode 0%o", sb->st_mode);
return -1;
}
if ((ms->flags & R_MAGIC_DEVICES) == 0 && sb->st_size == 0) {
if ((!mime || (mime & R_MAGIC_MIME_TYPE)) &&
file_printf (ms, mime ? "application/x-empty" : "empty") == -1)
return -1;
return 1;
}
return 0;
}
#endif
