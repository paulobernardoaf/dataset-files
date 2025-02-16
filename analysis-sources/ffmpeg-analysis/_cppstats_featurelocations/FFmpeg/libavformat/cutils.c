




















#include "libavutil/time_internal.h"
#include "avformat.h"
#include "internal.h"

#define ISLEAP(y) (((y) % 4 == 0) && (((y) % 100) != 0 || ((y) % 400) == 0))
#define LEAPS_COUNT(y) ((y)/4 - (y)/100 + (y)/400)



struct tm *ff_brktimegm(time_t secs, struct tm *tm)
{
tm = gmtime_r(&secs, tm);

tm->tm_year += 1900; 
tm->tm_mon += 1; 

return tm;
}
