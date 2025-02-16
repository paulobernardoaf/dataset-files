#include <stdio.h>
#include <curl/curl.h>
#if !CURL_AT_LEAST_VERSION(7, 62, 0)
#error "this example requires curl 7.62.0 or later"
#endif
int main(void)
{
CURLU *h;
CURLUcode uc;
char *host;
char *path;
h = curl_url(); 
if(!h)
return 1;
uc = curl_url_set(h, CURLUPART_URL, "http://example.com/path/index.html", 0);
if(uc)
goto fail;
uc = curl_url_get(h, CURLUPART_HOST, &host, 0);
if(!uc) {
printf("Host name: %s\n", host);
curl_free(host);
}
uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
if(!uc) {
printf("Path: %s\n", path);
curl_free(path);
}
uc = curl_url_set(h, CURLUPART_URL, "../another/second.html", 0);
if(uc)
goto fail;
uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
if(!uc) {
printf("Path: %s\n", path);
curl_free(path);
}
fail:
curl_url_cleanup(h); 
return 0;
}
