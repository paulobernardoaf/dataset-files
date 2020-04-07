#include <stdio.h>
#include <curl/curl.h>
#if !CURL_AT_LEAST_VERSION(7, 62, 0)
#error "this example requires curl 7.62.0 or later"
#endif
int main(void)
{
CURL *curl;
CURLcode res;
CURLU *urlp;
CURLUcode uc;
curl = curl_easy_init();
urlp = curl_url();
uc = curl_url_set(urlp, CURLUPART_URL,
"http://example.com/path/index.html", 0);
if(uc) {
fprintf(stderr, "curl_url_set() failed: %in", uc);
goto cleanup;
}
if(curl) {
curl_easy_setopt(curl, CURLOPT_CURLU, urlp);
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
res = curl_easy_perform(curl);
if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
goto cleanup;
}
cleanup:
curl_url_cleanup(urlp);
curl_easy_cleanup(curl);
return 0;
}
