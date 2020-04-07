#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
int main(int argc, char *argv[])
{
CURL *curl;
CURLcode res;
curl_mime *form = NULL;
curl_mimepart *field = NULL;
struct curl_slist *headerlist = NULL;
static const char buf[] = "Expect:";
curl_global_init(CURL_GLOBAL_ALL);
curl = curl_easy_init();
if(curl) {
form = curl_mime_init(curl);
field = curl_mime_addpart(form);
curl_mime_name(field, "sendfile");
curl_mime_filedata(field, "postit2.c");
field = curl_mime_addpart(form);
curl_mime_name(field, "filename");
curl_mime_data(field, "postit2.c", CURL_ZERO_TERMINATED);
field = curl_mime_addpart(form);
curl_mime_name(field, "submit");
curl_mime_data(field, "send", CURL_ZERO_TERMINATED);
headerlist = curl_slist_append(headerlist, buf);
curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/examplepost.cgi");
if((argc == 2) && (!strcmp(argv[1], "noexpectheader")))
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
res = curl_easy_perform(curl);
if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
curl_easy_cleanup(curl);
curl_mime_free(form);
curl_slist_free_all(headerlist);
}
return 0;
}
