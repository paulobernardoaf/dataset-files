#if defined(USE_HASHSWF)
#include "rtmp_sys.h"
#include "log.h"
#include "http.h"
#if defined(CRYPTO)
#if defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(USE_MBEDTLS)
#include <mbedtls/md.h>
#if !defined(SHA256_DIGEST_LENGTH)
#define SHA256_DIGEST_LENGTH 32
#endif
typedef mbedtls_md_context_t *HMAC_CTX;
#define HMAC_setup(ctx, key, len) ctx = malloc(sizeof(mbedtls_md_context_t)); mbedtls_md_init(ctx); mbedtls_md_setup(ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1); mbedtls_md_hmac_starts(ctx, (const unsigned char *)key, len)
#define HMAC_crunch(ctx, buf, len) mbedtls_md_hmac_update(ctx, buf, len)
#define HMAC_finish(ctx, dig) mbedtls_md_hmac_finish(ctx, dig)
#define HMAC_close(ctx) free(ctx); mbedtls_md_free(ctx); ctx = NULL
#elif defined(USE_POLARSSL)
#include <polarssl/sha2.h>
#if !defined(SHA256_DIGEST_LENGTH)
#define SHA256_DIGEST_LENGTH 32
#endif
#define HMAC_CTX sha2_context
#define HMAC_setup(ctx, key, len) sha2_hmac_starts(&ctx, (unsigned char *)key, len, 0)
#define HMAC_crunch(ctx, buf, len) sha2_hmac_update(&ctx, buf, len)
#define HMAC_finish(ctx, dig) sha2_hmac_finish(&ctx, dig)
#define HMAC_close(ctx)
#elif defined(USE_GNUTLS)
#include <nettle/hmac.h>
#if !defined(SHA256_DIGEST_LENGTH)
#define SHA256_DIGEST_LENGTH 32
#endif
#undef HMAC_CTX
#define HMAC_CTX struct hmac_sha256_ctx
#define HMAC_setup(ctx, key, len) hmac_sha256_set_key(&ctx, len, key)
#define HMAC_crunch(ctx, buf, len) hmac_sha256_update(&ctx, len, buf)
#define HMAC_finish(ctx, dig) hmac_sha256_digest(&ctx, SHA256_DIGEST_LENGTH, dig)
#define HMAC_close(ctx)
#else 
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/rc4.h>
#define HMAC_setup(ctx, key, len) HMAC_CTX_init(&ctx); HMAC_Init_ex(&ctx, (unsigned char *)key, len, EVP_sha256(), 0)
#define HMAC_crunch(ctx, buf, len) HMAC_Update(&ctx, (unsigned char *)buf, len)
#define HMAC_finish(ctx, dig, len) HMAC_Final(&ctx, (unsigned char *)dig, &len);
#define HMAC_close(ctx) HMAC_CTX_cleanup(&ctx)
#endif
#include <zlib.h>
#endif 
#define AGENT "Mozilla/5.0"
HTTPResult
HTTP_get(struct HTTP_ctx *http, const char *url, HTTP_read_callback *cb)
{
char *host, *path;
char *p1, *p2;
char hbuf[256];
int port = 80;
#if defined(CRYPTO)
int ssl = 0;
#endif
int hlen, flen = 0;
int rc, i;
int len_known;
HTTPResult ret = HTTPRES_OK;
struct sockaddr_in sa;
RTMPSockBuf sb = {0};
http->status = -1;
memset(&sa, 0, sizeof(struct sockaddr_in));
sa.sin_family = AF_INET;
if (strncasecmp(url, "http", 4))
return HTTPRES_BAD_REQUEST;
if (url[4] == 's')
{
#if defined(CRYPTO)
ssl = 1;
port = 443;
if (!RTMP_TLS_ctx)
RTMP_TLS_Init();
#else
return HTTPRES_BAD_REQUEST;
#endif
}
p1 = strchr(url + 4, ':');
if (!p1 || strncmp(p1, "://", 3))
return HTTPRES_BAD_REQUEST;
host = p1 + 3;
path = strchr(host, '/');
hlen = path - host;
strncpy(hbuf, host, hlen);
hbuf[hlen] = '\0';
host = hbuf;
p1 = strrchr(host, ':');
if (p1)
{
*p1++ = '\0';
port = atoi(p1);
}
sa.sin_addr.s_addr = inet_addr(host);
if (sa.sin_addr.s_addr == INADDR_NONE)
{
struct hostent *hp = gethostbyname(host);
if (!hp || !hp->h_addr)
return HTTPRES_LOST_CONNECTION;
sa.sin_addr = *(struct in_addr *)hp->h_addr;
}
sa.sin_port = htons(port);
sb.sb_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if (sb.sb_socket == INVALID_SOCKET)
return HTTPRES_LOST_CONNECTION;
i =
sprintf(sb.sb_buf,
"GET %s HTTP/1.0\r\nUser-Agent: %s\r\nHost: %s\r\nReferer: %.*s\r\n",
path, AGENT, host, (int)(path - url + 1), url);
if (http->date[0])
i += sprintf(sb.sb_buf + i, "If-Modified-Since: %s\r\n", http->date);
i += sprintf(sb.sb_buf + i, "\r\n");
if (connect
(sb.sb_socket, (struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0)
{
ret = HTTPRES_LOST_CONNECTION;
goto leave;
}
#if defined(CRYPTO)
if (ssl)
{
#if defined(NO_SSL)
RTMP_Log(RTMP_LOGERROR, "%s, No SSL/TLS support", __FUNCTION__);
ret = HTTPRES_BAD_REQUEST;
goto leave;
#else
TLS_client(RTMP_TLS_ctx, sb.sb_ssl);
#if defined(USE_MBEDTLS)
mbedtls_net_context *server_fd = &RTMP_TLS_ctx->net;
server_fd->fd = sb.sb_socket;
TLS_setfd(sb.sb_ssl, server_fd);
#else
TLS_setfd(sb.sb_ssl, sb.sb_socket);
#endif
int connect_return = TLS_connect(sb.sb_ssl);
if (connect_return < 0)
{
RTMP_Log(RTMP_LOGERROR, "%s, TLS_Connect failed", __FUNCTION__);
ret = HTTPRES_LOST_CONNECTION;
goto leave;
}
#endif
}
#endif
RTMPSockBuf_Send(&sb, sb.sb_buf, i);
#define HTTP_TIMEOUT 5
{
SET_RCVTIMEO(tv, HTTP_TIMEOUT);
if (setsockopt
(sb.sb_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)))
{
RTMP_Log(RTMP_LOGERROR, "%s, Setting socket timeout to %ds failed!",
__FUNCTION__, HTTP_TIMEOUT);
}
}
sb.sb_size = 0;
sb.sb_timedout = FALSE;
if (RTMPSockBuf_Fill(&sb) < 1)
{
ret = HTTPRES_LOST_CONNECTION;
goto leave;
}
if (strncmp(sb.sb_buf, "HTTP/1", 6))
{
ret = HTTPRES_BAD_REQUEST;
goto leave;
}
p1 = strchr(sb.sb_buf, ' ');
rc = atoi(p1 + 1);
http->status = rc;
if (rc >= 300)
{
if (rc == 304)
{
ret = HTTPRES_OK_NOT_MODIFIED;
goto leave;
}
else if (rc == 404)
ret = HTTPRES_NOT_FOUND;
else if (rc >= 500)
ret = HTTPRES_SERVER_ERROR;
else if (rc >= 400)
ret = HTTPRES_BAD_REQUEST;
else
ret = HTTPRES_REDIRECTED;
}
p1 = memchr(sb.sb_buf, '\n', sb.sb_size);
if (!p1)
{
ret = HTTPRES_BAD_REQUEST;
goto leave;
}
sb.sb_start = p1 + 1;
sb.sb_size -= sb.sb_start - sb.sb_buf;
while ((p2 = memchr(sb.sb_start, '\r', sb.sb_size)))
{
if (*sb.sb_start == '\r')
{
sb.sb_start += 2;
sb.sb_size -= 2;
break;
}
else if (!strncasecmp
(sb.sb_start, "Content-Length: ", sizeof("Content-Length: ") - 1))
{
flen = atoi(sb.sb_start + sizeof("Content-Length: ") - 1);
}
else if (!strncasecmp
(sb.sb_start, "Last-Modified: ", sizeof("Last-Modified: ") - 1))
{
*p2 = '\0';
strcpy(http->date, sb.sb_start + sizeof("Last-Modified: ") - 1);
}
p2 += 2;
sb.sb_size -= p2 - sb.sb_start;
sb.sb_start = p2;
if (sb.sb_size < 1)
{
if (RTMPSockBuf_Fill(&sb) < 1)
{
ret = HTTPRES_LOST_CONNECTION;
goto leave;
}
}
}
len_known = flen > 0;
while ((!len_known || flen > 0) &&
(sb.sb_size > 0 || RTMPSockBuf_Fill(&sb) > 0))
{
cb(sb.sb_start, 1, sb.sb_size, http->data);
if (len_known)
flen -= sb.sb_size;
http->size += sb.sb_size;
sb.sb_size = 0;
}
if (flen > 0)
ret = HTTPRES_LOST_CONNECTION;
leave:
RTMPSockBuf_Close(&sb);
return ret;
}
#if defined(CRYPTO)
#define CHUNK 16384
struct info
{
z_stream *zs;
HMAC_CTX ctx;
int first;
int zlib;
int size;
};
static size_t
swfcrunch(void *ptr, size_t size, size_t nmemb, void *stream)
{
struct info *i = stream;
char *p = ptr;
size_t len = size * nmemb;
if (i->first)
{
i->first = 0;
if (!strncmp(p, "CWS", 3))
{
*p = 'F';
i->zlib = 1;
}
HMAC_crunch(i->ctx, (unsigned char *)p, 8);
p += 8;
len -= 8;
i->size = 8;
}
if (i->zlib)
{
unsigned char out[CHUNK];
i->zs->next_in = (unsigned char *)p;
i->zs->avail_in = (uInt)len;
do
{
i->zs->avail_out = CHUNK;
i->zs->next_out = out;
inflate(i->zs, Z_NO_FLUSH);
len = CHUNK - i->zs->avail_out;
i->size += (int)len;
HMAC_crunch(i->ctx, out, len);
}
while (i->zs->avail_out == 0);
}
else
{
i->size += (int)len;
HMAC_crunch(i->ctx, (unsigned char *)p, len);
}
return size * nmemb;
}
static int tzoff;
static int tzchecked;
#define JAN02_1980 318340800
static const char *monthtab[12] = { "Jan", "Feb", "Mar",
"Apr", "May", "Jun",
"Jul", "Aug", "Sep",
"Oct", "Nov", "Dec"
};
static const char *days[] =
{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static time_t
make_unix_time(char *s)
{
struct tm time;
int i, ysub = 1900, fmt = 0;
char *month;
char *n;
time_t res;
if (s[3] != ' ')
{
fmt = 1;
if (s[3] != ',')
ysub = 0;
}
for (n = s; *n; ++n)
if (*n == '-' || *n == ':')
*n = ' ';
time.tm_mon = 0;
n = strchr(s, ' ');
if (fmt)
{
time.tm_mday = strtol(n + 1, &n, 0);
month = n + 1;
n = strchr(month, ' ');
time.tm_year = strtol(n + 1, &n, 0);
time.tm_hour = strtol(n + 1, &n, 0);
time.tm_min = strtol(n + 1, &n, 0);
time.tm_sec = strtol(n + 1, NULL, 0);
}
else
{
month = n + 1;
n = strchr(month, ' ');
while (isspace(*n))
n++;
time.tm_mday = strtol(n, &n, 0);
time.tm_hour = strtol(n + 1, &n, 0);
time.tm_min = strtol(n + 1, &n, 0);
time.tm_sec = strtol(n + 1, &n, 0);
time.tm_year = strtol(n + 1, NULL, 0);
}
if (time.tm_year > 100)
time.tm_year -= ysub;
for (i = 0; i < 12; i++)
if (!strncasecmp(month, monthtab[i], 3))
{
time.tm_mon = i;
break;
}
time.tm_isdst = 0; 
if (!tzchecked)
{
struct tm *tc;
time_t then = JAN02_1980;
tc = localtime(&then);
tzoff = (12 - tc->tm_hour) * 3600 + tc->tm_min * 60 + tc->tm_sec;
tzchecked = 1;
}
res = mktime(&time);
if (res != -1)
res += tzoff;
return res;
}
static void
strtime(time_t * t, char *s)
{
struct tm *tm;
tm = gmtime((time_t *) t);
sprintf(s, "%s, %02d %s %d %02d:%02d:%02d GMT",
days[tm->tm_wday], tm->tm_mday, monthtab[tm->tm_mon],
tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
}
#define HEX2BIN(a) (((a)&0x40)?((a)&0xf)+9:((a)&0xf))
int
RTMP_HashSWF(const char *url, unsigned int *size, unsigned char *hash,
int age)
{
FILE *f = NULL;
char *path, date[64], cctim[64];
long pos = 0;
time_t ctim = -1, cnow;
int i, got = 0, ret = 0;
unsigned int hlen;
struct info in = { 0 };
struct HTTP_ctx http = { 0 };
HTTPResult httpres;
z_stream zs = { 0 };
AVal home, hpre;
date[0] = '\0';
#if defined(_WIN32)
#if defined(XBMC4XBOX)
hpre.av_val = "Q:";
hpre.av_len = 2;
home.av_val = "\\UserData";
#else
hpre.av_val = getenv("HOMEDRIVE");
hpre.av_len = (int)strlen(hpre.av_val);
home.av_val = getenv("HOMEPATH");
#endif
#define DIRSEP "\\"
#else 
hpre.av_val = "";
hpre.av_len = 0;
home.av_val = getenv("HOME");
#define DIRSEP "/"
#endif
if (!home.av_val)
home.av_val = ".";
home.av_len = (int)strlen(home.av_val);
path = malloc(hpre.av_len + home.av_len + sizeof(DIRSEP ".swfinfo"));
sprintf(path, "%s%s" DIRSEP ".swfinfo", hpre.av_val, home.av_val);
f = fopen(path, "r+");
while (f)
{
char buf[4096], *file, *p;
file = strchr(url, '/');
if (!file)
break;
file += 2;
file = strchr(file, '/');
if (!file)
break;
file++;
hlen = file - url;
p = strrchr(file, '/');
if (p)
file = p;
else
file--;
while (fgets(buf, sizeof(buf), f))
{
char *r1;
got = 0;
if (strncmp(buf, "url: ", 5))
continue;
if (strncmp(buf + 5, url, hlen))
continue;
r1 = strrchr(buf, '/');
i = (int)strlen(r1);
r1[--i] = '\0';
if (strncmp(r1, file, i))
continue;
pos = ftell(f);
while (got < 4 && fgets(buf, sizeof(buf), f))
{
if (!strncmp(buf, "size: ", 6))
{
*size = strtol(buf + 6, NULL, 16);
got++;
}
else if (!strncmp(buf, "hash: ", 6))
{
unsigned char *ptr = hash, *in = (unsigned char *)buf + 6;
int l = (int)strlen((char *)in) - 1;
for (i = 0; i < l; i += 2)
*ptr++ = (HEX2BIN(in[i]) << 4) | HEX2BIN(in[i + 1]);
got++;
}
else if (!strncmp(buf, "date: ", 6))
{
buf[strlen(buf) - 1] = '\0';
strncpy(date, buf + 6, sizeof(date));
got++;
}
else if (!strncmp(buf, "ctim: ", 6))
{
buf[strlen(buf) - 1] = '\0';
ctim = make_unix_time(buf + 6);
got++;
}
else if (!strncmp(buf, "url: ", 5))
break;
}
break;
}
break;
}
cnow = time(NULL);
if (age && ctim > 0)
{
ctim = cnow - ctim;
ctim /= 3600 * 24; 
if (ctim < age) 
goto out;
}
in.first = 1;
HMAC_setup(in.ctx, "Genuine Adobe Flash Player 001", 30);
inflateInit(&zs);
in.zs = &zs;
http.date = date;
http.data = &in;
httpres = HTTP_get(&http, url, swfcrunch);
inflateEnd(&zs);
if (httpres != HTTPRES_OK && httpres != HTTPRES_OK_NOT_MODIFIED)
{
ret = -1;
if (httpres == HTTPRES_LOST_CONNECTION)
RTMP_Log(RTMP_LOGERROR, "%s: connection lost while downloading swfurl %s",
__FUNCTION__, url);
else if (httpres == HTTPRES_NOT_FOUND)
RTMP_Log(RTMP_LOGERROR, "%s: swfurl %s not found", __FUNCTION__, url);
else
RTMP_Log(RTMP_LOGERROR, "%s: couldn't contact swfurl %s (HTTP error %d)",
__FUNCTION__, url, http.status);
}
else
{
if (got && pos)
fseek(f, pos, SEEK_SET);
else
{
char *q;
if (!f)
f = fopen(path, "w");
if (!f)
{
int err = errno;
RTMP_Log(RTMP_LOGERROR,
"%s: couldn't open %s for writing, errno %d (%s)",
__FUNCTION__, path, err, strerror(err));
ret = -1;
goto out;
}
fseek(f, 0, SEEK_END);
q = strchr(url, '?');
if (q)
i = q - url;
else
i = (int)strlen(url);
fprintf(f, "url: %.*s\n", i, url);
}
strtime(&cnow, cctim);
fprintf(f, "ctim: %s\n", cctim);
if (!in.first)
{
#if defined(USE_MBEDTLS) || defined(USE_POLARSSL) || defined(USE_GNUTLS)
HMAC_finish(in.ctx, hash);
#else
HMAC_finish(in.ctx, hash, hlen);
#endif
*size = in.size;
fprintf(f, "date: %s\n", date);
fprintf(f, "size: %08x\n", in.size);
fprintf(f, "hash: ");
for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
fprintf(f, "%02x", hash[i]);
fprintf(f, "\n");
}
}
HMAC_close(in.ctx);
out:
free(path);
if (f)
fclose(f);
return ret;
}
#else
int
RTMP_HashSWF(const char *url, unsigned int *size, unsigned char *hash,
int age)
{
(void)url;
(void)size;
(void)hash;
(void)age;
return -1;
}
#endif
#endif
