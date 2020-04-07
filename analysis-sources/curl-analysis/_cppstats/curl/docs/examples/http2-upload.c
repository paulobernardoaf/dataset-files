#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <curl/curl.h>
#if !defined(CURLPIPE_MULTIPLEX)
#define CURLPIPE_MULTIPLEX 0
#endif
#define NUM_HANDLES 1000
struct input {
FILE *in;
size_t bytes_read; 
CURL *hnd;
int num;
};
static
void dump(const char *text, int num, unsigned char *ptr, size_t size,
char nohex)
{
size_t i;
size_t c;
unsigned int width = 0x10;
if(nohex)
width = 0x40;
fprintf(stderr, "%d %s, %lu bytes (0x%lx)\n",
num, text, (unsigned long)size, (unsigned long)size);
for(i = 0; i<size; i += width) {
fprintf(stderr, "%4.4lx: ", (unsigned long)i);
if(!nohex) {
for(c = 0; c < width; c++)
if(i + c < size)
fprintf(stderr, "%02x ", ptr[i + c]);
else
fputs(" ", stderr);
}
for(c = 0; (c < width) && (i + c < size); c++) {
if(nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
ptr[i + c + 1] == 0x0A) {
i += (c + 2 - width);
break;
}
fprintf(stderr, "%c",
(ptr[i + c] >= 0x20) && (ptr[i + c]<0x80)?ptr[i + c]:'.');
if(nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
ptr[i + c + 2] == 0x0A) {
i += (c + 3 - width);
break;
}
}
fputc('\n', stderr); 
}
}
static
int my_trace(CURL *handle, curl_infotype type,
char *data, size_t size,
void *userp)
{
char timebuf[60];
const char *text;
struct input *i = (struct input *)userp;
int num = i->num;
static time_t epoch_offset;
static int known_offset;
struct timeval tv;
time_t secs;
struct tm *now;
(void)handle; 
gettimeofday(&tv, NULL);
if(!known_offset) {
epoch_offset = time(NULL) - tv.tv_sec;
known_offset = 1;
}
secs = epoch_offset + tv.tv_sec;
now = localtime(&secs); 
snprintf(timebuf, sizeof(timebuf), "%02d:%02d:%02d.%06ld",
now->tm_hour, now->tm_min, now->tm_sec, (long)tv.tv_usec);
switch(type) {
case CURLINFO_TEXT:
fprintf(stderr, "%s [%d] Info: %s", timebuf, num, data);
default: 
return 0;
case CURLINFO_HEADER_OUT:
text = "=> Send header";
break;
case CURLINFO_DATA_OUT:
text = "=> Send data";
break;
case CURLINFO_SSL_DATA_OUT:
text = "=> Send SSL data";
break;
case CURLINFO_HEADER_IN:
text = "<= Recv header";
break;
case CURLINFO_DATA_IN:
text = "<= Recv data";
break;
case CURLINFO_SSL_DATA_IN:
text = "<= Recv SSL data";
break;
}
dump(text, num, (unsigned char *)data, size, 1);
return 0;
}
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
struct input *i = userp;
size_t retcode = fread(ptr, size, nmemb, i->in);
i->bytes_read += retcode;
return retcode;
}
static void setup(struct input *i, int num, const char *upload)
{
FILE *out;
char url[256];
char filename[128];
struct stat file_info;
curl_off_t uploadsize;
CURL *hnd;
hnd = i->hnd = curl_easy_init();
i->num = num;
snprintf(filename, 128, "dl-%d", num);
out = fopen(filename, "wb");
snprintf(url, 256, "https://localhost:8443/upload-%d", num);
stat(upload, &file_info);
uploadsize = file_info.st_size;
i->in = fopen(upload, "rb");
curl_easy_setopt(hnd, CURLOPT_WRITEDATA, out);
curl_easy_setopt(hnd, CURLOPT_READFUNCTION, read_callback);
curl_easy_setopt(hnd, CURLOPT_READDATA, i);
curl_easy_setopt(hnd, CURLOPT_INFILESIZE_LARGE, uploadsize);
curl_easy_setopt(hnd, CURLOPT_URL, url);
curl_easy_setopt(hnd, CURLOPT_UPLOAD, 1L);
curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
curl_easy_setopt(hnd, CURLOPT_DEBUGFUNCTION, my_trace);
curl_easy_setopt(hnd, CURLOPT_DEBUGDATA, i);
curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
#if (CURLPIPE_MULTIPLEX > 0)
curl_easy_setopt(hnd, CURLOPT_PIPEWAIT, 1L);
#endif
}
int main(int argc, char **argv)
{
struct input trans[NUM_HANDLES];
CURLM *multi_handle;
int i;
int still_running = 0; 
const char *filename = "index.html";
int num_transfers;
if(argc > 1) {
num_transfers = atoi(argv[1]);
if(!num_transfers || (num_transfers > NUM_HANDLES))
num_transfers = 3; 
if(argc > 2)
filename = argv[2];
}
else
num_transfers = 3;
multi_handle = curl_multi_init();
for(i = 0; i<num_transfers; i++) {
setup(&trans[i], i, filename);
curl_multi_add_handle(multi_handle, trans[i].hnd);
}
curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
curl_multi_setopt(multi_handle, CURLMOPT_MAX_HOST_CONNECTIONS, 1L);
curl_multi_perform(multi_handle, &still_running);
while(still_running) {
struct timeval timeout;
int rc; 
CURLMcode mc; 
fd_set fdread;
fd_set fdwrite;
fd_set fdexcep;
int maxfd = -1;
long curl_timeo = -1;
FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);
timeout.tv_sec = 1;
timeout.tv_usec = 0;
curl_multi_timeout(multi_handle, &curl_timeo);
if(curl_timeo >= 0) {
timeout.tv_sec = curl_timeo / 1000;
if(timeout.tv_sec > 1)
timeout.tv_sec = 1;
else
timeout.tv_usec = (curl_timeo % 1000) * 1000;
}
mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
if(mc != CURLM_OK) {
fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
break;
}
if(maxfd == -1) {
#if defined(_WIN32)
Sleep(100);
rc = 0;
#else
struct timeval wait = { 0, 100 * 1000 }; 
rc = select(0, NULL, NULL, NULL, &wait);
#endif
}
else {
rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
}
switch(rc) {
case -1:
break;
case 0:
default:
curl_multi_perform(multi_handle, &still_running);
break;
}
}
curl_multi_cleanup(multi_handle);
for(i = 0; i<num_transfers; i++) {
curl_multi_remove_handle(multi_handle, trans[i].hnd);
curl_easy_cleanup(trans[i].hnd);
}
return 0;
}
