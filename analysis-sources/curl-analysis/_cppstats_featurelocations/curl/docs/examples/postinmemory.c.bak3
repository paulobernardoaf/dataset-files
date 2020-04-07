
























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

int main(void)
{
CURL *curl;
CURLcode res;
struct MemoryStruct chunk;
static const char *postthis = "Field=1&Field=2&Field=3";

chunk.memory = malloc(1); 
chunk.size = 0; 

curl_global_init(CURL_GLOBAL_ALL);
curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL, "https://www.example.org/");


curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);


curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);



curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);



curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));


res = curl_easy_perform(curl);

if(res != CURLE_OK) {
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
}
else {






printf("%s\n",chunk.memory);
}


curl_easy_cleanup(curl);
}

free(chunk.memory);
curl_global_cleanup();
return 0;
}
