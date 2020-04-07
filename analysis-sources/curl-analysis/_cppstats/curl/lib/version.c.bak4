





















#include "curl_setup.h"

#include <curl/curl.h>
#include "urldata.h"
#include "vtls/vtls.h"
#include "http2.h"
#include "vssh/ssh.h"
#include "quic.h"
#include "curl_printf.h"

#if defined(USE_ARES)
#if defined(CURL_STATICLIB) && !defined(CARES_STATICLIB) && (defined(WIN32) || defined(__SYMBIAN32__))

#define CARES_STATICLIB
#endif
#include <ares.h>
#endif

#if defined(USE_LIBIDN2)
#include <idn2.h>
#endif

#if defined(USE_LIBPSL)
#include <libpsl.h>
#endif

#if defined(HAVE_ICONV) && defined(CURL_DOES_CONVERSIONS)
#include <iconv.h>
#endif

#if defined(USE_LIBRTMP)
#include <librtmp/rtmp.h>
#endif

#if defined(HAVE_ZLIB_H)
#include <zlib.h>
#if defined(__SYMBIAN32__)

#undef WIN32
#endif
#endif

#if defined(HAVE_BROTLI)
#include <brotli/decode.h>
#endif

#if defined(HAVE_BROTLI)
static size_t brotli_version(char *buf, size_t bufsz)
{
uint32_t brotli_version = BrotliDecoderVersion();
unsigned int major = brotli_version >> 24;
unsigned int minor = (brotli_version & 0x00FFFFFF) >> 12;
unsigned int patch = brotli_version & 0x00000FFF;

return msnprintf(buf, bufsz, "%u.%u.%u", major, minor, patch);
}
#endif








char *curl_version(void)
{
static char out[250];
char *outp;
size_t outlen;
const char *src[14];
#if defined(USE_SSL)
char ssl_version[40];
#endif
#if defined(HAVE_LIBZ)
char z_version[40];
#endif
#if defined(HAVE_BROTLI)
char br_version[40] = "brotli/";
#endif
#if defined(USE_ARES)
char cares_version[40];
#endif
#if defined(USE_LIBIDN2) || defined(USE_WIN32_IDN)
char idn_version[40];
#endif
#if defined(USE_LIBPSL)
char psl_version[40];
#endif
#if defined(HAVE_ICONV) && defined(CURL_DOES_CONVERSIONS)
char iconv_version[40]="iconv";
#endif
#if defined(USE_SSH)
char ssh_version[40];
#endif
#if defined(USE_NGHTTP2)
char h2_version[40];
#endif
#if defined(ENABLE_QUIC)
char h3_version[40];
#endif
#if defined(USE_LIBRTMP)
char rtmp_version[40];
#endif
int i = 0;
int j;

#if defined(DEBUGBUILD)

const char *debugversion = getenv("CURL_VERSION");
if(debugversion) {
strncpy(out, debugversion, sizeof(out)-1);
out[sizeof(out)-1] = '\0';
return out;
}
#endif

src[i++] = LIBCURL_NAME "/" LIBCURL_VERSION;
#if defined(USE_SSL)
Curl_ssl_version(ssl_version, sizeof(ssl_version));
src[i++] = ssl_version;
#endif
#if defined(HAVE_LIBZ)
msnprintf(z_version, sizeof(z_version), "zlib/%s", zlibVersion());
src[i++] = z_version;
#endif
#if defined(HAVE_BROTLI)
brotli_version(&br_version[7], sizeof(br_version) - 7);
src[i++] = br_version;
#endif
#if defined(USE_ARES)
msnprintf(cares_version, sizeof(cares_version),
"c-ares/%s", ares_version(NULL));
src[i++] = cares_version;
#endif
#if defined(USE_LIBIDN2)
if(idn2_check_version(IDN2_VERSION)) {
msnprintf(idn_version, sizeof(idn_version),
"libidn2/%s", idn2_check_version(NULL));
src[i++] = idn_version;
}
#elif defined(USE_WIN32_IDN)
msnprintf(idn_version, sizeof(idn_version), "WinIDN");
src[i++] = idn_version;
#endif

#if defined(USE_LIBPSL)
msnprintf(psl_version, sizeof(psl_version), "libpsl/%s", psl_get_version());
src[i++] = psl_version;
#endif
#if defined(HAVE_ICONV) && defined(CURL_DOES_CONVERSIONS)
#if defined(_LIBICONV_VERSION)
msnprintf(iconv_version, sizeof(iconv_version), "iconv/%d.%d",
_LIBICONV_VERSION >> 8, _LIBICONV_VERSION & 255);
#else

#endif 
src[i++] = iconv_version;
#endif
#if defined(USE_SSH)
Curl_ssh_version(ssh_version, sizeof(ssh_version));
src[i++] = ssh_version;
#endif
#if defined(USE_NGHTTP2)
Curl_http2_ver(h2_version, sizeof(h2_version));
src[i++] = h2_version;
#endif
#if defined(ENABLE_QUIC)
Curl_quic_ver(h3_version, sizeof(h3_version));
src[i++] = h3_version;
#endif
#if defined(USE_LIBRTMP)
{
char suff[2];
if(RTMP_LIB_VERSION & 0xff) {
suff[0] = (RTMP_LIB_VERSION & 0xff) + 'a' - 1;
suff[1] = '\0';
}
else
suff[0] = '\0';

msnprintf(rtmp_version, sizeof(rtmp_version), "librtmp/%d.%d%s",
RTMP_LIB_VERSION >> 16, (RTMP_LIB_VERSION >> 8) & 0xff,
suff);
src[i++] = rtmp_version;
}
#endif

outp = &out[0];
outlen = sizeof(out);
for(j = 0; j < i; j++) {
size_t n = strlen(src[j]);

if(outlen <= (n + 2))
break;
if(j) {

*outp++ = ' ';
outlen--;
}
memcpy(outp, src[j], n);
outp += n;
outlen -= n;
}
*outp = 0;

return out;
}







