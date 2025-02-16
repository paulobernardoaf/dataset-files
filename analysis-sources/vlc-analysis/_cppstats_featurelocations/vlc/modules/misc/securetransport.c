























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_tls.h>
#include <vlc_dialog.h>

#include <Security/Security.h>
#include <Security/SecureTransport.h>
#include <TargetConditionals.h>

#include <vlc_charset.h>


#if !defined(ioErr)
#define ioErr -36
#endif









static CFMutableArrayRef alpnToCFArray(const char *const *alpn)
{
CFMutableArrayRef alpnValues =
CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

for (size_t i = 0; alpn[i] != NULL; i++) {
CFStringRef alpnVal =
CFStringCreateWithCString(kCFAllocatorDefault, alpn[i], kCFStringEncodingASCII);
if (alpnVal == NULL) {

CFRelease(alpnValues);
return NULL;
}
CFArrayAppendValue(alpnValues, alpnVal);
CFRelease(alpnVal);
}
return alpnValues;
}




static char* CFArrayALPNCopyFirst(CFArrayRef alpnArray)
{
CFIndex count = CFArrayGetCount(alpnArray);

if (count <= 0)
return NULL;

CFStringRef alpnVal = CFArrayGetValueAtIndex(alpnArray, 0);
return FromCFString(alpnVal, kCFStringEncodingASCII);
}




static int OpenClient (vlc_tls_client_t *);

#if !TARGET_OS_IPHONE
static int OpenServer (vlc_tls_server_t *crd, const char *cert, const char *key);
#endif

vlc_module_begin ()
set_description(N_("TLS support for OS X and iOS"))
set_capability("tls client", 2)
set_callback(OpenClient)
set_category(CAT_ADVANCED)
set_subcategory(SUBCAT_ADVANCED_NETWORK)






#if !TARGET_OS_IPHONE
add_submodule()
set_description(N_("TLS server support for OS X"))
set_capability("tls server", 2)
set_callback(OpenServer)
set_category(CAT_ADVANCED)
set_subcategory(SUBCAT_ADVANCED_NETWORK)
#endif 

vlc_module_end ()


#define cfKeyHost CFSTR("host")
#define cfKeyCertificate CFSTR("certificate")

typedef struct {
CFMutableArrayRef whitelist;


CFArrayRef server_cert_chain;
} vlc_tls_creds_sys_t;

typedef struct {
vlc_tls_t tls;
SSLContextRef p_context;
vlc_tls_creds_sys_t *p_cred;
size_t i_send_buffered_bytes;
vlc_tls_t *sock;
vlc_object_t *obj;

bool b_blocking_send;
bool b_handshaked;
bool b_server_mode;

vlc_mutex_t lock;
} vlc_tls_st_t;

static int st_Error (vlc_tls_t *obj, int val)
{
vlc_tls_st_t *sys = (vlc_tls_st_t *)obj;

switch (val)
{
case errSSLWouldBlock:
errno = EAGAIN;
break;

case errSSLClosedGraceful:
case errSSLClosedAbort:
msg_Dbg(sys->obj, "Connection closed with code %d", val);
errno = ECONNRESET;
break;
default:
msg_Err(sys->obj, "Found error %d", val);
errno = ECONNRESET;
}
return -1;
}






static OSStatus st_SocketReadFunc (SSLConnectionRef connection,
void *data,
size_t *dataLength) {

vlc_tls_t *session = (vlc_tls_t *)connection;
vlc_tls_st_t *sys = (vlc_tls_st_t *)session;
struct iovec iov = {
.iov_base = data,
.iov_len = *dataLength,
};
OSStatus retValue = noErr;

while (iov.iov_len > 0) {
ssize_t val = sys->sock->ops->readv(sys->sock, &iov, 1);
if (val <= 0) {
if (val == 0) {
msg_Dbg(sys->obj, "found eof");
retValue = errSSLClosedGraceful;
} else { 
switch (errno) {
case ENOENT:

retValue = errSSLClosedGraceful;
break;
case ECONNRESET:
retValue = errSSLClosedAbort;
break;
case EAGAIN:
retValue = errSSLWouldBlock;
sys->b_blocking_send = false;
break;
default:
msg_Err(sys->obj, "try to read %zu bytes, "
"got error %d", iov.iov_len, errno);
retValue = ioErr;
break;
}
}
break;
}

iov.iov_base = (char *)iov.iov_base + val;
iov.iov_len -= val;
}

*dataLength -= iov.iov_len;
return retValue;
}






