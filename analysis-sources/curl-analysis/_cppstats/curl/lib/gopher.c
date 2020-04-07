#include "curl_setup.h"
#if !defined(CURL_DISABLE_GOPHER)
#include "urldata.h"
#include <curl/curl.h>
#include "transfer.h"
#include "sendf.h"
#include "progress.h"
#include "gopher.h"
#include "select.h"
#include "strdup.h"
#include "url.h"
#include "escape.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
static CURLcode gopher_do(struct connectdata *conn, bool *done);
const struct Curl_handler Curl_handler_gopher = {
"GOPHER", 
ZERO_NULL, 
gopher_do, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
PORT_GOPHER, 
CURLPROTO_GOPHER, 
PROTOPT_NONE 
};
static CURLcode gopher_do(struct connectdata *conn, bool *done)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
char *gopherpath;
char *path = data->state.up.path;
char *query = data->state.up.query;
char *sel = NULL;
char *sel_org = NULL;
ssize_t amount, k;
size_t len;
*done = TRUE; 
DEBUGASSERT(path);
if(query)
gopherpath = aprintf("%s?%s", path, query);
else
gopherpath = strdup(path);
if(!gopherpath)
return CURLE_OUT_OF_MEMORY;
if(strlen(gopherpath) <= 2) {
sel = (char *)"";
len = strlen(sel);
free(gopherpath);
}
else {
char *newp;
newp = gopherpath;
newp += 2;
result = Curl_urldecode(data, newp, 0, &sel, &len, FALSE);
free(gopherpath);
if(result)
return result;
sel_org = sel;
}
k = curlx_uztosz(len);
for(;;) {
result = Curl_write(conn, sockfd, sel, k, &amount);
if(!result) { 
result = Curl_client_write(conn, CLIENTWRITE_HEADER, sel, amount);
if(result)
break;
k -= amount;
sel += amount;
if(k < 1)
break; 
}
else
break;
if(SOCKET_WRITABLE(sockfd, 100) < 0) {
result = CURLE_SEND_ERROR;
break;
}
}
free(sel_org);
if(!result)
result = Curl_sendf(sockfd, conn, "\r\n");
if(result) {
failf(data, "Failed sending Gopher request");
return result;
}
result = Curl_client_write(conn, CLIENTWRITE_HEADER, (char *)"\r\n", 2);
if(result)
return result;
Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
return CURLE_OK;
}
#endif 
