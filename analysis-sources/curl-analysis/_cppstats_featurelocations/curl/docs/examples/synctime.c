
























































































#include <stdio.h>
#include <time.h>
#if !defined(__CYGWIN__)
#include <winsock2.h>
#include <windows.h>
#endif
#include <curl/curl.h>


#define MAX_STRING 256
#define MAX_STRING1 MAX_STRING + 1

#define SYNCTIME_UA "synctime/1.0"

typedef struct
{
char http_proxy[MAX_STRING1];
char proxy_user[MAX_STRING1];
char timeserver[MAX_STRING1];
} conf_t;

const char DefaultTimeServer[3][MAX_STRING1] =
{
"https://nist.time.gov/",
"https://www.google.com/"
};

const char *DayStr[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char *MthStr[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int ShowAllHeader;
int AutoSyncTime;
SYSTEMTIME SYSTime;
SYSTEMTIME LOCALTime;

#define HTTP_COMMAND_HEAD 0
#define HTTP_COMMAND_GET 1


size_t SyncTime_CURL_WriteOutput(void *ptr, size_t size, size_t nmemb,
void *stream)
{
fwrite(ptr, size, nmemb, stream);
return (nmemb*size);
}

size_t SyncTime_CURL_WriteHeader(void *ptr, size_t size, size_t nmemb,
void *stream)
{
char TmpStr1[26], TmpStr2[26];

(void)stream;

if(ShowAllHeader == 1)
fprintf(stderr, "%s", (char *)(ptr));

if(strncmp((char *)(ptr), "Date:", 5) == 0) {
if(ShowAllHeader == 0)
fprintf(stderr, "HTTP Server. %s", (char *)(ptr));

if(AutoSyncTime == 1) {
*TmpStr1 = 0;
*TmpStr2 = 0;
if(strlen((char *)(ptr)) > 50) 

AutoSyncTime = 0;
else {
int RetVal = sscanf((char *)(ptr), "Date: %s %hu %s %hu %hu:%hu:%hu",
TmpStr1, &SYSTime.wDay, TmpStr2, &SYSTime.wYear,
&SYSTime.wHour, &SYSTime.wMinute,
&SYSTime.wSecond);

if(RetVal == 7) {
int i;
SYSTime.wMilliseconds = 500; 
for(i = 0; i<12; i++) {
if(strcmp(MthStr[i], TmpStr2) == 0) {
SYSTime.wMonth = i + 1;
break;
}
}
AutoSyncTime = 3; 
}
else {
AutoSyncTime = 0; 
}
}
}
}

if(strncmp((char *)(ptr), "X-Cache: HIT", 12) == 0) {
fprintf(stderr, "ERROR: HTTP Server data is cached."
" Server Date is no longer valid.\n");
AutoSyncTime = 0;
}
return (nmemb*size);
}

void SyncTime_CURL_Init(CURL *curl, char *proxy_port,
char *proxy_user_password)
{
if(strlen(proxy_port) > 0)
curl_easy_setopt(curl, CURLOPT_PROXY, proxy_port);

if(strlen(proxy_user_password) > 0)
curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxy_user_password);

#if defined(SYNCTIME_UA)
curl_easy_setopt(curl, CURLOPT_USERAGENT, SYNCTIME_UA);
#endif
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, *SyncTime_CURL_WriteOutput);
curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, *SyncTime_CURL_WriteHeader);
}

int SyncTime_CURL_Fetch(CURL *curl, char *URL_Str, char *OutFileName,
int HttpGetBody)
{
FILE *outfile;
CURLcode res;

outfile = NULL;
if(HttpGetBody == HTTP_COMMAND_HEAD)
curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
else {
outfile = fopen(OutFileName, "wb");
curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
}

curl_easy_setopt(curl, CURLOPT_URL, URL_Str);
res = curl_easy_perform(curl);
if(outfile != NULL)
fclose(outfile);
return res; 
}

void showUsage(void)
{
fprintf(stderr, "SYNCTIME: Synchronising computer clock with time server"
" using HTTP protocol.\n");
fprintf(stderr, "Usage : SYNCTIME [Option]\n");
fprintf(stderr, "Options :\n");
fprintf(stderr, " --server=WEBSERVER Use this time server instead"
" of default.\n");
fprintf(stderr, " --showall Show all HTTP header.\n");
fprintf(stderr, " --synctime Synchronising computer clock"
" with time server.\n");
fprintf(stderr, " --proxy-user=USER[:PASS] Set proxy username and"
" password.\n");
fprintf(stderr, " --proxy=HOST[:PORT] Use HTTP proxy on given"
" port.\n");
fprintf(stderr, " --help Print this help.\n");
fprintf(stderr, "\n");
return;
}

