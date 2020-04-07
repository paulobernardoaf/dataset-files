#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
int main(int argc, char *argv[])
{
CURL *curl;
CURLcode res;
struct curl_httppost *formpost = NULL;
struct curl_httppost *lastptr = NULL;
struct curl_slist *headerlist = NULL;
static const char buf[] = "Expect:";
curl_global_init(CURL_GLOBAL_ALL);
curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "sendfile",
CURLFORM_FILE, "postit2.c",
CURLFORM_END);
curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "filename",
CURLFORM_COPYCONTENTS, "postit2.c",
CURLFORM_END);
curl_formadd(&formpost,
&lastptr,
CURLFORM_COPYNAME, "submit",
CURLFORM_COPYCONTENTS, "send",
CURLFORM_END);
curl = curl_easy_init();
headerlist = curl_slist_append(headerlist, buf);
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/examplepost.cgi");
if((argc == 2) && (!strcmp(argv[1], "noexpectheader")))
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
res = curl_easy_perform(curl);
if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
curl_easy_cleanup(curl);
curl_formfree(formpost);
curl_slist_free_all(headerlist);
}
return 0;
}
