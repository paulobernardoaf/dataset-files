#include "curlcheck.h"
#include "curl/mprintf.h"
static CURLcode unit_setup(void) {return CURLE_OK;}
static void unit_stop(void) {}
UNITTEST_START
int rc;
char buf[3] = {'b', 'u', 'g'};
const char *str = "bug";
int width = 3;
char output[24];
rc = curl_msnprintf(output, 4, "%.*s", width, buf);
fail_unless(rc == 3, "return code should be 3");
fail_unless(!strcmp(output, "bug"), "wrong output");
rc = curl_msnprintf(output, 4, "%.*s", width, str);
fail_unless(rc == 3, "return code should be 3");
fail_unless(!strcmp(output, "bug"), "wrong output");
width = 2;
rc = curl_msnprintf(output, 4, "%.*s", width, buf);
fail_unless(rc == 2, "return code should be 2");
fail_unless(!strcmp(output, "bu"), "wrong output");
rc = curl_msnprintf(output, 8, "%.8s", str);
fail_unless(rc == 3, "return code should be 3");
fail_unless(!strcmp(output, "bug"), "wrong output");
rc = curl_msnprintf(output, 8, "%.3s", "0123456789");
fail_unless(rc == 3, "return code should be 3");
fail_unless(!strcmp(output, "012"), "wrong output");
rc = curl_msnprintf(output, 8, "%-8s", str);
fail_unless(rc == 8, "return code should be 8");
fail_unless(!strcmp(output, "bug "), "wrong output");
rc = curl_msnprintf(output, 8, "%8s", str);
fail_unless(rc == 8, "return code should be 8");
fail_unless(!strcmp(output, " bu"), "wrong output");
rc = curl_msnprintf(output, 4, "%d", 10240);
fail_unless(rc == 4, "return code should be 4");
fail_unless(!strcmp(output, "102"), "wrong output");
rc = curl_msnprintf(output, 16, "%8s%8s", str, str);
fail_unless(rc == 16, "return code should be 16");
fail_unless(!strcmp(output, " bug bu"), "wrong output");
rc = curl_msnprintf(output, 16, "%8d%8d", 1234, 5678);
fail_unless(rc == 16, "return code should be 16");
fail_unless(!strcmp(output, " 1234 567"), "wrong output");
UNITTEST_STOP
