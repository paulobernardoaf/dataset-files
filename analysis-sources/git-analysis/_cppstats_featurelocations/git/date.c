





#include "cache.h"




static time_t tm_to_time_t(const struct tm *tm)
{
static const int mdays[] = {
0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};
int year = tm->tm_year - 70;
int month = tm->tm_mon;
int day = tm->tm_mday;

if (year < 0 || year > 129) 
return -1;
if (month < 0 || month > 11) 
return -1;
if (month < 2 || (year + 2) % 4)
day--;
if (tm->tm_hour < 0 || tm->tm_min < 0 || tm->tm_sec < 0)
return -1;
return (year * 365 + (year + 1) / 4 + mdays[month] + day) * 24*60*60UL +
tm->tm_hour * 60*60 + tm->tm_min * 60 + tm->tm_sec;
}

static const char *month_names[] = {
"January", "February", "March", "April", "May", "June",
"July", "August", "September", "October", "November", "December"
};

static const char *weekday_names[] = {
"Sundays", "Mondays", "Tuesdays", "Wednesdays", "Thursdays", "Fridays", "Saturdays"
};

static time_t gm_time_t(timestamp_t time, int tz)
{
int minutes;

minutes = tz < 0 ? -tz : tz;
minutes = (minutes / 100)*60 + (minutes % 100);
minutes = tz < 0 ? -minutes : minutes;

if (minutes > 0) {
if (unsigned_add_overflows(time, minutes * 60))
die("Timestamp+tz too large: %"PRItime" +%04d",
time, tz);
} else if (time < -minutes * 60)
die("Timestamp before Unix epoch: %"PRItime" %04d", time, tz);
time += minutes * 60;
if (date_overflows(time))
die("Timestamp too large for this system: %"PRItime, time);
return (time_t)time;
}






static struct tm *time_to_tm(timestamp_t time, int tz, struct tm *tm)
{
time_t t = gm_time_t(time, tz);
return gmtime_r(&t, tm);
}

static struct tm *time_to_tm_local(timestamp_t time, struct tm *tm)
{
time_t t = time;
return localtime_r(&t, tm);
}





static int local_time_tzoffset(time_t t, struct tm *tm)
{
time_t t_local;
int offset, eastwest;

localtime_r(&t, tm);
t_local = tm_to_time_t(tm);
if (t_local == -1)
return 0; 
if (t_local < t) {
eastwest = -1;
offset = t - t_local;
} else {
eastwest = 1;
offset = t_local - t;
}
offset /= 60; 
offset = (offset % 60) + ((offset / 60) * 100);
return offset * eastwest;
}





static int local_tzoffset(timestamp_t time)
{
struct tm tm;

if (date_overflows(time))
die("Timestamp too large for this system: %"PRItime, time);

return local_time_tzoffset((time_t)time, &tm);
}

static void get_time(struct timeval *now)
{
const char *x;

x = getenv("GIT_TEST_DATE_NOW");
if (x) {
now->tv_sec = atoi(x);
now->tv_usec = 0;
}
else
gettimeofday(now, NULL);
}

