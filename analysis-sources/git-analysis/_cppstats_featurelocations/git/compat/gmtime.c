#include "../git-compat-util.h"
#undef gmtime
#undef gmtime_r

struct tm *git_gmtime(const time_t *timep)
{
static struct tm result;
return git_gmtime_r(timep, &result);
}

struct tm *git_gmtime_r(const time_t *timep, struct tm *result)
{
struct tm *ret;

memset(result, 0, sizeof(*result));
ret = gmtime_r(timep, result);






if (ret && !ret->tm_mday) {
ret = NULL;
errno = EOVERFLOW;
}

return ret;
}
