#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
struct MemoryStruct {
char *memory;
size_t size;
};
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
size_t realsize = size * nmemb;
struct MemoryStruct *mem = (struct MemoryStruct *)userp;
char *ptr = realloc(mem->memory, mem->size + realsize + 1);
if(ptr == NULL) {
printf("not enough memory (realloc returned NULL)\n");
return 0;
}
mem->memory = ptr;
memcpy(&(mem->memory[mem->size]), contents, realsize);
mem->size += realsize;
mem->memory[mem->size] = 0;
return realsize;
}
int main(void)
{
CURL *curl_handle;
CURLcode res;
struct MemoryStruct chunk;
chunk.memory = malloc(1); 
chunk.size = 0; 
curl_global_init(CURL_GLOBAL_ALL);
curl_handle = curl_easy_init();
curl_easy_setopt(curl_handle, CURLOPT_URL, "https://www.example.com/");
curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
res = curl_easy_perform(curl_handle);
if(res != CURLE_OK) {
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
}
else {
printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
}
curl_easy_cleanup(curl_handle);
free(chunk.memory);
curl_global_cleanup();
return 0;
}