void show_date_relative(timestamp_t time, struct strbuf *timebuf)
{
struct timeval now;
timestamp_t diff;

get_time(&now);
if (now.tv_sec < time) {
strbuf_addstr(timebuf, _("in the future"));
return;
}
diff = now.tv_sec - time;
if (diff < 90) {
strbuf_addf(timebuf,
Q_("%"PRItime" second ago", "%"PRItime" seconds ago", diff), diff);
return;
}

diff = (diff + 30) / 60;
if (diff < 90) {
strbuf_addf(timebuf,
Q_("%"PRItime" minute ago", "%"PRItime" minutes ago", diff), diff);
return;
}

diff = (diff + 30) / 60;
if (diff < 36) {
strbuf_addf(timebuf,
Q_("%"PRItime" hour ago", "%"PRItime" hours ago", diff), diff);
return;
}

diff = (diff + 12) / 24;
if (diff < 14) {
strbuf_addf(timebuf,
Q_("%"PRItime" day ago", "%"PRItime" days ago", diff), diff);
return;
}

if (diff < 70) {
strbuf_addf(timebuf,
Q_("%"PRItime" week ago", "%"PRItime" weeks ago", (diff + 3) / 7),
(diff + 3) / 7);
return;
}

if (diff < 365) {
strbuf_addf(timebuf,
Q_("%"PRItime" month ago", "%"PRItime" months ago", (diff + 15) / 30),
(diff + 15) / 30);
return;
}

if (diff < 1825) {
timestamp_t totalmonths = (diff * 12 * 2 + 365) / (365 * 2);
timestamp_t years = totalmonths / 12;
timestamp_t months = totalmonths % 12;
if (months) {
struct strbuf sb = STRBUF_INIT;
strbuf_addf(&sb, Q_("%"PRItime" year", "%"PRItime" years", years), years);
strbuf_addf(timebuf,

Q_("%s, %"PRItime" month ago", "%s, %"PRItime" months ago", months),
sb.buf, months);
strbuf_release(&sb);
} else
strbuf_addf(timebuf,
Q_("%"PRItime" year ago", "%"PRItime" years ago", years), years);
return;
}

strbuf_addf(timebuf,
Q_("%"PRItime" year ago", "%"PRItime" years ago", (diff + 183) / 365),
(diff + 183) / 365);
}

struct date_mode *date_mode_from_type(enum date_mode_type type)
{
static struct date_mode mode;
if (type == DATE_STRFTIME)
BUG("cannot create anonymous strftime date_mode struct");
mode.type = type;
mode.local = 0;
return &mode;
}

static void show_date_normal(struct strbuf *buf, timestamp_t time, struct tm *tm, int tz, struct tm *human_tm, int human_tz, int local)
{
struct {
unsigned int year:1,
date:1,
wday:1,
time:1,
seconds:1,
tz:1;
} hide = { 0 };

hide.tz = local || tz == human_tz;
hide.year = tm->tm_year == human_tm->tm_year;
if (hide.year) {
if (tm->tm_mon == human_tm->tm_mon) {
if (tm->tm_mday > human_tm->tm_mday) {

} else if (tm->tm_mday == human_tm->tm_mday) {
hide.date = hide.wday = 1;
} else if (tm->tm_mday + 5 > human_tm->tm_mday) {

hide.date = 1;
}
}
}


if (hide.wday) {
show_date_relative(time, buf);
return;
}










if (human_tm->tm_year) {
hide.seconds = 1;
hide.tz |= !hide.date;
hide.wday = hide.time = !hide.year;
}

if (!hide.wday)
strbuf_addf(buf, "%.3s ", weekday_names[tm->tm_wday]);
if (!hide.date)
strbuf_addf(buf, "%.3s %d ", month_names[tm->tm_mon], tm->tm_mday);


if (!hide.time) {
strbuf_addf(buf, "%02d:%02d", tm->tm_hour, tm->tm_min);
if (!hide.seconds)
strbuf_addf(buf, ":%02d", tm->tm_sec);
} else
strbuf_rtrim(buf);

if (!hide.year)
strbuf_addf(buf, " %d", tm->tm_year + 1900);

if (!hide.tz)
strbuf_addf(buf, " %+05d", tz);
}

