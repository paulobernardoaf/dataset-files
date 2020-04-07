




















#include "curlcheck.h"

#include "urldata.h"
#include "connect.h"
#include "share.h"

#include "memdebug.h" 

static void unit_stop(void)
{
curl_global_cleanup();
}

static CURLcode unit_setup(void)
{
int res = CURLE_OK;

global_init(CURL_GLOBAL_ALL);

return res;
}

struct testcase {

const char *optval;


const char *host;
int port;


const char *address[10];
};




static const char skip = 0;
#if defined(ENABLE_IPV6)
#define IPV6ONLY(x) x
#else
#define IPV6ONLY(x) &skip
#endif


static const struct testcase tests[] = {

{ "test.com:80:127.0.0.1, 127.0.0.2",
"test.com", 80, { NULL, }
},
{ "TEST.com:80:,,127.0.0.1,,,127.0.0.2,,,,::1,,,",
"test.com", 80, { "127.0.0.1", "127.0.0.2", IPV6ONLY("::1"), }
},
{ "test.com:80:::1,127.0.0.1",
"test.com", 80, { IPV6ONLY("::1"), "127.0.0.1", }
},
{ "test.com:80:[::1],127.0.0.1",
"test.com", 80, { IPV6ONLY("::1"), "127.0.0.1", }
},
{ "test.com:80:::1",
"test.com", 80, { IPV6ONLY("::1"), }
},
{ "test.com:80:[::1]",
"test.com", 80, { IPV6ONLY("::1"), }
},
{ "test.com:80:127.0.0.1",
"test.com", 80, { "127.0.0.1", }
},
{ "test.com:80:,127.0.0.1",
"test.com", 80, { "127.0.0.1", }
},
{ "test.com:80:127.0.0.1,",
"test.com", 80, { "127.0.0.1", }
},
{ "test.com:0:127.0.0.1",
"test.com", 0, { "127.0.0.1", }
},
};

UNITTEST_START
{
int i;
int testnum = sizeof(tests) / sizeof(struct testcase);
struct Curl_multi *multi = NULL;
struct Curl_easy *easy = NULL;
struct curl_slist *list = NULL;

for(i = 0; i < testnum; ++i) {
int j;
int addressnum = sizeof(tests[i].address) / sizeof(*tests[i].address);
struct Curl_addrinfo *addr;
struct Curl_dns_entry *dns;
void *entry_id;
bool problem = false;
easy = curl_easy_init();
if(!easy)
goto error;



multi = curl_multi_init();
curl_multi_add_handle(multi, easy);

list = curl_slist_append(NULL, tests[i].optval);
if(!list)
goto error;
curl_easy_setopt(easy, CURLOPT_RESOLVE, list);

Curl_loadhostpairs(easy);

entry_id = (void *)aprintf("%s:%d", tests[i].host, tests[i].port);
if(!entry_id)
goto error;
dns = Curl_hash_pick(easy->dns.hostcache, entry_id, strlen(entry_id) + 1);
free(entry_id);
entry_id = NULL;

addr = dns ? dns->addr : NULL;

for(j = 0; j < addressnum; ++j) {
long port = 0;
char ipaddress[MAX_IPADR_LEN] = {0};

if(!addr && !tests[i].address[j])
break;

if(tests[i].address[j] == &skip)
continue;

if(addr && !Curl_addr2string(addr->ai_addr, addr->ai_addrlen,
ipaddress, &port)) {
fprintf(stderr, "%s:%d tests[%d] failed. getaddressinfo failed.\n",
__FILE__, __LINE__, i);
problem = true;
break;
}

if(addr && !tests[i].address[j]) {
fprintf(stderr, "%s:%d tests[%d] failed. the retrieved addr "
"is %s but tests[%d].address[%d] is NULL.\n",
__FILE__, __LINE__, i, ipaddress, i, j);
problem = true;
break;
}

if(!addr && tests[i].address[j]) {
fprintf(stderr, "%s:%d tests[%d] failed. the retrieved addr "
"is NULL but tests[%d].address[%d] is %s.\n",
__FILE__, __LINE__, i, i, j, tests[i].address[j]);
problem = true;
break;
}

if(!curl_strequal(ipaddress, tests[i].address[j])) {
fprintf(stderr, "%s:%d tests[%d] failed. the retrieved addr "
"%s is not equal to tests[%d].address[%d] %s.\n",
__FILE__, __LINE__, i, ipaddress, i, j, tests[i].address[j]);
problem = true;
break;
}

if(port != tests[i].port) {
fprintf(stderr, "%s:%d tests[%d] failed. the retrieved port "
"for tests[%d].address[%d] is %ld but tests[%d].port is %d.\n",
__FILE__, __LINE__, i, i, j, port, i, tests[i].port);
problem = true;
break;
}

if(dns->timestamp != 0) {
fprintf(stderr, "%s:%d tests[%d] failed. the timestamp is not zero. "
"for tests[%d].address[%d\n",
__FILE__, __LINE__, i, i, j);
problem = true;
break;
}

addr = addr->ai_next;
}

curl_easy_cleanup(easy);
easy = NULL;
curl_multi_cleanup(multi);
multi = NULL;
curl_slist_free_all(list);
list = NULL;

if(problem) {
unitfail++;
continue;
}
}
error:
curl_easy_cleanup(easy);
curl_multi_cleanup(multi);
curl_slist_free_all(list);
}
UNITTEST_STOP
