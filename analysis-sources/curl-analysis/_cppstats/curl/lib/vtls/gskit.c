#include "curl_setup.h"
#if defined(USE_GSKIT)
#include <gskssl.h>
#include <qsoasync.h>
#undef HAVE_SOCKETPAIR 
#include "socketpair.h"
#if !defined(GSK_SSL_EXTN_SERVERNAME_REQUEST)
#define GSK_SSL_EXTN_SERVERNAME_REQUEST 230
#endif
#if !defined(GSK_TLSV10_CIPHER_SPECS)
#define GSK_TLSV10_CIPHER_SPECS 236
#endif
#if !defined(GSK_TLSV11_CIPHER_SPECS)
#define GSK_TLSV11_CIPHER_SPECS 237
#endif
#if !defined(GSK_TLSV12_CIPHER_SPECS)
#define GSK_TLSV12_CIPHER_SPECS 238
#endif
#if !defined(GSK_PROTOCOL_TLSV11)
#define GSK_PROTOCOL_TLSV11 437
#endif
#if !defined(GSK_PROTOCOL_TLSV12)
#define GSK_PROTOCOL_TLSV12 438
#endif
#if !defined(GSK_FALSE)
#define GSK_FALSE 0
#endif
#if !defined(GSK_TRUE)
#define GSK_TRUE 1
#endif
#include <limits.h>
#include <curl/curl.h>
#include "urldata.h"
#include "sendf.h"
#include "gskit.h"
#include "vtls.h"
#include "connect.h" 
#include "select.h"
#include "strcase.h"
#include "x509asn1.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#define SOS_READ 0x01
#define SOS_WRITE 0x02
#define CURL_GSKPROTO_SSLV2 0
#define CURL_GSKPROTO_SSLV2_MASK (1 << CURL_GSKPROTO_SSLV2)
#define CURL_GSKPROTO_SSLV3 1
#define CURL_GSKPROTO_SSLV3_MASK (1 << CURL_GSKPROTO_SSLV3)
#define CURL_GSKPROTO_TLSV10 2
#define CURL_GSKPROTO_TLSV10_MASK (1 << CURL_GSKPROTO_TLSV10)
#define CURL_GSKPROTO_TLSV11 3
#define CURL_GSKPROTO_TLSV11_MASK (1 << CURL_GSKPROTO_TLSV11)
#define CURL_GSKPROTO_TLSV12 4
#define CURL_GSKPROTO_TLSV12_MASK (1 << CURL_GSKPROTO_TLSV12)
#define CURL_GSKPROTO_LAST 5
struct ssl_backend_data {
gsk_handle handle;
int iocport;
int localfd;
int remotefd;
};
#define BACKEND connssl->backend
typedef struct {
const char *name; 
const char *gsktoken; 
unsigned int versions; 
} gskit_cipher;
static const gskit_cipher ciphertable[] = {
{ "null-md5", "01",
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK |
CURL_GSKPROTO_TLSV11_MASK | CURL_GSKPROTO_TLSV12_MASK },
{ "null-sha", "02",
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK |
CURL_GSKPROTO_TLSV11_MASK | CURL_GSKPROTO_TLSV12_MASK },
{ "exp-rc4-md5", "03",
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK },
{ "rc4-md5", "04",
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK |
CURL_GSKPROTO_TLSV11_MASK | CURL_GSKPROTO_TLSV12_MASK },
{ "rc4-sha", "05",
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK |
CURL_GSKPROTO_TLSV11_MASK | CURL_GSKPROTO_TLSV12_MASK },
{ "exp-rc2-cbc-md5", "06",
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK },
{ "exp-des-cbc-sha", "09",
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK |
CURL_GSKPROTO_TLSV11_MASK },
{ "des-cbc3-sha", "0A",
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK |
CURL_GSKPROTO_TLSV11_MASK | CURL_GSKPROTO_TLSV12_MASK },
{ "aes128-sha", "2F",
CURL_GSKPROTO_TLSV10_MASK | CURL_GSKPROTO_TLSV11_MASK |
CURL_GSKPROTO_TLSV12_MASK },
{ "aes256-sha", "35",
CURL_GSKPROTO_TLSV10_MASK | CURL_GSKPROTO_TLSV11_MASK |
CURL_GSKPROTO_TLSV12_MASK },
{ "null-sha256", "3B", CURL_GSKPROTO_TLSV12_MASK },
{ "aes128-sha256", "3C", CURL_GSKPROTO_TLSV12_MASK },
{ "aes256-sha256", "3D", CURL_GSKPROTO_TLSV12_MASK },
{ "aes128-gcm-sha256",
"9C", CURL_GSKPROTO_TLSV12_MASK },
{ "aes256-gcm-sha384",
"9D", CURL_GSKPROTO_TLSV12_MASK },
{ "rc4-md5", "1", CURL_GSKPROTO_SSLV2_MASK },
{ "exp-rc4-md5", "2", CURL_GSKPROTO_SSLV2_MASK },
{ "rc2-md5", "3", CURL_GSKPROTO_SSLV2_MASK },
{ "exp-rc2-md5", "4", CURL_GSKPROTO_SSLV2_MASK },
{ "des-cbc-md5", "6", CURL_GSKPROTO_SSLV2_MASK },
{ "des-cbc3-md5", "7", CURL_GSKPROTO_SSLV2_MASK },
{ (const char *) NULL, (const char *) NULL, 0 }
};
static bool is_separator(char c)
{
switch(c) {
case ' ':
case '\t':
case ':':
case ',':
case ';':
return true;
}
return false;
}
static CURLcode gskit_status(struct Curl_easy *data, int rc,
const char *procname, CURLcode defcode)
{
switch(rc) {
case GSK_OK:
case GSK_OS400_ASYNCHRONOUS_SOC_INIT:
return CURLE_OK;
case GSK_KEYRING_OPEN_ERROR:
case GSK_OS400_ERROR_NO_ACCESS:
return CURLE_SSL_CACERT_BADFILE;
case GSK_INSUFFICIENT_STORAGE:
return CURLE_OUT_OF_MEMORY;
case GSK_ERROR_BAD_V2_CIPHER:
case GSK_ERROR_BAD_V3_CIPHER:
case GSK_ERROR_NO_CIPHERS:
return CURLE_SSL_CIPHER;
case GSK_OS400_ERROR_NOT_TRUSTED_ROOT:
case GSK_ERROR_CERT_VALIDATION:
return CURLE_PEER_FAILED_VERIFICATION;
case GSK_OS400_ERROR_TIMED_OUT:
return CURLE_OPERATION_TIMEDOUT;
case GSK_WOULD_BLOCK:
return CURLE_AGAIN;
case GSK_OS400_ERROR_NOT_REGISTERED:
break;
case GSK_ERROR_IO:
switch(errno) {
case ENOMEM:
return CURLE_OUT_OF_MEMORY;
default:
failf(data, "%s I/O error: %s", procname, strerror(errno));
break;
}
break;
default:
failf(data, "%s: %s", procname, gsk_strerror(rc));
break;
}
return defcode;
}
static CURLcode set_enum(struct Curl_easy *data, gsk_handle h,
GSK_ENUM_ID id, GSK_ENUM_VALUE value, bool unsupported_ok)
{
int rc = gsk_attribute_set_enum(h, id, value);
switch(rc) {
case GSK_OK:
return CURLE_OK;
case GSK_ERROR_IO:
failf(data, "gsk_attribute_set_enum() I/O error: %s", strerror(errno));
break;
case GSK_ATTRIBUTE_INVALID_ID:
if(unsupported_ok)
return CURLE_UNSUPPORTED_PROTOCOL;
default:
failf(data, "gsk_attribute_set_enum(): %s", gsk_strerror(rc));
break;
}
return CURLE_SSL_CONNECT_ERROR;
}
static CURLcode set_buffer(struct Curl_easy *data, gsk_handle h,
GSK_BUF_ID id, const char *buffer, bool unsupported_ok)
{
int rc = gsk_attribute_set_buffer(h, id, buffer, 0);
switch(rc) {
case GSK_OK:
return CURLE_OK;
case GSK_ERROR_IO:
failf(data, "gsk_attribute_set_buffer() I/O error: %s", strerror(errno));
break;
case GSK_ATTRIBUTE_INVALID_ID:
if(unsupported_ok)
return CURLE_UNSUPPORTED_PROTOCOL;
default:
failf(data, "gsk_attribute_set_buffer(): %s", gsk_strerror(rc));
break;
}
return CURLE_SSL_CONNECT_ERROR;
}
static CURLcode set_numeric(struct Curl_easy *data,
gsk_handle h, GSK_NUM_ID id, int value)
{
int rc = gsk_attribute_set_numeric_value(h, id, value);
switch(rc) {
case GSK_OK:
return CURLE_OK;
case GSK_ERROR_IO:
failf(data, "gsk_attribute_set_numeric_value() I/O error: %s",
strerror(errno));
break;
default:
failf(data, "gsk_attribute_set_numeric_value(): %s", gsk_strerror(rc));
break;
}
return CURLE_SSL_CONNECT_ERROR;
}
static CURLcode set_callback(struct Curl_easy *data,
gsk_handle h, GSK_CALLBACK_ID id, void *info)
{
int rc = gsk_attribute_set_callback(h, id, info);
switch(rc) {
case GSK_OK:
return CURLE_OK;
case GSK_ERROR_IO:
failf(data, "gsk_attribute_set_callback() I/O error: %s", strerror(errno));
break;
default:
failf(data, "gsk_attribute_set_callback(): %s", gsk_strerror(rc));
break;
}
return CURLE_SSL_CONNECT_ERROR;
}
static CURLcode set_ciphers(struct connectdata *conn,
gsk_handle h, unsigned int *protoflags)
{
struct Curl_easy *data = conn->data;
const char *cipherlist = SSL_CONN_CONFIG(cipher_list);
const char *clp;
const gskit_cipher *ctp;
int i;
int l;
bool unsupported;
CURLcode result;
struct {
char *buf;
char *ptr;
} ciphers[CURL_GSKPROTO_LAST];
if(!cipherlist)
return CURLE_OK;
while(is_separator(*cipherlist)) 
cipherlist++;
if(!*cipherlist)
return CURLE_OK;
l = strlen(cipherlist) + 1;
memset((char *) ciphers, 0, sizeof(ciphers));
for(i = 0; i < CURL_GSKPROTO_LAST; i++) {
ciphers[i].buf = malloc(l);
if(!ciphers[i].buf) {
while(i--)
free(ciphers[i].buf);
return CURLE_OUT_OF_MEMORY;
}
ciphers[i].ptr = ciphers[i].buf;
*ciphers[i].ptr = '\0';
}
unsupported = FALSE;
result = CURLE_OK;
for(;;) {
for(clp = cipherlist; *cipherlist && !is_separator(*cipherlist);)
cipherlist++;
l = cipherlist - clp;
if(!l)
break;
for(ctp = ciphertable; ctp->name; ctp++)
if(strncasecompare(ctp->name, clp, l) && !ctp->name[l])
break;
if(!ctp->name) {
failf(data, "Unknown cipher %.*s", l, clp);
result = CURLE_SSL_CIPHER;
}
else {
unsupported |= !(ctp->versions & (CURL_GSKPROTO_SSLV2_MASK |
CURL_GSKPROTO_SSLV3_MASK | CURL_GSKPROTO_TLSV10_MASK));
for(i = 0; i < CURL_GSKPROTO_LAST; i++) {
if(ctp->versions & (1 << i)) {
strcpy(ciphers[i].ptr, ctp->gsktoken);
ciphers[i].ptr += strlen(ctp->gsktoken);
}
}
}
while(is_separator(*cipherlist))
cipherlist++;
}
for(i = 0; i < CURL_GSKPROTO_LAST; i++) {
if(!(*protoflags & (1 << i)) || !ciphers[i].buf[0]) {
*protoflags &= ~(1 << i);
ciphers[i].buf[0] = '\0';
}
}
if(*protoflags & CURL_GSKPROTO_TLSV11_MASK) {
result = set_buffer(data, h, GSK_TLSV11_CIPHER_SPECS,
ciphers[CURL_GSKPROTO_TLSV11].buf, TRUE);
if(result == CURLE_UNSUPPORTED_PROTOCOL) {
result = CURLE_OK;
if(unsupported) {
failf(data, "TLSv1.1-only ciphers are not yet supported");
result = CURLE_SSL_CIPHER;
}
}
}
if(!result && (*protoflags & CURL_GSKPROTO_TLSV12_MASK)) {
result = set_buffer(data, h, GSK_TLSV12_CIPHER_SPECS,
ciphers[CURL_GSKPROTO_TLSV12].buf, TRUE);
if(result == CURLE_UNSUPPORTED_PROTOCOL) {
result = CURLE_OK;
if(unsupported) {
failf(data, "TLSv1.2-only ciphers are not yet supported");
result = CURLE_SSL_CIPHER;
}
}
}
if(!result && (*protoflags & CURL_GSKPROTO_TLSV10_MASK)) {
result = set_buffer(data, h, GSK_TLSV10_CIPHER_SPECS,
ciphers[CURL_GSKPROTO_TLSV10].buf, TRUE);
if(result == CURLE_UNSUPPORTED_PROTOCOL) {
result = CURLE_OK;
strcpy(ciphers[CURL_GSKPROTO_SSLV3].ptr,
ciphers[CURL_GSKPROTO_TLSV10].ptr);
}
}
if(!result && (*protoflags & CURL_GSKPROTO_SSLV3_MASK))
result = set_buffer(data, h, GSK_V3_CIPHER_SPECS,
ciphers[CURL_GSKPROTO_SSLV3].buf, FALSE);
if(!result && (*protoflags & CURL_GSKPROTO_SSLV2_MASK))
result = set_buffer(data, h, GSK_V2_CIPHER_SPECS,
ciphers[CURL_GSKPROTO_SSLV2].buf, FALSE);
for(i = 0; i < CURL_GSKPROTO_LAST; i++)
free(ciphers[i].buf);
return result;
}
static int Curl_gskit_init(void)
{
return 1;
}
static void Curl_gskit_cleanup(void)
{
}
static CURLcode init_environment(struct Curl_easy *data,
gsk_handle *envir, const char *appid,
const char *file, const char *label,
const char *password)
{
int rc;
CURLcode result;
gsk_handle h;
rc = gsk_environment_open(&h);
switch(rc) {
case GSK_OK:
break;
case GSK_INSUFFICIENT_STORAGE:
return CURLE_OUT_OF_MEMORY;
default:
failf(data, "gsk_environment_open(): %s", gsk_strerror(rc));
return CURLE_SSL_CONNECT_ERROR;
}
result = set_enum(data, h, GSK_SESSION_TYPE, GSK_CLIENT_SESSION, FALSE);
if(!result && appid)
result = set_buffer(data, h, GSK_OS400_APPLICATION_ID, appid, FALSE);
if(!result && file)
result = set_buffer(data, h, GSK_KEYRING_FILE, file, FALSE);
if(!result && label)
result = set_buffer(data, h, GSK_KEYRING_LABEL, label, FALSE);
if(!result && password)
result = set_buffer(data, h, GSK_KEYRING_PW, password, FALSE);
if(!result) {
result = gskit_status(data, gsk_environment_init(h),
"gsk_environment_init()", CURLE_SSL_CERTPROBLEM);
if(!result) {
*envir = h;
return result;
}
}
gsk_environment_close(&h);
return result;
}
static void cancel_async_handshake(struct connectdata *conn, int sockindex)
{
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
Qso_OverlappedIO_t cstat;
if(QsoCancelOperation(conn->sock[sockindex], 0) > 0)
QsoWaitForIOCompletion(BACKEND->iocport, &cstat, (struct timeval *) NULL);
}
static void close_async_handshake(struct ssl_connect_data *connssl)
{
QsoDestroyIOCompletionPort(BACKEND->iocport);
BACKEND->iocport = -1;
}
static int pipe_ssloverssl(struct connectdata *conn, int sockindex,
int directions)
{
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct ssl_connect_data *connproxyssl = &conn->proxy_ssl[sockindex];
fd_set fds_read;
fd_set fds_write;
int n;
int m;
int i;
int ret = 0;
char buf[CURL_MAX_WRITE_SIZE];
if(!connssl->use || !connproxyssl->use)
return 0; 
FD_ZERO(&fds_read);
FD_ZERO(&fds_write);
n = -1;
if(directions & SOS_READ) {
FD_SET(BACKEND->remotefd, &fds_write);
n = BACKEND->remotefd;
}
if(directions & SOS_WRITE) {
FD_SET(BACKEND->remotefd, &fds_read);
n = BACKEND->remotefd;
FD_SET(conn->sock[sockindex], &fds_write);
if(n < conn->sock[sockindex])
n = conn->sock[sockindex];
}
i = Curl_select(n + 1, &fds_read, &fds_write, NULL, 0);
if(i < 0)
return -1; 
if(FD_ISSET(BACKEND->remotefd, &fds_write)) {
n = 0;
i = gsk_secure_soc_read(connproxyssl->backend->handle,
buf, sizeof(buf), &n);
switch(i) {
case GSK_OK:
if(n) {
i = write(BACKEND->remotefd, buf, n);
if(i < 0)
return -1;
ret = 1;
}
break;
case GSK_OS400_ERROR_TIMED_OUT:
case GSK_WOULD_BLOCK:
break;
default:
return -1;
}
}
if(FD_ISSET(BACKEND->remotefd, &fds_read) &&
FD_ISSET(conn->sock[sockindex], &fds_write)) {
n = read(BACKEND->remotefd, buf, sizeof(buf));
if(n < 0)
return -1;
if(n) {
i = gsk_secure_soc_write(connproxyssl->backend->handle, buf, n, &m);
if(i != GSK_OK || n != m)
return -1;
ret = 1;
}
}
return ret; 
}
static void close_one(struct ssl_connect_data *connssl,
struct connectdata *conn, int sockindex)
{
if(BACKEND->handle) {
gskit_status(conn->data, gsk_secure_soc_close(&BACKEND->handle),
"gsk_secure_soc_close()", 0);
while(pipe_ssloverssl(conn, sockindex, SOS_WRITE) > 0)
;
BACKEND->handle = (gsk_handle) NULL;
if(BACKEND->localfd >= 0) {
close(BACKEND->localfd);
BACKEND->localfd = -1;
}
if(BACKEND->remotefd >= 0) {
close(BACKEND->remotefd);
BACKEND->remotefd = -1;
}
}
if(BACKEND->iocport >= 0)
close_async_handshake(connssl);
}
static ssize_t gskit_send(struct connectdata *conn, int sockindex,
const void *mem, size_t len, CURLcode *curlcode)
{
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct Curl_easy *data = conn->data;
CURLcode cc = CURLE_SEND_ERROR;
int written;
if(pipe_ssloverssl(conn, sockindex, SOS_WRITE) >= 0) {
cc = gskit_status(data,
gsk_secure_soc_write(BACKEND->handle,
(char *) mem, (int) len, &written),
"gsk_secure_soc_write()", CURLE_SEND_ERROR);
if(cc == CURLE_OK)
if(pipe_ssloverssl(conn, sockindex, SOS_WRITE) < 0)
cc = CURLE_SEND_ERROR;
}
if(cc != CURLE_OK) {
*curlcode = cc;
written = -1;
}
return (ssize_t) written; 
}
static ssize_t gskit_recv(struct connectdata *conn, int num, char *buf,
size_t buffersize, CURLcode *curlcode)
{
struct ssl_connect_data *connssl = &conn->ssl[num];
struct Curl_easy *data = conn->data;
int nread;
CURLcode cc = CURLE_RECV_ERROR;
if(pipe_ssloverssl(conn, num, SOS_READ) >= 0) {
int buffsize = buffersize > (size_t) INT_MAX? INT_MAX: (int) buffersize;
cc = gskit_status(data, gsk_secure_soc_read(BACKEND->handle,
buf, buffsize, &nread),
"gsk_secure_soc_read()", CURLE_RECV_ERROR);
}
switch(cc) {
case CURLE_OK:
break;
case CURLE_OPERATION_TIMEDOUT:
cc = CURLE_AGAIN;
default:
*curlcode = cc;
nread = -1;
break;
}
return (ssize_t) nread;
}
static CURLcode
set_ssl_version_min_max(unsigned int *protoflags, struct connectdata *conn)
{
struct Curl_easy *data = conn->data;
long ssl_version = SSL_CONN_CONFIG(version);
long ssl_version_max = SSL_CONN_CONFIG(version_max);
long i = ssl_version;
switch(ssl_version_max) {
case CURL_SSLVERSION_MAX_NONE:
case CURL_SSLVERSION_MAX_DEFAULT:
ssl_version_max = CURL_SSLVERSION_TLSv1_2;
break;
}
for(; i <= (ssl_version_max >> 16); ++i) {
switch(i) {
case CURL_SSLVERSION_TLSv1_0:
*protoflags |= CURL_GSKPROTO_TLSV10_MASK;
break;
case CURL_SSLVERSION_TLSv1_1:
*protoflags |= CURL_GSKPROTO_TLSV11_MASK;
break;
case CURL_SSLVERSION_TLSv1_2:
*protoflags |= CURL_GSKPROTO_TLSV11_MASK;
break;
case CURL_SSLVERSION_TLSv1_3:
failf(data, "GSKit: TLS 1.3 is not yet supported");
return CURLE_SSL_CONNECT_ERROR;
}
}
return CURLE_OK;
}
static CURLcode gskit_connect_step1(struct connectdata *conn, int sockindex)
{
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
gsk_handle envir;
CURLcode result;
int rc;
const char * const keyringfile = SSL_CONN_CONFIG(CAfile);
const char * const keyringpwd = SSL_SET_OPTION(key_passwd);
const char * const keyringlabel = SSL_SET_OPTION(cert);
const long int ssl_version = SSL_CONN_CONFIG(version);
const bool verifypeer = SSL_CONN_CONFIG(verifypeer);
const char * const hostname = SSL_IS_PROXY()? conn->http_proxy.host.name:
conn->host.name;
const char *sni;
unsigned int protoflags = 0;
Qso_OverlappedIO_t commarea;
int sockpair[2];
static const int sobufsize = CURL_MAX_WRITE_SIZE;
BACKEND->handle = (gsk_handle) NULL;
BACKEND->iocport = -1;
BACKEND->localfd = -1;
BACKEND->remotefd = -1;
envir = (gsk_handle) NULL;
if(keyringlabel && *keyringlabel && !keyringpwd &&
!strcmp(keyringfile, CURL_CA_BUNDLE)) {
init_environment(data, &envir, keyringlabel, (const char *) NULL,
(const char *) NULL, (const char *) NULL);
}
if(!envir) {
result = init_environment(data, &envir, (const char *) NULL,
keyringfile, keyringlabel, keyringpwd);
if(result)
return result;
}
result = gskit_status(data, gsk_secure_soc_open(envir, &BACKEND->handle),
"gsk_secure_soc_open()", CURLE_SSL_CONNECT_ERROR);
gsk_environment_close(&envir);
if(result)
return result;
if(conn->proxy_ssl[sockindex].use) {
if(Curl_socketpair(0, 0, 0, sockpair))
return CURLE_SSL_CONNECT_ERROR;
BACKEND->localfd = sockpair[0];
BACKEND->remotefd = sockpair[1];
setsockopt(BACKEND->localfd, SOL_SOCKET, SO_RCVBUF,
(void *) sobufsize, sizeof(sobufsize));
setsockopt(BACKEND->remotefd, SOL_SOCKET, SO_RCVBUF,
(void *) sobufsize, sizeof(sobufsize));
setsockopt(BACKEND->localfd, SOL_SOCKET, SO_SNDBUF,
(void *) sobufsize, sizeof(sobufsize));
setsockopt(BACKEND->remotefd, SOL_SOCKET, SO_SNDBUF,
(void *) sobufsize, sizeof(sobufsize));
curlx_nonblock(BACKEND->localfd, TRUE);
curlx_nonblock(BACKEND->remotefd, TRUE);
}
sni = hostname;
switch(ssl_version) {
case CURL_SSLVERSION_SSLv2:
protoflags = CURL_GSKPROTO_SSLV2_MASK;
sni = NULL;
break;
case CURL_SSLVERSION_SSLv3:
protoflags = CURL_GSKPROTO_SSLV3_MASK;
sni = NULL;
break;
case CURL_SSLVERSION_DEFAULT:
case CURL_SSLVERSION_TLSv1:
protoflags = CURL_GSKPROTO_TLSV10_MASK |
CURL_GSKPROTO_TLSV11_MASK | CURL_GSKPROTO_TLSV12_MASK;
break;
case CURL_SSLVERSION_TLSv1_0:
case CURL_SSLVERSION_TLSv1_1:
case CURL_SSLVERSION_TLSv1_2:
case CURL_SSLVERSION_TLSv1_3:
result = set_ssl_version_min_max(&protoflags, conn);
if(result != CURLE_OK)
return result;
break;
default:
failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
return CURLE_SSL_CONNECT_ERROR;
}
if(sni) {
result = set_buffer(data, BACKEND->handle,
GSK_SSL_EXTN_SERVERNAME_REQUEST, sni, TRUE);
if(result == CURLE_UNSUPPORTED_PROTOCOL)
result = CURLE_OK;
}
if(!result) {
long timeout = Curl_timeleft(data, NULL, TRUE);
if(timeout < 0)
result = CURLE_OPERATION_TIMEDOUT;
else
result = set_numeric(data, BACKEND->handle, GSK_HANDSHAKE_TIMEOUT,
(timeout + 999) / 1000);
}
if(!result)
result = set_numeric(data, BACKEND->handle, GSK_OS400_READ_TIMEOUT, 1);
if(!result)
result = set_numeric(data, BACKEND->handle, GSK_FD, BACKEND->localfd >= 0?
BACKEND->localfd: conn->sock[sockindex]);
if(!result)
result = set_ciphers(conn, BACKEND->handle, &protoflags);
if(!protoflags) {
failf(data, "No SSL protocol/cipher combination enabled");
result = CURLE_SSL_CIPHER;
}
if(!result)
result = set_enum(data, BACKEND->handle, GSK_PROTOCOL_SSLV2,
(protoflags & CURL_GSKPROTO_SSLV2_MASK)?
GSK_PROTOCOL_SSLV2_ON: GSK_PROTOCOL_SSLV2_OFF, FALSE);
if(!result)
result = set_enum(data, BACKEND->handle, GSK_PROTOCOL_SSLV3,
(protoflags & CURL_GSKPROTO_SSLV3_MASK)?
GSK_PROTOCOL_SSLV3_ON: GSK_PROTOCOL_SSLV3_OFF, FALSE);
if(!result)
result = set_enum(data, BACKEND->handle, GSK_PROTOCOL_TLSV1,
(protoflags & CURL_GSKPROTO_TLSV10_MASK)?
GSK_PROTOCOL_TLSV1_ON: GSK_PROTOCOL_TLSV1_OFF, FALSE);
if(!result) {
result = set_enum(data, BACKEND->handle, GSK_PROTOCOL_TLSV11,
(protoflags & CURL_GSKPROTO_TLSV11_MASK)?
GSK_TRUE: GSK_FALSE, TRUE);
if(result == CURLE_UNSUPPORTED_PROTOCOL) {
result = CURLE_OK;
if(protoflags == CURL_GSKPROTO_TLSV11_MASK) {
failf(data, "TLS 1.1 not yet supported");
result = CURLE_SSL_CIPHER;
}
}
}
if(!result) {
result = set_enum(data, BACKEND->handle, GSK_PROTOCOL_TLSV12,
(protoflags & CURL_GSKPROTO_TLSV12_MASK)?
GSK_TRUE: GSK_FALSE, TRUE);
if(result == CURLE_UNSUPPORTED_PROTOCOL) {
result = CURLE_OK;
if(protoflags == CURL_GSKPROTO_TLSV12_MASK) {
failf(data, "TLS 1.2 not yet supported");
result = CURLE_SSL_CIPHER;
}
}
}
if(!result)
result = set_enum(data, BACKEND->handle, GSK_SERVER_AUTH_TYPE,
verifypeer? GSK_SERVER_AUTH_FULL:
GSK_SERVER_AUTH_PASSTHRU, FALSE);
if(!result) {
memset(&commarea, 0, sizeof(commarea));
BACKEND->iocport = QsoCreateIOCompletionPort();
if(BACKEND->iocport != -1) {
result = gskit_status(data,
gsk_secure_soc_startInit(BACKEND->handle,
BACKEND->iocport,
&commarea),
"gsk_secure_soc_startInit()",
CURLE_SSL_CONNECT_ERROR);
if(!result) {
connssl->connecting_state = ssl_connect_2;
return CURLE_OK;
}
else
close_async_handshake(connssl);
}
else if(errno != ENOBUFS)
result = gskit_status(data, GSK_ERROR_IO,
"QsoCreateIOCompletionPort()", 0);
else if(conn->proxy_ssl[sockindex].use) {
result = CURLE_SSL_CONNECT_ERROR;
}
else {
result = gskit_status(data, gsk_secure_soc_init(BACKEND->handle),
"gsk_secure_soc_init()", CURLE_SSL_CONNECT_ERROR);
if(!result) {
connssl->connecting_state = ssl_connect_3;
return CURLE_OK;
}
}
}
close_one(connssl, conn, sockindex);
return result;
}
static CURLcode gskit_connect_step2(struct connectdata *conn, int sockindex,
bool nonblocking)
{
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
Qso_OverlappedIO_t cstat;
struct timeval stmv;
CURLcode result;
for(;;) {
long timeout_ms = nonblocking? 0: Curl_timeleft(data, NULL, TRUE);
if(timeout_ms < 0)
timeout_ms = 0;
stmv.tv_sec = timeout_ms / 1000;
stmv.tv_usec = (timeout_ms - stmv.tv_sec * 1000) * 1000;
switch(QsoWaitForIOCompletion(BACKEND->iocport, &cstat, &stmv)) {
case 1: 
break;
case -1: 
if(errno == EINTR) {
if(nonblocking)
return CURLE_OK;
continue; 
}
if(errno != ETIME) {
failf(data, "QsoWaitForIOCompletion() I/O error: %s", strerror(errno));
cancel_async_handshake(conn, sockindex);
close_async_handshake(connssl);
return CURLE_SSL_CONNECT_ERROR;
}
case 0: 
if(nonblocking)
return CURLE_OK;
cancel_async_handshake(conn, sockindex);
close_async_handshake(connssl);
return CURLE_OPERATION_TIMEDOUT;
}
break;
}
result = gskit_status(data, cstat.returnValue, "SSL handshake",
CURLE_SSL_CONNECT_ERROR);
if(!result)
connssl->connecting_state = ssl_connect_3;
close_async_handshake(connssl);
return result;
}
static CURLcode gskit_connect_step3(struct connectdata *conn, int sockindex)
{
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
const gsk_cert_data_elem *cdev;
int cdec;
const gsk_cert_data_elem *p;
const char *cert = (const char *) NULL;
const char *certend;
const char *ptr;
CURLcode result;
if(gskit_status(data, gsk_attribute_get_cert_info(BACKEND->handle,
GSK_PARTNER_CERT_INFO,
&cdev, &cdec),
"gsk_attribute_get_cert_info()", CURLE_SSL_CONNECT_ERROR) ==
CURLE_OK) {
int i;
infof(data, "Server certificate:\n");
p = cdev;
for(i = 0; i++ < cdec; p++)
switch(p->cert_data_id) {
case CERT_BODY_DER:
cert = p->cert_data_p;
certend = cert + cdev->cert_data_l;
break;
case CERT_DN_PRINTABLE:
infof(data, "\t subject: %.*s\n", p->cert_data_l, p->cert_data_p);
break;
case CERT_ISSUER_DN_PRINTABLE:
infof(data, "\t issuer: %.*s\n", p->cert_data_l, p->cert_data_p);
break;
case CERT_VALID_FROM:
infof(data, "\t start date: %.*s\n", p->cert_data_l, p->cert_data_p);
break;
case CERT_VALID_TO:
infof(data, "\t expire date: %.*s\n", p->cert_data_l, p->cert_data_p);
break;
}
}
result = Curl_verifyhost(conn, cert, certend);
if(result)
return result;
if(data->set.ssl.certinfo) {
result = Curl_ssl_init_certinfo(data, 1);
if(result)
return result;
if(cert) {
result = Curl_extract_certinfo(conn, 0, cert, certend);
if(result)
return result;
}
}
ptr = SSL_IS_PROXY() ? data->set.str[STRING_SSL_PINNEDPUBLICKEY_PROXY] :
data->set.str[STRING_SSL_PINNEDPUBLICKEY_ORIG];
if(!result && ptr) {
curl_X509certificate x509;
curl_asn1Element *p;
if(Curl_parseX509(&x509, cert, certend))
return CURLE_SSL_PINNEDPUBKEYNOTMATCH;
p = &x509.subjectPublicKeyInfo;
result = Curl_pin_peer_pubkey(data, ptr, p->header, p->end - p->header);
if(result) {
failf(data, "SSL: public key does not match pinned public key!");
return result;
}
}
connssl->connecting_state = ssl_connect_done;
return CURLE_OK;
}
static CURLcode gskit_connect_common(struct connectdata *conn, int sockindex,
bool nonblocking, bool *done)
{
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
timediff_t timeout_ms;
CURLcode result = CURLE_OK;
*done = connssl->state == ssl_connection_complete;
if(*done)
return CURLE_OK;
if(connssl->connecting_state == ssl_connect_1) {
timeout_ms = Curl_timeleft(data, NULL, TRUE);
if(timeout_ms < 0) {
failf(data, "SSL connection timeout");
result = CURLE_OPERATION_TIMEDOUT;
}
else
result = gskit_connect_step1(conn, sockindex);
}
if(!result)
if(pipe_ssloverssl(conn, sockindex, SOS_READ | SOS_WRITE) < 0)
result = CURLE_SSL_CONNECT_ERROR;
if(!result && connssl->connecting_state == ssl_connect_2) {
timeout_ms = Curl_timeleft(data, NULL, TRUE);
if(timeout_ms < 0) {
failf(data, "SSL connection timeout");
result = CURLE_OPERATION_TIMEDOUT;
}
else
result = gskit_connect_step2(conn, sockindex, nonblocking);
}
if(!result)
if(pipe_ssloverssl(conn, sockindex, SOS_READ | SOS_WRITE) < 0)
result = CURLE_SSL_CONNECT_ERROR;
if(!result && connssl->connecting_state == ssl_connect_3)
result = gskit_connect_step3(conn, sockindex);
if(result)
close_one(connssl, conn, sockindex);
else if(connssl->connecting_state == ssl_connect_done) {
connssl->state = ssl_connection_complete;
connssl->connecting_state = ssl_connect_1;
conn->recv[sockindex] = gskit_recv;
conn->send[sockindex] = gskit_send;
*done = TRUE;
}
return result;
}
static CURLcode Curl_gskit_connect_nonblocking(struct connectdata *conn,
int sockindex, bool *done)
{
CURLcode result;
result = gskit_connect_common(conn, sockindex, TRUE, done);
if(*done || result)
conn->ssl[sockindex].connecting_state = ssl_connect_1;
return result;
}
static CURLcode Curl_gskit_connect(struct connectdata *conn, int sockindex)
{
CURLcode result;
bool done;
conn->ssl[sockindex].connecting_state = ssl_connect_1;
result = gskit_connect_common(conn, sockindex, FALSE, &done);
if(result)
return result;
DEBUGASSERT(done);
return CURLE_OK;
}
static void Curl_gskit_close(struct connectdata *conn, int sockindex)
{
close_one(&conn->ssl[sockindex], conn, sockindex);
close_one(&conn->proxy_ssl[sockindex], conn, sockindex);
}
static int Curl_gskit_shutdown(struct connectdata *conn, int sockindex)
{
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct Curl_easy *data = conn->data;
int what;
int rc;
char buf[120];
if(!BACKEND->handle)
return 0;
#if !defined(CURL_DISABLE_FTP)
if(data->set.ftp_ccc != CURLFTPSSL_CCC_ACTIVE)
return 0;
#endif
close_one(connssl, conn, sockindex);
rc = 0;
what = SOCKET_READABLE(conn->sock[sockindex],
SSL_SHUTDOWN_TIMEOUT);
for(;;) {
ssize_t nread;
if(what < 0) {
failf(data, "select/poll on SSL socket, errno: %d", SOCKERRNO);
rc = -1;
break;
}
if(!what) { 
failf(data, "SSL shutdown timeout");
break;
}
nread = read(conn->sock[sockindex], buf, sizeof(buf));
if(nread < 0) {
failf(data, "read: %s", strerror(errno));
rc = -1;
}
if(nread <= 0)
break;
what = SOCKET_READABLE(conn->sock[sockindex], 0);
}
return rc;
}
static size_t Curl_gskit_version(char *buffer, size_t size)
{
return msnprintf(buffer, size, "GSKit");
}
static int Curl_gskit_check_cxn(struct connectdata *cxn)
{
struct ssl_connect_data *connssl = &cxn->ssl[FIRSTSOCKET];
int err;
int errlen;
if(!BACKEND->handle)
return 0; 
err = 0;
errlen = sizeof(err);
if(getsockopt(cxn->sock[FIRSTSOCKET], SOL_SOCKET, SO_ERROR,
(unsigned char *) &err, &errlen) ||
errlen != sizeof(err) || err)
return 0; 
return -1; 
}
static void *Curl_gskit_get_internals(struct ssl_connect_data *connssl,
CURLINFO info UNUSED_PARAM)
{
(void)info;
return BACKEND->handle;
}
const struct Curl_ssl Curl_ssl_gskit = {
{ CURLSSLBACKEND_GSKIT, "gskit" }, 
SSLSUPP_CERTINFO |
SSLSUPP_PINNEDPUBKEY,
sizeof(struct ssl_backend_data),
Curl_gskit_init, 
Curl_gskit_cleanup, 
Curl_gskit_version, 
Curl_gskit_check_cxn, 
Curl_gskit_shutdown, 
Curl_none_data_pending, 
Curl_none_random, 
Curl_none_cert_status_request, 
Curl_gskit_connect, 
Curl_gskit_connect_nonblocking, 
Curl_gskit_get_internals, 
Curl_gskit_close, 
Curl_none_close_all, 
Curl_none_session_free, 
Curl_none_set_engine, 
Curl_none_set_engine_default, 
Curl_none_engines_list, 
Curl_none_false_start, 
Curl_none_md5sum, 
NULL 
};
#endif 
