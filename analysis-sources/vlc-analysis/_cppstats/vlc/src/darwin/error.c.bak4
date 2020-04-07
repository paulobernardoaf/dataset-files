




















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdlib.h>
#include <errno.h>

#include <vlc_common.h>

const char *vlc_strerror_c(int errnum)
{

if ((unsigned)errnum < (unsigned)sys_nerr)
return sys_errlist[errnum];

return _("Unknown error");
}

const char *vlc_strerror(int errnum)
{
return vlc_strerror_c(errnum);
}
