#include "curlcheck.h"
#include "timeval.h"
static CURLcode unit_setup(void)
{
return CURLE_OK;
}
static void unit_stop(void)
{
}
struct a {
struct curltime first;
struct curltime second;
time_t result;
};
UNITTEST_START
{
struct a tests[] = {
{ {36762, 8345 }, {36761, 995926 }, 13 },
{ {36761, 995926 }, {36762, 8345 }, -13 },
{ {36761, 995926 }, {0, 0}, 36761995 },
{ {0, 0}, {36761, 995926 }, -36761995 },
};
size_t i;
for(i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
timediff_t result = Curl_timediff(tests[i].first, tests[i].second);
if(result != tests[i].result) {
printf("%d.%06u to %d.%06u got %d, but expected %d\n",
tests[i].first.tv_sec,
tests[i].first.tv_usec,
tests[i].second.tv_sec,
tests[i].second.tv_usec,
result,
tests[i].result);
fail("unexpected result!");
}
}
}
UNITTEST_STOP