const char *show_date(timestamp_t time, int tz, const struct date_mode *mode)
{
struct tm *tm;
struct tm tmbuf = { 0 };
struct tm human_tm = { 0 };
int human_tz = -1;
static struct strbuf timebuf = STRBUF_INIT;

if (mode->type == DATE_UNIX) {
strbuf_reset(&timebuf);
strbuf_addf(&timebuf, "%"PRItime, time);
return timebuf.buf;
}

if (mode->type == DATE_HUMAN) {
struct timeval now;

get_time(&now);


human_tz = local_time_tzoffset(now.tv_sec, &human_tm);
}

if (mode->local)
tz = local_tzoffset(time);

if (mode->type == DATE_RAW) {
strbuf_reset(&timebuf);
strbuf_addf(&timebuf, "%"PRItime" %+05d", time, tz);
return timebuf.buf;
}

if (mode->type == DATE_RELATIVE) {
strbuf_reset(&timebuf);
show_date_relative(time, &timebuf);
return timebuf.buf;
}

if (mode->local)
tm = time_to_tm_local(time, &tmbuf);
else
tm = time_to_tm(time, tz, &tmbuf);
if (!tm) {
tm = time_to_tm(0, 0, &tmbuf);
tz = 0;
}

strbuf_reset(&timebuf);
if (mode->type == DATE_SHORT)
strbuf_addf(&timebuf, "%04d-%02d-%02d", tm->tm_year + 1900,
tm->tm_mon + 1, tm->tm_mday);
else if (mode->type == DATE_ISO8601)
strbuf_addf(&timebuf, "%04d-%02d-%02d %02d:%02d:%02d %+05d",
tm->tm_year + 1900,
tm->tm_mon + 1,
tm->tm_mday,
tm->tm_hour, tm->tm_min, tm->tm_sec,
tz);
else if (mode->type == DATE_ISO8601_STRICT) {
char sign = (tz >= 0) ? '+' : '-';
tz = abs(tz);
strbuf_addf(&timebuf, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
tm->tm_year + 1900,
tm->tm_mon + 1,
tm->tm_mday,
tm->tm_hour, tm->tm_min, tm->tm_sec,
sign, tz / 100, tz % 100);
} else if (mode->type == DATE_RFC2822)
strbuf_addf(&timebuf, "%.3s, %d %.3s %d %02d:%02d:%02d %+05d",
weekday_names[tm->tm_wday], tm->tm_mday,
month_names[tm->tm_mon], tm->tm_year + 1900,
tm->tm_hour, tm->tm_min, tm->tm_sec, tz);
else if (mode->type == DATE_STRFTIME)
strbuf_addftime(&timebuf, mode->strftime_fmt, tm, tz,
!mode->local);
else
show_date_normal(&timebuf, time, tm, tz, &human_tm, human_tz, mode->local);
return timebuf.buf;
}







static const struct {
const char *name;
int offset;
int dst;
} timezone_names[] = {
{ "IDLW", -12, 0, }, 
{ "NT", -11, 0, }, 
{ "CAT", -10, 0, }, 
{ "HST", -10, 0, }, 
{ "HDT", -10, 1, }, 
{ "YST", -9, 0, }, 
{ "YDT", -9, 1, }, 
{ "PST", -8, 0, }, 
{ "PDT", -8, 1, }, 
{ "MST", -7, 0, }, 
{ "MDT", -7, 1, }, 
{ "CST", -6, 0, }, 
{ "CDT", -6, 1, }, 
{ "EST", -5, 0, }, 
{ "EDT", -5, 1, }, 
{ "AST", -3, 0, }, 
{ "ADT", -3, 1, }, 
{ "WAT", -1, 0, }, 

{ "GMT", 0, 0, }, 
{ "UTC", 0, 0, }, 
{ "Z", 0, 0, }, 

{ "WET", 0, 0, }, 
{ "BST", 0, 1, }, 
{ "CET", +1, 0, }, 
{ "MET", +1, 0, }, 
{ "MEWT", +1, 0, }, 
{ "MEST", +1, 1, }, 
{ "CEST", +1, 1, }, 
{ "MESZ", +1, 1, }, 
{ "FWT", +1, 0, }, 
{ "FST", +1, 1, }, 
{ "EET", +2, 0, }, 
{ "EEST", +2, 1, }, 
{ "WAST", +7, 0, }, 
{ "WADT", +7, 1, }, 
{ "CCT", +8, 0, }, 
{ "JST", +9, 0, }, 
{ "EAST", +10, 0, }, 
{ "EADT", +10, 1, }, 
{ "GST", +10, 0, }, 
{ "NZT", +12, 0, }, 
{ "NZST", +12, 0, }, 
{ "NZDT", +12, 1, }, 
{ "IDLE", +12, 0, }, 
};

