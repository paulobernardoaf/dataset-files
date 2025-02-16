
























#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/time.h>
#include <unistd.h>


#include <curl/curl.h>

#if !defined(CURLPIPE_MULTIPLEX)
#error "too old libcurl, can't do HTTP/2 server push!"
#endif

static
void dump(const char *text, unsigned char *ptr, size_t size,
char nohex)
{
size_t i;
size_t c;

unsigned int width = 0x10;

if(nohex)

width = 0x40;

fprintf(stderr, "%s, %lu bytes (0x%lx)\n",
text, (unsigned long)size, (unsigned long)size);

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
const char *text;
(void)handle; 
(void)userp;
switch(type) {
case CURLINFO_TEXT:
fprintf(stderr, "== Info: %s", data);

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

dump(text, (unsigned char *)data, size, 1);
return 0;
}

#define OUTPUTFILE "dl"

static int setup(CURL *hnd)
{
FILE *out = fopen(OUTPUTFILE, "wb");
if(!out)

return 1;


curl_easy_setopt(hnd, CURLOPT_WRITEDATA, out);


curl_easy_setopt(hnd, CURLOPT_URL, "https://localhost:8443/index.html");


curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
curl_easy_setopt(hnd, CURLOPT_DEBUGFUNCTION, my_trace);


curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);


curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);

#if (CURLPIPE_MULTIPLEX > 0)

curl_easy_setopt(hnd, CURLOPT_PIPEWAIT, 1L);
#endif
return 0; 
}


static int server_push_callback(CURL *parent,
CURL *easy,
size_t num_headers,
struct curl_pushheaders *headers,
void *userp)
{
char *headp;
size_t i;
int *transfers = (int *)userp;
char filename[128];
FILE *out;
static unsigned int count = 0;

(void)parent; 

snprintf(filename, 128, "push%u", count++);


out = fopen(filename, "wb");
if(!out) {

fprintf(stderr, "Failed to create output file for push\n");
return CURL_PUSH_DENY;
}


curl_easy_setopt(easy, CURLOPT_WRITEDATA, out);

fprintf(stderr, "**** push callback approves stream %u, got %lu headers!\n",
count, (unsigned long)num_headers);

for(i = 0; i<num_headers; i++) {
headp = curl_pushheader_bynum(headers, i);
fprintf(stderr, "**** header %lu: %s\n", (unsigned long)i, headp);
}

headp = curl_pushheader_byname(headers, ":path");
if(headp) {
fprintf(stderr, "**** The PATH is %s\n", headp );
}

(*transfers)++; 
return CURL_PUSH_OK;
}





int main(void)
{
CURL *easy;
CURLM *multi_handle;
int still_running; 
int transfers = 1; 
struct CURLMsg *m;


multi_handle = curl_multi_init();

easy = curl_easy_init();


if(setup(easy)) {
fprintf(stderr, "failed\n");
return 1;
}


curl_multi_add_handle(multi_handle, easy);

curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
curl_multi_setopt(multi_handle, CURLMOPT_PUSHFUNCTION, server_push_callback);
curl_multi_setopt(multi_handle, CURLMOPT_PUSHDATA, &transfers);


curl_multi_perform(multi_handle, &still_running);

do {
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







do {
int msgq = 0;;
m = curl_multi_info_read(multi_handle, &msgq);
if(m && (m->msg == CURLMSG_DONE)) {
CURL *e = m->easy_handle;
transfers--;
curl_multi_remove_handle(multi_handle, e);
curl_easy_cleanup(e);
}
} while(m);

} while(transfers); 

curl_multi_cleanup(multi_handle);


return 0;
}