static OSStatus st_SocketWriteFunc (SSLConnectionRef connection,
const void *data,
size_t *dataLength) {

vlc_tls_t *session = (vlc_tls_t *)connection;
vlc_tls_st_t *sys = (vlc_tls_st_t *)session;
struct iovec iov = {
.iov_base = (void *)data,
.iov_len = *dataLength,
};
OSStatus retValue = noErr;

while (iov.iov_len > 0) {
ssize_t val = sys->sock->ops->writev(sys->sock, &iov, 1);
if (val < 0) {
switch (errno) {
case EAGAIN:
retValue = errSSLWouldBlock;
sys->b_blocking_send = true;
break;

case EPIPE:
case ECONNRESET:
retValue = errSSLClosedAbort;
break;

default:
msg_Err(sys->obj, "error while writing: %d", errno);
retValue = ioErr;
break;
}
break;
}

iov.iov_base = (char *)iov.iov_base + val;
iov.iov_len -= val;
}

*dataLength -= iov.iov_len;
return retValue;
}

static int st_validateServerCertificate (vlc_tls_t *session, const char *hostname) {

vlc_tls_st_t *sys = (vlc_tls_st_t *)session;
int result = -1;
SecCertificateRef leaf_cert = NULL;

SecTrustRef trust = NULL;
OSStatus ret = SSLCopyPeerTrust(sys->p_context, &trust);
if (ret != noErr || trust == NULL) {
msg_Err(sys->obj, "error getting certifictate chain");
return -1;
}

CFStringRef cfHostname = CFStringCreateWithCString(kCFAllocatorDefault,
hostname,
kCFStringEncodingUTF8);



ret = SecTrustSetAnchorCertificates(trust, NULL);
if (ret != noErr) {
msg_Err(sys->obj, "error setting anchor certificates");
result = -1;
goto out;
}

SecTrustResultType trust_eval_result = 0;

ret = SecTrustEvaluate(trust, &trust_eval_result);
if (ret != noErr) {
msg_Err(sys->obj, "error calling SecTrustEvaluate");
result = -1;
goto out;
}

switch (trust_eval_result) {
case kSecTrustResultUnspecified:
case kSecTrustResultProceed:
msg_Dbg(sys->obj, "cerfificate verification successful, result is %d", trust_eval_result);
result = 0;
goto out;

case kSecTrustResultRecoverableTrustFailure:
case kSecTrustResultDeny:
default:
msg_Warn(sys->obj, "cerfificate verification failed, result is %d", trust_eval_result);
}



#if !TARGET_OS_IPHONE
CFArrayRef cert_chain = NULL;
ret = SSLCopyPeerCertificates(sys->p_context, &cert_chain);
if (ret != noErr || !cert_chain) {
result = -1;
goto out;
}

if (CFArrayGetCount(cert_chain) == 0) {
CFRelease(cert_chain);
result = -1;
goto out;
}

leaf_cert = (SecCertificateRef)CFArrayGetValueAtIndex(cert_chain, 0);
CFRetain(leaf_cert);
CFRelease(cert_chain);
#else

if (SecTrustGetCertificateCount(trust) == 0) {
result = -1;
goto out;
}

leaf_cert = SecTrustGetCertificateAtIndex(trust, 0);
CFRetain(leaf_cert);
#endif



CFIndex max = CFArrayGetCount(sys->p_cred->whitelist);
for (CFIndex i = 0; i < max; ++i) {
CFDictionaryRef dict = CFArrayGetValueAtIndex(sys->p_cred->whitelist, i);
CFStringRef knownHost = (CFStringRef)CFDictionaryGetValue(dict, cfKeyHost);
SecCertificateRef knownCert = (SecCertificateRef)CFDictionaryGetValue(dict, cfKeyCertificate);

if (!knownHost || !knownCert)
continue;

if (CFEqual(knownHost, cfHostname) && CFEqual(knownCert, leaf_cert)) {
msg_Warn(sys->obj, "certificate already accepted, continuing");
result = 0;
goto out;
}
}











const char *msg = N_("You attempted to reach %s. "
"However the security certificate presented by the server "
"is unknown and could not be authenticated by any trusted "
"Certification Authority. "
"This problem may be caused by a configuration error "
"or an attempt to breach your security or your privacy.\n\n"
"If in doubt, abort now.\n");
int answer = vlc_dialog_wait_question(sys->obj,
VLC_DIALOG_QUESTION_WARNING, _("Abort"),
_("Accept certificate temporarily"),
NULL, _("Insecure site"),
vlc_gettext (msg), hostname);
if (answer == 1) {
msg_Warn(sys->obj, "Proceeding despite of failed certificate validation");


const void *keys[] = {cfKeyHost, cfKeyCertificate};
const void *values[] = {cfHostname, leaf_cert};
CFDictionaryRef dict = CFDictionaryCreate(kCFAllocatorDefault,
keys, values, 2,
&kCFTypeDictionaryKeyCallBacks,
&kCFTypeDictionaryValueCallBacks);
if (!dict) {
msg_Err(sys->obj, "error creating dict");
result = -1;
goto out;
}

CFArrayAppendValue(sys->p_cred->whitelist, dict);
CFRelease(dict);

result = 0;
goto out;

} else {
result = -1;
goto out;
}

out:
CFRelease(trust);

if (cfHostname)
CFRelease(cfHostname);
if (leaf_cert)
CFRelease(leaf_cert);

return result;
}






