#include <stdio.h>
#include <curl/curl.h>
int main(void)
{
curl_version_info_data *ver;
curl_global_init(CURL_GLOBAL_ALL);
ver = curl_version_info(CURLVERSION_NOW);
if(ver->features & CURL_VERSION_HTTP2)
printf("HTTP/2 support is present\n");
if(ver->features & CURL_VERSION_HTTP3)
printf("HTTP/3 support is present\n");
if(ver->features & CURL_VERSION_ALTSVC)
printf("Alt-svc support is present\n");
curl_global_cleanup();
return 0;
}
