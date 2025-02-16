#include "test.h"
#include "testutil.h"
#include "warnless.h"
#include "memdebug.h"
#define USERNAME "user@example.com"
#define PASSWORD "123qwerty"
#define RECIPIENT "<1507-recipient@example.com>"
#define MAILFROM "<1507-realuser@example.com>"
#define MULTI_PERFORM_HANG_TIMEOUT 60 * 1000
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
(void)ptr;
(void)size;
(void)nmemb;
(void)userp;
return CURL_READFUNC_ABORT;
}
int test(char *URL)
{
int res = 0;
CURL *curl = NULL;
CURLM *mcurl = NULL;
int still_running = 1;
struct timeval mp_start;
struct curl_slist *rcpt_list = NULL;
curl_global_init(CURL_GLOBAL_DEFAULT);
easy_init(curl);
multi_init(mcurl);
rcpt_list = curl_slist_append(rcpt_list, RECIPIENT);
curl_easy_setopt(curl, CURLOPT_URL, URL);
#if 0
curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);
#endif
curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
curl_easy_setopt(curl, CURLOPT_MAIL_FROM, MAILFROM);
curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, rcpt_list);
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
multi_add_handle(mcurl, curl);
mp_start = tutil_tvnow();
curl_multi_perform(mcurl, &still_running);
while(still_running) {
struct timeval timeout;
int rc; 
fd_set fdread;
fd_set fdwrite;
fd_set fdexcep;
int maxfd = -1;
long curl_timeo = -1;
FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);
timeout.tv_sec = 1;
timeout.tv_usec = 0;
curl_multi_timeout(mcurl, &curl_timeo);
if(curl_timeo >= 0) {
timeout.tv_sec = curl_timeo / 1000;
if(timeout.tv_sec > 1)
timeout.tv_sec = 1;
else
timeout.tv_usec = (curl_timeo % 1000) * 1000;
}
curl_multi_fdset(mcurl, &fdread, &fdwrite, &fdexcep, &maxfd);
rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
if(tutil_tvdiff(tutil_tvnow(), mp_start) > MULTI_PERFORM_HANG_TIMEOUT) {
fprintf(stderr, "ABORTING TEST, since it seems "
"that it would have run forever.\n");
break;
}
switch(rc) {
case -1:
break;
case 0: 
default: 
curl_multi_perform(mcurl, &still_running);
break;
}
}
test_cleanup:
curl_slist_free_all(rcpt_list);
curl_multi_remove_handle(mcurl, curl);
curl_multi_cleanup(mcurl);
curl_easy_cleanup(curl);
curl_global_cleanup();
return res;
}
