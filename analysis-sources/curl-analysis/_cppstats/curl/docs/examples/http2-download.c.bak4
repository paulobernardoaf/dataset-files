
























#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/time.h>
#include <unistd.h>


#include <curl/curl.h>

#if !defined(CURLPIPE_MULTIPLEX)



#define CURLPIPE_MULTIPLEX 0
#endif

struct transfer {
CURL *easy;
unsigned int num;
FILE *out;
};

#define NUM_HANDLES 1000

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
const char *text;
struct transfer *t = (struct transfer *)userp;
unsigned int num = t->num;
(void)handle; 

switch(type) {
case CURLINFO_TEXT:
fprintf(stderr, "== %u Info: %s", num, data);

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

static void setup(struct transfer *t, int num)
{
char filename[128];
CURL *hnd;

hnd = t->easy = curl_easy_init();

snprintf(filename, 128, "dl-%d", num);

t->out = fopen(filename, "wb");


curl_easy_setopt(hnd, CURLOPT_WRITEDATA, t->out);


curl_easy_setopt(hnd, CURLOPT_URL, "https://localhost:8443/index.html");


curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
curl_easy_setopt(hnd, CURLOPT_DEBUGFUNCTION, my_trace);
curl_easy_setopt(hnd, CURLOPT_DEBUGDATA, t);


curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);


curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);

#if (CURLPIPE_MULTIPLEX > 0)

curl_easy_setopt(hnd, CURLOPT_PIPEWAIT, 1L);
#endif
}




int main(int argc, char **argv)
{
struct transfer trans[NUM_HANDLES];
CURLM *multi_handle;
int i;
int still_running = 0; 
int num_transfers;
if(argc > 1) {

num_transfers = atoi(argv[1]);
if((num_transfers < 1) || (num_transfers > NUM_HANDLES))
num_transfers = 3; 
}
else
num_transfers = 3; 


multi_handle = curl_multi_init();

for(i = 0; i < num_transfers; i++) {
setup(&trans[i], i);


curl_multi_add_handle(multi_handle, trans[i].easy);
}

curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);


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

for(i = 0; i < num_transfers; i++) {
curl_multi_remove_handle(multi_handle, trans[i].easy);
curl_easy_cleanup(trans[i].easy);
}

curl_multi_cleanup(multi_handle);

return 0;
}