static int st_Handshake (vlc_tls_t *session,
const char *host, const char *service,
char **restrict alp) {

vlc_tls_st_t *sys = (vlc_tls_st_t *)session;

VLC_UNUSED(service);

OSStatus retValue = SSLHandshake(sys->p_context);



#if (TARGET_OS_OSX && MAC_OS_X_VERSION_MAX_ALLOWED >= 101302) || (TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000) || (TARGET_OS_TV && __TV_OS_VERSION_MAX_ALLOWED >= 110000) || (TARGET_OS_WATCH && __WATCH_OS_VERSION_MAX_ALLOWED >= 40000)



#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpartial-availability"


if (alp != NULL) {
if (SSLCopyALPNProtocols != NULL) {
CFArrayRef alpnArray = NULL;
OSStatus res = SSLCopyALPNProtocols(sys->p_context, &alpnArray);
if (res == noErr && alpnArray) {
*alp = CFArrayALPNCopyFirst(alpnArray);
CFRelease(alpnArray);
if (unlikely(*alp == NULL))
return -1;
} else {
*alp = NULL;
}
} else {
*alp = NULL;
}
}

#pragma clang diagnostic pop
#else


if (alp != NULL) {
*alp = NULL;
}

#endif

if (retValue == errSSLWouldBlock) {
msg_Dbg(sys->obj, "handshake is blocked, try again later");
return 1 + (sys->b_blocking_send ? 1 : 0);
}

switch (retValue) {
case noErr:
if (sys->b_server_mode == false && st_validateServerCertificate(session, host) != 0) {
return -1;
}
msg_Dbg(sys->obj, "handshake completed successfully");
sys->b_handshaked = true;
return 0;

case errSSLServerAuthCompleted:
msg_Dbg(sys->obj, "SSLHandshake returned errSSLServerAuthCompleted, continuing handshake");
return st_Handshake(session, host, service, alp);

case errSSLConnectionRefused:
msg_Err(sys->obj, "connection was refused");
return -1;
case errSSLNegotiation:
msg_Err(sys->obj, "cipher suite negotiation failed");
return -1;
case errSSLFatalAlert:
msg_Err(sys->obj, "fatal error occurred during handshake");
return -1;

default:
msg_Err(sys->obj, "handshake returned error %d", (int)retValue);
return -1;
}
}

static int st_GetFD (vlc_tls_t *session, short *restrict events)
{
vlc_tls_st_t *sys = (vlc_tls_st_t *)session;
vlc_tls_t *sock = sys->sock;

return vlc_tls_GetPollFD(sock, events);
}




