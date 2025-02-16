




















#include "curlcheck.h"

#include "urldata.h"
#include "url.h" 
#include "curl_base64.h"
#include "memdebug.h" 

static struct Curl_easy *data;

static CURLcode unit_setup(void)
{
int res = CURLE_OK;

global_init(CURL_GLOBAL_ALL);
data = curl_easy_init();
if(!data)
return CURLE_OUT_OF_MEMORY;
return res;
}

static void unit_stop(void)
{
curl_easy_cleanup(data);
curl_global_cleanup();
}

UNITTEST_START

char *output;
unsigned char *decoded;
size_t size = 0;
unsigned char anychar = 'x';
CURLcode rc;

rc = Curl_base64_encode(data, "i", 1, &output, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 4, "size should be 4");
verify_memory(output, "aQ==", 4);
Curl_safefree(output);

rc = Curl_base64_encode(data, "ii", 2, &output, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 4, "size should be 4");
verify_memory(output, "aWk=", 4);
Curl_safefree(output);

rc = Curl_base64_encode(data, "iii", 3, &output, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 4, "size should be 4");
verify_memory(output, "aWlp", 4);
Curl_safefree(output);

rc = Curl_base64_encode(data, "iiii", 4, &output, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 8, "size should be 8");
verify_memory(output, "aWlpaQ==", 8);
Curl_safefree(output);

rc = Curl_base64_encode(data, "\xff\x01\xfe\x02", 4, &output, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 8, "size should be 8");
verify_memory(output, "/wH+Ag==", 8);
Curl_safefree(output);

rc = Curl_base64url_encode(data, "\xff\x01\xfe\x02", 4, &output, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 8, "size should be 8");
verify_memory(output, "_wH-Ag==", 8);
Curl_safefree(output);

rc = Curl_base64url_encode(data, "iiii", 4, &output, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 8, "size should be 8");
verify_memory(output, "aWlpaQ==", 8);
Curl_safefree(output);


rc = Curl_base64_encode(data, "iiii", 0, &output, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 8, "size should be 8");
verify_memory(output, "aWlpaQ==", 8);
Curl_safefree(output);

rc = Curl_base64_decode("aWlpaQ==", &decoded, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 4, "size should be 4");
verify_memory(decoded, "iiii", 4);
Curl_safefree(decoded);

rc = Curl_base64_decode("aWlp", &decoded, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 3, "size should be 3");
verify_memory(decoded, "iii", 3);
Curl_safefree(decoded);

rc = Curl_base64_decode("aWk=", &decoded, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 2, "size should be 2");
verify_memory(decoded, "ii", 2);
Curl_safefree(decoded);

rc = Curl_base64_decode("aQ==", &decoded, &size);
fail_unless(rc == CURLE_OK, "return code should be CURLE_OK");
fail_unless(size == 1, "size should be 1");
verify_memory(decoded, "i", 2);
Curl_safefree(decoded);


size = 1; 
decoded = &anychar; 
rc = Curl_base64_decode("aQ", &decoded, &size);
fail_unless(rc == CURLE_BAD_CONTENT_ENCODING,
"return code should be CURLE_BAD_CONTENT_ENCODING");
fail_unless(size == 0, "size should be 0");
fail_if(decoded, "returned pointer should be NULL");


size = 1; 
decoded = &anychar; 
rc = Curl_base64_decode("a===", &decoded, &size);
fail_unless(rc == CURLE_BAD_CONTENT_ENCODING,
"return code should be CURLE_BAD_CONTENT_ENCODING");
fail_unless(size == 0, "size should be 0");
fail_if(decoded, "returned pointer should be NULL");


size = 1; 
decoded = &anychar; 
rc = Curl_base64_decode("a=Q=", &decoded, &size);
fail_unless(rc == CURLE_BAD_CONTENT_ENCODING,
"return code should be CURLE_BAD_CONTENT_ENCODING");
fail_unless(size == 0, "size should be 0");
fail_if(decoded, "returned pointer should be NULL");


size = 1; 
decoded = &anychar; 
rc = Curl_base64_decode("a\x1f==", &decoded, &size);
fail_unless(rc == CURLE_BAD_CONTENT_ENCODING,
"return code should be CURLE_BAD_CONTENT_ENCODING");
fail_unless(size == 0, "size should be 0");
fail_if(decoded, "returned pointer should be NULL");

UNITTEST_STOP
