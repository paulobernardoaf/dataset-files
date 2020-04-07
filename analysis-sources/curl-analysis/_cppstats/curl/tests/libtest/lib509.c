#include "test.h"
#include <string.h>
static int seen_malloc = 0;
static int seen_free = 0;
static int seen_realloc = 0;
static int seen_strdup = 0;
static int seen_calloc = 0;
void *custom_malloc(size_t size);
void custom_free(void *ptr);
void *custom_realloc(void *ptr, size_t size);
char *custom_strdup(const char *ptr);
void *custom_calloc(size_t nmemb, size_t size);
void *custom_calloc(size_t nmemb, size_t size)
{
if(!seen_calloc) {
printf("seen custom_calloc()\n");
seen_calloc = 1;
}
return (calloc)(nmemb, size);
}
void *custom_malloc(size_t size)
{
if(!seen_malloc && seen_calloc) {
printf("seen custom_malloc()\n");
seen_malloc = 1;
}
return (malloc)(size);
}
char *custom_strdup(const char *ptr)
{
if(!seen_strdup && seen_malloc) {
seen_strdup = 1;
}
return (strdup)(ptr);
}
void *custom_realloc(void *ptr, size_t size)
{
if(!seen_realloc && seen_malloc) {
printf("seen custom_realloc()\n");
seen_realloc = 1;
}
return (realloc)(ptr, size);
}
void custom_free(void *ptr)
{
if(!seen_free && seen_realloc) {
printf("seen custom_free()\n");
seen_free = 1;
}
(free)(ptr);
}
int test(char *URL)
{
unsigned char a[] = {0x2f, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
0x91, 0xa2, 0xb3, 0xc4, 0xd5, 0xe6, 0xf7};
CURLcode res;
CURL *curl;
int asize;
char *str = NULL;
(void)URL;
res = curl_global_init_mem(CURL_GLOBAL_ALL,
custom_malloc,
custom_free,
custom_realloc,
custom_strdup,
custom_calloc);
if(res != CURLE_OK) {
fprintf(stderr, "curl_global_init_mem() failed\n");
return TEST_ERR_MAJOR_BAD;
}
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
curl_global_cleanup();
return TEST_ERR_MAJOR_BAD;
}
test_setopt(curl, CURLOPT_USERAGENT, "test509"); 
asize = (int)sizeof(a);
str = curl_easy_escape(curl, (char *)a, asize); 
test_cleanup:
if(str)
curl_free(str);
curl_easy_cleanup(curl);
curl_global_cleanup();
return (int)res;
}