static ssize_t st_Send (vlc_tls_t *session, const struct iovec *iov,
unsigned count)
{
vlc_tls_st_t *sys = (vlc_tls_st_t *)session;
OSStatus ret = noErr;

if (unlikely(count == 0))
return 0;

vlc_mutex_lock(&sys->lock);

















int againErr = sys->b_server_mode ? EAGAIN : EINTR;

size_t actualSize;
if (sys->i_send_buffered_bytes > 0) {
ret = SSLWrite(sys->p_context, NULL, 0, &actualSize);

if (ret == noErr) {

actualSize = sys->i_send_buffered_bytes;
sys->i_send_buffered_bytes = 0;

} else if (ret == errSSLWouldBlock) {
vlc_mutex_unlock(&sys->lock);
errno = againErr;
return -1;
}

} else {
ret = SSLWrite(sys->p_context, iov->iov_base, iov->iov_len,
&actualSize);

if (ret == errSSLWouldBlock) {
sys->i_send_buffered_bytes = iov->iov_len;
errno = againErr;
vlc_mutex_unlock(&sys->lock);
return -1;
}
}

vlc_mutex_unlock(&sys->lock);
return ret != noErr ? st_Error(session, ret) : actualSize;
}




static ssize_t st_Recv (vlc_tls_t *session, struct iovec *iov, unsigned count)
{
vlc_tls_st_t *sys = (vlc_tls_st_t *)session;

if (unlikely(count == 0))
return 0;

vlc_mutex_lock(&sys->lock);

size_t actualSize;
OSStatus ret = SSLRead(sys->p_context, iov->iov_base, iov->iov_len,
&actualSize);

if (ret == errSSLWouldBlock && actualSize) {
vlc_mutex_unlock(&sys->lock);
return actualSize;
}


if (ret == errSSLClosedNoNotify || ret == errSSLClosedGraceful) {
msg_Dbg(sys->obj, "Got close notification with code %i", (int)ret);
vlc_mutex_unlock(&sys->lock);
return 0;
}

vlc_mutex_unlock(&sys->lock);
return ret != noErr ? st_Error(session, ret) : actualSize;
}





static int st_SessionShutdown (vlc_tls_t *session, bool duplex) {

vlc_tls_st_t *sys = (vlc_tls_st_t *)session;

msg_Dbg(sys->obj, "shutdown TLS session");

OSStatus ret = noErr;
VLC_UNUSED(duplex);

if (sys->b_handshaked) {
ret = SSLClose(sys->p_context);
}

if (ret != noErr) {
msg_Warn(sys->obj, "Cannot close ssl context (%i)", (int)ret);
return ret;
}

return 0;
}

static void st_SessionClose (vlc_tls_t *session) {

vlc_tls_st_t *sys = (vlc_tls_st_t *)session;
msg_Dbg(sys->obj, "close TLS session");

if (sys->p_context) {
#if TARGET_OS_IPHONE
CFRelease(sys->p_context);
#else
if (SSLDisposeContext(sys->p_context) != noErr) {
msg_Err(sys->obj, "error deleting context");
}
#endif
}
free(sys);
}

static const struct vlc_tls_operations st_ops =
{
st_GetFD,
st_Recv,
st_Send,
st_SessionShutdown,
st_SessionClose,
};





static vlc_tls_t *st_SessionOpenCommon(vlc_object_t *obj,
vlc_tls_creds_sys_t *crd,
vlc_tls_t *sock, bool b_server)
{
vlc_tls_st_t *sys = malloc(sizeof (*sys));
if (unlikely(sys == NULL))
return NULL;

sys->p_cred = crd;
sys->b_handshaked = false;
sys->b_blocking_send = false;
sys->i_send_buffered_bytes = 0;
sys->p_context = NULL;
sys->sock = sock;
sys->b_server_mode = b_server;
vlc_mutex_init(&sys->lock);
sys->obj = obj;

vlc_tls_t *tls = &sys->tls;

tls->ops = &st_ops;

SSLContextRef p_context = NULL;
#if TARGET_OS_IPHONE
p_context = SSLCreateContext(NULL, b_server ? kSSLServerSide : kSSLClientSide, kSSLStreamType);
if (p_context == NULL) {
msg_Err(obj, "cannot create ssl context");
goto error;
}
#else
if (SSLNewContext(b_server, &p_context) != noErr) {
msg_Err(obj, "error calling SSLNewContext");
goto error;
}
#endif

sys->p_context = p_context;

OSStatus ret = SSLSetIOFuncs(p_context, st_SocketReadFunc, st_SocketWriteFunc);
if (ret != noErr) {
msg_Err(obj, "cannot set io functions");
goto error;
}

ret = SSLSetConnection(p_context, tls);
if (ret != noErr) {
msg_Err(obj, "cannot set connection");
goto error;
}

return tls;

error:
st_SessionClose(tls);
return NULL;
}

