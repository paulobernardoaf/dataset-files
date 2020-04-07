#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#define URL_BASE "http://speedtest.your.domain/"
#define URL_1M URL_BASE "file_1M.bin"
#define URL_2M URL_BASE "file_2M.bin"
#define URL_5M URL_BASE "file_5M.bin"
#define URL_10M URL_BASE "file_10M.bin"
#define URL_20M URL_BASE "file_20M.bin"
#define URL_50M URL_BASE "file_50M.bin"
#define URL_100M URL_BASE "file_100M.bin"
#define CHKSPEED_VERSION "1.0"
static size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
(void)ptr; 
(void)data; 
return (size_t)(size * nmemb);
}
int main(int argc, char *argv[])
{
CURL *curl_handle;
CURLcode res;
int prtall = 0, prtsep = 0, prttime = 0;
const char *url = URL_1M;
char *appname = argv[0];
if(argc > 1) {
for(argc--, argv++; *argv; argc--, argv++) {
if(strncasecmp(*argv, "-", 1) == 0) {
if(strncasecmp(*argv, "-H", 2) == 0) {
fprintf(stderr,
"\rUsage: %s [-m=1|2|5|10|20|50|100] [-t] [-x] [url]\n",
appname);
exit(1);
}
else if(strncasecmp(*argv, "-V", 2) == 0) {
fprintf(stderr, "\r%s %s - %s\n",
appname, CHKSPEED_VERSION, curl_version());
exit(1);
}
else if(strncasecmp(*argv, "-A", 2) == 0) {
prtall = 1;
}
else if(strncasecmp(*argv, "-X", 2) == 0) {
prtsep = 1;
}
else if(strncasecmp(*argv, "-T", 2) == 0) {
prttime = 1;
}
else if(strncasecmp(*argv, "-M=", 3) == 0) {
long m = strtol((*argv) + 3, NULL, 10);
switch(m) {
case 1:
url = URL_1M;
break;
case 2:
url = URL_2M;
break;
case 5:
url = URL_5M;
break;
case 10:
url = URL_10M;
break;
case 20:
url = URL_20M;
break;
case 50:
url = URL_50M;
break;
case 100:
url = URL_100M;
break;
default:
fprintf(stderr, "\r%s: invalid parameter %s\n",
appname, *argv + 3);
exit(1);
}
}
else {
fprintf(stderr, "\r%s: invalid or unknown option %s\n",
appname, *argv);
exit(1);
}
}
else {
url = *argv;
}
}
}
if(prtsep) {
printf("-------------------------------------------------\n");
}
if(prttime) {
time_t t = time(NULL);
printf("Localtime: %s", ctime(&t));
}
curl_global_init(CURL_GLOBAL_ALL);
curl_handle = curl_easy_init();
curl_easy_setopt(curl_handle, CURLOPT_URL, url);
curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
curl_easy_setopt(curl_handle, CURLOPT_USERAGENT,
"libcurl-speedchecker/" CHKSPEED_VERSION);
res = curl_easy_perform(curl_handle);
if(CURLE_OK == res) {
curl_off_t val;
res = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD_T, &val);
if((CURLE_OK == res) && (val>0))
printf("Data downloaded: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", val);
res = curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME_T, &val);
if((CURLE_OK == res) && (val>0))
printf("Total download time: %" CURL_FORMAT_CURL_OFF_T ".%06ld sec.\n",
(val / 1000000), (long)(val % 1000000));
res = curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD_T, &val);
if((CURLE_OK == res) && (val>0))
printf("Average download speed: %" CURL_FORMAT_CURL_OFF_T
" kbyte/sec.\n", val / 1024);
if(prtall) {
res = curl_easy_getinfo(curl_handle, CURLINFO_NAMELOOKUP_TIME_T, &val);
if((CURLE_OK == res) && (val>0))
printf("Name lookup time: %" CURL_FORMAT_CURL_OFF_T ".%06ld sec.\n",
(val / 1000000), (long)(val % 1000000));
res = curl_easy_getinfo(curl_handle, CURLINFO_CONNECT_TIME_T, &val);
if((CURLE_OK == res) && (val>0))
printf("Connect time: %" CURL_FORMAT_CURL_OFF_T ".%06ld sec.\n",
(val / 1000000), (long)(val % 1000000));
}
}
else {
fprintf(stderr, "Error while fetching '%s' : %s\n",
url, curl_easy_strerror(res));
}
curl_easy_cleanup(curl_handle);
curl_global_cleanup();
return 0;
}
