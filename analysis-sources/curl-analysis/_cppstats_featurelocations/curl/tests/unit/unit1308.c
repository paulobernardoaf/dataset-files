




















#include "curlcheck.h"

#include <curl/curl.h>

static CURLcode unit_setup(void)
{
return CURLE_OK;
}

static void unit_stop(void)
{

}

static size_t print_httppost_callback(void *arg, const char *buf, size_t len)
{
fwrite(buf, len, 1, stdout);
(*(size_t *) arg) += len;
return len;
}

UNITTEST_START
int rc;
struct curl_httppost *post = NULL;
struct curl_httppost *last = NULL;
size_t total_size = 0;
char buffer[] = "test buffer";

rc = curl_formadd(&post, &last, CURLFORM_COPYNAME, "name",
CURLFORM_COPYCONTENTS, "content", CURLFORM_END);

fail_unless(rc == 0, "curl_formadd returned error");



fail_unless(post == last, "post and last weren't the same");

rc = curl_formadd(&post, &last, CURLFORM_COPYNAME, "htmlcode",
CURLFORM_COPYCONTENTS, "<HTML></HTML>",
CURLFORM_CONTENTTYPE, "text/html", CURLFORM_END);

fail_unless(rc == 0, "curl_formadd returned error");

rc = curl_formadd(&post, &last, CURLFORM_COPYNAME, "name_for_ptrcontent",
CURLFORM_PTRCONTENTS, buffer, CURLFORM_END);

fail_unless(rc == 0, "curl_formadd returned error");

rc = curl_formget(post, &total_size, print_httppost_callback);

fail_unless(rc == 0, "curl_formget returned error");

fail_unless(total_size == 488, "curl_formget got wrong size back");

curl_formfree(post);


post = last = NULL;

rc = curl_formadd(&post, &last,
CURLFORM_PTRNAME, "name of file field",
CURLFORM_FILE, "log/test-1308",
CURLFORM_FILENAME, "custom named file",
CURLFORM_END);

fail_unless(rc == 0, "curl_formadd returned error");

rc = curl_formget(post, &total_size, print_httppost_callback);
fail_unless(rc == 0, "curl_formget returned error");
fail_unless(total_size == 851, "curl_formget got wrong size back");

curl_formfree(post);

UNITTEST_STOP