static vlc_tls_t *st_ClientSessionOpen(vlc_tls_client_t *crd, vlc_tls_t *sock,
const char *hostname, const char *const *alpn)
{
msg_Dbg(crd, "open TLS session for %s", hostname);

vlc_tls_t *tls = st_SessionOpenCommon(VLC_OBJECT(crd), crd->sys, sock,
false);
if (tls == NULL)
return NULL;

vlc_tls_st_t *sys = (vlc_tls_st_t *)tls;

OSStatus ret = SSLSetPeerDomainName(sys->p_context, hostname, strlen(hostname));
if (ret != noErr) {
msg_Err(crd, "cannot set peer domain name");
goto error;
}



#if (TARGET_OS_OSX && MAC_OS_X_VERSION_MAX_ALLOWED >= 101302) || (TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000) || (TARGET_OS_TV && __TV_OS_VERSION_MAX_ALLOWED >= 110000) || (TARGET_OS_WATCH && __WATCH_OS_VERSION_MAX_ALLOWED >= 40000)



#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpartial-availability"


if (alpn != NULL) {
if (SSLSetALPNProtocols != NULL) {
CFMutableArrayRef alpnValues = alpnToCFArray(alpn);

if (alpnValues == NULL) {
msg_Err(crd, "cannot create CFMutableArray for ALPN values");
goto error;
}

OSStatus ret = SSLSetALPNProtocols(sys->p_context, alpnValues);
if (ret != noErr){
msg_Err(crd, "failed setting ALPN protocols (%i)", ret);
}
CFRelease(alpnValues);
} else {
msg_Warn(crd, "Ignoring ALPN request due to lack of support in the backend. Proxy behavior potentially undefined.");
}
}

#pragma clang diagnostic pop
#else


if (alpn != NULL) {

msg_Warn(crd, "Compiled in SDK without ALPN support. Proxy behavior potentially undefined.");
#warning ALPN support in your SDK version missing (need 10.13.2), proxy behavior potentially undefined (rdar:
}

#endif





ret = SSLSetSessionOption(sys->p_context, kSSLSessionOptionBreakOnServerAuth, true);
if (ret != noErr) {
msg_Err (crd, "cannot set session option");
goto error;
}
#if !TARGET_OS_IPHONE

ret = SSLSetEnableCertVerify(sys->p_context, false);
if (ret != noErr) {
msg_Err(crd, "error setting enable cert verify");
goto error;
}
#endif

return tls;

error:
st_SessionShutdown(tls, true);
st_SessionClose(tls);
return NULL;
}

static void st_ClientDestroy (vlc_tls_client_t *crd) {
msg_Dbg(crd, "close secure transport client");

vlc_tls_creds_sys_t *sys = crd->sys;

if (sys->whitelist)
CFRelease(sys->whitelist);

free(sys);
}

static const struct vlc_tls_client_operations st_ClientOps =
{
.open = st_ClientSessionOpen,
.handshake = st_Handshake,
.destroy = st_ClientDestroy,
};




