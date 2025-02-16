




















#include "curlcheck.h"

#include "urldata.h"
#include "connect.h"
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



#define BASE 1000000


#define NOW(x,y) now.tv_sec = x; now.tv_usec = y

#define TIMEOUTS(x,y) data->set.timeout = x; data->set.connecttimeout = y










struct timetest {
int now_s;
int now_us;
int timeout_ms;
int connecttimeout_ms;
bool connecting;
timediff_t result;
const char *comment;
};

UNITTEST_START
{
struct curltime now;
unsigned int i;

const struct timetest run[] = {

{BASE + 4, 0, 10000, 8000, FALSE, 6000, "6 seconds should be left"},
{BASE + 4, 990000, 10000, 8000, FALSE, 5010, "5010 ms should be left"},
{BASE + 10, 0, 10000, 8000, FALSE, -1, "timeout is -1, expired"},
{BASE + 12, 0, 10000, 8000, FALSE, -2000, "-2000, overdue 2 seconds"},


{BASE + 4, 0, 10000, 8000, TRUE, 4000, "4 seconds should be left"},
{BASE + 4, 990000, 10000, 8000, TRUE, 3010, "3010 ms should be left"},
{BASE + 8, 0, 10000, 8000, TRUE, -1, "timeout is -1, expired"},
{BASE + 10, 0, 10000, 8000, TRUE, -2000, "-2000, overdue 2 seconds"},


{BASE + 4, 0, 10000, 0, FALSE, 6000, "6 seconds should be left"},
{BASE + 4, 990000, 10000, 0, FALSE, 5010, "5010 ms should be left"},
{BASE + 10, 0, 10000, 0, FALSE, -1, "timeout is -1, expired"},
{BASE + 12, 0, 10000, 0, FALSE, -2000, "-2000, overdue 2 seconds"},


{BASE + 4, 0, 10000, 0, FALSE, 6000, "6 seconds should be left"},
{BASE + 4, 990000, 10000, 0, FALSE, 5010, "5010 ms should be left"},
{BASE + 10, 0, 10000, 0, FALSE, -1, "timeout is -1, expired"},
{BASE + 12, 0, 10000, 0, FALSE, -2000, "-2000, overdue 2 seconds"},


{BASE + 4, 0, 0, 10000, FALSE, 0, "no timeout active"},
{BASE + 4, 990000, 0, 10000, FALSE, 0, "no timeout active"},
{BASE + 10, 0, 0, 10000, FALSE, 0, "no timeout active"},
{BASE + 12, 0, 0, 10000, FALSE, 0, "no timeout active"},


{BASE + 4, 0, 0, 10000, TRUE, 6000, "6 seconds should be left"},
{BASE + 4, 990000, 0, 10000, TRUE, 5010, "5010 ms should be left"},
{BASE + 10, 0, 0, 10000, TRUE, -1, "timeout is -1, expired"},
{BASE + 12, 0, 0, 10000, TRUE, -2000, "-2000, overdue 2 seconds"},


{BASE + 4, 0, 0, 0, FALSE, 0, "no timeout active"},
{BASE + 4, 990000, 0, 0, FALSE, 0, "no timeout active"},
{BASE + 10, 0, 0, 0, FALSE, 0, "no timeout active"},
{BASE + 12, 0, 0, 0, FALSE, 0, "no timeout active"},


{BASE + 4, 0, 0, 0, TRUE, 296000, "no timeout active"},
{BASE + 4, 990000, 0, 0, TRUE, 295010, "no timeout active"},
{BASE + 10, 0, 0, 0, TRUE, 290000, "no timeout active"},
{BASE + 12, 0, 0, 0, TRUE, 288000, "no timeout active"},


{BASE + 4, 0, 10000, 12000, TRUE, 6000, "6 seconds should be left"},

};


data->progress.t_startsingle.tv_sec = BASE;
data->progress.t_startsingle.tv_usec = 0;
data->progress.t_startop.tv_sec = BASE;
data->progress.t_startop.tv_usec = 0;

for(i = 0; i < sizeof(run)/sizeof(run[0]); i++) {
timediff_t timeout;
NOW(run[i].now_s, run[i].now_us);
TIMEOUTS(run[i].timeout_ms, run[i].connecttimeout_ms);
timeout = Curl_timeleft(data, &now, run[i].connecting);
if(timeout != run[i].result)
fail(run[i].comment);
}
}
UNITTEST_STOP
