#include "curl_setup.h"
#include "urldata.h" 
#include "curl_base64.h"
#include "strtok.h"
#include "multiif.h"
#if defined(USE_SECTRANSP)
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-pointer-compare"
#endif 
#include <limits.h>
#include <Security/Security.h>
#include <Security/SecureTransport.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonDigest.h>
#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE))
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1050
#error "The Secure Transport back-end requires Leopard or later."
#endif 
#define CURL_BUILD_IOS 0
#define CURL_BUILD_IOS_7 0
#define CURL_BUILD_IOS_9 0
#define CURL_BUILD_IOS_11 0
#define CURL_BUILD_MAC 1
#define CURL_BUILD_MAC_10_5 MAC_OS_X_VERSION_MAX_ALLOWED >= 1050
#define CURL_BUILD_MAC_10_6 MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
#define CURL_BUILD_MAC_10_7 MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
#define CURL_BUILD_MAC_10_8 MAC_OS_X_VERSION_MAX_ALLOWED >= 1080
#define CURL_BUILD_MAC_10_9 MAC_OS_X_VERSION_MAX_ALLOWED >= 1090
#define CURL_BUILD_MAC_10_11 MAC_OS_X_VERSION_MAX_ALLOWED >= 101100
#define CURL_BUILD_MAC_10_13 MAC_OS_X_VERSION_MAX_ALLOWED >= 101300
#define CURL_SUPPORT_MAC_10_5 MAC_OS_X_VERSION_MIN_REQUIRED <= 1050
#define CURL_SUPPORT_MAC_10_6 MAC_OS_X_VERSION_MIN_REQUIRED <= 1060
#define CURL_SUPPORT_MAC_10_7 MAC_OS_X_VERSION_MIN_REQUIRED <= 1070
#define CURL_SUPPORT_MAC_10_8 MAC_OS_X_VERSION_MIN_REQUIRED <= 1080
#define CURL_SUPPORT_MAC_10_9 MAC_OS_X_VERSION_MIN_REQUIRED <= 1090
#elif TARGET_OS_EMBEDDED || TARGET_OS_IPHONE
#define CURL_BUILD_IOS 1
#define CURL_BUILD_IOS_7 __IPHONE_OS_VERSION_MAX_ALLOWED >= 70000
#define CURL_BUILD_IOS_9 __IPHONE_OS_VERSION_MAX_ALLOWED >= 90000
#define CURL_BUILD_IOS_11 __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#define CURL_BUILD_MAC 0
#define CURL_BUILD_MAC_10_5 0
#define CURL_BUILD_MAC_10_6 0
#define CURL_BUILD_MAC_10_7 0
#define CURL_BUILD_MAC_10_8 0
#define CURL_BUILD_MAC_10_9 0
#define CURL_BUILD_MAC_10_11 0
#define CURL_BUILD_MAC_10_13 0
#define CURL_SUPPORT_MAC_10_5 0
#define CURL_SUPPORT_MAC_10_6 0
#define CURL_SUPPORT_MAC_10_7 0
#define CURL_SUPPORT_MAC_10_8 0
#define CURL_SUPPORT_MAC_10_9 0
#else
#error "The Secure Transport back-end requires iOS or macOS."
#endif 
#if CURL_BUILD_MAC
#include <sys/sysctl.h>
#endif 
#include "urldata.h"
#include "sendf.h"
#include "inet_pton.h"
#include "connect.h"
#include "select.h"
#include "vtls.h"
#include "sectransp.h"
#include "curl_printf.h"
#include "strdup.h"
#include "curl_memory.h"
#include "memdebug.h"
#define ioErr -36
#define paramErr -50
struct ssl_backend_data {
SSLContextRef ssl_ctx;
curl_socket_t ssl_sockfd;
bool ssl_direction; 
size_t ssl_write_buffered_length;
};
#if ((TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000) || (!TARGET_OS_IPHONE && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200))
#define SECTRANSP_PINNEDPUBKEY_V1 1
#endif
#if (!TARGET_OS_IPHONE && __MAC_OS_X_VERSION_MIN_REQUIRED >= 1070)
#define SECTRANSP_PINNEDPUBKEY_V2 1
#endif
#if defined(SECTRANSP_PINNEDPUBKEY_V1) || defined(SECTRANSP_PINNEDPUBKEY_V2)
#define SECTRANSP_PINNEDPUBKEY 1
#endif 
#if defined(SECTRANSP_PINNEDPUBKEY)
static const unsigned char rsa4096SpkiHeader[] = {
0x30, 0x82, 0x02, 0x22, 0x30, 0x0d,
0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05,
0x00, 0x03, 0x82, 0x02, 0x0f, 0x00};
static const unsigned char rsa2048SpkiHeader[] = {
0x30, 0x82, 0x01, 0x22, 0x30, 0x0d,
0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05,
0x00, 0x03, 0x82, 0x01, 0x0f, 0x00};
#if defined(SECTRANSP_PINNEDPUBKEY_V1)
static const unsigned char ecDsaSecp256r1SpkiHeader[] = {
0x30, 0x59, 0x30, 0x13, 0x06, 0x07,
0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
0x01, 0x06, 0x08, 0x2a, 0x86, 0x48,
0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
0x42, 0x00};
static const unsigned char ecDsaSecp384r1SpkiHeader[] = {
0x30, 0x76, 0x30, 0x10, 0x06, 0x07,
0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
0x01, 0x06, 0x05, 0x2b, 0x81, 0x04,
0x00, 0x22, 0x03, 0x62, 0x00};
#endif 
#endif 
static OSStatus SocketRead(SSLConnectionRef connection,
void *data, 
size_t *dataLength) 
{
size_t bytesToGo = *dataLength;
size_t initLen = bytesToGo;
UInt8 *currData = (UInt8 *)data;
struct ssl_connect_data *connssl = (struct ssl_connect_data *)connection;
struct ssl_backend_data *backend = connssl->backend;
int sock = backend->ssl_sockfd;
OSStatus rtn = noErr;
size_t bytesRead;
ssize_t rrtn;
int theErr;
*dataLength = 0;
for(;;) {
bytesRead = 0;
rrtn = read(sock, currData, bytesToGo);
if(rrtn <= 0) {
theErr = errno;
if(rrtn == 0) { 
rtn = errSSLClosedGraceful;
}
else 
switch(theErr) {
case ENOENT:
rtn = errSSLClosedGraceful;
break;
case ECONNRESET:
rtn = errSSLClosedAbort;
break;
case EAGAIN:
rtn = errSSLWouldBlock;
backend->ssl_direction = false;
break;
default:
rtn = ioErr;
break;
}
break;
}
else {
bytesRead = rrtn;
}
bytesToGo -= bytesRead;
currData += bytesRead;
if(bytesToGo == 0) {
break;
}
}
*dataLength = initLen - bytesToGo;
return rtn;
}
static OSStatus SocketWrite(SSLConnectionRef connection,
const void *data,
size_t *dataLength) 
{
size_t bytesSent = 0;
struct ssl_connect_data *connssl = (struct ssl_connect_data *)connection;
struct ssl_backend_data *backend = connssl->backend;
int sock = backend->ssl_sockfd;
ssize_t length;
size_t dataLen = *dataLength;
const UInt8 *dataPtr = (UInt8 *)data;
OSStatus ortn;
int theErr;
*dataLength = 0;
do {
length = write(sock,
(char *)dataPtr + bytesSent,
dataLen - bytesSent);
} while((length > 0) &&
( (bytesSent += length) < dataLen) );
if(length <= 0) {
theErr = errno;
if(theErr == EAGAIN) {
ortn = errSSLWouldBlock;
backend->ssl_direction = true;
}
else {
ortn = ioErr;
}
}
else {
ortn = noErr;
}
*dataLength = bytesSent;
return ortn;
}
#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
CF_INLINE const char *SSLCipherNameForNumber(SSLCipherSuite cipher)
{
switch(cipher) {
case SSL_RSA_WITH_NULL_MD5:
return "SSL_RSA_WITH_NULL_MD5";
break;
case SSL_RSA_WITH_NULL_SHA:
return "SSL_RSA_WITH_NULL_SHA";
break;
case SSL_RSA_EXPORT_WITH_RC4_40_MD5:
return "SSL_RSA_EXPORT_WITH_RC4_40_MD5";
break;
case SSL_RSA_WITH_RC4_128_MD5:
return "SSL_RSA_WITH_RC4_128_MD5";
break;
case SSL_RSA_WITH_RC4_128_SHA:
return "SSL_RSA_WITH_RC4_128_SHA";
break;
case SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5:
return "SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5";
break;
case SSL_RSA_WITH_IDEA_CBC_SHA:
return "SSL_RSA_WITH_IDEA_CBC_SHA";
break;
case SSL_RSA_EXPORT_WITH_DES40_CBC_SHA:
return "SSL_RSA_EXPORT_WITH_DES40_CBC_SHA";
break;
case SSL_RSA_WITH_DES_CBC_SHA:
return "SSL_RSA_WITH_DES_CBC_SHA";
break;
case SSL_RSA_WITH_3DES_EDE_CBC_SHA:
return "SSL_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA:
return "SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA";
break;
case SSL_DH_DSS_WITH_DES_CBC_SHA:
return "SSL_DH_DSS_WITH_DES_CBC_SHA";
break;
case SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA:
return "SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA";
break;
case SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA:
return "SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA";
break;
case SSL_DH_RSA_WITH_DES_CBC_SHA:
return "SSL_DH_RSA_WITH_DES_CBC_SHA";
break;
case SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA:
return "SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA:
return "SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA";
break;
case SSL_DHE_DSS_WITH_DES_CBC_SHA:
return "SSL_DHE_DSS_WITH_DES_CBC_SHA";
break;
case SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
return "SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA";
break;
case SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA:
return "SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA";
break;
case SSL_DHE_RSA_WITH_DES_CBC_SHA:
return "SSL_DHE_RSA_WITH_DES_CBC_SHA";
break;
case SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
return "SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case SSL_DH_anon_EXPORT_WITH_RC4_40_MD5:
return "SSL_DH_anon_EXPORT_WITH_RC4_40_MD5";
break;
case SSL_DH_anon_WITH_RC4_128_MD5:
return "SSL_DH_anon_WITH_RC4_128_MD5";
break;
case SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA:
return "SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA";
break;
case SSL_DH_anon_WITH_DES_CBC_SHA:
return "SSL_DH_anon_WITH_DES_CBC_SHA";
break;
case SSL_DH_anon_WITH_3DES_EDE_CBC_SHA:
return "SSL_DH_anon_WITH_3DES_EDE_CBC_SHA";
break;
case SSL_FORTEZZA_DMS_WITH_NULL_SHA:
return "SSL_FORTEZZA_DMS_WITH_NULL_SHA";
break;
case SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA:
return "SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA";
break;
case TLS_RSA_WITH_AES_128_CBC_SHA:
return "TLS_RSA_WITH_AES_128_CBC_SHA";
break;
case TLS_DH_DSS_WITH_AES_128_CBC_SHA:
return "TLS_DH_DSS_WITH_AES_128_CBC_SHA";
break;
case TLS_DH_RSA_WITH_AES_128_CBC_SHA:
return "TLS_DH_RSA_WITH_AES_128_CBC_SHA";
break;
case TLS_DHE_DSS_WITH_AES_128_CBC_SHA:
return "TLS_DHE_DSS_WITH_AES_128_CBC_SHA";
break;
case TLS_DHE_RSA_WITH_AES_128_CBC_SHA:
return "TLS_DHE_RSA_WITH_AES_128_CBC_SHA";
break;
case TLS_DH_anon_WITH_AES_128_CBC_SHA:
return "TLS_DH_anon_WITH_AES_128_CBC_SHA";
break;
case TLS_RSA_WITH_AES_256_CBC_SHA:
return "TLS_RSA_WITH_AES_256_CBC_SHA";
break;
case TLS_DH_DSS_WITH_AES_256_CBC_SHA:
return "TLS_DH_DSS_WITH_AES_256_CBC_SHA";
break;
case TLS_DH_RSA_WITH_AES_256_CBC_SHA:
return "TLS_DH_RSA_WITH_AES_256_CBC_SHA";
break;
case TLS_DHE_DSS_WITH_AES_256_CBC_SHA:
return "TLS_DHE_DSS_WITH_AES_256_CBC_SHA";
break;
case TLS_DHE_RSA_WITH_AES_256_CBC_SHA:
return "TLS_DHE_RSA_WITH_AES_256_CBC_SHA";
break;
case TLS_DH_anon_WITH_AES_256_CBC_SHA:
return "TLS_DH_anon_WITH_AES_256_CBC_SHA";
break;
case SSL_RSA_WITH_RC2_CBC_MD5:
return "SSL_RSA_WITH_RC2_CBC_MD5";
break;
case SSL_RSA_WITH_IDEA_CBC_MD5:
return "SSL_RSA_WITH_IDEA_CBC_MD5";
break;
case SSL_RSA_WITH_DES_CBC_MD5:
return "SSL_RSA_WITH_DES_CBC_MD5";
break;
case SSL_RSA_WITH_3DES_EDE_CBC_MD5:
return "SSL_RSA_WITH_3DES_EDE_CBC_MD5";
break;
}
return "SSL_NULL_WITH_NULL_NULL";
}
CF_INLINE const char *TLSCipherNameForNumber(SSLCipherSuite cipher)
{
switch(cipher) {
case TLS_RSA_WITH_AES_128_CBC_SHA:
return "TLS_RSA_WITH_AES_128_CBC_SHA";
break;
case TLS_DH_DSS_WITH_AES_128_CBC_SHA:
return "TLS_DH_DSS_WITH_AES_128_CBC_SHA";
break;
case TLS_DH_RSA_WITH_AES_128_CBC_SHA:
return "TLS_DH_RSA_WITH_AES_128_CBC_SHA";
break;
case TLS_DHE_DSS_WITH_AES_128_CBC_SHA:
return "TLS_DHE_DSS_WITH_AES_128_CBC_SHA";
break;
case TLS_DHE_RSA_WITH_AES_128_CBC_SHA:
return "TLS_DHE_RSA_WITH_AES_128_CBC_SHA";
break;
case TLS_DH_anon_WITH_AES_128_CBC_SHA:
return "TLS_DH_anon_WITH_AES_128_CBC_SHA";
break;
case TLS_RSA_WITH_AES_256_CBC_SHA:
return "TLS_RSA_WITH_AES_256_CBC_SHA";
break;
case TLS_DH_DSS_WITH_AES_256_CBC_SHA:
return "TLS_DH_DSS_WITH_AES_256_CBC_SHA";
break;
case TLS_DH_RSA_WITH_AES_256_CBC_SHA:
return "TLS_DH_RSA_WITH_AES_256_CBC_SHA";
break;
case TLS_DHE_DSS_WITH_AES_256_CBC_SHA:
return "TLS_DHE_DSS_WITH_AES_256_CBC_SHA";
break;
case TLS_DHE_RSA_WITH_AES_256_CBC_SHA:
return "TLS_DHE_RSA_WITH_AES_256_CBC_SHA";
break;
case TLS_DH_anon_WITH_AES_256_CBC_SHA:
return "TLS_DH_anon_WITH_AES_256_CBC_SHA";
break;
#if CURL_BUILD_MAC_10_6 || CURL_BUILD_IOS
case TLS_ECDH_ECDSA_WITH_NULL_SHA:
return "TLS_ECDH_ECDSA_WITH_NULL_SHA";
break;
case TLS_ECDH_ECDSA_WITH_RC4_128_SHA:
return "TLS_ECDH_ECDSA_WITH_RC4_128_SHA";
break;
case TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA:
return "TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA:
return "TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA";
break;
case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA:
return "TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA";
break;
case TLS_ECDHE_ECDSA_WITH_NULL_SHA:
return "TLS_ECDHE_ECDSA_WITH_NULL_SHA";
break;
case TLS_ECDHE_ECDSA_WITH_RC4_128_SHA:
return "TLS_ECDHE_ECDSA_WITH_RC4_128_SHA";
break;
case TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA:
return "TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA:
return "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA";
break;
case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA:
return "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA";
break;
case TLS_ECDH_RSA_WITH_NULL_SHA:
return "TLS_ECDH_RSA_WITH_NULL_SHA";
break;
case TLS_ECDH_RSA_WITH_RC4_128_SHA:
return "TLS_ECDH_RSA_WITH_RC4_128_SHA";
break;
case TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA:
return "TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA:
return "TLS_ECDH_RSA_WITH_AES_128_CBC_SHA";
break;
case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA:
return "TLS_ECDH_RSA_WITH_AES_256_CBC_SHA";
break;
case TLS_ECDHE_RSA_WITH_NULL_SHA:
return "TLS_ECDHE_RSA_WITH_NULL_SHA";
break;
case TLS_ECDHE_RSA_WITH_RC4_128_SHA:
return "TLS_ECDHE_RSA_WITH_RC4_128_SHA";
break;
case TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA:
return "TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA:
return "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA";
break;
case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA:
return "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA";
break;
case TLS_ECDH_anon_WITH_NULL_SHA:
return "TLS_ECDH_anon_WITH_NULL_SHA";
break;
case TLS_ECDH_anon_WITH_RC4_128_SHA:
return "TLS_ECDH_anon_WITH_RC4_128_SHA";
break;
case TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA:
return "TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_ECDH_anon_WITH_AES_128_CBC_SHA:
return "TLS_ECDH_anon_WITH_AES_128_CBC_SHA";
break;
case TLS_ECDH_anon_WITH_AES_256_CBC_SHA:
return "TLS_ECDH_anon_WITH_AES_256_CBC_SHA";
break;
#endif 
#if CURL_BUILD_MAC_10_8 || CURL_BUILD_IOS
case TLS_RSA_WITH_NULL_MD5:
return "TLS_RSA_WITH_NULL_MD5";
break;
case TLS_RSA_WITH_NULL_SHA:
return "TLS_RSA_WITH_NULL_SHA";
break;
case TLS_RSA_WITH_RC4_128_MD5:
return "TLS_RSA_WITH_RC4_128_MD5";
break;
case TLS_RSA_WITH_RC4_128_SHA:
return "TLS_RSA_WITH_RC4_128_SHA";
break;
case TLS_RSA_WITH_3DES_EDE_CBC_SHA:
return "TLS_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_RSA_WITH_NULL_SHA256:
return "TLS_RSA_WITH_NULL_SHA256";
break;
case TLS_RSA_WITH_AES_128_CBC_SHA256:
return "TLS_RSA_WITH_AES_128_CBC_SHA256";
break;
case TLS_RSA_WITH_AES_256_CBC_SHA256:
return "TLS_RSA_WITH_AES_256_CBC_SHA256";
break;
case TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA:
return "TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA:
return "TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
return "TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
return "TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_DH_DSS_WITH_AES_128_CBC_SHA256:
return "TLS_DH_DSS_WITH_AES_128_CBC_SHA256";
break;
case TLS_DH_RSA_WITH_AES_128_CBC_SHA256:
return "TLS_DH_RSA_WITH_AES_128_CBC_SHA256";
break;
case TLS_DHE_DSS_WITH_AES_128_CBC_SHA256:
return "TLS_DHE_DSS_WITH_AES_128_CBC_SHA256";
break;
case TLS_DHE_RSA_WITH_AES_128_CBC_SHA256:
return "TLS_DHE_RSA_WITH_AES_128_CBC_SHA256";
break;
case TLS_DH_DSS_WITH_AES_256_CBC_SHA256:
return "TLS_DH_DSS_WITH_AES_256_CBC_SHA256";
break;
case TLS_DH_RSA_WITH_AES_256_CBC_SHA256:
return "TLS_DH_RSA_WITH_AES_256_CBC_SHA256";
break;
case TLS_DHE_DSS_WITH_AES_256_CBC_SHA256:
return "TLS_DHE_DSS_WITH_AES_256_CBC_SHA256";
break;
case TLS_DHE_RSA_WITH_AES_256_CBC_SHA256:
return "TLS_DHE_RSA_WITH_AES_256_CBC_SHA256";
break;
case TLS_DH_anon_WITH_RC4_128_MD5:
return "TLS_DH_anon_WITH_RC4_128_MD5";
break;
case TLS_DH_anon_WITH_3DES_EDE_CBC_SHA:
return "TLS_DH_anon_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_DH_anon_WITH_AES_128_CBC_SHA256:
return "TLS_DH_anon_WITH_AES_128_CBC_SHA256";
break;
case TLS_DH_anon_WITH_AES_256_CBC_SHA256:
return "TLS_DH_anon_WITH_AES_256_CBC_SHA256";
break;
case TLS_RSA_WITH_AES_128_GCM_SHA256:
return "TLS_RSA_WITH_AES_128_GCM_SHA256";
break;
case TLS_RSA_WITH_AES_256_GCM_SHA384:
return "TLS_RSA_WITH_AES_256_GCM_SHA384";
break;
case TLS_DHE_RSA_WITH_AES_128_GCM_SHA256:
return "TLS_DHE_RSA_WITH_AES_128_GCM_SHA256";
break;
case TLS_DHE_RSA_WITH_AES_256_GCM_SHA384:
return "TLS_DHE_RSA_WITH_AES_256_GCM_SHA384";
break;
case TLS_DH_RSA_WITH_AES_128_GCM_SHA256:
return "TLS_DH_RSA_WITH_AES_128_GCM_SHA256";
break;
case TLS_DH_RSA_WITH_AES_256_GCM_SHA384:
return "TLS_DH_RSA_WITH_AES_256_GCM_SHA384";
break;
case TLS_DHE_DSS_WITH_AES_128_GCM_SHA256:
return "TLS_DHE_DSS_WITH_AES_128_GCM_SHA256";
break;
case TLS_DHE_DSS_WITH_AES_256_GCM_SHA384:
return "TLS_DHE_DSS_WITH_AES_256_GCM_SHA384";
break;
case TLS_DH_DSS_WITH_AES_128_GCM_SHA256:
return "TLS_DH_DSS_WITH_AES_128_GCM_SHA256";
break;
case TLS_DH_DSS_WITH_AES_256_GCM_SHA384:
return "TLS_DH_DSS_WITH_AES_256_GCM_SHA384";
break;
case TLS_DH_anon_WITH_AES_128_GCM_SHA256:
return "TLS_DH_anon_WITH_AES_128_GCM_SHA256";
break;
case TLS_DH_anon_WITH_AES_256_GCM_SHA384:
return "TLS_DH_anon_WITH_AES_256_GCM_SHA384";
break;
case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256:
return "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256";
break;
case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384:
return "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384";
break;
case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256:
return "TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256";
break;
case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384:
return "TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384";
break;
case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256:
return "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256";
break;
case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384:
return "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384";
break;
case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256:
return "TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256";
break;
case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384:
return "TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384";
break;
case TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:
return "TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256";
break;
case TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:
return "TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384";
break;
case TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256:
return "TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256";
break;
case TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384:
return "TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384";
break;
case TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256:
return "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256";
break;
case TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384:
return "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384";
break;
case TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256:
return "TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256";
break;
case TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384:
return "TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384";
break;
case TLS_EMPTY_RENEGOTIATION_INFO_SCSV:
return "TLS_EMPTY_RENEGOTIATION_INFO_SCSV";
break;
#else
case SSL_RSA_WITH_NULL_MD5:
return "TLS_RSA_WITH_NULL_MD5";
break;
case SSL_RSA_WITH_NULL_SHA:
return "TLS_RSA_WITH_NULL_SHA";
break;
case SSL_RSA_WITH_RC4_128_MD5:
return "TLS_RSA_WITH_RC4_128_MD5";
break;
case SSL_RSA_WITH_RC4_128_SHA:
return "TLS_RSA_WITH_RC4_128_SHA";
break;
case SSL_RSA_WITH_3DES_EDE_CBC_SHA:
return "TLS_RSA_WITH_3DES_EDE_CBC_SHA";
break;
case SSL_DH_anon_WITH_RC4_128_MD5:
return "TLS_DH_anon_WITH_RC4_128_MD5";
break;
case SSL_DH_anon_WITH_3DES_EDE_CBC_SHA:
return "TLS_DH_anon_WITH_3DES_EDE_CBC_SHA";
break;
#endif 
#if CURL_BUILD_MAC_10_9 || CURL_BUILD_IOS_7
case TLS_PSK_WITH_RC4_128_SHA:
return "TLS_PSK_WITH_RC4_128_SHA";
break;
case TLS_PSK_WITH_3DES_EDE_CBC_SHA:
return "TLS_PSK_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_PSK_WITH_AES_128_CBC_SHA:
return "TLS_PSK_WITH_AES_128_CBC_SHA";
break;
case TLS_PSK_WITH_AES_256_CBC_SHA:
return "TLS_PSK_WITH_AES_256_CBC_SHA";
break;
case TLS_DHE_PSK_WITH_RC4_128_SHA:
return "TLS_DHE_PSK_WITH_RC4_128_SHA";
break;
case TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA:
return "TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_DHE_PSK_WITH_AES_128_CBC_SHA:
return "TLS_DHE_PSK_WITH_AES_128_CBC_SHA";
break;
case TLS_DHE_PSK_WITH_AES_256_CBC_SHA:
return "TLS_DHE_PSK_WITH_AES_256_CBC_SHA";
break;
case TLS_RSA_PSK_WITH_RC4_128_SHA:
return "TLS_RSA_PSK_WITH_RC4_128_SHA";
break;
case TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA:
return "TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA";
break;
case TLS_RSA_PSK_WITH_AES_128_CBC_SHA:
return "TLS_RSA_PSK_WITH_AES_128_CBC_SHA";
break;
case TLS_RSA_PSK_WITH_AES_256_CBC_SHA:
return "TLS_RSA_PSK_WITH_AES_256_CBC_SHA";
break;
case TLS_PSK_WITH_NULL_SHA:
return "TLS_PSK_WITH_NULL_SHA";
break;
case TLS_DHE_PSK_WITH_NULL_SHA:
return "TLS_DHE_PSK_WITH_NULL_SHA";
break;
case TLS_RSA_PSK_WITH_NULL_SHA:
return "TLS_RSA_PSK_WITH_NULL_SHA";
break;
case TLS_PSK_WITH_AES_128_GCM_SHA256:
return "TLS_PSK_WITH_AES_128_GCM_SHA256";
break;
case TLS_PSK_WITH_AES_256_GCM_SHA384:
return "TLS_PSK_WITH_AES_256_GCM_SHA384";
break;
case TLS_DHE_PSK_WITH_AES_128_GCM_SHA256:
return "TLS_DHE_PSK_WITH_AES_128_GCM_SHA256";
break;
case TLS_DHE_PSK_WITH_AES_256_GCM_SHA384:
return "TLS_DHE_PSK_WITH_AES_256_GCM_SHA384";
break;
case TLS_RSA_PSK_WITH_AES_128_GCM_SHA256:
return "TLS_RSA_PSK_WITH_AES_128_GCM_SHA256";
break;
case TLS_RSA_PSK_WITH_AES_256_GCM_SHA384:
return "TLS_PSK_WITH_AES_256_GCM_SHA384";
break;
case TLS_PSK_WITH_AES_128_CBC_SHA256:
return "TLS_PSK_WITH_AES_128_CBC_SHA256";
break;
case TLS_PSK_WITH_AES_256_CBC_SHA384:
return "TLS_PSK_WITH_AES_256_CBC_SHA384";
break;
case TLS_PSK_WITH_NULL_SHA256:
return "TLS_PSK_WITH_NULL_SHA256";
break;
case TLS_PSK_WITH_NULL_SHA384:
return "TLS_PSK_WITH_NULL_SHA384";
break;
case TLS_DHE_PSK_WITH_AES_128_CBC_SHA256:
return "TLS_DHE_PSK_WITH_AES_128_CBC_SHA256";
break;
case TLS_DHE_PSK_WITH_AES_256_CBC_SHA384:
return "TLS_DHE_PSK_WITH_AES_256_CBC_SHA384";
break;
case TLS_DHE_PSK_WITH_NULL_SHA256:
return "TLS_DHE_PSK_WITH_NULL_SHA256";
break;
case TLS_DHE_PSK_WITH_NULL_SHA384:
return "TLS_RSA_PSK_WITH_NULL_SHA384";
break;
case TLS_RSA_PSK_WITH_AES_128_CBC_SHA256:
return "TLS_RSA_PSK_WITH_AES_128_CBC_SHA256";
break;
case TLS_RSA_PSK_WITH_AES_256_CBC_SHA384:
return "TLS_RSA_PSK_WITH_AES_256_CBC_SHA384";
break;
case TLS_RSA_PSK_WITH_NULL_SHA256:
return "TLS_RSA_PSK_WITH_NULL_SHA256";
break;
case TLS_RSA_PSK_WITH_NULL_SHA384:
return "TLS_RSA_PSK_WITH_NULL_SHA384";
break;
#endif 
#if CURL_BUILD_MAC_10_13 || CURL_BUILD_IOS_11
case TLS_AES_128_GCM_SHA256:
return "TLS_AES_128_GCM_SHA256";
break;
case TLS_AES_256_GCM_SHA384:
return "TLS_AES_256_GCM_SHA384";
break;
case TLS_CHACHA20_POLY1305_SHA256:
return "TLS_CHACHA20_POLY1305_SHA256";
break;
case TLS_AES_128_CCM_SHA256:
return "TLS_AES_128_CCM_SHA256";
break;
case TLS_AES_128_CCM_8_SHA256:
return "TLS_AES_128_CCM_8_SHA256";
break;
case TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256:
return "TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256";
break;
case TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256:
return "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256";
break;
#endif 
}
return "TLS_NULL_WITH_NULL_NULL";
}
#endif 
#if CURL_BUILD_MAC
CF_INLINE void GetDarwinVersionNumber(int *major, int *minor)
{
int mib[2];
char *os_version;
size_t os_version_len;
char *os_version_major, *os_version_minor;
char *tok_buf;
mib[0] = CTL_KERN;
mib[1] = KERN_OSRELEASE;
if(sysctl(mib, 2, NULL, &os_version_len, NULL, 0) == -1)
return;
os_version = malloc(os_version_len*sizeof(char));
if(!os_version)
return;
if(sysctl(mib, 2, os_version, &os_version_len, NULL, 0) == -1) {
free(os_version);
return;
}
os_version_major = strtok_r(os_version, ".", &tok_buf);
os_version_minor = strtok_r(NULL, ".", &tok_buf);
*major = atoi(os_version_major);
*minor = atoi(os_version_minor);
free(os_version);
}
#endif 
CF_INLINE CFStringRef getsubject(SecCertificateRef cert)
{
CFStringRef server_cert_summary = CFSTR("(null)");
#if CURL_BUILD_IOS
server_cert_summary = SecCertificateCopySubjectSummary(cert);
#else
#if CURL_BUILD_MAC_10_7
if(SecCertificateCopyLongDescription != NULL)
server_cert_summary =
SecCertificateCopyLongDescription(NULL, cert, NULL);
else
#endif 
#if CURL_BUILD_MAC_10_6
if(SecCertificateCopySubjectSummary != NULL)
server_cert_summary = SecCertificateCopySubjectSummary(cert);
else
#endif 
(void)SecCertificateCopyCommonName(cert, &server_cert_summary);
#endif 
return server_cert_summary;
}
static CURLcode CopyCertSubject(struct Curl_easy *data,
SecCertificateRef cert, char **certp)
{
CFStringRef c = getsubject(cert);
CURLcode result = CURLE_OK;
const char *direct;
char *cbuf = NULL;
*certp = NULL;
if(!c) {
failf(data, "SSL: invalid CA certificate subject");
return CURLE_PEER_FAILED_VERIFICATION;
}
direct = CFStringGetCStringPtr(c, kCFStringEncodingUTF8);
if(direct) {
*certp = strdup(direct);
if(!*certp) {
failf(data, "SSL: out of memory");
result = CURLE_OUT_OF_MEMORY;
}
}
else {
size_t cbuf_size = ((size_t)CFStringGetLength(c) * 4) + 1;
cbuf = calloc(cbuf_size, 1);
if(cbuf) {
if(!CFStringGetCString(c, cbuf, cbuf_size,
kCFStringEncodingUTF8)) {
failf(data, "SSL: invalid CA certificate subject");
result = CURLE_PEER_FAILED_VERIFICATION;
}
else
*certp = cbuf;
}
else {
failf(data, "SSL: couldn't allocate %zu bytes of memory", cbuf_size);
result = CURLE_OUT_OF_MEMORY;
}
}
if(result)
free(cbuf);
CFRelease(c);
return result;
}
#if CURL_SUPPORT_MAC_10_6
static OSStatus CopyIdentityWithLabelOldSchool(char *label,
SecIdentityRef *out_c_a_k)
{
OSStatus status = errSecItemNotFound;
SecKeychainAttributeList attr_list;
SecKeychainAttribute attr;
SecKeychainSearchRef search = NULL;
SecCertificateRef cert = NULL;
attr_list.count = 1L;
attr_list.attr = &attr;
attr.tag = kSecLabelItemAttr;
attr.data = label;
attr.length = (UInt32)strlen(label);
status = SecKeychainSearchCreateFromAttributes(NULL,
kSecCertificateItemClass,
&attr_list,
&search);
if(status == noErr) {
status = SecKeychainSearchCopyNext(search,
(SecKeychainItemRef *)&cert);
if(status == noErr && cert) {
status = SecIdentityCreateWithCertificate(NULL, cert, out_c_a_k);
CFRelease(cert);
}
}
if(search)
CFRelease(search);
return status;
}
#endif 
static OSStatus CopyIdentityWithLabel(char *label,
SecIdentityRef *out_cert_and_key)
{
OSStatus status = errSecItemNotFound;
#if CURL_BUILD_MAC_10_7 || CURL_BUILD_IOS
CFArrayRef keys_list;
CFIndex keys_list_count;
CFIndex i;
CFStringRef common_name;
if(SecItemCopyMatching != NULL && kSecClassIdentity != NULL) {
CFTypeRef keys[5];
CFTypeRef values[5];
CFDictionaryRef query_dict;
CFStringRef label_cf = CFStringCreateWithCString(NULL, label,
kCFStringEncodingUTF8);
values[0] = kSecClassIdentity; 
keys[0] = kSecClass;
values[1] = kCFBooleanTrue; 
keys[1] = kSecReturnRef;
values[2] = kSecMatchLimitAll; 
keys[2] = kSecMatchLimit;
values[3] = SecPolicyCreateSSL(false, NULL);
keys[3] = kSecMatchPolicy;
values[4] = label_cf;
keys[4] = kSecAttrLabel;
query_dict = CFDictionaryCreate(NULL, (const void **)keys,
(const void **)values, 5L,
&kCFCopyStringDictionaryKeyCallBacks,
&kCFTypeDictionaryValueCallBacks);
CFRelease(values[3]);
status = SecItemCopyMatching(query_dict, (CFTypeRef *) &keys_list);
if(status == noErr) {
keys_list_count = CFArrayGetCount(keys_list);
*out_cert_and_key = NULL;
status = 1;
for(i = 0; i<keys_list_count; i++) {
OSStatus err = noErr;
SecCertificateRef cert = NULL;
SecIdentityRef identity =
(SecIdentityRef) CFArrayGetValueAtIndex(keys_list, i);
err = SecIdentityCopyCertificate(identity, &cert);
if(err == noErr) {
#if CURL_BUILD_IOS
common_name = SecCertificateCopySubjectSummary(cert);
#elif CURL_BUILD_MAC_10_7
SecCertificateCopyCommonName(cert, &common_name);
#endif
if(CFStringCompare(common_name, label_cf, 0) == kCFCompareEqualTo) {
CFRelease(cert);
CFRelease(common_name);
CFRetain(identity);
*out_cert_and_key = identity;
status = noErr;
break;
}
CFRelease(common_name);
}
CFRelease(cert);
}
}
if(keys_list)
CFRelease(keys_list);
CFRelease(query_dict);
CFRelease(label_cf);
}
else {
#if CURL_SUPPORT_MAC_10_6
status = CopyIdentityWithLabelOldSchool(label, out_cert_and_key);
#endif 
}
#elif CURL_SUPPORT_MAC_10_6
status = CopyIdentityWithLabelOldSchool(label, out_cert_and_key);
#endif 
return status;
}
static OSStatus CopyIdentityFromPKCS12File(const char *cPath,
const char *cPassword,
SecIdentityRef *out_cert_and_key)
{
OSStatus status = errSecItemNotFound;
CFURLRef pkcs_url = CFURLCreateFromFileSystemRepresentation(NULL,
(const UInt8 *)cPath, strlen(cPath), false);
CFStringRef password = cPassword ? CFStringCreateWithCString(NULL,
cPassword, kCFStringEncodingUTF8) : NULL;
CFDataRef pkcs_data = NULL;
#if CURL_BUILD_MAC_10_7 || CURL_BUILD_IOS
if(CFURLCreateDataAndPropertiesFromResource(NULL, pkcs_url, &pkcs_data,
NULL, NULL, &status)) {
CFArrayRef items = NULL;
#if CURL_BUILD_IOS
const void *cKeys[] = {kSecImportExportPassphrase};
const void *cValues[] = {password};
CFDictionaryRef options = CFDictionaryCreate(NULL, cKeys, cValues,
password ? 1L : 0L, NULL, NULL);
if(options != NULL) {
status = SecPKCS12Import(pkcs_data, options, &items);
CFRelease(options);
}
#elif CURL_BUILD_MAC_10_7
SecItemImportExportKeyParameters keyParams;
SecExternalFormat inputFormat = kSecFormatPKCS12;
SecExternalItemType inputType = kSecItemTypeCertificate;
memset(&keyParams, 0x00, sizeof(keyParams));
keyParams.version = SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;
keyParams.passphrase = password;
status = SecItemImport(pkcs_data, NULL, &inputFormat, &inputType,
0, &keyParams, NULL, &items);
#endif
if(status == errSecSuccess && items && CFArrayGetCount(items)) {
CFIndex i, count;
count = CFArrayGetCount(items);
for(i = 0; i < count; i++) {
CFTypeRef item = (CFTypeRef) CFArrayGetValueAtIndex(items, i);
CFTypeID itemID = CFGetTypeID(item);
if(itemID == CFDictionaryGetTypeID()) {
CFTypeRef identity = (CFTypeRef) CFDictionaryGetValue(
(CFDictionaryRef) item,
kSecImportItemIdentity);
CFRetain(identity);
*out_cert_and_key = (SecIdentityRef) identity;
break;
}
#if CURL_BUILD_MAC_10_7
else if(itemID == SecCertificateGetTypeID()) {
status = SecIdentityCreateWithCertificate(NULL,
(SecCertificateRef) item,
out_cert_and_key);
break;
}
#endif
}
}
if(items)
CFRelease(items);
CFRelease(pkcs_data);
}
#endif 
if(password)
CFRelease(password);
CFRelease(pkcs_url);
return status;
}
CF_INLINE bool is_file(const char *filename)
{
struct_stat st;
if(filename == NULL)
return false;
if(stat(filename, &st) == 0)
return S_ISREG(st.st_mode);
return false;
}
#if CURL_BUILD_MAC_10_8 || CURL_BUILD_IOS
static CURLcode sectransp_version_from_curl(SSLProtocol *darwinver,
long ssl_version)
{
switch(ssl_version) {
case CURL_SSLVERSION_TLSv1_0:
*darwinver = kTLSProtocol1;
return CURLE_OK;
case CURL_SSLVERSION_TLSv1_1:
*darwinver = kTLSProtocol11;
return CURLE_OK;
case CURL_SSLVERSION_TLSv1_2:
*darwinver = kTLSProtocol12;
return CURLE_OK;
case CURL_SSLVERSION_TLSv1_3:
#if (CURL_BUILD_MAC_10_13 || CURL_BUILD_IOS_11) && HAVE_BUILTIN_AVAILABLE == 1
if(__builtin_available(macOS 10.13, iOS 11.0, *)) {
*darwinver = kTLSProtocol13;
return CURLE_OK;
}
#endif 
break;
}
return CURLE_SSL_CONNECT_ERROR;
}
#endif
static CURLcode
set_ssl_version_min_max(struct connectdata *conn, int sockindex)
{
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct ssl_backend_data *backend = connssl->backend;
long ssl_version = SSL_CONN_CONFIG(version);
long ssl_version_max = SSL_CONN_CONFIG(version_max);
long max_supported_version_by_os;
#if (CURL_BUILD_MAC_10_13 || CURL_BUILD_IOS_11) && HAVE_BUILTIN_AVAILABLE == 1
if(__builtin_available(macOS 10.13, iOS 11.0, *)) {
max_supported_version_by_os = CURL_SSLVERSION_MAX_TLSv1_3;
}
else {
max_supported_version_by_os = CURL_SSLVERSION_MAX_TLSv1_2;
}
#else
max_supported_version_by_os = CURL_SSLVERSION_MAX_TLSv1_2;
#endif 
switch(ssl_version) {
case CURL_SSLVERSION_DEFAULT:
case CURL_SSLVERSION_TLSv1:
ssl_version = CURL_SSLVERSION_TLSv1_0;
break;
}
switch(ssl_version_max) {
case CURL_SSLVERSION_MAX_NONE:
case CURL_SSLVERSION_MAX_DEFAULT:
ssl_version_max = max_supported_version_by_os;
break;
}
#if CURL_BUILD_MAC_10_8 || CURL_BUILD_IOS
if(SSLSetProtocolVersionMax != NULL) {
SSLProtocol darwin_ver_min = kTLSProtocol1;
SSLProtocol darwin_ver_max = kTLSProtocol1;
CURLcode result = sectransp_version_from_curl(&darwin_ver_min,
ssl_version);
if(result) {
failf(data, "unsupported min version passed via CURLOPT_SSLVERSION");
return result;
}
result = sectransp_version_from_curl(&darwin_ver_max,
ssl_version_max >> 16);
if(result) {
failf(data, "unsupported max version passed via CURLOPT_SSLVERSION");
return result;
}
(void)SSLSetProtocolVersionMin(backend->ssl_ctx, darwin_ver_min);
(void)SSLSetProtocolVersionMax(backend->ssl_ctx, darwin_ver_max);
return result;
}
else {
#if CURL_SUPPORT_MAC_10_8
long i = ssl_version;
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kSSLProtocolAll,
false);
for(; i <= (ssl_version_max >> 16); i++) {
switch(i) {
case CURL_SSLVERSION_TLSv1_0:
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kTLSProtocol1,
true);
break;
case CURL_SSLVERSION_TLSv1_1:
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kTLSProtocol11,
true);
break;
case CURL_SSLVERSION_TLSv1_2:
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kTLSProtocol12,
true);
break;
case CURL_SSLVERSION_TLSv1_3:
failf(data, "Your version of the OS does not support TLSv1.3");
return CURLE_SSL_CONNECT_ERROR;
}
}
return CURLE_OK;
#endif 
}
#endif 
failf(data, "Secure Transport: cannot set SSL protocol");
return CURLE_SSL_CONNECT_ERROR;
}
static CURLcode sectransp_connect_step1(struct connectdata *conn,
int sockindex)
{
struct Curl_easy *data = conn->data;
curl_socket_t sockfd = conn->sock[sockindex];
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct ssl_backend_data *backend = connssl->backend;
const char * const ssl_cafile = SSL_CONN_CONFIG(CAfile);
const bool verifypeer = SSL_CONN_CONFIG(verifypeer);
char * const ssl_cert = SSL_SET_OPTION(cert);
const char * const hostname = SSL_IS_PROXY() ? conn->http_proxy.host.name :
conn->host.name;
const long int port = SSL_IS_PROXY() ? conn->port : conn->remote_port;
#if defined(ENABLE_IPV6)
struct in6_addr addr;
#else
struct in_addr addr;
#endif 
size_t all_ciphers_count = 0UL, allowed_ciphers_count = 0UL, i;
SSLCipherSuite *all_ciphers = NULL, *allowed_ciphers = NULL;
OSStatus err = noErr;
#if CURL_BUILD_MAC
int darwinver_maj = 0, darwinver_min = 0;
GetDarwinVersionNumber(&darwinver_maj, &darwinver_min);
#endif 
#if CURL_BUILD_MAC_10_8 || CURL_BUILD_IOS
if(SSLCreateContext != NULL) { 
if(backend->ssl_ctx)
CFRelease(backend->ssl_ctx);
backend->ssl_ctx = SSLCreateContext(NULL, kSSLClientSide, kSSLStreamType);
if(!backend->ssl_ctx) {
failf(data, "SSL: couldn't create a context!");
return CURLE_OUT_OF_MEMORY;
}
}
else {
#if CURL_SUPPORT_MAC_10_8
if(backend->ssl_ctx)
(void)SSLDisposeContext(backend->ssl_ctx);
err = SSLNewContext(false, &(backend->ssl_ctx));
if(err != noErr) {
failf(data, "SSL: couldn't create a context: OSStatus %d", err);
return CURLE_OUT_OF_MEMORY;
}
#endif 
}
#else
if(backend->ssl_ctx)
(void)SSLDisposeContext(backend->ssl_ctx);
err = SSLNewContext(false, &(backend->ssl_ctx));
if(err != noErr) {
failf(data, "SSL: couldn't create a context: OSStatus %d", err);
return CURLE_OUT_OF_MEMORY;
}
#endif 
backend->ssl_write_buffered_length = 0UL; 
#if CURL_BUILD_MAC_10_8 || CURL_BUILD_IOS
if(SSLSetProtocolVersionMax != NULL) {
switch(conn->ssl_config.version) {
case CURL_SSLVERSION_TLSv1:
(void)SSLSetProtocolVersionMin(backend->ssl_ctx, kTLSProtocol1);
#if (CURL_BUILD_MAC_10_13 || CURL_BUILD_IOS_11) && HAVE_BUILTIN_AVAILABLE == 1
if(__builtin_available(macOS 10.13, iOS 11.0, *)) {
(void)SSLSetProtocolVersionMax(backend->ssl_ctx, kTLSProtocol13);
}
else {
(void)SSLSetProtocolVersionMax(backend->ssl_ctx, kTLSProtocol12);
}
#else
(void)SSLSetProtocolVersionMax(backend->ssl_ctx, kTLSProtocol12);
#endif 
break;
case CURL_SSLVERSION_DEFAULT:
case CURL_SSLVERSION_TLSv1_0:
case CURL_SSLVERSION_TLSv1_1:
case CURL_SSLVERSION_TLSv1_2:
case CURL_SSLVERSION_TLSv1_3:
{
CURLcode result = set_ssl_version_min_max(conn, sockindex);
if(result != CURLE_OK)
return result;
break;
}
case CURL_SSLVERSION_SSLv3:
err = SSLSetProtocolVersionMin(backend->ssl_ctx, kSSLProtocol3);
if(err != noErr) {
failf(data, "Your version of the OS does not support SSLv3");
return CURLE_SSL_CONNECT_ERROR;
}
(void)SSLSetProtocolVersionMax(backend->ssl_ctx, kSSLProtocol3);
break;
case CURL_SSLVERSION_SSLv2:
err = SSLSetProtocolVersionMin(backend->ssl_ctx, kSSLProtocol2);
if(err != noErr) {
failf(data, "Your version of the OS does not support SSLv2");
return CURLE_SSL_CONNECT_ERROR;
}
(void)SSLSetProtocolVersionMax(backend->ssl_ctx, kSSLProtocol2);
break;
default:
failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
return CURLE_SSL_CONNECT_ERROR;
}
}
else {
#if CURL_SUPPORT_MAC_10_8
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kSSLProtocolAll,
false);
switch(conn->ssl_config.version) {
case CURL_SSLVERSION_DEFAULT:
case CURL_SSLVERSION_TLSv1:
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kTLSProtocol1,
true);
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kTLSProtocol11,
true);
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kTLSProtocol12,
true);
break;
case CURL_SSLVERSION_TLSv1_0:
case CURL_SSLVERSION_TLSv1_1:
case CURL_SSLVERSION_TLSv1_2:
case CURL_SSLVERSION_TLSv1_3:
{
CURLcode result = set_ssl_version_min_max(conn, sockindex);
if(result != CURLE_OK)
return result;
break;
}
case CURL_SSLVERSION_SSLv3:
err = SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kSSLProtocol3,
true);
if(err != noErr) {
failf(data, "Your version of the OS does not support SSLv3");
return CURLE_SSL_CONNECT_ERROR;
}
break;
case CURL_SSLVERSION_SSLv2:
err = SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kSSLProtocol2,
true);
if(err != noErr) {
failf(data, "Your version of the OS does not support SSLv2");
return CURLE_SSL_CONNECT_ERROR;
}
break;
default:
failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
return CURLE_SSL_CONNECT_ERROR;
}
#endif 
}
#else
if(conn->ssl_config.version_max != CURL_SSLVERSION_MAX_NONE) {
failf(data, "Your version of the OS does not support to set maximum"
" SSL/TLS version");
return CURLE_SSL_CONNECT_ERROR;
}
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx, kSSLProtocolAll, false);
switch(conn->ssl_config.version) {
case CURL_SSLVERSION_DEFAULT:
case CURL_SSLVERSION_TLSv1:
case CURL_SSLVERSION_TLSv1_0:
(void)SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kTLSProtocol1,
true);
break;
case CURL_SSLVERSION_TLSv1_1:
failf(data, "Your version of the OS does not support TLSv1.1");
return CURLE_SSL_CONNECT_ERROR;
case CURL_SSLVERSION_TLSv1_2:
failf(data, "Your version of the OS does not support TLSv1.2");
return CURLE_SSL_CONNECT_ERROR;
case CURL_SSLVERSION_TLSv1_3:
failf(data, "Your version of the OS does not support TLSv1.3");
return CURLE_SSL_CONNECT_ERROR;
case CURL_SSLVERSION_SSLv2:
err = SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kSSLProtocol2,
true);
if(err != noErr) {
failf(data, "Your version of the OS does not support SSLv2");
return CURLE_SSL_CONNECT_ERROR;
}
break;
case CURL_SSLVERSION_SSLv3:
err = SSLSetProtocolVersionEnabled(backend->ssl_ctx,
kSSLProtocol3,
true);
if(err != noErr) {
failf(data, "Your version of the OS does not support SSLv3");
return CURLE_SSL_CONNECT_ERROR;
}
break;
default:
failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
return CURLE_SSL_CONNECT_ERROR;
}
#endif 
#if (CURL_BUILD_MAC_10_13 || CURL_BUILD_IOS_11) && HAVE_BUILTIN_AVAILABLE == 1
if(conn->bits.tls_enable_alpn) {
if(__builtin_available(macOS 10.13.4, iOS 11, tvOS 11, *)) {
CFMutableArrayRef alpnArr = CFArrayCreateMutable(NULL, 0,
&kCFTypeArrayCallBacks);
#if defined(USE_NGHTTP2)
if(data->set.httpversion >= CURL_HTTP_VERSION_2 &&
(!SSL_IS_PROXY() || !conn->bits.tunnel_proxy)) {
CFArrayAppendValue(alpnArr, CFSTR(NGHTTP2_PROTO_VERSION_ID));
infof(data, "ALPN, offering %s\n", NGHTTP2_PROTO_VERSION_ID);
}
#endif
CFArrayAppendValue(alpnArr, CFSTR(ALPN_HTTP_1_1));
infof(data, "ALPN, offering %s\n", ALPN_HTTP_1_1);
err = SSLSetALPNProtocols(backend->ssl_ctx, alpnArr);
if(err != noErr)
infof(data, "WARNING: failed to set ALPN protocols; OSStatus %d\n",
err);
CFRelease(alpnArr);
}
}
#endif
if(SSL_SET_OPTION(key)) {
infof(data, "WARNING: SSL: CURLOPT_SSLKEY is ignored by Secure "
"Transport. The private key must be in the Keychain.\n");
}
if(ssl_cert) {
SecIdentityRef cert_and_key = NULL;
bool is_cert_file = is_file(ssl_cert);
if(is_cert_file) {
if(!SSL_SET_OPTION(cert_type))
infof(data, "WARNING: SSL: Certificate type not set, assuming "
"PKCS#12 format.\n");
else if(strncmp(SSL_SET_OPTION(cert_type), "P12",
strlen(SSL_SET_OPTION(cert_type))) != 0)
infof(data, "WARNING: SSL: The Security framework only supports "
"loading identities that are in PKCS#12 format.\n");
err = CopyIdentityFromPKCS12File(ssl_cert,
SSL_SET_OPTION(key_passwd), &cert_and_key);
}
else
err = CopyIdentityWithLabel(ssl_cert, &cert_and_key);
if(err == noErr && cert_and_key) {
SecCertificateRef cert = NULL;
CFTypeRef certs_c[1];
CFArrayRef certs;
err = SecIdentityCopyCertificate(cert_and_key, &cert);
if(err == noErr) {
char *certp;
CURLcode result = CopyCertSubject(data, cert, &certp);
if(!result) {
infof(data, "Client certificate: %s\n", certp);
free(certp);
}
CFRelease(cert);
if(result == CURLE_PEER_FAILED_VERIFICATION)
return CURLE_SSL_CERTPROBLEM;
if(result)
return result;
}
certs_c[0] = cert_and_key;
certs = CFArrayCreate(NULL, (const void **)certs_c, 1L,
&kCFTypeArrayCallBacks);
err = SSLSetCertificate(backend->ssl_ctx, certs);
if(certs)
CFRelease(certs);
if(err != noErr) {
failf(data, "SSL: SSLSetCertificate() failed: OSStatus %d", err);
return CURLE_SSL_CERTPROBLEM;
}
CFRelease(cert_and_key);
}
else {
switch(err) {
case errSecAuthFailed: case -25264: 
failf(data, "SSL: Incorrect password for the certificate \"%s\" "
"and its private key.", ssl_cert);
break;
case -26275: case -25257: 
failf(data, "SSL: Couldn't make sense of the data in the "
"certificate \"%s\" and its private key.",
ssl_cert);
break;
case -25260: 
failf(data, "SSL The certificate \"%s\" requires a password.",
ssl_cert);
break;
case errSecItemNotFound:
failf(data, "SSL: Can't find the certificate \"%s\" and its private "
"key in the Keychain.", ssl_cert);
break;
default:
failf(data, "SSL: Can't load the certificate \"%s\" and its private "
"key: OSStatus %d", ssl_cert, err);
break;
}
return CURLE_SSL_CERTPROBLEM;
}
}
#if CURL_BUILD_MAC_10_6 || CURL_BUILD_IOS
#if CURL_BUILD_MAC
if(SSLSetSessionOption != NULL && darwinver_maj >= 13) {
#else
if(SSLSetSessionOption != NULL) {
#endif 
bool break_on_auth = !conn->ssl_config.verifypeer || ssl_cafile;
err = SSLSetSessionOption(backend->ssl_ctx,
kSSLSessionOptionBreakOnServerAuth,
break_on_auth);
if(err != noErr) {
failf(data, "SSL: SSLSetSessionOption() failed: OSStatus %d", err);
return CURLE_SSL_CONNECT_ERROR;
}
}
else {
#if CURL_SUPPORT_MAC_10_8
err = SSLSetEnableCertVerify(backend->ssl_ctx,
conn->ssl_config.verifypeer?true:false);
if(err != noErr) {
failf(data, "SSL: SSLSetEnableCertVerify() failed: OSStatus %d", err);
return CURLE_SSL_CONNECT_ERROR;
}
#endif 
}
#else
err = SSLSetEnableCertVerify(backend->ssl_ctx,
conn->ssl_config.verifypeer?true:false);
if(err != noErr) {
failf(data, "SSL: SSLSetEnableCertVerify() failed: OSStatus %d", err);
return CURLE_SSL_CONNECT_ERROR;
}
#endif 
if(ssl_cafile && verifypeer) {
bool is_cert_file = is_file(ssl_cafile);
if(!is_cert_file) {
failf(data, "SSL: can't load CA certificate file %s", ssl_cafile);
return CURLE_SSL_CACERT_BADFILE;
}
}
if(conn->ssl_config.verifyhost) {
err = SSLSetPeerDomainName(backend->ssl_ctx, hostname,
strlen(hostname));
if(err != noErr) {
infof(data, "WARNING: SSL: SSLSetPeerDomainName() failed: OSStatus %d\n",
err);
}
if((Curl_inet_pton(AF_INET, hostname, &addr))
#if defined(ENABLE_IPV6)
|| (Curl_inet_pton(AF_INET6, hostname, &addr))
#endif
) {
infof(data, "WARNING: using IP address, SNI is being disabled by "
"the OS.\n");
}
}
else {
infof(data, "WARNING: disabling hostname validation also disables SNI.\n");
}
err = SSLGetNumberSupportedCiphers(backend->ssl_ctx, &all_ciphers_count);
if(err != noErr) {
failf(data, "SSL: SSLGetNumberSupportedCiphers() failed: OSStatus %d",
err);
return CURLE_SSL_CIPHER;
}
all_ciphers = malloc(all_ciphers_count*sizeof(SSLCipherSuite));
if(!all_ciphers) {
failf(data, "SSL: Failed to allocate memory for all ciphers");
return CURLE_OUT_OF_MEMORY;
}
allowed_ciphers = malloc(all_ciphers_count*sizeof(SSLCipherSuite));
if(!allowed_ciphers) {
Curl_safefree(all_ciphers);
failf(data, "SSL: Failed to allocate memory for allowed ciphers");
return CURLE_OUT_OF_MEMORY;
}
err = SSLGetSupportedCiphers(backend->ssl_ctx, all_ciphers,
&all_ciphers_count);
if(err != noErr) {
Curl_safefree(all_ciphers);
Curl_safefree(allowed_ciphers);
return CURLE_SSL_CIPHER;
}
for(i = 0UL ; i < all_ciphers_count ; i++) {
#if CURL_BUILD_MAC
if(darwinver_maj == 12 && darwinver_min <= 3 &&
all_ciphers[i] >= 0xC001 && all_ciphers[i] <= 0xC032) {
continue;
}
#endif 
switch(all_ciphers[i]) {
case SSL_NULL_WITH_NULL_NULL:
case SSL_RSA_WITH_NULL_MD5:
case SSL_RSA_WITH_NULL_SHA:
case 0x003B: 
case SSL_FORTEZZA_DMS_WITH_NULL_SHA:
case 0xC001: 
case 0xC006: 
case 0xC00B: 
case 0xC010: 
case 0x002C: 
case 0x002D: 
case 0x002E: 
case 0x00B0: 
case 0x00B1: 
case 0x00B4: 
case 0x00B5: 
case 0x00B8: 
case 0x00B9: 
case SSL_DH_anon_EXPORT_WITH_RC4_40_MD5:
case SSL_DH_anon_WITH_RC4_128_MD5:
case SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA:
case SSL_DH_anon_WITH_DES_CBC_SHA:
case SSL_DH_anon_WITH_3DES_EDE_CBC_SHA:
case TLS_DH_anon_WITH_AES_128_CBC_SHA:
case TLS_DH_anon_WITH_AES_256_CBC_SHA:
case 0xC015: 
case 0xC016: 
case 0xC017: 
case 0xC018: 
case 0xC019: 
case 0x006C: 
case 0x006D: 
case 0x00A6: 
case 0x00A7: 
case SSL_RSA_EXPORT_WITH_RC4_40_MD5:
case SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5:
case SSL_RSA_EXPORT_WITH_DES40_CBC_SHA:
case SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA:
case SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA:
case SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA:
case SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA:
case SSL_RSA_WITH_DES_CBC_SHA:
case SSL_DH_DSS_WITH_DES_CBC_SHA:
case SSL_DH_RSA_WITH_DES_CBC_SHA:
case SSL_DHE_DSS_WITH_DES_CBC_SHA:
case SSL_DHE_RSA_WITH_DES_CBC_SHA:
case SSL_RSA_WITH_IDEA_CBC_SHA:
case SSL_RSA_WITH_IDEA_CBC_MD5:
case SSL_RSA_WITH_RC4_128_MD5:
case SSL_RSA_WITH_RC4_128_SHA:
case 0xC002: 
case 0xC007: 
case 0xC00C: 
case 0xC011: 
case 0x008A: 
case 0x008E: 
case 0x0092: 
break;
default: 
allowed_ciphers[allowed_ciphers_count++] = all_ciphers[i];
break;
}
}
err = SSLSetEnabledCiphers(backend->ssl_ctx, allowed_ciphers,
allowed_ciphers_count);
Curl_safefree(all_ciphers);
Curl_safefree(allowed_ciphers);
if(err != noErr) {
failf(data, "SSL: SSLSetEnabledCiphers() failed: OSStatus %d", err);
return CURLE_SSL_CIPHER;
}
#if CURL_BUILD_MAC_10_9 || CURL_BUILD_IOS_7
if(SSLSetSessionOption != NULL) {
SSLSetSessionOption(backend->ssl_ctx, kSSLSessionOptionSendOneByteRecord,
!data->set.ssl.enable_beast);
SSLSetSessionOption(backend->ssl_ctx, kSSLSessionOptionFalseStart,
data->set.ssl.falsestart); 
}
#endif 
if(SSL_SET_OPTION(primary.sessionid)) {
char *ssl_sessionid;
size_t ssl_sessionid_len;
Curl_ssl_sessionid_lock(conn);
if(!Curl_ssl_getsessionid(conn, (void **)&ssl_sessionid,
&ssl_sessionid_len, sockindex)) {
err = SSLSetPeerID(backend->ssl_ctx, ssl_sessionid, ssl_sessionid_len);
Curl_ssl_sessionid_unlock(conn);
if(err != noErr) {
failf(data, "SSL: SSLSetPeerID() failed: OSStatus %d", err);
return CURLE_SSL_CONNECT_ERROR;
}
infof(data, "SSL re-using session ID\n");
}
else {
CURLcode result;
ssl_sessionid =
aprintf("%s:%d:%d:%s:%hu", ssl_cafile,
verifypeer, SSL_CONN_CONFIG(verifyhost), hostname, port);
ssl_sessionid_len = strlen(ssl_sessionid);
err = SSLSetPeerID(backend->ssl_ctx, ssl_sessionid, ssl_sessionid_len);
if(err != noErr) {
Curl_ssl_sessionid_unlock(conn);
failf(data, "SSL: SSLSetPeerID() failed: OSStatus %d", err);
return CURLE_SSL_CONNECT_ERROR;
}
result = Curl_ssl_addsessionid(conn, ssl_sessionid, ssl_sessionid_len,
sockindex);
Curl_ssl_sessionid_unlock(conn);
if(result) {
failf(data, "failed to store ssl session");
return result;
}
}
}
err = SSLSetIOFuncs(backend->ssl_ctx, SocketRead, SocketWrite);
if(err != noErr) {
failf(data, "SSL: SSLSetIOFuncs() failed: OSStatus %d", err);
return CURLE_SSL_CONNECT_ERROR;
}
backend->ssl_sockfd = sockfd;
err = SSLSetConnection(backend->ssl_ctx, connssl);
if(err != noErr) {
failf(data, "SSL: SSLSetConnection() failed: %d", err);
return CURLE_SSL_CONNECT_ERROR;
}
connssl->connecting_state = ssl_connect_2;
return CURLE_OK;
}
static long pem_to_der(const char *in, unsigned char **out, size_t *outlen)
{
char *sep_start, *sep_end, *cert_start, *cert_end;
size_t i, j, err;
size_t len;
unsigned char *b64;
sep_start = strstr(in, "-----");
if(sep_start == NULL)
return 0;
cert_start = strstr(sep_start + 1, "-----");
if(cert_start == NULL)
return -1;
cert_start += 5;
cert_end = strstr(cert_start, "-----");
if(cert_end == NULL)
return -1;
sep_end = strstr(cert_end + 1, "-----");
if(sep_end == NULL)
return -1;
sep_end += 5;
len = cert_end - cert_start;
b64 = malloc(len + 1);
if(!b64)
return -1;
for(i = 0, j = 0; i < len; i++) {
if(cert_start[i] != '\r' && cert_start[i] != '\n')
b64[j++] = cert_start[i];
}
b64[j] = '\0';
err = Curl_base64_decode((const char *)b64, out, outlen);
free(b64);
if(err) {
free(*out);
return -1;
}
return sep_end - in;
}
static int read_cert(const char *file, unsigned char **out, size_t *outlen)
{
int fd;
ssize_t n, len = 0, cap = 512;
unsigned char buf[512], *data;
fd = open(file, 0);
if(fd < 0)
return -1;
data = malloc(cap);
if(!data) {
close(fd);
return -1;
}
for(;;) {
n = read(fd, buf, sizeof(buf));
if(n < 0) {
close(fd);
free(data);
return -1;
}
else if(n == 0) {
close(fd);
break;
}
if(len + n >= cap) {
cap *= 2;
data = Curl_saferealloc(data, cap);
if(!data) {
close(fd);
return -1;
}
}
memcpy(data + len, buf, n);
len += n;
}
data[len] = '\0';
*out = data;
*outlen = len;
return 0;
}
static int append_cert_to_array(struct Curl_easy *data,
unsigned char *buf, size_t buflen,
CFMutableArrayRef array)
{
CFDataRef certdata = CFDataCreate(kCFAllocatorDefault, buf, buflen);
char *certp;
CURLcode result;
if(!certdata) {
failf(data, "SSL: failed to allocate array for CA certificate");
return CURLE_OUT_OF_MEMORY;
}
SecCertificateRef cacert =
SecCertificateCreateWithData(kCFAllocatorDefault, certdata);
CFRelease(certdata);
if(!cacert) {
failf(data, "SSL: failed to create SecCertificate from CA certificate");
return CURLE_SSL_CACERT_BADFILE;
}
result = CopyCertSubject(data, cacert, &certp);
switch(result) {
case CURLE_OK:
break;
case CURLE_PEER_FAILED_VERIFICATION:
return CURLE_SSL_CACERT_BADFILE;
case CURLE_OUT_OF_MEMORY:
default:
return result;
}
free(certp);
CFArrayAppendValue(array, cacert);
CFRelease(cacert);
return CURLE_OK;
}
static CURLcode verify_cert(const char *cafile, struct Curl_easy *data,
SSLContextRef ctx)
{
int n = 0, rc;
long res;
unsigned char *certbuf, *der;
size_t buflen, derlen, offset = 0;
if(read_cert(cafile, &certbuf, &buflen) < 0) {
failf(data, "SSL: failed to read or invalid CA certificate");
return CURLE_SSL_CACERT_BADFILE;
}
CFMutableArrayRef array = CFArrayCreateMutable(kCFAllocatorDefault, 0,
&kCFTypeArrayCallBacks);
if(array == NULL) {
free(certbuf);
failf(data, "SSL: out of memory creating CA certificate array");
return CURLE_OUT_OF_MEMORY;
}
while(offset < buflen) {
n++;
res = pem_to_der((const char *)certbuf + offset, &der, &derlen);
if(res < 0) {
free(certbuf);
CFRelease(array);
failf(data, "SSL: invalid CA certificate #%d (offset %d) in bundle",
n, offset);
return CURLE_SSL_CACERT_BADFILE;
}
offset += res;
if(res == 0 && offset == 0) {
rc = append_cert_to_array(data, certbuf, buflen, array);
free(certbuf);
if(rc != CURLE_OK) {
CFRelease(array);
return rc;
}
break;
}
else if(res == 0) {
free(certbuf);
break;
}
rc = append_cert_to_array(data, der, derlen, array);
free(der);
if(rc != CURLE_OK) {
free(certbuf);
CFRelease(array);
return rc;
}
}
SecTrustRef trust;
OSStatus ret = SSLCopyPeerTrust(ctx, &trust);
if(trust == NULL) {
failf(data, "SSL: error getting certificate chain");
CFRelease(array);
return CURLE_PEER_FAILED_VERIFICATION;
}
else if(ret != noErr) {
CFRelease(array);
failf(data, "SSLCopyPeerTrust() returned error %d", ret);
return CURLE_PEER_FAILED_VERIFICATION;
}
ret = SecTrustSetAnchorCertificates(trust, array);
if(ret != noErr) {
CFRelease(array);
CFRelease(trust);
failf(data, "SecTrustSetAnchorCertificates() returned error %d", ret);
return CURLE_PEER_FAILED_VERIFICATION;
}
ret = SecTrustSetAnchorCertificatesOnly(trust, true);
if(ret != noErr) {
CFRelease(array);
CFRelease(trust);
failf(data, "SecTrustSetAnchorCertificatesOnly() returned error %d", ret);
return CURLE_PEER_FAILED_VERIFICATION;
}
SecTrustResultType trust_eval = 0;
ret = SecTrustEvaluate(trust, &trust_eval);
CFRelease(array);
CFRelease(trust);
if(ret != noErr) {
failf(data, "SecTrustEvaluate() returned error %d", ret);
return CURLE_PEER_FAILED_VERIFICATION;
}
switch(trust_eval) {
case kSecTrustResultUnspecified:
case kSecTrustResultProceed:
return CURLE_OK;
case kSecTrustResultRecoverableTrustFailure:
case kSecTrustResultDeny:
default:
failf(data, "SSL: certificate verification failed (result: %d)",
trust_eval);
return CURLE_PEER_FAILED_VERIFICATION;
}
}
#if defined(SECTRANSP_PINNEDPUBKEY)
static CURLcode pkp_pin_peer_pubkey(struct Curl_easy *data,
SSLContextRef ctx,
const char *pinnedpubkey)
{ 
size_t pubkeylen, realpubkeylen, spkiHeaderLength = 24;
unsigned char *pubkey = NULL, *realpubkey = NULL;
const unsigned char *spkiHeader = NULL;
CFDataRef publicKeyBits = NULL;
CURLcode result = CURLE_SSL_PINNEDPUBKEYNOTMATCH;
if(!pinnedpubkey)
return CURLE_OK;
if(!ctx)
return result;
do {
SecTrustRef trust;
OSStatus ret = SSLCopyPeerTrust(ctx, &trust);
if(ret != noErr || trust == NULL)
break;
SecKeyRef keyRef = SecTrustCopyPublicKey(trust);
CFRelease(trust);
if(keyRef == NULL)
break;
#if defined(SECTRANSP_PINNEDPUBKEY_V1)
publicKeyBits = SecKeyCopyExternalRepresentation(keyRef, NULL);
CFRelease(keyRef);
if(publicKeyBits == NULL)
break;
#elif SECTRANSP_PINNEDPUBKEY_V2
OSStatus success = SecItemExport(keyRef, kSecFormatOpenSSL, 0, NULL,
&publicKeyBits);
CFRelease(keyRef);
if(success != errSecSuccess || publicKeyBits == NULL)
break;
#endif 
pubkeylen = CFDataGetLength(publicKeyBits);
pubkey = (unsigned char *)CFDataGetBytePtr(publicKeyBits);
switch(pubkeylen) {
case 526:
spkiHeader = rsa4096SpkiHeader;
break;
case 270:
spkiHeader = rsa2048SpkiHeader;
break;
#if defined(SECTRANSP_PINNEDPUBKEY_V1)
case 65:
spkiHeader = ecDsaSecp256r1SpkiHeader;
spkiHeaderLength = 26;
break;
case 97:
spkiHeader = ecDsaSecp384r1SpkiHeader;
spkiHeaderLength = 23;
break;
default:
infof(data, "SSL: unhandled public key length: %d\n", pubkeylen);
#elif SECTRANSP_PINNEDPUBKEY_V2
default:
result = Curl_pin_peer_pubkey(data, pinnedpubkey, pubkey,
pubkeylen);
#endif 
continue; 
}
realpubkeylen = pubkeylen + spkiHeaderLength;
realpubkey = malloc(realpubkeylen);
if(!realpubkey)
break;
memcpy(realpubkey, spkiHeader, spkiHeaderLength);
memcpy(realpubkey + spkiHeaderLength, pubkey, pubkeylen);
result = Curl_pin_peer_pubkey(data, pinnedpubkey, realpubkey,
realpubkeylen);
} while(0);
Curl_safefree(realpubkey);
if(publicKeyBits != NULL)
CFRelease(publicKeyBits);
return result;
}
#endif 
static CURLcode
sectransp_connect_step2(struct connectdata *conn, int sockindex)
{
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct ssl_backend_data *backend = connssl->backend;
OSStatus err;
SSLCipherSuite cipher;
SSLProtocol protocol = 0;
const char * const hostname = SSL_IS_PROXY() ? conn->http_proxy.host.name :
conn->host.name;
DEBUGASSERT(ssl_connect_2 == connssl->connecting_state
|| ssl_connect_2_reading == connssl->connecting_state
|| ssl_connect_2_writing == connssl->connecting_state);
err = SSLHandshake(backend->ssl_ctx);
if(err != noErr) {
switch(err) {
case errSSLWouldBlock: 
connssl->connecting_state = backend->ssl_direction ?
ssl_connect_2_writing : ssl_connect_2_reading;
return CURLE_OK;
case -9841:
if(SSL_CONN_CONFIG(CAfile) && SSL_CONN_CONFIG(verifypeer)) {
CURLcode result = verify_cert(SSL_CONN_CONFIG(CAfile), data,
backend->ssl_ctx);
if(result)
return result;
}
return sectransp_connect_step2(conn, sockindex);
case errSSLPeerDecodeError:
failf(data, "Decode failed");
break;
case errSSLDecryptionFail:
case errSSLPeerDecryptionFail:
failf(data, "Decryption failed");
break;
case errSSLPeerDecryptError:
failf(data, "A decryption error occurred");
break;
case errSSLBadCipherSuite:
failf(data, "A bad SSL cipher suite was encountered");
break;
case errSSLCrypto:
failf(data, "An underlying cryptographic error was encountered");
break;
#if CURL_BUILD_MAC_10_11 || CURL_BUILD_IOS_9
case errSSLWeakPeerEphemeralDHKey:
failf(data, "Indicates a weak ephemeral Diffie-Hellman key");
break;
#endif
case errSSLBadRecordMac:
case errSSLPeerBadRecordMac:
failf(data, "A record with a bad message authentication code (MAC) "
"was encountered");
break;
case errSSLRecordOverflow:
case errSSLPeerRecordOverflow:
failf(data, "A record overflow occurred");
break;
case errSSLPeerDecompressFail:
failf(data, "Decompression failed");
break;
case errSSLPeerAccessDenied:
failf(data, "Access was denied");
break;
case errSSLPeerInsufficientSecurity:
failf(data, "There is insufficient security for this operation");
break;
case errSSLXCertChainInvalid:
failf(data, "SSL certificate problem: Invalid certificate chain");
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLUnknownRootCert:
failf(data, "SSL certificate problem: Untrusted root certificate");
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLNoRootCert:
failf(data, "SSL certificate problem: No root certificate");
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLCertNotYetValid:
failf(data, "SSL certificate problem: The certificate chain had a "
"certificate that is not yet valid");
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLCertExpired:
case errSSLPeerCertExpired:
failf(data, "SSL certificate problem: Certificate chain had an "
"expired certificate");
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLBadCert:
case errSSLPeerBadCert:
failf(data, "SSL certificate problem: Couldn't understand the server "
"certificate format");
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLPeerUnsupportedCert:
failf(data, "SSL certificate problem: An unsupported certificate "
"format was encountered");
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLPeerCertRevoked:
failf(data, "SSL certificate problem: The certificate was revoked");
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLPeerCertUnknown:
failf(data, "SSL certificate problem: The certificate is unknown");
return CURLE_PEER_FAILED_VERIFICATION;
case errSecAuthFailed:
failf(data, "SSL authentication failed");
break;
case errSSLPeerHandshakeFail:
failf(data, "SSL peer handshake failed, the server most likely "
"requires a client certificate to connect");
break;
case errSSLPeerUnknownCA:
failf(data, "SSL server rejected the client certificate due to "
"the certificate being signed by an unknown certificate "
"authority");
break;
case errSSLHostNameMismatch:
failf(data, "SSL certificate peer verification failed, the "
"certificate did not match \"%s\"\n", conn->host.dispname);
return CURLE_PEER_FAILED_VERIFICATION;
case errSSLNegotiation:
failf(data, "Could not negotiate an SSL cipher suite with the server");
break;
case errSSLBadConfiguration:
failf(data, "A configuration error occurred");
break;
case errSSLProtocol:
failf(data, "SSL protocol error");
break;
case errSSLPeerProtocolVersion:
failf(data, "A bad protocol version was encountered");
break;
case errSSLPeerNoRenegotiation:
failf(data, "No renegotiation is allowed");
break;
case errSSLConnectionRefused:
failf(data, "Server dropped the connection during the SSL handshake");
break;
case errSSLClosedAbort:
failf(data, "Server aborted the SSL handshake");
break;
case errSSLClosedGraceful:
failf(data, "The connection closed gracefully");
break;
case errSSLClosedNoNotify:
failf(data, "The server closed the session with no notification");
break;
case paramErr:
case errSSLInternal:
case errSSLPeerInternalError:
failf(data, "Internal SSL engine error encountered during the "
"SSL handshake");
break;
case errSSLFatalAlert:
failf(data, "Fatal SSL engine error encountered during the SSL "
"handshake");
break;
case errSSLBufferOverflow:
failf(data, "An insufficient buffer was provided");
break;
case errSSLIllegalParam:
failf(data, "An illegal parameter was encountered");
break;
case errSSLModuleAttach:
failf(data, "Module attach failure");
break;
case errSSLSessionNotFound:
failf(data, "An attempt to restore an unknown session failed");
break;
case errSSLPeerExportRestriction:
failf(data, "An export restriction occurred");
break;
case errSSLPeerUserCancelled:
failf(data, "The user canceled the operation");
break;
case errSSLPeerUnexpectedMsg:
failf(data, "Peer rejected unexpected message");
break;
#if CURL_BUILD_MAC_10_11 || CURL_BUILD_IOS_9
case errSSLClientHelloReceived:
failf(data, "A non-fatal result for providing a server name "
"indication");
break;
#endif
#if CURL_BUILD_MAC_10_6
case errSSLClientCertRequested:
failf(data, "The server has requested a client certificate");
break;
#endif
#if CURL_BUILD_MAC_10_9
case errSSLUnexpectedRecord:
failf(data, "Unexpected (skipped) record in DTLS");
break;
#endif
default:
failf(data, "Unknown SSL protocol error in connection to %s:%d",
hostname, err);
break;
}
return CURLE_SSL_CONNECT_ERROR;
}
else {
connssl->connecting_state = ssl_connect_3;
#if defined(SECTRANSP_PINNEDPUBKEY)
if(data->set.str[STRING_SSL_PINNEDPUBLICKEY_ORIG]) {
CURLcode result = pkp_pin_peer_pubkey(data, backend->ssl_ctx,
data->set.str[STRING_SSL_PINNEDPUBLICKEY_ORIG]);
if(result) {
failf(data, "SSL: public key does not match pinned public key!");
return result;
}
}
#endif 
(void)SSLGetNegotiatedCipher(backend->ssl_ctx, &cipher);
(void)SSLGetNegotiatedProtocolVersion(backend->ssl_ctx, &protocol);
switch(protocol) {
case kSSLProtocol2:
infof(data, "SSL 2.0 connection using %s\n",
SSLCipherNameForNumber(cipher));
break;
case kSSLProtocol3:
infof(data, "SSL 3.0 connection using %s\n",
SSLCipherNameForNumber(cipher));
break;
case kTLSProtocol1:
infof(data, "TLS 1.0 connection using %s\n",
TLSCipherNameForNumber(cipher));
break;
#if CURL_BUILD_MAC_10_8 || CURL_BUILD_IOS
case kTLSProtocol11:
infof(data, "TLS 1.1 connection using %s\n",
TLSCipherNameForNumber(cipher));
break;
case kTLSProtocol12:
infof(data, "TLS 1.2 connection using %s\n",
TLSCipherNameForNumber(cipher));
break;
#endif 
#if CURL_BUILD_MAC_10_13 || CURL_BUILD_IOS_11
case kTLSProtocol13:
infof(data, "TLS 1.3 connection using %s\n",
TLSCipherNameForNumber(cipher));
break;
#endif 
default:
infof(data, "Unknown protocol connection\n");
break;
}
#if(CURL_BUILD_MAC_10_13 || CURL_BUILD_IOS_11) && HAVE_BUILTIN_AVAILABLE == 1
if(conn->bits.tls_enable_alpn) {
if(__builtin_available(macOS 10.13.4, iOS 11, tvOS 11, *)) {
CFArrayRef alpnArr = NULL;
CFStringRef chosenProtocol = NULL;
err = SSLCopyALPNProtocols(backend->ssl_ctx, &alpnArr);
if(err == noErr && alpnArr && CFArrayGetCount(alpnArr) >= 1)
chosenProtocol = CFArrayGetValueAtIndex(alpnArr, 0);
#if defined(USE_NGHTTP2)
if(chosenProtocol &&
!CFStringCompare(chosenProtocol, CFSTR(NGHTTP2_PROTO_VERSION_ID),
0)) {
conn->negnpn = CURL_HTTP_VERSION_2;
}
else
#endif
if(chosenProtocol &&
!CFStringCompare(chosenProtocol, CFSTR(ALPN_HTTP_1_1), 0)) {
conn->negnpn = CURL_HTTP_VERSION_1_1;
}
else
infof(data, "ALPN, server did not agree to a protocol\n");
Curl_multiuse_state(conn, conn->negnpn == CURL_HTTP_VERSION_2 ?
BUNDLE_MULTIPLEX : BUNDLE_NO_MULTIUSE);
if(alpnArr)
CFRelease(alpnArr);
}
}
#endif
return CURLE_OK;
}
}
#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
static void
show_verbose_server_cert(struct connectdata *conn,
int sockindex)
{
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct ssl_backend_data *backend = connssl->backend;
CFArrayRef server_certs = NULL;
SecCertificateRef server_cert;
OSStatus err;
CFIndex i, count;
SecTrustRef trust = NULL;
if(!backend->ssl_ctx)
return;
#if CURL_BUILD_MAC_10_7 || CURL_BUILD_IOS
#if CURL_BUILD_IOS
#pragma unused(server_certs)
err = SSLCopyPeerTrust(backend->ssl_ctx, &trust);
if(err == noErr && trust) {
count = SecTrustGetCertificateCount(trust);
for(i = 0L ; i < count ; i++) {
CURLcode result;
char *certp;
server_cert = SecTrustGetCertificateAtIndex(trust, i);
result = CopyCertSubject(data, server_cert, &certp);
if(!result) {
infof(data, "Server certificate: %s\n", certp);
free(certp);
}
}
CFRelease(trust);
}
#else
if(SecTrustEvaluateAsync != NULL) {
#pragma unused(server_certs)
err = SSLCopyPeerTrust(backend->ssl_ctx, &trust);
if(err == noErr && trust) {
count = SecTrustGetCertificateCount(trust);
for(i = 0L ; i < count ; i++) {
char *certp;
CURLcode result;
server_cert = SecTrustGetCertificateAtIndex(trust, i);
result = CopyCertSubject(data, server_cert, &certp);
if(!result) {
infof(data, "Server certificate: %s\n", certp);
free(certp);
}
}
CFRelease(trust);
}
}
else {
#if CURL_SUPPORT_MAC_10_8
err = SSLCopyPeerCertificates(backend->ssl_ctx, &server_certs);
if(err == noErr && server_certs) {
count = CFArrayGetCount(server_certs);
for(i = 0L ; i < count ; i++) {
char *certp;
CURLcode result;
server_cert = (SecCertificateRef)CFArrayGetValueAtIndex(server_certs,
i);
result = CopyCertSubject(data, server_cert, &certp);
if(!result) {
infof(data, "Server certificate: %s\n", certp);
free(certp);
}
}
CFRelease(server_certs);
}
#endif 
}
#endif 
#else
#pragma unused(trust)
err = SSLCopyPeerCertificates(backend->ssl_ctx, &server_certs);
if(err == noErr) {
count = CFArrayGetCount(server_certs);
for(i = 0L ; i < count ; i++) {
CURLcode result;
char *certp;
server_cert = (SecCertificateRef)CFArrayGetValueAtIndex(server_certs, i);
result = CopyCertSubject(data, server_cert, &certp);
if(!result) {
infof(data, "Server certificate: %s\n", certp);
free(certp);
}
}
CFRelease(server_certs);
}
#endif 
}
#endif 
static CURLcode
sectransp_connect_step3(struct connectdata *conn,
int sockindex)
{
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
if(data->set.verbose)
show_verbose_server_cert(conn, sockindex);
#endif
connssl->connecting_state = ssl_connect_done;
return CURLE_OK;
}
static Curl_recv sectransp_recv;
static Curl_send sectransp_send;
static CURLcode
sectransp_connect_common(struct connectdata *conn,
int sockindex,
bool nonblocking,
bool *done)
{
CURLcode result;
struct Curl_easy *data = conn->data;
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
curl_socket_t sockfd = conn->sock[sockindex];
timediff_t timeout_ms;
int what;
if(ssl_connection_complete == connssl->state) {
*done = TRUE;
return CURLE_OK;
}
if(ssl_connect_1 == connssl->connecting_state) {
timeout_ms = Curl_timeleft(data, NULL, TRUE);
if(timeout_ms < 0) {
failf(data, "SSL connection timeout");
return CURLE_OPERATION_TIMEDOUT;
}
result = sectransp_connect_step1(conn, sockindex);
if(result)
return result;
}
while(ssl_connect_2 == connssl->connecting_state ||
ssl_connect_2_reading == connssl->connecting_state ||
ssl_connect_2_writing == connssl->connecting_state) {
timeout_ms = Curl_timeleft(data, NULL, TRUE);
if(timeout_ms < 0) {
failf(data, "SSL connection timeout");
return CURLE_OPERATION_TIMEDOUT;
}
if(connssl->connecting_state == ssl_connect_2_reading ||
connssl->connecting_state == ssl_connect_2_writing) {
curl_socket_t writefd = ssl_connect_2_writing ==
connssl->connecting_state?sockfd:CURL_SOCKET_BAD;
curl_socket_t readfd = ssl_connect_2_reading ==
connssl->connecting_state?sockfd:CURL_SOCKET_BAD;
what = Curl_socket_check(readfd, CURL_SOCKET_BAD, writefd,
nonblocking?0:(time_t)timeout_ms);
if(what < 0) {
failf(data, "select/poll on SSL socket, errno: %d", SOCKERRNO);
return CURLE_SSL_CONNECT_ERROR;
}
else if(0 == what) {
if(nonblocking) {
*done = FALSE;
return CURLE_OK;
}
else {
failf(data, "SSL connection timeout");
return CURLE_OPERATION_TIMEDOUT;
}
}
}
result = sectransp_connect_step2(conn, sockindex);
if(result || (nonblocking &&
(ssl_connect_2 == connssl->connecting_state ||
ssl_connect_2_reading == connssl->connecting_state ||
ssl_connect_2_writing == connssl->connecting_state)))
return result;
} 
if(ssl_connect_3 == connssl->connecting_state) {
result = sectransp_connect_step3(conn, sockindex);
if(result)
return result;
}
if(ssl_connect_done == connssl->connecting_state) {
connssl->state = ssl_connection_complete;
conn->recv[sockindex] = sectransp_recv;
conn->send[sockindex] = sectransp_send;
*done = TRUE;
}
else
*done = FALSE;
connssl->connecting_state = ssl_connect_1;
return CURLE_OK;
}
static CURLcode Curl_sectransp_connect_nonblocking(struct connectdata *conn,
int sockindex, bool *done)
{
return sectransp_connect_common(conn, sockindex, TRUE, done);
}
static CURLcode Curl_sectransp_connect(struct connectdata *conn, int sockindex)
{
CURLcode result;
bool done = FALSE;
result = sectransp_connect_common(conn, sockindex, FALSE, &done);
if(result)
return result;
DEBUGASSERT(done);
return CURLE_OK;
}
static void Curl_sectransp_close(struct connectdata *conn, int sockindex)
{
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct ssl_backend_data *backend = connssl->backend;
if(backend->ssl_ctx) {
(void)SSLClose(backend->ssl_ctx);
#if CURL_BUILD_MAC_10_8 || CURL_BUILD_IOS
if(SSLCreateContext != NULL)
CFRelease(backend->ssl_ctx);
#if CURL_SUPPORT_MAC_10_8
else
(void)SSLDisposeContext(backend->ssl_ctx);
#endif 
#else
(void)SSLDisposeContext(backend->ssl_ctx);
#endif 
backend->ssl_ctx = NULL;
}
backend->ssl_sockfd = 0;
}
static int Curl_sectransp_shutdown(struct connectdata *conn, int sockindex)
{
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct ssl_backend_data *backend = connssl->backend;
struct Curl_easy *data = conn->data;
ssize_t nread;
int what;
int rc;
char buf[120];
if(!backend->ssl_ctx)
return 0;
#if !defined(CURL_DISABLE_FTP)
if(data->set.ftp_ccc != CURLFTPSSL_CCC_ACTIVE)
return 0;
#endif
Curl_sectransp_close(conn, sockindex);
rc = 0;
what = SOCKET_READABLE(conn->sock[sockindex], SSL_SHUTDOWN_TIMEOUT);
for(;;) {
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
static void Curl_sectransp_session_free(void *ptr)
{
Curl_safefree(ptr);
}
static size_t Curl_sectransp_version(char *buffer, size_t size)
{
return msnprintf(buffer, size, "SecureTransport");
}
static int Curl_sectransp_check_cxn(struct connectdata *conn)
{
struct ssl_connect_data *connssl = &conn->ssl[FIRSTSOCKET];
struct ssl_backend_data *backend = connssl->backend;
OSStatus err;
SSLSessionState state;
if(backend->ssl_ctx) {
err = SSLGetSessionState(backend->ssl_ctx, &state);
if(err == noErr)
return state == kSSLConnected || state == kSSLHandshake;
return -1;
}
return 0;
}
static bool Curl_sectransp_data_pending(const struct connectdata *conn,
int connindex)
{
const struct ssl_connect_data *connssl = &conn->ssl[connindex];
struct ssl_backend_data *backend = connssl->backend;
OSStatus err;
size_t buffer;
if(backend->ssl_ctx) { 
err = SSLGetBufferedReadSize(backend->ssl_ctx, &buffer);
if(err == noErr)
return buffer > 0UL;
return false;
}
else
return false;
}
static CURLcode Curl_sectransp_random(struct Curl_easy *data UNUSED_PARAM,
unsigned char *entropy, size_t length)
{
size_t i;
u_int32_t random_number = 0;
(void)data;
for(i = 0 ; i < length ; i++) {
if(i % sizeof(u_int32_t) == 0)
random_number = arc4random();
entropy[i] = random_number & 0xFF;
random_number >>= 8;
}
i = random_number = 0;
return CURLE_OK;
}
static CURLcode Curl_sectransp_md5sum(unsigned char *tmp, 
size_t tmplen,
unsigned char *md5sum, 
size_t md5len)
{
(void)md5len;
(void)CC_MD5(tmp, (CC_LONG)tmplen, md5sum);
return CURLE_OK;
}
static CURLcode Curl_sectransp_sha256sum(const unsigned char *tmp, 
size_t tmplen,
unsigned char *sha256sum, 
size_t sha256len)
{
assert(sha256len >= CURL_SHA256_DIGEST_LENGTH);
(void)CC_SHA256(tmp, (CC_LONG)tmplen, sha256sum);
return CURLE_OK;
}
static bool Curl_sectransp_false_start(void)
{
#if CURL_BUILD_MAC_10_9 || CURL_BUILD_IOS_7
if(SSLSetSessionOption != NULL)
return TRUE;
#endif
return FALSE;
}
static ssize_t sectransp_send(struct connectdata *conn,
int sockindex,
const void *mem,
size_t len,
CURLcode *curlcode)
{
struct ssl_connect_data *connssl = &conn->ssl[sockindex];
struct ssl_backend_data *backend = connssl->backend;
size_t processed = 0UL;
OSStatus err;
if(backend->ssl_write_buffered_length) {
err = SSLWrite(backend->ssl_ctx, NULL, 0UL, &processed);
switch(err) {
case noErr:
processed = backend->ssl_write_buffered_length;
backend->ssl_write_buffered_length = 0UL;
break;
case errSSLWouldBlock: 
*curlcode = CURLE_AGAIN;
return -1L;
default:
failf(conn->data, "SSLWrite() returned error %d", err);
*curlcode = CURLE_SEND_ERROR;
return -1L;
}
}
else {
err = SSLWrite(backend->ssl_ctx, mem, len, &processed);
if(err != noErr) {
switch(err) {
case errSSLWouldBlock:
backend->ssl_write_buffered_length = len;
*curlcode = CURLE_AGAIN;
return -1L;
default:
failf(conn->data, "SSLWrite() returned error %d", err);
*curlcode = CURLE_SEND_ERROR;
return -1L;
}
}
}
return (ssize_t)processed;
}
static ssize_t sectransp_recv(struct connectdata *conn,
int num,
char *buf,
size_t buffersize,
CURLcode *curlcode)
{
struct ssl_connect_data *connssl = &conn->ssl[num];
struct ssl_backend_data *backend = connssl->backend;
size_t processed = 0UL;
OSStatus err;
again:
err = SSLRead(backend->ssl_ctx, buf, buffersize, &processed);
if(err != noErr) {
switch(err) {
case errSSLWouldBlock: 
if(processed)
return (ssize_t)processed;
*curlcode = CURLE_AGAIN;
return -1L;
break;
case errSSLClosedGraceful:
case errSSLClosedNoNotify:
*curlcode = CURLE_OK;
return -1L;
break;
case -9841:
if(SSL_CONN_CONFIG(CAfile) && SSL_CONN_CONFIG(verifypeer)) {
CURLcode result = verify_cert(SSL_CONN_CONFIG(CAfile), conn->data,
backend->ssl_ctx);
if(result)
return result;
}
goto again;
default:
failf(conn->data, "SSLRead() return error %d", err);
*curlcode = CURLE_RECV_ERROR;
return -1L;
break;
}
}
return (ssize_t)processed;
}
static void *Curl_sectransp_get_internals(struct ssl_connect_data *connssl,
CURLINFO info UNUSED_PARAM)
{
struct ssl_backend_data *backend = connssl->backend;
(void)info;
return backend->ssl_ctx;
}
const struct Curl_ssl Curl_ssl_sectransp = {
{ CURLSSLBACKEND_SECURETRANSPORT, "secure-transport" }, 
#if defined(SECTRANSP_PINNEDPUBKEY)
SSLSUPP_PINNEDPUBKEY,
#else
0,
#endif 
sizeof(struct ssl_backend_data),
Curl_none_init, 
Curl_none_cleanup, 
Curl_sectransp_version, 
Curl_sectransp_check_cxn, 
Curl_sectransp_shutdown, 
Curl_sectransp_data_pending, 
Curl_sectransp_random, 
Curl_none_cert_status_request, 
Curl_sectransp_connect, 
Curl_sectransp_connect_nonblocking, 
Curl_sectransp_get_internals, 
Curl_sectransp_close, 
Curl_none_close_all, 
Curl_sectransp_session_free, 
Curl_none_set_engine, 
Curl_none_set_engine_default, 
Curl_none_engines_list, 
Curl_sectransp_false_start, 
Curl_sectransp_md5sum, 
Curl_sectransp_sha256sum 
};
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif 
