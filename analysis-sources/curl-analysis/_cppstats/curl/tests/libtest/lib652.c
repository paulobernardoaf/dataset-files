#include "test.h"
#include "memdebug.h"
static char buffer[17000]; 
int test(char *URL)
{
CURL *curl = NULL;
CURLcode res = CURLE_OK;
curl_mime *mime = NULL;
curl_mimepart *part;
struct curl_slist *recipients = NULL;
int i;
int size = (int)sizeof(buffer) / 10;
for(i = 0; i < size ; i++)
memset(&buffer[i * 10], 65 + (i % 26), 10);
if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
fprintf(stderr, "curl_global_init() failed\n");
return TEST_ERR_MAJOR_BAD;
}
curl = curl_easy_init();
if(!curl) {
fprintf(stderr, "curl_easy_init() failed\n");
res = (CURLcode) TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
mime = curl_mime_init(curl);
if(!mime) {
fprintf(stderr, "curl_mime_init() failed\n");
res = (CURLcode) TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
part = curl_mime_addpart(mime);
if(!part) {
fprintf(stderr, "curl_mime_addpart() failed\n");
res = (CURLcode) TEST_ERR_MAJOR_BAD;
goto test_cleanup;
}
res = curl_mime_filename(part, "myfile.jpg");
if(res) {
fprintf(stderr, "curl_mime_filename() failed\n");
goto test_cleanup;
}
res = curl_mime_type(part, "image/jpeg");
if(res) {
fprintf(stderr, "curl_mime_type() failed\n");
goto test_cleanup;
}
res = curl_mime_data(part, buffer, sizeof(buffer));
if(res) {
fprintf(stderr, "curl_mime_data() failed\n");
goto test_cleanup;
}
res = curl_mime_encoder(part, "base64");
if(res) {
fprintf(stderr, "curl_mime_encoder() failed\n");
goto test_cleanup;
}
recipients = curl_slist_append(NULL, "someone@example.com");
if(!recipients) {
fprintf(stderr, "curl_slist_append() failed\n");
goto test_cleanup;
}
test_setopt(curl, CURLOPT_URL, URL);
test_setopt(curl, CURLOPT_MAIL_FROM, "somebody@example.com");
test_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
test_setopt(curl, CURLOPT_MIMEPOST, mime);
test_setopt(curl, CURLOPT_UPLOAD_BUFFERSIZE, 16411L);
test_setopt(curl, CURLOPT_VERBOSE, 1L);
res = curl_easy_perform(curl);
test_cleanup:
curl_easy_cleanup(curl);
curl_mime_free(mime);
curl_slist_free_all(recipients);
curl_global_cleanup();
return res;
}