static int match_string(const char *date, const char *str)
{
int i = 0;

for (i = 0; *date; date++, str++, i++) {
if (*date == *str)
continue;
if (toupper(*date) == toupper(*str))
continue;
if (!isalnum(*date))
break;
return 0;
}
return i;
}

static int skip_alpha(const char *date)
{
int i = 0;
do {
i++;
} while (isalpha(date[i]));
return i;
}




static int match_alpha(const char *date, struct tm *tm, int *offset)
{
int i;

for (i = 0; i < 12; i++) {
int match = match_string(date, month_names[i]);
if (match >= 3) {
tm->tm_mon = i;
return match;
}
}

for (i = 0; i < 7; i++) {
int match = match_string(date, weekday_names[i]);
if (match >= 3) {
tm->tm_wday = i;
return match;
}
}

for (i = 0; i < ARRAY_SIZE(timezone_names); i++) {
int match = match_string(date, timezone_names[i].name);
if (match >= 3 || match == strlen(timezone_names[i].name)) {
int off = timezone_names[i].offset;


off += timezone_names[i].dst;


if (*offset == -1)
*offset = 60*off;

return match;
}
}

if (match_string(date, "PM") == 2) {
tm->tm_hour = (tm->tm_hour % 12) + 12;
return 2;
}

if (match_string(date, "AM") == 2) {
tm->tm_hour = (tm->tm_hour % 12) + 0;
return 2;
}


return skip_alpha(date);
}

static int is_date(int year, int month, int day, struct tm *now_tm, time_t now, struct tm *tm)
{
if (month > 0 && month < 13 && day > 0 && day < 32) {
struct tm check = *tm;
struct tm *r = (now_tm ? &check : tm);
time_t specified;

r->tm_mon = month - 1;
r->tm_mday = day;
if (year == -1) {
if (!now_tm)
return 1;
r->tm_year = now_tm->tm_year;
}
else if (year >= 1970 && year < 2100)
r->tm_year = year - 1900;
else if (year > 70 && year < 100)
r->tm_year = year;
else if (year < 38)
r->tm_year = year + 100;
else
return 0;
if (!now_tm)
return 1;

specified = tm_to_time_t(r);





if ((specified != -1) && (now + 10*24*3600 < specified))
return 0;
tm->tm_mon = r->tm_mon;
tm->tm_mday = r->tm_mday;
if (year != -1)
tm->tm_year = r->tm_year;
return 1;
}
return 0;
}

static int match_multi_number(timestamp_t num, char c, const char *date,
char *end, struct tm *tm, time_t now)
{
struct tm now_tm;
struct tm *refuse_future;
long num2, num3;

num2 = strtol(end+1, &end, 10);
num3 = -1;
if (*end == c && isdigit(end[1]))
num3 = strtol(end+1, &end, 10);


switch (c) {
case ':':
if (num3 < 0)
num3 = 0;
if (num < 25 && num2 >= 0 && num2 < 60 && num3 >= 0 && num3 <= 60) {
tm->tm_hour = num;
tm->tm_min = num2;
tm->tm_sec = num3;
break;
}
return 0;

case '-':
case '/':
case '.':
if (!now)
now = time(NULL);
refuse_future = NULL;
if (gmtime_r(&now, &now_tm))
refuse_future = &now_tm;

if (num > 70) {

if (is_date(num, num2, num3, NULL, now, tm))
break;

if (is_date(num, num3, num2, NULL, now, tm))
break;
}




if (c != '.' &&
is_date(num3, num, num2, refuse_future, now, tm))
break;

if (is_date(num3, num2, num, refuse_future, now, tm))
break;

if (c == '.' &&
is_date(num3, num, num2, refuse_future, now, tm))
break;
return 0;
}
return end - date;
}






