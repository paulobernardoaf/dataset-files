#include "curlcheck.h"
#include "hostcheck.h" 
static CURLcode unit_setup(void)
{
return CURLE_OK;
}
static void unit_stop(void)
{
}
UNITTEST_START
#if defined(USE_OPENSSL) || defined(USE_GSKIT)
fail_unless(Curl_cert_hostcheck("www.example.com", "www.example.com"),
"good 1");
fail_unless(Curl_cert_hostcheck("*.example.com", "www.example.com"),
"good 2");
fail_unless(Curl_cert_hostcheck("xxx*.example.com", "xxxwww.example.com"),
"good 3");
fail_unless(Curl_cert_hostcheck("f*.example.com", "foo.example.com"),
"good 4");
fail_unless(Curl_cert_hostcheck("192.168.0.0", "192.168.0.0"),
"good 5");
fail_if(Curl_cert_hostcheck("xxx.example.com", "www.example.com"), "bad 1");
fail_if(Curl_cert_hostcheck("*", "www.example.com"), "bad 2");
fail_if(Curl_cert_hostcheck("*.*.com", "www.example.com"), "bad 3");
fail_if(Curl_cert_hostcheck("*.example.com", "baa.foo.example.com"), "bad 4");
fail_if(Curl_cert_hostcheck("f*.example.com", "baa.example.com"), "bad 5");
fail_if(Curl_cert_hostcheck("*.com", "example.com"), "bad 6");
fail_if(Curl_cert_hostcheck("*fail.com", "example.com"), "bad 7");
fail_if(Curl_cert_hostcheck("*.example.", "www.example."), "bad 8");
fail_if(Curl_cert_hostcheck("*.example.", "www.example"), "bad 9");
fail_if(Curl_cert_hostcheck("", "www"), "bad 10");
fail_if(Curl_cert_hostcheck("*", "www"), "bad 11");
fail_if(Curl_cert_hostcheck("*.168.0.0", "192.168.0.0"), "bad 12");
fail_if(Curl_cert_hostcheck("www.example.com", "192.168.0.0"), "bad 13");
#if defined(ENABLE_IPV6)
fail_if(Curl_cert_hostcheck("*::3285:a9ff:fe46:b619",
"fe80::3285:a9ff:fe46:b619"), "bad 14");
fail_unless(Curl_cert_hostcheck("fe80::3285:a9ff:fe46:b619",
"fe80::3285:a9ff:fe46:b619"), "good 6");
#endif
#endif
UNITTEST_STOP
