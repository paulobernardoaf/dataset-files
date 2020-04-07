

























#include <stdio.h>
#include <pthread.h>
#include <curl/curl.h>

#define NUMT 4










const char * const urls[NUMT]= {
"https://curl.haxx.se/",
"ftp://cool.haxx.se/",
"https://www.cag.se/",
"www.haxx.se"
};

static void *pull_one_url(void *url)
{
CURL *curl;

curl = curl_easy_init();
curl_easy_setopt(curl, CURLOPT_URL, url);
curl_easy_perform(curl); 
curl_easy_cleanup(curl);

return NULL;
}








int main(int argc, char **argv)
{
pthread_t tid[NUMT];
int i;


curl_global_init(CURL_GLOBAL_ALL);

for(i = 0; i< NUMT; i++) {
int error = pthread_create(&tid[i],
NULL, 
pull_one_url,
(void *)urls[i]);
if(0 != error)
fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
else
fprintf(stderr, "Thread %d, gets %s\n", i, urls[i]);
}


for(i = 0; i< NUMT; i++) {
pthread_join(tid[i], NULL);
fprintf(stderr, "Thread %d terminated\n", i);
}

return 0;
}
