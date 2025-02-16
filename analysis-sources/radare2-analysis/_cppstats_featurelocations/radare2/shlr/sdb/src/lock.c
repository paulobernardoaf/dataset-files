

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "sdb.h"

SDB_API const char *sdb_lock_file(const char *f) {
static char buf[128];
size_t len;
if (!f || !*f) {
return NULL;
}
len = strlen (f);
if (len + 10 > sizeof buf) {
return NULL;
}
memcpy (buf, f, len);
strcpy (buf + len, ".lock");
return buf;
}

#define os_getpid() getpid()

SDB_API bool sdb_lock(const char *s) {
int fd;
char *pid, pidstr[64];
if (!s) {
return false;
}
fd = open (s, O_CREAT | O_TRUNC | O_WRONLY | O_EXCL, SDB_MODE);
if (fd == -1) {
return false;
}
pid = sdb_itoa (getpid(), pidstr, 10);
if (pid) {
if ((write (fd, pid, strlen (pid)) < 0)
|| (write (fd, "\n", 1) < 0)) {
close (fd);
return false;
}
}
close (fd);
return true;
}

SDB_API int sdb_lock_wait(const char *s) {


while (!sdb_lock (s)) {

#if __SDB_WINDOWS__
Sleep (500); 
#else

sleep (1); 
#endif
}
return 1;
}

SDB_API void sdb_unlock(const char *s) {

unlink (s);
}

#if TEST
main () {
int r;
r = sdb_lock (".lock");
printf ("%d\n", r);
r = sdb_lock (".lock");
printf ("%d\n", r);
sdb_unlock (".lock");
r = sdb_lock (".lock");
printf ("%d\n", r);
}
#endif
