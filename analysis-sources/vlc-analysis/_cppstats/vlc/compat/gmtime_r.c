#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
struct tm *gmtime_r (const time_t *timep, struct tm *tm)
{
static const unsigned short normal[12] =
{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const unsigned short leap[12] =
{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
lldiv_t d;
const unsigned short *months;
d.quot = 60LL * 60 * 24 * (4 * 146097 + 135140) + *timep;
d = lldiv (d.quot, 60); 
if (*timep < 0)
{
d.rem += 60;
d.quot--;
}
tm->tm_sec = d.rem;
d = lldiv (d.quot, 60); 
tm->tm_min = d.rem;
d = lldiv (d.quot, 24); 
tm->tm_hour = d.rem;
tm->tm_wday = (d.quot + 6) % 7; 
d = lldiv (d.quot, 146097); 
tm->tm_year = 400 * d.quot;
if (d.rem >= 36525)
{ 
d.rem -= 36525;
tm->tm_year += 100;
d = lldiv (d.rem, 36524); 
tm->tm_year += d.quot * 100;
if (d.rem >= 59) 
d.rem++; 
}
tm->tm_year -= 1900; 
d = lldiv (d.rem, 1461); 
tm->tm_year += 4 * d.quot;
if (d.rem >= 366)
{ 
d.rem -= 366;
tm->tm_year++;
d = lldiv (d.rem, 365); 
tm->tm_year += d.quot;
months = normal;
}
else
months = leap;
tm->tm_yday = d.rem;
for (tm->tm_mon = 0; d.rem >= months[tm->tm_mon]; tm->tm_mon++)
d.rem -= months[tm->tm_mon]; 
tm->tm_mday = d.rem + 1;
tm->tm_isdst = 0; 
return tm;
}
