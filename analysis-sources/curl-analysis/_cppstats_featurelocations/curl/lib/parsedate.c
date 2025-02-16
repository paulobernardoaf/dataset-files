










































































#include "curl_setup.h"

#include <limits.h>

#include <curl/curl.h>
#include "strcase.h"
#include "warnless.h"
#include "parsedate.h"












static int parsedate(const char *date, time_t *output);

#define PARSEDATE_OK 0
#define PARSEDATE_FAIL -1
#define PARSEDATE_LATER 1
#define PARSEDATE_SOONER 2

#if !defined(CURL_DISABLE_PARSEDATE) || !defined(CURL_DISABLE_FTP) || !defined(CURL_DISABLE_FILE)


const char * const Curl_wkday[] =
{"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char * const Curl_month[]=
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
#endif

#if !defined(CURL_DISABLE_PARSEDATE)
static const char * const weekday[] =
{ "Monday", "Tuesday", "Wednesday", "Thursday",
"Friday", "Saturday", "Sunday" };

struct tzinfo {
char name[5];
int offset; 
};



#define tDAYZONE -60 
static const struct tzinfo tz[]= {
{"GMT", 0}, 
{"UT", 0}, 
{"UTC", 0}, 
{"WET", 0}, 
{"BST", 0 tDAYZONE}, 
{"WAT", 60}, 
{"AST", 240}, 
{"ADT", 240 tDAYZONE}, 
{"EST", 300}, 
{"EDT", 300 tDAYZONE}, 
{"CST", 360}, 
{"CDT", 360 tDAYZONE}, 
{"MST", 420}, 
{"MDT", 420 tDAYZONE}, 
{"PST", 480}, 
{"PDT", 480 tDAYZONE}, 
{"YST", 540}, 
{"YDT", 540 tDAYZONE}, 
{"HST", 600}, 
{"HDT", 600 tDAYZONE}, 
{"CAT", 600}, 
{"AHST", 600}, 
{"NT", 660}, 
{"IDLW", 720}, 
{"CET", -60}, 
{"MET", -60}, 
{"MEWT", -60}, 
{"MEST", -60 tDAYZONE}, 
{"CEST", -60 tDAYZONE}, 
{"MESZ", -60 tDAYZONE}, 
{"FWT", -60}, 
{"FST", -60 tDAYZONE}, 
{"EET", -120}, 
{"WAST", -420}, 
{"WADT", -420 tDAYZONE}, 
{"CCT", -480}, 
{"JST", -540}, 
{"EAST", -600}, 
{"EADT", -600 tDAYZONE}, 
{"GST", -600}, 
{"NZT", -720}, 
{"NZST", -720}, 
{"NZDT", -720 tDAYZONE}, 
{"IDLE", -720}, 




{"A", 1 * 60}, 
{"B", 2 * 60}, 
{"C", 3 * 60}, 
{"D", 4 * 60}, 
{"E", 5 * 60}, 
{"F", 6 * 60}, 
{"G", 7 * 60}, 
{"H", 8 * 60}, 
{"I", 9 * 60}, 


{"K", 10 * 60}, 
{"L", 11 * 60}, 
{"M", 12 * 60}, 
{"N", -1 * 60}, 
{"O", -2 * 60}, 
{"P", -3 * 60}, 
{"Q", -4 * 60}, 
{"R", -5 * 60}, 
{"S", -6 * 60}, 
{"T", -7 * 60}, 
{"U", -8 * 60}, 
{"V", -9 * 60}, 
{"W", -10 * 60}, 
{"X", -11 * 60}, 
{"Y", -12 * 60}, 
{"Z", 0}, 
};






static int checkday(const char *check, size_t len)
{
int i;
const char * const *what;
bool found = FALSE;
if(len > 3)
what = &weekday[0];
else
what = &Curl_wkday[0];
for(i = 0; i<7; i++) {
if(strcasecompare(check, what[0])) {
found = TRUE;
break;
}
what++;
}
return found?i:-1;
}

static int checkmonth(const char *check)
{
int i;
const char * const *what;
bool found = FALSE;

what = &Curl_month[0];
for(i = 0; i<12; i++) {
if(strcasecompare(check, what[0])) {
found = TRUE;
break;
}
what++;
}
return found?i:-1; 
}




static int checktz(const char *check)
{
unsigned int i;
const struct tzinfo *what;
bool found = FALSE;

what = tz;
for(i = 0; i< sizeof(tz)/sizeof(tz[0]); i++) {
if(strcasecompare(check, what->name)) {
found = TRUE;
break;
}
what++;
}
return found?what->offset*60:-1;
}

static void skip(const char **date)
{

while(**date && !ISALNUM(**date))
(*date)++;
}

enum assume {
DATE_MDAY,
DATE_YEAR,
DATE_TIME
};



struct my_tm {
int tm_sec;
int tm_min;
int tm_hour;
int tm_mday;
int tm_mon;
int tm_year; 
};








static void my_timegm(struct my_tm *tm, time_t *t)
{
static const int month_days_cumulative [12] =
{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
int month, year, leap_days;

year = tm->tm_year;
month = tm->tm_mon;
if(month < 0) {
year += (11 - month) / 12;
month = 11 - (11 - month) % 12;
}
else if(month >= 12) {
year -= month / 12;
month = month % 12;
}

leap_days = year - (tm->tm_mon <= 1);
leap_days = ((leap_days / 4) - (leap_days / 100) + (leap_days / 400)
- (1969 / 4) + (1969 / 100) - (1969 / 400));

*t = ((((time_t) (year - 1970) * 365
+ leap_days + month_days_cumulative[month] + tm->tm_mday - 1) * 24
+ tm->tm_hour) * 60 + tm->tm_min) * 60 + tm->tm_sec;
}












static int parsedate(const char *date, time_t *output)
{
time_t t = 0;
int wdaynum = -1; 
int monnum = -1; 
int mdaynum = -1; 
int hournum = -1;
int minnum = -1;
int secnum = -1;
int yearnum = -1;
int tzoff = -1;
struct my_tm tm;
enum assume dignext = DATE_MDAY;
const char *indate = date; 
int part = 0; 

while(*date && (part < 6)) {
bool found = FALSE;

skip(&date);

if(ISALPHA(*date)) {

char buf[32]="";
size_t len;
if(sscanf(date, "%31[ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz]", buf))
len = strlen(buf);
else
len = 0;

if(wdaynum == -1) {
wdaynum = checkday(buf, len);
if(wdaynum != -1)
found = TRUE;
}
if(!found && (monnum == -1)) {
monnum = checkmonth(buf);
if(monnum != -1)
found = TRUE;
}

if(!found && (tzoff == -1)) {

tzoff = checktz(buf);
if(tzoff != -1)
found = TRUE;
}

if(!found)
return PARSEDATE_FAIL; 

date += len;
}
else if(ISDIGIT(*date)) {

int val;
char *end;
int len = 0;
if((secnum == -1) &&
(3 == sscanf(date, "%02d:%02d:%02d%n",
&hournum, &minnum, &secnum, &len))) {

date += len;
}
else if((secnum == -1) &&
(2 == sscanf(date, "%02d:%02d%n", &hournum, &minnum, &len))) {

date += len;
secnum = 0;
}
else {
long lval;
int error;
int old_errno;

old_errno = errno;
errno = 0;
lval = strtol(date, &end, 10);
error = errno;
if(errno != old_errno)
errno = old_errno;

if(error)
return PARSEDATE_FAIL;

#if LONG_MAX != INT_MAX
if((lval > (long)INT_MAX) || (lval < (long)INT_MIN))
return PARSEDATE_FAIL;
#endif

val = curlx_sltosi(lval);

if((tzoff == -1) &&
((end - date) == 4) &&
(val <= 1400) &&
(indate< date) &&
((date[-1] == '+' || date[-1] == '-'))) {








found = TRUE;
tzoff = (val/100 * 60 + val%100)*60;



tzoff = date[-1]=='+'?-tzoff:tzoff;
}

if(((end - date) == 8) &&
(yearnum == -1) &&
(monnum == -1) &&
(mdaynum == -1)) {

found = TRUE;
yearnum = val/10000;
monnum = (val%10000)/100-1; 
mdaynum = val%100;
}

if(!found && (dignext == DATE_MDAY) && (mdaynum == -1)) {
if((val > 0) && (val<32)) {
mdaynum = val;
found = TRUE;
}
dignext = DATE_YEAR;
}

if(!found && (dignext == DATE_YEAR) && (yearnum == -1)) {
yearnum = val;
found = TRUE;
if(yearnum < 100) {
if(yearnum > 70)
yearnum += 1900;
else
yearnum += 2000;
}
if(mdaynum == -1)
dignext = DATE_MDAY;
}

if(!found)
return PARSEDATE_FAIL;

date = end;
}
}

part++;
}

if(-1 == secnum)
secnum = minnum = hournum = 0; 

if((-1 == mdaynum) ||
(-1 == monnum) ||
(-1 == yearnum))

return PARSEDATE_FAIL;

#if defined(HAVE_TIME_T_UNSIGNED)
if(yearnum < 1970) {

*output = TIME_T_MIN;
return PARSEDATE_SOONER;
}
#endif

#if (SIZEOF_TIME_T < 5)

#if defined(HAVE_TIME_T_UNSIGNED)

if(yearnum > 2105) {
*output = TIME_T_MAX;
return PARSEDATE_LATER;
}
#else

if(yearnum > 2037) {
*output = TIME_T_MAX;
return PARSEDATE_LATER;
}
if(yearnum < 1903) {
*output = TIME_T_MIN;
return PARSEDATE_SOONER;
}
#endif

#else

if(yearnum < 1583)
return PARSEDATE_FAIL;
#endif

if((mdaynum > 31) || (monnum > 11) ||
(hournum > 23) || (minnum > 59) || (secnum > 60))
return PARSEDATE_FAIL; 

tm.tm_sec = secnum;
tm.tm_min = minnum;
tm.tm_hour = hournum;
tm.tm_mday = mdaynum;
tm.tm_mon = monnum;
tm.tm_year = yearnum;





my_timegm(&tm, &t);


if(tzoff == -1)
tzoff = 0;

if((tzoff > 0) && (t > TIME_T_MAX - tzoff)) {
*output = TIME_T_MAX;
return PARSEDATE_LATER; 
}

t += tzoff;

*output = t;

return PARSEDATE_OK;
}
#else

static int parsedate(const char *date, time_t *output)
{
(void)date;
*output = 0;
return PARSEDATE_OK; 
}
#endif

time_t curl_getdate(const char *p, const time_t *now)
{
time_t parsed = -1;
int rc = parsedate(p, &parsed);
(void)now; 

if(rc == PARSEDATE_OK) {
if(parsed == -1)

parsed++;
return parsed;
}

return -1;
}





time_t Curl_getdate_capped(const char *p)
{
time_t parsed = -1;
int rc = parsedate(p, &parsed);

switch(rc) {
case PARSEDATE_OK:
if(parsed == -1)

parsed++;
return parsed;
case PARSEDATE_LATER:

return parsed;
default:
return -1; 
}

}







CURLcode Curl_gmtime(time_t intime, struct tm *store)
{
const struct tm *tm;
#if defined(HAVE_GMTIME_R)

tm = (struct tm *)gmtime_r(&intime, store);
#else
tm = gmtime(&intime);
if(tm)
*store = *tm; 
#endif

if(!tm)
return CURLE_BAD_FUNCTION_ARGUMENT;
return CURLE_OK;
}
