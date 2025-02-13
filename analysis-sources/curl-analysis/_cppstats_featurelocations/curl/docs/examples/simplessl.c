
























#include <stdio.h>

#include <curl/curl.h>



















int main(void)
{
CURL *curl;
CURLcode res;
FILE *headerfile;
const char *pPassphrase = NULL;

static const char *pCertFile = "testcert.pem";
static const char *pCACertFile = "cacert.pem";
static const char *pHeaderFile = "dumpit";

const char *pKeyName;
const char *pKeyType;

const char *pEngine;

#if defined(USE_ENGINE)
pKeyName = "rsa_test";
pKeyType = "ENG";
pEngine = "chil"; 
#else
pKeyName = "testkey.pem";
pKeyType = "PEM";
pEngine = NULL;
#endif

headerfile = fopen(pHeaderFile, "wb");

curl_global_init(CURL_GLOBAL_DEFAULT);

curl = curl_easy_init();
if(curl) {

curl_easy_setopt(curl, CURLOPT_URL, "HTTPS://your.favourite.ssl.site");
curl_easy_setopt(curl, CURLOPT_HEADERDATA, headerfile);

do { 
if(pEngine) {

if(curl_easy_setopt(curl, CURLOPT_SSLENGINE, pEngine) != CURLE_OK) {

fprintf(stderr, "can't set crypto engine\n");
break;
}
if(curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L) != CURLE_OK) {



fprintf(stderr, "can't set crypto engine as default\n");
break;
}
}


curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");


curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);



if(pPassphrase)
curl_easy_setopt(curl, CURLOPT_KEYPASSWD, pPassphrase);



curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, pKeyType);


curl_easy_setopt(curl, CURLOPT_SSLKEY, pKeyName);


curl_easy_setopt(curl, CURLOPT_CAINFO, pCACertFile);


curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);


res = curl_easy_perform(curl);

if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));


} while(0);

curl_easy_cleanup(curl);
}

curl_global_cleanup();

return 0;
}
