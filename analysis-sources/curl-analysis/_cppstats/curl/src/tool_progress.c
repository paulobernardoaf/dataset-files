#include "tool_setup.h"
#include "tool_operate.h"
#include "tool_progress.h"
#include "tool_util.h"
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
static char *max5data(curl_off_t bytes, char *max5)
{
#define ONE_KILOBYTE CURL_OFF_T_C(1024)
#define ONE_MEGABYTE (CURL_OFF_T_C(1024) * ONE_KILOBYTE)
#define ONE_GIGABYTE (CURL_OFF_T_C(1024) * ONE_MEGABYTE)
#define ONE_TERABYTE (CURL_OFF_T_C(1024) * ONE_GIGABYTE)
#define ONE_PETABYTE (CURL_OFF_T_C(1024) * ONE_TERABYTE)
if(bytes < CURL_OFF_T_C(100000))
msnprintf(max5, 6, "%5" CURL_FORMAT_CURL_OFF_T, bytes);
else if(bytes < CURL_OFF_T_C(10000) * ONE_KILOBYTE)
msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "k", bytes/ONE_KILOBYTE);
else if(bytes < CURL_OFF_T_C(100) * ONE_MEGABYTE)
msnprintf(max5, 6, "%2" CURL_FORMAT_CURL_OFF_T ".%0"
CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE,
(bytes%ONE_MEGABYTE) / (ONE_MEGABYTE/CURL_OFF_T_C(10)) );
#if (CURL_SIZEOF_CURL_OFF_T > 4)
else if(bytes < CURL_OFF_T_C(10000) * ONE_MEGABYTE)
msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE);
else if(bytes < CURL_OFF_T_C(100) * ONE_GIGABYTE)
msnprintf(max5, 6, "%2" CURL_FORMAT_CURL_OFF_T ".%0"
CURL_FORMAT_CURL_OFF_T "G", bytes/ONE_GIGABYTE,
(bytes%ONE_GIGABYTE) / (ONE_GIGABYTE/CURL_OFF_T_C(10)) );
else if(bytes < CURL_OFF_T_C(10000) * ONE_GIGABYTE)
msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "G", bytes/ONE_GIGABYTE);
else if(bytes < CURL_OFF_T_C(10000) * ONE_TERABYTE)
msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "T", bytes/ONE_TERABYTE);
else
msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "P", bytes/ONE_PETABYTE);
#else
else
msnprintf(max5, 6, "%4" CURL_FORMAT_CURL_OFF_T "M", bytes/ONE_MEGABYTE);
#endif
return max5;
}
int xferinfo_cb(void *clientp,
curl_off_t dltotal,
curl_off_t dlnow,
curl_off_t ultotal,
curl_off_t ulnow)
{
struct per_transfer *per = clientp;
struct OperationConfig *config = per->config;
per->dltotal = dltotal;
per->dlnow = dlnow;
per->ultotal = ultotal;
per->ulnow = ulnow;
if(config->readbusy) {
config->readbusy = FALSE;
curl_easy_pause(per->curl, CURLPAUSE_CONT);
}
return 0;
}
static void time2str(char *r, curl_off_t seconds)
{
curl_off_t h;
if(seconds <= 0) {
strcpy(r, "--:--:--");
return;
}
h = seconds / CURL_OFF_T_C(3600);
if(h <= CURL_OFF_T_C(99)) {
curl_off_t m = (seconds - (h*CURL_OFF_T_C(3600))) / CURL_OFF_T_C(60);
curl_off_t s = (seconds - (h*CURL_OFF_T_C(3600))) - (m*CURL_OFF_T_C(60));
msnprintf(r, 9, "%2" CURL_FORMAT_CURL_OFF_T ":%02" CURL_FORMAT_CURL_OFF_T
":%02" CURL_FORMAT_CURL_OFF_T, h, m, s);
}
else {
curl_off_t d = seconds / CURL_OFF_T_C(86400);
h = (seconds - (d*CURL_OFF_T_C(86400))) / CURL_OFF_T_C(3600);
if(d <= CURL_OFF_T_C(999))
msnprintf(r, 9, "%3" CURL_FORMAT_CURL_OFF_T
"d %02" CURL_FORMAT_CURL_OFF_T "h", d, h);
else
msnprintf(r, 9, "%7" CURL_FORMAT_CURL_OFF_T "d", d);
}
}
static curl_off_t all_dltotal = 0;
static curl_off_t all_ultotal = 0;
static curl_off_t all_dlalready = 0;
static curl_off_t all_ulalready = 0;
curl_off_t all_xfers = 0; 
struct speedcount {
curl_off_t dl;
curl_off_t ul;
struct timeval stamp;
};
#define SPEEDCNT 10
static unsigned int speedindex;
static bool indexwrapped;
static struct speedcount speedstore[SPEEDCNT];
bool progress_meter(struct GlobalConfig *global,
struct timeval *start,
bool final)
{
static struct timeval stamp;
static bool header = FALSE;
struct timeval now;
long diff;
if(global->noprogress)
return FALSE;
now = tvnow();
diff = tvdiff(now, stamp);
if(!header) {
header = TRUE;
fputs("DL% UL% Dled Uled Xfers Live Qd "
"Total Current Left Speed\n",
global->errors);
}
if(final || (diff > 500)) {
char time_left[10];
char time_total[10];
char time_spent[10];
char buffer[3][6];
curl_off_t spent = tvdiff(now, *start)/1000;
char dlpercen[4]="--";
char ulpercen[4]="--";
struct per_transfer *per;
curl_off_t all_dlnow = 0;
curl_off_t all_ulnow = 0;
bool dlknown = TRUE;
bool ulknown = TRUE;
curl_off_t all_running = 0; 
curl_off_t all_queued = 0; 
curl_off_t speed = 0;
unsigned int i;
stamp = now;
all_dlnow += all_dlalready;
all_ulnow += all_ulalready;
for(per = transfers; per; per = per->next) {
all_dlnow += per->dlnow;
all_ulnow += per->ulnow;
if(!per->dltotal)
dlknown = FALSE;
else if(!per->dltotal_added) {
all_dltotal += per->dltotal;
per->dltotal_added = TRUE;
}
if(!per->ultotal)
ulknown = FALSE;
else if(!per->ultotal_added) {
all_ultotal += per->ultotal;
per->ultotal_added = TRUE;
}
if(!per->added)
all_queued++;
else
all_running++;
}
if(dlknown && all_dltotal)
msnprintf(dlpercen, sizeof(dlpercen), "%3d",
all_dlnow * 100 / all_dltotal);
if(ulknown && all_ultotal)
msnprintf(ulpercen, sizeof(ulpercen), "%3d",
all_ulnow * 100 / all_ultotal);
i = speedindex;
speedstore[i].dl = all_dlnow;
speedstore[i].ul = all_ulnow;
speedstore[i].stamp = now;
if(++speedindex >= SPEEDCNT) {
indexwrapped = TRUE;
speedindex = 0;
}
{
long deltams;
curl_off_t dl;
curl_off_t ul;
curl_off_t dls;
curl_off_t uls;
if(indexwrapped) {
deltams = tvdiff(now, speedstore[speedindex].stamp);
dl = all_dlnow - speedstore[speedindex].dl;
ul = all_ulnow - speedstore[speedindex].ul;
}
else {
deltams = tvdiff(now, *start);
dl = all_dlnow;
ul = all_ulnow;
}
dls = (curl_off_t)((double)dl / ((double)deltams/1000.0));
uls = (curl_off_t)((double)ul / ((double)deltams/1000.0));
speed = dls > uls ? dls : uls;
}
if(dlknown && speed) {
curl_off_t est = all_dltotal / speed;
curl_off_t left = (all_dltotal - all_dlnow) / speed;
time2str(time_left, left);
time2str(time_total, est);
}
else {
time2str(time_left, 0);
time2str(time_total, 0);
}
time2str(time_spent, spent);
fprintf(global->errors,
"\r"
"%-3s " 
"%-3s " 
"%s " 
"%s " 
"%5" CURL_FORMAT_CURL_OFF_T " " 
"%5" CURL_FORMAT_CURL_OFF_T " " 
"%5" CURL_FORMAT_CURL_OFF_T " " 
"%s " 
"%s " 
"%s " 
"%s " 
"%5s" ,
dlpercen, 
ulpercen, 
max5data(all_dlnow, buffer[0]),
max5data(all_ulnow, buffer[1]),
all_xfers,
all_running,
all_queued,
time_total,
time_spent,
time_left,
max5data(speed, buffer[2]), 
final ? "\n" :"");
return TRUE;
}
return FALSE;
}
void progress_finalize(struct per_transfer *per)
{
all_dlalready += per->dlnow;
all_ulalready += per->ulnow;
}
