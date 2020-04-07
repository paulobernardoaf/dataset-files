#include <stdio.h>
#include <curl/curl.h>
static size_t
write_response(void *ptr, size_t size, size_t nmemb, void *data)
{
FILE *writehere = (FILE *)data;
return fwrite(ptr, size, nmemb, writehere);
}
#define FTPBODY "ftp-list"
#define FTPHEADERS "ftp-responses"
int main(void)
{
CURL *curl;
CURLcode res;
FILE *ftpfile;
FILE *respfile;
ftpfile = fopen(FTPBODY, "wb"); 
respfile = fopen(FTPHEADERS, "wb"); 
curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_URL, "ftp://ftp.example.com/");
curl_easy_setopt(curl, CURLOPT_WRITEDATA, ftpfile);
curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_response);
curl_easy_setopt(curl, CURLOPT_HEADERDATA, respfile);
res = curl_easy_perform(curl);
if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
curl_easy_cleanup(curl);
}
fclose(ftpfile); 
fclose(respfile); 
return 0;
}