static inline int nodate(struct tm *tm)
{
return (tm->tm_year &
tm->tm_mon &
tm->tm_mday &
tm->tm_hour &
tm->tm_min &
tm->tm_sec) < 0;
}




static int match_digit(const char *date, struct tm *tm, int *offset, int *tm_gmt)
{
int n;
char *end;
timestamp_t num;

num = parse_timestamp(date, &end, 10);






if (num >= 100000000 && nodate(tm)) {
time_t time = num;
if (gmtime_r(&time, tm)) {
*tm_gmt = 1;
return end - date;
}
}




switch (*end) {
case ':':
case '.':
case '/':
case '-':
if (isdigit(end[1])) {
int match = match_multi_number(num, *end, date, end, tm, 0);
if (match)
return match;
}
}






n = 0;
do {
n++;
} while (isdigit(date[n]));


if (n == 4) {
if (num <= 1400 && *offset == -1) {
unsigned int minutes = num % 100;
unsigned int hours = num / 100;
*offset = hours*60 + minutes;
} else if (num > 1900 && num < 2100)
tm->tm_year = num - 1900;
return n;
}





if (n > 2)
return n;








if (num > 0 && num < 32 && tm->tm_mday < 0) {
tm->tm_mday = num;
return n;
}


if (n == 2 && tm->tm_year < 0) {
if (num < 10 && tm->tm_mday >= 0) {
tm->tm_year = num + 100;
return n;
}
if (num >= 70) {
tm->tm_year = num;
return n;
}
}

if (num > 0 && num < 13 && tm->tm_mon < 0)
tm->tm_mon = num-1;

return n;
}

static int match_tz(const char *date, int *offp)
{
char *end;
int hour = strtoul(date + 1, &end, 10);
int n = end - (date + 1);
int min = 0;

if (n == 4) {

min = hour % 100;
hour = hour / 100;
} else if (n != 2) {
min = 99; 
} else if (*end == ':') {

min = strtoul(end + 1, &end, 10);
if (end - (date + 1) != 5)
min = 99; 
} 









if (min < 60 && hour < 24) {
int offset = hour * 60 + min;
if (*date == '-')
offset = -offset;
*offp = offset;
}
return end - date;
}

static void date_string(timestamp_t date, int offset, struct strbuf *buf)
{
int sign = '+';

if (offset < 0) {
offset = -offset;
sign = '-';
}
strbuf_addf(buf, "%"PRItime" %c%02d%02d", date, sign, offset / 60, offset % 60);
}





static int match_object_header_date(const char *date, timestamp_t *timestamp, int *offset)
{
char *end;
timestamp_t stamp;
int ofs;

if (*date < '0' || '9' < *date)
return -1;
stamp = parse_timestamp(date, &end, 10);
if (*end != ' ' || stamp == TIME_MAX || (end[1] != '+' && end[1] != '-'))
return -1;
date = end + 2;
ofs = strtol(date, &end, 10);
if ((*end != '\0' && (*end != '\n')) || end != date + 4)
return -1;
ofs = (ofs / 100) * 60 + (ofs % 100);
if (date[-1] == '-')
ofs = -ofs;
*timestamp = stamp;
*offset = ofs;
return 0;
}