static const char * const protocols[] = {
#if !defined(CURL_DISABLE_DICT)
"dict",
#endif
#if !defined(CURL_DISABLE_FILE)
"file",
#endif
#if !defined(CURL_DISABLE_FTP)
"ftp",
#endif
#if defined(USE_SSL) && !defined(CURL_DISABLE_FTP)
"ftps",
#endif
#if !defined(CURL_DISABLE_GOPHER)
"gopher",
#endif
#if !defined(CURL_DISABLE_HTTP)
"http",
#endif
#if defined(USE_SSL) && !defined(CURL_DISABLE_HTTP)
"https",
#endif
#if !defined(CURL_DISABLE_IMAP)
"imap",
#endif
#if defined(USE_SSL) && !defined(CURL_DISABLE_IMAP)
"imaps",
#endif
#if !defined(CURL_DISABLE_LDAP)
"ldap",
#if !defined(CURL_DISABLE_LDAPS) && ((defined(USE_OPENLDAP) && defined(USE_SSL)) || (!defined(USE_OPENLDAP) && defined(HAVE_LDAP_SSL)))


"ldaps",
#endif
#endif
#if !defined(CURL_DISABLE_POP3)
"pop3",
#endif
#if defined(USE_SSL) && !defined(CURL_DISABLE_POP3)
"pop3s",
#endif
#if defined(USE_LIBRTMP)
"rtmp",
#endif
#if !defined(CURL_DISABLE_RTSP)
"rtsp",
#endif
#if defined(USE_SSH) && !defined(USE_WOLFSSH)
"scp",
#endif
#if defined(USE_SSH)
"sftp",
#endif
#if !defined(CURL_DISABLE_SMB) && defined(USE_NTLM) && (CURL_SIZEOF_CURL_OFF_T > 4) && (!defined(USE_WINDOWS_SSPI) || defined(USE_WIN32_CRYPTO))


"smb",
#if defined(USE_SSL)
"smbs",
#endif
#endif
#if !defined(CURL_DISABLE_SMTP)
"smtp",
#endif
#if defined(USE_SSL) && !defined(CURL_DISABLE_SMTP)
"smtps",
#endif
#if !defined(CURL_DISABLE_TELNET)
"telnet",
#endif
#if !defined(CURL_DISABLE_TFTP)
"tftp",
#endif

NULL
};

