#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#define FROM "<sender@example.org>"
#define TO "<addressee@example.net>"
#define CC "<info@example.org>"
static const char *payload_text[] = {
"Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
"To: " TO "\r\n",
"From: " FROM " (Example User)\r\n",
"Cc: " CC " (Another example User)\r\n",
"Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
"rfcpedant.example.org>\r\n",
"Subject: SMTP TLS example message\r\n",
"\r\n", 
"The body of the message starts here.\r\n",
"\r\n",
"It could be a lot of lines, could be MIME encoded, whatever.\r\n",
"Check RFC5322.\r\n",
NULL
};
struct upload_status {
int lines_read;
};
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
struct upload_status *upload_ctx = (struct upload_status *)userp;
const char *data;
if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
return 0;
}
data = payload_text[upload_ctx->lines_read];
if(data) {
size_t len = strlen(data);
memcpy(ptr, data, len);
upload_ctx->lines_read++;
return len;
}
return 0;
}
int main(void)
{
CURL *curl;
CURLcode res = CURLE_OK;
struct curl_slist *recipients = NULL;
struct upload_status upload_ctx;
upload_ctx.lines_read = 0;
curl = curl_easy_init();
if(curl) {
curl_easy_setopt(curl, CURLOPT_USERNAME, "user");
curl_easy_setopt(curl, CURLOPT_PASSWORD, "secret");
curl_easy_setopt(curl, CURLOPT_URL, "smtp://mainserver.example.net:587");
curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");
curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
recipients = curl_slist_append(recipients, TO);
recipients = curl_slist_append(recipients, CC);
curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
res = curl_easy_perform(curl);
if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
curl_slist_free_all(recipients);
curl_easy_cleanup(curl);
}
return (int)res;
}