int parse_date_basic(const char *date, timestamp_t *timestamp, int *offset)
{
struct tm tm;
int tm_gmt;
timestamp_t dummy_timestamp;
int dummy_offset;

if (!timestamp)
timestamp = &dummy_timestamp;
if (!offset)
offset = &dummy_offset;

memset(&tm, 0, sizeof(tm));
tm.tm_year = -1;
tm.tm_mon = -1;
tm.tm_mday = -1;
tm.tm_isdst = -1;
tm.tm_hour = -1;
tm.tm_min = -1;
tm.tm_sec = -1;
*offset = -1;
tm_gmt = 0;

if (*date == '@' &&
!match_object_header_date(date + 1, timestamp, offset))
return 0; 
for (;;) {
int match = 0;
unsigned char c = *date;


if (!c || c == '\n')
break;

if (isalpha(c))
match = match_alpha(date, &tm, offset);
else if (isdigit(c))
match = match_digit(date, &tm, offset, &tm_gmt);
else if ((c == '-' || c == '+') && isdigit(date[1]))
match = match_tz(date, offset);

if (!match) {

match = 1;
}

date += match;
}


*timestamp = tm_to_time_t(&tm);
if (*timestamp == -1)
return -1;

if (*offset == -1) {
time_t temp_time;


tm.tm_isdst = -1;
temp_time = mktime(&tm);
if ((time_t)*timestamp > temp_time) {
*offset = ((time_t)*timestamp - temp_time) / 60;
} else {
*offset = -(int)((temp_time - (time_t)*timestamp) / 60);
}
}

if (!tm_gmt)
*timestamp -= *offset * 60;
return 0; 
}

int parse_expiry_date(const char *date, timestamp_t *timestamp)
{
int errors = 0;

if (!strcmp(date, "never") || !strcmp(date, "false"))
*timestamp = 0;
else if (!strcmp(date, "all") || !strcmp(date, "now"))








*timestamp = TIME_MAX;
else
*timestamp = approxidate_careful(date, &errors);

return errors;
}

int parse_date(const char *date, struct strbuf *result)
{
timestamp_t timestamp;
int offset;
if (parse_date_basic(date, &timestamp, &offset))
return -1;
date_string(timestamp, offset, result);
return 0;
}

static enum date_mode_type parse_date_type(const char *format, const char **end)
{
if (skip_prefix(format, "relative", end))
return DATE_RELATIVE;
if (skip_prefix(format, "iso8601-strict", end) ||
skip_prefix(format, "iso-strict", end))
return DATE_ISO8601_STRICT;
if (skip_prefix(format, "iso8601", end) ||
skip_prefix(format, "iso", end))
return DATE_ISO8601;
if (skip_prefix(format, "rfc2822", end) ||
skip_prefix(format, "rfc", end))
return DATE_RFC2822;
if (skip_prefix(format, "short", end))
return DATE_SHORT;
if (skip_prefix(format, "default", end))
return DATE_NORMAL;
if (skip_prefix(format, "human", end))
return DATE_HUMAN;
if (skip_prefix(format, "raw", end))
return DATE_RAW;
if (skip_prefix(format, "unix", end))
return DATE_UNIX;
if (skip_prefix(format, "format", end))
return DATE_STRFTIME;





die("unknown date format %s", format);
}

void parse_date_format(const char *format, struct date_mode *mode)
{
const char *p;


if (skip_prefix(format, "auto:", &p)) {
if (isatty(1) || pager_in_use())
format = p;
else
format = "default";
}


if (!strcmp(format, "local"))
format = "default-local";

mode->type = parse_date_type(format, &p);
mode->local = 0;

if (skip_prefix(p, "-local", &p))
mode->local = 1;

if (mode->type == DATE_STRFTIME) {
if (!skip_prefix(p, ":", &p))
die("date format missing colon separator: %s", format);
mode->strftime_fmt = xstrdup(p);
} else if (*p)
die("unknown date format %s", format);
}

void datestamp(struct strbuf *out)
{
time_t now;
int offset;
struct tm tm = { 0 };

time(&now);

offset = tm_to_time_t(localtime_r(&now, &tm)) - now;
offset /= 60;

date_string(now, offset, out);
}





