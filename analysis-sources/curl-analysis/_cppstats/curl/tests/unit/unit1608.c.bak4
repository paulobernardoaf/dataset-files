




















#include "curlcheck.h"

#include "hostip.h"

CURLcode Curl_shuffle_addr(struct Curl_easy *data,
Curl_addrinfo **addr);

#define NUM_ADDRS 8
static struct Curl_addrinfo addrs[NUM_ADDRS];

static CURLcode unit_setup(void)
{
int i;
for(i = 0; i < NUM_ADDRS - 1; i++) {
addrs[i].ai_next = &addrs[i + 1];
}

return CURLE_OK;
}

static void unit_stop(void)
{

}

UNITTEST_START
{
int i;
CURLcode code;
struct Curl_addrinfo* addrhead = addrs;

struct Curl_easy *easy = curl_easy_init();
abort_unless(easy, "out of memory");

code = curl_easy_setopt(easy, CURLOPT_DNS_SHUFFLE_ADDRESSES, 1L);
abort_unless(code == CURLE_OK, "curl_easy_setopt failed");


for(i = 0; i < 10; i++) {
if(CURLE_OK != Curl_shuffle_addr(easy, &addrhead))
break;
if(addrhead != addrs)
break;
}

curl_easy_cleanup(easy);
curl_global_cleanup();

abort_unless(addrhead != addrs, "addresses are not being reordered");

return 0;
}
UNITTEST_STOP
