#include "curl_setup.h"
#if !defined(CURL_DISABLE_HTTP) && defined(USE_SPNEGO)
#include "urldata.h"
#include "sendf.h"
#include "http_negotiate.h"
#include "vauth/vauth.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
CURLcode Curl_input_negotiate(struct connectdata *conn, bool proxy,
const char *header)
{
CURLcode result;
struct Curl_easy *data = conn->data;
size_t len;
const char *userp;
const char *passwdp;
const char *service;
const char *host;
struct negotiatedata *neg_ctx;
curlnegotiate state;
if(proxy) {
userp = conn->http_proxy.user;
passwdp = conn->http_proxy.passwd;
service = data->set.str[STRING_PROXY_SERVICE_NAME] ?
data->set.str[STRING_PROXY_SERVICE_NAME] : "HTTP";
host = conn->http_proxy.host.name;
neg_ctx = &conn->proxyneg;
state = conn->proxy_negotiate_state;
}
else {
userp = conn->user;
passwdp = conn->passwd;
service = data->set.str[STRING_SERVICE_NAME] ?
data->set.str[STRING_SERVICE_NAME] : "HTTP";
host = conn->host.name;
neg_ctx = &conn->negotiate;
state = conn->http_negotiate_state;
}
if(!userp)
userp = "";
if(!passwdp)
passwdp = "";
header += strlen("Negotiate");
while(*header && ISSPACE(*header))
header++;
len = strlen(header);
neg_ctx->havenegdata = len != 0;
if(!len) {
if(state == GSS_AUTHSUCC) {
infof(conn->data, "Negotiate auth restarted\n");
Curl_http_auth_cleanup_negotiate(conn);
}
else if(state != GSS_AUTHNONE) {
Curl_http_auth_cleanup_negotiate(conn);
return CURLE_LOGIN_DENIED;
}
}
#if defined(USE_WINDOWS_SSPI) && defined(SECPKG_ATTR_ENDPOINT_BINDINGS)
neg_ctx->sslContext = conn->sslContext;
#endif
result = Curl_auth_decode_spnego_message(data, userp, passwdp, service,
host, header, neg_ctx);
if(result)
Curl_http_auth_cleanup_negotiate(conn);
return result;
}
CURLcode Curl_output_negotiate(struct connectdata *conn, bool proxy)
{
struct negotiatedata *neg_ctx = proxy ? &conn->proxyneg :
&conn->negotiate;
struct auth *authp = proxy ? &conn->data->state.authproxy :
&conn->data->state.authhost;
curlnegotiate *state = proxy ? &conn->proxy_negotiate_state :
&conn->http_negotiate_state;
char *base64 = NULL;
size_t len = 0;
char *userp;
CURLcode result;
authp->done = FALSE;
if(*state == GSS_AUTHRECV) {
if(neg_ctx->havenegdata) {
neg_ctx->havemultiplerequests = TRUE;
}
}
else if(*state == GSS_AUTHSUCC) {
if(!neg_ctx->havenoauthpersist) {
neg_ctx->noauthpersist = !neg_ctx->havemultiplerequests;
}
}
if(neg_ctx->noauthpersist ||
(*state != GSS_AUTHDONE && *state != GSS_AUTHSUCC)) {
if(neg_ctx->noauthpersist && *state == GSS_AUTHSUCC) {
infof(conn->data, "Curl_output_negotiate, "
"no persistent authentication: cleanup existing context");
Curl_http_auth_cleanup_negotiate(conn);
}
if(!neg_ctx->context) {
result = Curl_input_negotiate(conn, proxy, "Negotiate");
if(result == CURLE_AUTH_ERROR) {
authp->done = TRUE;
return CURLE_OK;
}
else if(result)
return result;
}
result = Curl_auth_create_spnego_message(conn->data,
neg_ctx, &base64, &len);
if(result)
return result;
userp = aprintf("%sAuthorization: Negotiate %s\r\n", proxy ? "Proxy-" : "",
base64);
if(proxy) {
Curl_safefree(conn->allocptr.proxyuserpwd);
conn->allocptr.proxyuserpwd = userp;
}
else {
Curl_safefree(conn->allocptr.userpwd);
conn->allocptr.userpwd = userp;
}
free(base64);
if(userp == NULL) {
return CURLE_OUT_OF_MEMORY;
}
*state = GSS_AUTHSENT;
#if defined(HAVE_GSSAPI)
if(neg_ctx->status == GSS_S_COMPLETE ||
neg_ctx->status == GSS_S_CONTINUE_NEEDED) {
*state = GSS_AUTHDONE;
}
#else
#if defined(USE_WINDOWS_SSPI)
if(neg_ctx->status == SEC_E_OK ||
neg_ctx->status == SEC_I_CONTINUE_NEEDED) {
*state = GSS_AUTHDONE;
}
#endif
#endif
}
if(*state == GSS_AUTHDONE || *state == GSS_AUTHSUCC) {
authp->done = TRUE;
}
neg_ctx->havenegdata = FALSE;
return CURLE_OK;
}
void Curl_http_auth_cleanup_negotiate(struct connectdata *conn)
{
conn->http_negotiate_state = GSS_AUTHNONE;
conn->proxy_negotiate_state = GSS_AUTHNONE;
Curl_auth_cleanup_spnego(&conn->negotiate);
Curl_auth_cleanup_spnego(&conn->proxyneg);
}
#endif 
