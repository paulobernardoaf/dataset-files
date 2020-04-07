#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <curl/curl.h>
struct Memory {
char *memory;
size_t size;
};
static size_t
write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
size_t realsize = size * nmemb;
struct Memory *mem = (struct Memory *)userp;
char *ptr = realloc(mem->memory, mem->size + realsize + 1);
if(!ptr) {
printf("not enough memory (realloc returned NULL)\n");
return 0;
}
mem->memory = ptr;
memcpy(&(mem->memory[mem->size]), contents, realsize);
mem->size += realsize;
mem->memory[mem->size] = 0;
return realsize;
}
#define MAX_FILES 10
static struct Memory files[MAX_FILES];
static int pushindex = 1;
static void init_memory(struct Memory *chunk)
{
chunk->memory = malloc(1); 
chunk->size = 0; 
}
static void setup(CURL *hnd)
{
curl_easy_setopt(hnd, CURLOPT_URL, "https://localhost:8443/index.html");
curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_cb);
init_memory(&files[0]);
curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &files[0]);
curl_easy_setopt(hnd, CURLOPT_PIPEWAIT, 1L);
}
static int server_push_callback(CURL *parent,
CURL *easy,
size_t num_headers,
struct curl_pushheaders *headers,
void *userp)
{
char *headp;
int *transfers = (int *)userp;
(void)parent; 
(void)num_headers; 
if(pushindex == MAX_FILES)
return CURL_PUSH_DENY;
init_memory(&files[pushindex]);
curl_easy_setopt(easy, CURLOPT_WRITEDATA, &files[pushindex]);
pushindex++;
headp = curl_pushheader_byname(headers, ":path");
if(headp)
fprintf(stderr, "* Pushed :path '%s'\n", headp );
(*transfers)++; 
return CURL_PUSH_OK;
}
int main(void)
{
CURL *easy;
CURLM *multi;
int still_running; 
int transfers = 1; 
int i;
struct CURLMsg *m;
multi = curl_multi_init();
easy = curl_easy_init();
setup(easy);
curl_multi_add_handle(multi, easy);
curl_multi_setopt(multi, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
curl_multi_setopt(multi, CURLMOPT_PUSHFUNCTION, server_push_callback);
curl_multi_setopt(multi, CURLMOPT_PUSHDATA, &transfers);
while(transfers) {
int rc;
CURLMcode mcode = curl_multi_perform(multi, &still_running);
if(mcode)
break;
mcode = curl_multi_wait(multi, NULL, 0, 1000, &rc);
if(mcode)
break;
do {
int msgq = 0;;
m = curl_multi_info_read(multi, &msgq);
if(m && (m->msg == CURLMSG_DONE)) {
CURL *e = m->easy_handle;
transfers--;
curl_multi_remove_handle(multi, e);
curl_easy_cleanup(e);
}
} while(m);
}
curl_multi_cleanup(multi);
for(i = 0; i < pushindex; i++) {
free(files[i].memory);
}
return 0;
}