static curl_version_info_data version_info = {
CURLVERSION_NOW,
LIBCURL_VERSION,
LIBCURL_VERSION_NUM,
OS, 
0 
#if defined(ENABLE_IPV6)
| CURL_VERSION_IPV6
#endif
#if defined(USE_SSL)
| CURL_VERSION_SSL
#endif
#if defined(USE_NTLM)
| CURL_VERSION_NTLM
#endif
#if !defined(CURL_DISABLE_HTTP) && defined(USE_NTLM) && defined(NTLM_WB_ENABLED)

| CURL_VERSION_NTLM_WB
#endif
#if defined(USE_SPNEGO)
| CURL_VERSION_SPNEGO
#endif
#if defined(USE_KERBEROS5)
| CURL_VERSION_KERBEROS5
#endif
#if defined(HAVE_GSSAPI)
| CURL_VERSION_GSSAPI
#endif
#if defined(USE_WINDOWS_SSPI)
| CURL_VERSION_SSPI
#endif
#if defined(HAVE_LIBZ)
| CURL_VERSION_LIBZ
#endif
#if defined(DEBUGBUILD)
| CURL_VERSION_DEBUG
#endif
#if defined(CURLDEBUG)
| CURL_VERSION_CURLDEBUG
#endif
#if defined(CURLRES_ASYNCH)
| CURL_VERSION_ASYNCHDNS
#endif
#if (CURL_SIZEOF_CURL_OFF_T > 4) && ( (SIZEOF_OFF_T > 4) || defined(USE_WIN32_LARGE_FILES) )

| CURL_VERSION_LARGEFILE
#endif
#if defined(CURL_DOES_CONVERSIONS)
| CURL_VERSION_CONV
#endif
#if defined(USE_TLS_SRP)
| CURL_VERSION_TLSAUTH_SRP
#endif
#if defined(USE_NGHTTP2)
| CURL_VERSION_HTTP2
#endif
#if defined(ENABLE_QUIC)
| CURL_VERSION_HTTP3
#endif
#if defined(USE_UNIX_SOCKETS)
| CURL_VERSION_UNIX_SOCKETS
#endif
#if defined(USE_LIBPSL)
| CURL_VERSION_PSL
#endif
#if defined(CURL_WITH_MULTI_SSL)
| CURL_VERSION_MULTI_SSL
#endif
#if defined(HAVE_BROTLI)
| CURL_VERSION_BROTLI
#endif
#if defined(USE_ALTSVC)
| CURL_VERSION_ALTSVC
#endif
,
NULL, 
0, 
NULL, 
protocols,
NULL, 
0, 
NULL, 
0, 
NULL, 
0, 
NULL, 
0, 
NULL, 
NULL, 
#if defined(CURL_CA_BUNDLE)
CURL_CA_BUNDLE, 
#else
NULL,
#endif
#if defined(CURL_CA_PATH)
CURL_CA_PATH 
#else
NULL
#endif
};

curl_version_info_data *curl_version_info(CURLversion stamp)
{
#if defined(USE_SSH)
static char ssh_buffer[80];
#endif
#if defined(USE_SSL)
#if defined(CURL_WITH_MULTI_SSL)
static char ssl_buffer[200];
#else
static char ssl_buffer[80];
#endif
#endif
#if defined(HAVE_BROTLI)
static char brotli_buffer[80];
#endif

#if defined(USE_SSL)
Curl_ssl_version(ssl_buffer, sizeof(ssl_buffer));
version_info.ssl_version = ssl_buffer;
if(Curl_ssl->supports & SSLSUPP_HTTPS_PROXY)
version_info.features |= CURL_VERSION_HTTPS_PROXY;
else
version_info.features &= ~CURL_VERSION_HTTPS_PROXY;
#endif

#if defined(HAVE_LIBZ)
version_info.libz_version = zlibVersion();

#endif
#if defined(USE_ARES)
{
int aresnum;
version_info.ares = ares_version(&aresnum);
version_info.ares_num = aresnum;
}
#endif
#if defined(USE_LIBIDN2)


version_info.libidn = idn2_check_version(IDN2_VERSION);
if(version_info.libidn)
version_info.features |= CURL_VERSION_IDN;
#elif defined(USE_WIN32_IDN)
version_info.features |= CURL_VERSION_IDN;
#endif

#if defined(HAVE_ICONV) && defined(CURL_DOES_CONVERSIONS)
#if defined(_LIBICONV_VERSION)
version_info.iconv_ver_num = _LIBICONV_VERSION;
#else

version_info.iconv_ver_num = -1;
#endif 
#endif

#if defined(USE_SSH)
Curl_ssh_version(ssh_buffer, sizeof(ssh_buffer));
version_info.libssh_version = ssh_buffer;
#endif

#if defined(HAVE_BROTLI)
version_info.brotli_ver_num = BrotliDecoderVersion();
brotli_version(brotli_buffer, sizeof(brotli_buffer));
version_info.brotli_version = brotli_buffer;
#endif

#if defined(USE_NGHTTP2)
{
nghttp2_info *h2 = nghttp2_version(0);
version_info.nghttp2_ver_num = h2->version_num;
version_info.nghttp2_version = h2->version_str;
}
#endif

#if defined(ENABLE_QUIC)
{
static char quicbuffer[80];
Curl_quic_ver(quicbuffer, sizeof(quicbuffer));
version_info.quic_version = quicbuffer;
}
#endif

(void)stamp; 
return &version_info;
}