static int OpenClient (vlc_tls_client_t *crd) {

msg_Dbg(crd, "open st client");

vlc_tls_creds_sys_t *sys = malloc (sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;

sys->whitelist = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
sys->server_cert_chain = NULL;

crd->ops = &st_ClientOps;
crd->sys = sys;
return VLC_SUCCESS;
}


#if !TARGET_OS_IPHONE




static vlc_tls_t *st_ServerSessionOpen (vlc_tls_server_t *crd, vlc_tls_t *sock,
const char *const *alpn) {

VLC_UNUSED(alpn);
msg_Dbg(crd, "open TLS server session");

vlc_tls_t *tls = st_SessionOpenCommon(VLC_OBJECT(crd), crd->sys, sock,
true);
if (tls != NULL)
return NULL;

vlc_tls_st_t *sys = (vlc_tls_st_t *)tls;
vlc_tls_creds_sys_t *p_cred_sys = crd->sys;

OSStatus ret = SSLSetCertificate(sys->p_context, p_cred_sys->server_cert_chain);
if (ret != noErr) {
msg_Err(crd, "cannot set server certificate");
goto error;
}

return tls;

error:
st_SessionShutdown(tls, true);
st_SessionClose(tls);
return NULL;
}

static int st_ServerHandshake (vlc_tls_t *session, char **restrict alp) {

return st_Handshake(session, NULL, NULL, alp);
}

static void st_ServerDestroy (vlc_tls_server_t *crd) {
msg_Dbg(crd, "close secure transport server");

vlc_tls_creds_sys_t *sys = crd->sys;

if (sys->server_cert_chain)
CFRelease(sys->server_cert_chain);

free(sys);
}

static const struct vlc_tls_server_operations st_ServerOps =
{
.open = st_ServerSessionOpen,
.handshake = st_ServerHandshake,
.destroy = st_ServerDestroy,
};




static int OpenServer (vlc_tls_server_t *crd, const char *cert, const char *key) {





VLC_UNUSED(key);
OSStatus ret;

msg_Dbg(crd, "open st server");







SecKeychainAttribute attrib = { kSecLabelItemAttr, strlen(cert), (void *)cert };
SecKeychainAttributeList attrList = { 1, &attrib };

SecKeychainSearchRef searchReference = NULL;
ret = SecKeychainSearchCreateFromAttributes(NULL, kSecCertificateItemClass,
&attrList, &searchReference);
if (ret != noErr || searchReference == NULL) {
msg_Err(crd, "Cannot find certificate with alias %s", cert);
return VLC_EGENERIC;
}

SecKeychainItemRef itemRef = NULL;
ret = SecKeychainSearchCopyNext(searchReference, &itemRef);
if (ret != noErr) {
msg_Err(crd, "Cannot get certificate with alias %s, error: %d", cert, ret);
return VLC_EGENERIC;
}
CFRelease(searchReference);


SecCertificateRef certificate = (SecCertificateRef)itemRef;

SecIdentityRef cert_identity = NULL;
ret = SecIdentityCreateWithCertificate(NULL, certificate, &cert_identity);
if (ret != noErr) {
msg_Err(crd, "Cannot get private key for certificate");
CFRelease(certificate);
return VLC_EGENERIC;
}





SecPolicyRef policy = SecPolicyCreateSSL(true, NULL);
SecTrustRef trust_ref = NULL;
int result = VLC_SUCCESS;


ret = SecTrustCreateWithCertificates((CFArrayRef)certificate, policy, &trust_ref);
if (ret != noErr) {
msg_Err(crd, "Cannot create trust");
result = VLC_EGENERIC;
goto out;
}

SecTrustResultType status;
ret = SecTrustEvaluate(trust_ref, &status);
if (ret != noErr) {
msg_Err(crd, "Error evaluating trust");
result = VLC_EGENERIC;
goto out;
}

CFArrayRef cert_chain = NULL;
CSSM_TP_APPLE_EVIDENCE_INFO *status_chain;
ret = SecTrustGetResult(trust_ref, &status, &cert_chain, &status_chain);
if (ret != noErr || !cert_chain) {
msg_Err(crd, "error while getting certificate chain");
result = VLC_EGENERIC;
goto out;
}

CFIndex num_cert_chain = CFArrayGetCount(cert_chain);


CFMutableArrayRef server_cert_chain = CFArrayCreateMutable(kCFAllocatorDefault, num_cert_chain, &kCFTypeArrayCallBacks);
CFArrayAppendValue(server_cert_chain, cert_identity);

msg_Dbg(crd, "Found certificate chain with %ld entries for server certificate", num_cert_chain);
if (num_cert_chain > 1)
CFArrayAppendArray(server_cert_chain, cert_chain, CFRangeMake(1, num_cert_chain - 1));
CFRelease(cert_chain);

vlc_tls_creds_sys_t *sys = malloc(sizeof(*sys));
if (unlikely(sys == NULL)) {
CFRelease(server_cert_chain);
result = VLC_ENOMEM;
goto out;
}

sys->server_cert_chain = server_cert_chain;
sys->whitelist = NULL;

crd->ops = &st_ServerOps;
crd->sys = sys;

out:
if (policy)
CFRelease(policy);
if (trust_ref)
CFRelease(trust_ref);

if (certificate)
CFRelease(certificate);
if (cert_identity)
CFRelease(cert_identity);

return result;
}

#endif 
