











#include "tuklib_open_stdxxx.h"

#if !defined(TUKLIB_DOSLIKE)
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif


extern void
tuklib_open_stdxxx(int err_status)
{
#if defined(TUKLIB_DOSLIKE)

(void)err_status;

#else
for (int i = 0; i <= 2; ++i) {

if (fcntl(i, F_GETFD) == -1 && errno == EBADF) {




const int fd = open("/dev/null", O_NOCTTY
| (i == 0 ? O_WRONLY : O_RDONLY));

if (fd != i) {
if (fd != -1)
(void)close(fd);






exit(err_status);
}
}
}
#endif

return;
}
