#include "curlcheck.h"
#include "urldata.h"
#include "url.h"
#include "memdebug.h" 
static CURLcode unit_setup(void)
{
return CURLE_OK;
}
static void unit_stop(void)
{
}
UNITTEST_START
{
int rc;
struct Curl_easy *empty;
const char *hostname = "hostname";
enum dupstring i;
bool async = FALSE;
bool protocol_connect = FALSE;
rc = Curl_open(&empty);
if(rc)
goto unit_test_abort;
fail_unless(rc == CURLE_OK, "Curl_open() failed");
rc = Curl_connect(empty, &async, &protocol_connect);
fail_unless(rc == CURLE_URL_MALFORMAT,
"Curl_connect() failed to return CURLE_URL_MALFORMAT");
fail_unless(empty->magic == CURLEASY_MAGIC_NUMBER,
"empty->magic should be equal to CURLEASY_MAGIC_NUMBER");
rc = Curl_connect(empty, &async, &protocol_connect);
fail_unless(rc == CURLE_URL_MALFORMAT,
"Curl_connect() failed to return CURLE_URL_MALFORMAT");
rc = Curl_init_userdefined(empty);
fail_unless(rc == CURLE_OK, "Curl_userdefined() failed");
rc = Curl_init_do(empty, empty->conn);
fail_unless(rc == CURLE_OK, "Curl_init_do() failed");
rc = Curl_parse_login_details(
hostname, strlen(hostname), NULL, NULL, NULL);
fail_unless(rc == CURLE_OK,
"Curl_parse_login_details() failed");
rc = Curl_disconnect(empty, empty->conn, FALSE);
fail_unless(rc == CURLE_OK,
"Curl_disconnect() with dead_connection set FALSE failed");
Curl_freeset(empty);
for(i = (enum dupstring)0; i < STRING_LAST; i++) {
fail_unless(empty->set.str[i] == NULL,
"Curl_free() did not set to NULL");
}
Curl_free_request_state(empty);
rc = Curl_close(&empty);
fail_unless(rc == CURLE_OK, "Curl_close() failed");
}
UNITTEST_STOP
