#include "curl_setup.h"
#if defined(USE_LIBRTMP)
#include "curl_rtmp.h"
#include "urldata.h"
#include "nonblock.h" 
#include "progress.h" 
#include "transfer.h"
#include "warnless.h"
#include <curl/curl.h>
#include <librtmp/rtmp.h>
#include "curl_memory.h"
#include "memdebug.h"
#if defined(WIN32) && !defined(USE_LWIPSOCK)
#define setsockopt(a,b,c,d,e) (setsockopt)(a,b,c,(const char *)d,(int)e)
#define SET_RCVTIMEO(tv,s) int tv = s*1000
#elif defined(LWIP_SO_SNDRCVTIMEO_NONSTANDARD)
#define SET_RCVTIMEO(tv,s) int tv = s*1000
#else
#define SET_RCVTIMEO(tv,s) struct timeval tv = {s,0}
#endif
#define DEF_BUFTIME (2*60*60*1000) 
static CURLcode rtmp_setup_connection(struct connectdata *conn);
static CURLcode rtmp_do(struct connectdata *conn, bool *done);
static CURLcode rtmp_done(struct connectdata *conn, CURLcode, bool premature);
static CURLcode rtmp_connect(struct connectdata *conn, bool *done);
static CURLcode rtmp_disconnect(struct connectdata *conn, bool dead);
static Curl_recv rtmp_recv;
static Curl_send rtmp_send;
const struct Curl_handler Curl_handler_rtmp = {
"RTMP", 
rtmp_setup_connection, 
rtmp_do, 
rtmp_done, 
ZERO_NULL, 
rtmp_connect, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
rtmp_disconnect, 
ZERO_NULL, 
ZERO_NULL, 
PORT_RTMP, 
CURLPROTO_RTMP, 
PROTOPT_NONE 
};
const struct Curl_handler Curl_handler_rtmpt = {
"RTMPT", 
rtmp_setup_connection, 
rtmp_do, 
rtmp_done, 
ZERO_NULL, 
rtmp_connect, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
rtmp_disconnect, 
ZERO_NULL, 
ZERO_NULL, 
PORT_RTMPT, 
CURLPROTO_RTMPT, 
PROTOPT_NONE 
};
const struct Curl_handler Curl_handler_rtmpe = {
"RTMPE", 
rtmp_setup_connection, 
rtmp_do, 
rtmp_done, 
ZERO_NULL, 
rtmp_connect, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
rtmp_disconnect, 
ZERO_NULL, 
ZERO_NULL, 
PORT_RTMP, 
CURLPROTO_RTMPE, 
PROTOPT_NONE 
};
const struct Curl_handler Curl_handler_rtmpte = {
"RTMPTE", 
rtmp_setup_connection, 
rtmp_do, 
rtmp_done, 
ZERO_NULL, 
rtmp_connect, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
rtmp_disconnect, 
ZERO_NULL, 
ZERO_NULL, 
PORT_RTMPT, 
CURLPROTO_RTMPTE, 
PROTOPT_NONE 
};
const struct Curl_handler Curl_handler_rtmps = {
"RTMPS", 
rtmp_setup_connection, 
rtmp_do, 
rtmp_done, 
ZERO_NULL, 
rtmp_connect, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
rtmp_disconnect, 
ZERO_NULL, 
ZERO_NULL, 
PORT_RTMPS, 
CURLPROTO_RTMPS, 
PROTOPT_NONE 
};
const struct Curl_handler Curl_handler_rtmpts = {
"RTMPTS", 
rtmp_setup_connection, 
rtmp_do, 
rtmp_done, 
ZERO_NULL, 
rtmp_connect, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
ZERO_NULL, 
rtmp_disconnect, 
ZERO_NULL, 
ZERO_NULL, 
PORT_RTMPS, 
CURLPROTO_RTMPTS, 
PROTOPT_NONE 
};
static CURLcode rtmp_setup_connection(struct connectdata *conn)
{
RTMP *r = RTMP_Alloc();
if(!r)
return CURLE_OUT_OF_MEMORY;
RTMP_Init(r);
RTMP_SetBufferMS(r, DEF_BUFTIME);
if(!RTMP_SetupURL(r, conn->data->change.url)) {
RTMP_Free(r);
return CURLE_URL_MALFORMAT;
}
conn->proto.rtmp = r;
return CURLE_OK;
}
static CURLcode rtmp_connect(struct connectdata *conn, bool *done)
{
RTMP *r = conn->proto.rtmp;
SET_RCVTIMEO(tv, 10);
r->m_sb.sb_socket = (int)conn->sock[FIRSTSOCKET];
if(conn->data->set.upload)
r->Link.protocol |= RTMP_FEATURE_WRITE;
if(!(r->Link.lFlags & RTMP_LF_LIVE) &&
!(r->Link.protocol & RTMP_FEATURE_HTTP))
r->Link.lFlags |= RTMP_LF_BUFX;
(void)curlx_nonblock(r->m_sb.sb_socket, FALSE);
setsockopt(r->m_sb.sb_socket, SOL_SOCKET, SO_RCVTIMEO,
(char *)&tv, sizeof(tv));
if(!RTMP_Connect1(r, NULL))
return CURLE_FAILED_INIT;
r->m_bSendCounter = true;
*done = TRUE;
conn->recv[FIRSTSOCKET] = rtmp_recv;
conn->send[FIRSTSOCKET] = rtmp_send;
return CURLE_OK;
}
static CURLcode rtmp_do(struct connectdata *conn, bool *done)
{
struct Curl_easy *data = conn->data;
RTMP *r = conn->proto.rtmp;
if(!RTMP_ConnectStream(r, 0))
return CURLE_FAILED_INIT;
if(conn->data->set.upload) {
Curl_pgrsSetUploadSize(data, data->state.infilesize);
Curl_setup_transfer(data, -1, -1, FALSE, FIRSTSOCKET);
}
else
Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
*done = TRUE;
return CURLE_OK;
}
static CURLcode rtmp_done(struct connectdata *conn, CURLcode status,
bool premature)
{
(void)conn; 
(void)status; 
(void)premature; 
return CURLE_OK;
}
static CURLcode rtmp_disconnect(struct connectdata *conn,
bool dead_connection)
{
RTMP *r = conn->proto.rtmp;
(void)dead_connection;
if(r) {
conn->proto.rtmp = NULL;
RTMP_Close(r);
RTMP_Free(r);
}
return CURLE_OK;
}
static ssize_t rtmp_recv(struct connectdata *conn, int sockindex, char *buf,
size_t len, CURLcode *err)
{
RTMP *r = conn->proto.rtmp;
ssize_t nread;
(void)sockindex; 
nread = RTMP_Read(r, buf, curlx_uztosi(len));
if(nread < 0) {
if(r->m_read.status == RTMP_READ_COMPLETE ||
r->m_read.status == RTMP_READ_EOF) {
conn->data->req.size = conn->data->req.bytecount;
nread = 0;
}
else
*err = CURLE_RECV_ERROR;
}
return nread;
}
static ssize_t rtmp_send(struct connectdata *conn, int sockindex,
const void *buf, size_t len, CURLcode *err)
{
RTMP *r = conn->proto.rtmp;
ssize_t num;
(void)sockindex; 
num = RTMP_Write(r, (char *)buf, curlx_uztosi(len));
if(num < 0)
*err = CURLE_SEND_ERROR;
return num;
}
#endif 