int conf_init(conf_t *conf)
{
int i;

*conf->http_proxy = 0;
for(i = 0; i<MAX_STRING1; i++)
conf->proxy_user[i] = 0; 
*conf->timeserver = 0;
return 1;
}

int main(int argc, char *argv[])
{
CURL *curl;
conf_t conf[1];
int RetValue;

ShowAllHeader = 0; 
AutoSyncTime = 0; 
RetValue = 0; 
conf_init(conf);

if(argc > 1) {
int OptionIndex = 0;
while(OptionIndex < argc) {
if(strncmp(argv[OptionIndex], "--server=", 9) == 0)
snprintf(conf->timeserver, MAX_STRING, "%s", &argv[OptionIndex][9]);

if(strcmp(argv[OptionIndex], "--showall") == 0)
ShowAllHeader = 1;

if(strcmp(argv[OptionIndex], "--synctime") == 0)
AutoSyncTime = 1;

if(strncmp(argv[OptionIndex], "--proxy-user=", 13) == 0)
snprintf(conf->proxy_user, MAX_STRING, "%s", &argv[OptionIndex][13]);

if(strncmp(argv[OptionIndex], "--proxy=", 8) == 0)
snprintf(conf->http_proxy, MAX_STRING, "%s", &argv[OptionIndex][8]);

if((strcmp(argv[OptionIndex], "--help") == 0) ||
(strcmp(argv[OptionIndex], "/?") == 0)) {
showUsage();
return 0;
}
OptionIndex++;
}
}

if(*conf->timeserver == 0) 
snprintf(conf->timeserver, MAX_STRING, "%s", DefaultTimeServer[0]);


curl_global_init(CURL_GLOBAL_ALL);
curl = curl_easy_init();
if(curl) {
struct tm *lt;
struct tm *gmt;
time_t tt;
time_t tt_local;
time_t tt_gmt;
double tzonediffFloat;
int tzonediffWord;
char timeBuf[61];
char tzoneBuf[16];

SyncTime_CURL_Init(curl, conf->http_proxy, conf->proxy_user);


tt = time(0);
lt = localtime(&tt);
tt_local = mktime(lt);
gmt = gmtime(&tt);
tt_gmt = mktime(gmt);
tzonediffFloat = difftime(tt_local, tt_gmt);
tzonediffWord = (int)(tzonediffFloat/3600.0);

if((double)(tzonediffWord * 3600) == tzonediffFloat)
snprintf(tzoneBuf, 15, "%+03d'00'", tzonediffWord);
else
snprintf(tzoneBuf, 15, "%+03d'30'", tzonediffWord);


GetSystemTime(&SYSTime);
GetLocalTime(&LOCALTime);
snprintf(timeBuf, 60, "%s, %02d %s %04d %02d:%02d:%02d.%03d, ",
DayStr[LOCALTime.wDayOfWeek], LOCALTime.wDay,
MthStr[LOCALTime.wMonth-1], LOCALTime.wYear,
LOCALTime.wHour, LOCALTime.wMinute, LOCALTime.wSecond,
LOCALTime.wMilliseconds);

fprintf(stderr, "Fetch: %s\n\n", conf->timeserver);
fprintf(stderr, "Before HTTP. Date: %s%s\n\n", timeBuf, tzoneBuf);


SyncTime_CURL_Fetch(curl, conf->timeserver, "index.htm",
HTTP_COMMAND_HEAD);

GetLocalTime(&LOCALTime);
snprintf(timeBuf, 60, "%s, %02d %s %04d %02d:%02d:%02d.%03d, ",
DayStr[LOCALTime.wDayOfWeek], LOCALTime.wDay,
MthStr[LOCALTime.wMonth-1], LOCALTime.wYear,
LOCALTime.wHour, LOCALTime.wMinute, LOCALTime.wSecond,
LOCALTime.wMilliseconds);
fprintf(stderr, "\nAfter HTTP. Date: %s%s\n", timeBuf, tzoneBuf);

if(AutoSyncTime == 3) {

if(!SetSystemTime(&SYSTime)) { 
fprintf(stderr, "ERROR: Unable to set system time.\n");
RetValue = 1;
}
else {

GetLocalTime(&LOCALTime);
snprintf(timeBuf, 60, "%s, %02d %s %04d %02d:%02d:%02d.%03d, ",
DayStr[LOCALTime.wDayOfWeek], LOCALTime.wDay,
MthStr[LOCALTime.wMonth-1], LOCALTime.wYear,
LOCALTime.wHour, LOCALTime.wMinute, LOCALTime.wSecond,
LOCALTime.wMilliseconds);
fprintf(stderr, "\nNew System's Date: %s%s\n", timeBuf, tzoneBuf);
}
}


conf_init(conf);
curl_easy_cleanup(curl);
}
return RetValue;
}
