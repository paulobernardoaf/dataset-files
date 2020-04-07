



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <string.h>
#include <errno.h>
#include <locale.h>
#include <assert.h>

#include <vlc_common.h>

static const char *vlc_strerror_l(int errnum, const char *lname)
{
int saved_errno = errno;
locale_t loc = newlocale(LC_MESSAGES_MASK, lname, (locale_t)0);

if (unlikely(loc == (locale_t)0))
{
if (errno == ENOENT) 
loc = newlocale(LC_MESSAGES_MASK, "C", (locale_t)0);

if (unlikely(loc == (locale_t)0))
{
assert(errno != EINVAL && errno != ENOENT);
errno = saved_errno;
return "Error message unavailable";
}
errno = saved_errno;
}

const char *buf = strerror_l(errnum, loc);

freelocale(loc);
return buf;
}







const char *vlc_strerror(int errnum)
{

return vlc_strerror_l(errnum, "");
}







const char *vlc_strerror_c(int errnum)
{
return vlc_strerror_l(errnum, "C");
}