static time_t update_tm(struct tm *tm, struct tm *now, time_t sec)
{
time_t n;

if (tm->tm_mday < 0)
tm->tm_mday = now->tm_mday;
if (tm->tm_mon < 0)
tm->tm_mon = now->tm_mon;
if (tm->tm_year < 0) {
tm->tm_year = now->tm_year;
if (tm->tm_mon > now->tm_mon)
tm->tm_year--;
}

n = mktime(tm) - sec;
localtime_r(&n, tm);
return n;
}






static void pending_number(struct tm *tm, int *num)
{
int number = *num;

if (number) {
*num = 0;
if (tm->tm_mday < 0 && number < 32)
tm->tm_mday = number;
else if (tm->tm_mon < 0 && number < 13)
tm->tm_mon = number-1;
else if (tm->tm_year < 0) {
if (number > 1969 && number < 2100)
tm->tm_year = number - 1900;
else if (number > 69 && number < 100)
tm->tm_year = number;
else if (number < 38)
tm->tm_year = 100 + number;

}
}
}

static void date_now(struct tm *tm, struct tm *now, int *num)
{
*num = 0;
update_tm(tm, now, 0);
}

static void date_yesterday(struct tm *tm, struct tm *now, int *num)
{
*num = 0;
update_tm(tm, now, 24*60*60);
}

static void date_time(struct tm *tm, struct tm *now, int hour)
{
if (tm->tm_hour < hour)
update_tm(tm, now, 24*60*60);
tm->tm_hour = hour;
tm->tm_min = 0;
tm->tm_sec = 0;
}

static void date_midnight(struct tm *tm, struct tm *now, int *num)
{
pending_number(tm, num);
date_time(tm, now, 0);
}

static void date_noon(struct tm *tm, struct tm *now, int *num)
{
pending_number(tm, num);
date_time(tm, now, 12);
}

static void date_tea(struct tm *tm, struct tm *now, int *num)
{
pending_number(tm, num);
date_time(tm, now, 17);
}

static void date_pm(struct tm *tm, struct tm *now, int *num)
{
int hour, n = *num;
*num = 0;

hour = tm->tm_hour;
if (n) {
hour = n;
tm->tm_min = 0;
tm->tm_sec = 0;
}
tm->tm_hour = (hour % 12) + 12;
}

static void date_am(struct tm *tm, struct tm *now, int *num)
{
int hour, n = *num;
*num = 0;

hour = tm->tm_hour;
if (n) {
hour = n;
tm->tm_min = 0;
tm->tm_sec = 0;
}
tm->tm_hour = (hour % 12);
}

static void date_never(struct tm *tm, struct tm *now, int *num)
{
time_t n = 0;
localtime_r(&n, tm);
*num = 0;
}

static const struct special {
const char *name;
void (*fn)(struct tm *, struct tm *, int *);
} special[] = {
{ "yesterday", date_yesterday },
{ "noon", date_noon },
{ "midnight", date_midnight },
{ "tea", date_tea },
{ "PM", date_pm },
{ "AM", date_am },
{ "never", date_never },
{ "now", date_now },
{ NULL }
};

static const char *number_name[] = {
"zero", "one", "two", "three", "four",
"five", "six", "seven", "eight", "nine", "ten",
};

static const struct typelen {
const char *type;
int length;
} typelen[] = {
{ "seconds", 1 },
{ "minutes", 60 },
{ "hours", 60*60 },
{ "days", 24*60*60 },
{ "weeks", 7*24*60*60 },
{ NULL }
};

static const char *approxidate_alpha(const char *date, struct tm *tm, struct tm *now, int *num, int *touched)
{
const struct typelen *tl;
const struct special *s;
const char *end = date;
int i;

while (isalpha(*++end))
;

for (i = 0; i < 12; i++) {
int match = match_string(date, month_names[i]);
if (match >= 3) {
tm->tm_mon = i;
*touched = 1;
return end;
}
}

for (s = special; s->name; s++) {
int len = strlen(s->name);
if (match_string(date, s->name) == len) {
s->fn(tm, now, num);
*touched = 1;
return end;
}
}

if (!*num) {
for (i = 1; i < 11; i++) {
int len = strlen(number_name[i]);
if (match_string(date, number_name[i]) == len) {
*num = i;
*touched = 1;
return end;
}
}
if (match_string(date, "last") == 4) {
*num = 1;
*touched = 1;
}
return end;
}

tl = typelen;
while (tl->type) {
int len = strlen(tl->type);
if (match_string(date, tl->type) >= len-1) {
update_tm(tm, now, tl->length * *num);
*num = 0;
*touched = 1;
return end;
}
tl++;
}

for (i = 0; i < 7; i++) {
int match = match_string(date, weekday_names[i]);
if (match >= 3) {
int diff, n = *num -1;
*num = 0;

diff = tm->tm_wday - i;
if (diff <= 0)
n++;
diff += 7*n;

update_tm(tm, now, diff * 24 * 60 * 60);
*touched = 1;
return end;
}
}

if (match_string(date, "months") >= 5) {
int n;
update_tm(tm, now, 0); 
n = tm->tm_mon - *num;
*num = 0;
while (n < 0) {
n += 12;
tm->tm_year--;
}
tm->tm_mon = n;
*touched = 1;
return end;
}

if (match_string(date, "years") >= 4) {
update_tm(tm, now, 0); 
tm->tm_year -= *num;
*num = 0;
*touched = 1;
return end;
}

return end;
}

static const char *approxidate_digit(const char *date, struct tm *tm, int *num,
time_t now)
{
char *end;
timestamp_t number = parse_timestamp(date, &end, 10);

switch (*end) {
case ':':
case '.':
case '/':
case '-':
if (isdigit(end[1])) {
int match = match_multi_number(number, *end, date, end,
tm, now);
if (match)
return date + match;
}
}


if (date[0] != '0' || end - date <= 2)
*num = number;
return end;
}

static timestamp_t approxidate_str(const char *date,
const struct timeval *tv,
int *error_ret)
{
int number = 0;
int touched = 0;
struct tm tm, now;
time_t time_sec;

time_sec = tv->tv_sec;
localtime_r(&time_sec, &tm);
now = tm;

tm.tm_year = -1;
tm.tm_mon = -1;
tm.tm_mday = -1;

for (;;) {
unsigned char c = *date;
if (!c)
break;
date++;
if (isdigit(c)) {
pending_number(&tm, &number);
date = approxidate_digit(date-1, &tm, &number, time_sec);
touched = 1;
continue;
}
if (isalpha(c))
date = approxidate_alpha(date-1, &tm, &now, &number, &touched);
}
pending_number(&tm, &number);
if (!touched)
*error_ret = 1;
return (timestamp_t)update_tm(&tm, &now, 0);
}

timestamp_t approxidate_relative(const char *date)
{
struct timeval tv;
timestamp_t timestamp;
int offset;
int errors = 0;

if (!parse_date_basic(date, &timestamp, &offset))
return timestamp;

get_time(&tv);
return approxidate_str(date, (const struct timeval *) &tv, &errors);
}

timestamp_t approxidate_careful(const char *date, int *error_ret)
{
struct timeval tv;
timestamp_t timestamp;
int offset;
int dummy = 0;
if (!error_ret)
error_ret = &dummy;

if (!parse_date_basic(date, &timestamp, &offset)) {
*error_ret = 0;
return timestamp;
}

get_time(&tv);
return approxidate_str(date, &tv, error_ret);
}

int date_overflows(timestamp_t t)
{
time_t sys;


if ((uintmax_t)t >= TIME_MAX)
return 1;






sys = t;
return t != sys || (t < 1) != (sys < 1);
}
