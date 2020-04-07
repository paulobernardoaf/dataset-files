#include "curl_setup.h"
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(HAVE_NETDB_H)
#include <netdb.h>
#endif
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#if defined(HAVE_NET_IF_H)
#include <net/if.h>
#endif
#if defined(HAVE_IPHLPAPI_H)
#include <Iphlpapi.h>
#endif
#if defined(HAVE_SYS_IOCTL_H)
#include <sys/ioctl.h>
#endif
#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#if defined(__VMS)
#include <in.h>
#include <inet.h>
#endif
#if defined(HAVE_SYS_UN_H)
#include <sys/un.h>
#endif
#if !defined(HAVE_SOCKET)
#error "We can't compile without socket() support!"
#endif
#include <limits.h>
#if defined(USE_LIBIDN2)
#include <idn2.h>
#elif defined(USE_WIN32_IDN)
bool curl_win32_idn_to_ascii(const char *in, char **out);
#endif 
#include "urldata.h"
#include "netrc.h"
#include "formdata.h"
#include "mime.h"
#include "vtls/vtls.h"
#include "hostip.h"
#include "transfer.h"
#include "sendf.h"
#include "progress.h"
#include "cookie.h"
#include "strcase.h"
#include "strerror.h"
#include "escape.h"
#include "strtok.h"
#include "share.h"
#include "content_encoding.h"
#include "http_digest.h"
#include "http_negotiate.h"
#include "select.h"
#include "multiif.h"
#include "easyif.h"
#include "speedcheck.h"
#include "warnless.h"
#include "non-ascii.h"
#include "inet_pton.h"
#include "getinfo.h"
#include "urlapi-int.h"
#include "system_win32.h"
#include "ftp.h"
#include "dict.h"
#include "telnet.h"
#include "tftp.h"
#include "http.h"
#include "http2.h"
#include "file.h"
#include "curl_ldap.h"
#include "vssh/ssh.h"
#include "imap.h"
#include "url.h"
#include "connect.h"
#include "inet_ntop.h"
#include "http_ntlm.h"
#include "curl_rtmp.h"
#include "gopher.h"
#include "http_proxy.h"
#include "conncache.h"
#include "multihandle.h"
#include "dotdot.h"
#include "strdup.h"
#include "setopt.h"
#include "altsvc.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
static void conn_free(struct connectdata *conn);
static unsigned int get_protocol_family(unsigned int protocol);
#if READBUFFER_SIZE < READBUFFER_MIN
#error READBUFFER_SIZE is too small
#endif
static const struct Curl_handler * const protocols[] = {
#if !defined(CURL_DISABLE_HTTP)
&Curl_handler_http,
#endif
#if defined(USE_SSL) && !defined(CURL_DISABLE_HTTP)
&Curl_handler_https,
#endif
#if !defined(CURL_DISABLE_FTP)
&Curl_handler_ftp,
#endif
#if defined(USE_SSL) && !defined(CURL_DISABLE_FTP)
&Curl_handler_ftps,
#endif
#if !defined(CURL_DISABLE_TELNET)
&Curl_handler_telnet,
#endif
#if !defined(CURL_DISABLE_DICT)
&Curl_handler_dict,
#endif
#if !defined(CURL_DISABLE_LDAP)
&Curl_handler_ldap,
#if !defined(CURL_DISABLE_LDAPS) && ((defined(USE_OPENLDAP) && defined(USE_SSL)) || (!defined(USE_OPENLDAP) && defined(HAVE_LDAP_SSL)))
&Curl_handler_ldaps,
#endif
#endif
#if !defined(CURL_DISABLE_FILE)
&Curl_handler_file,
#endif
#if !defined(CURL_DISABLE_TFTP)
&Curl_handler_tftp,
#endif
#if defined(USE_SSH) && !defined(USE_WOLFSSH)
&Curl_handler_scp,
#endif
#if defined(USE_SSH)
&Curl_handler_sftp,
#endif
#if !defined(CURL_DISABLE_IMAP)
&Curl_handler_imap,
#if defined(USE_SSL)
&Curl_handler_imaps,
#endif
#endif
#if !defined(CURL_DISABLE_POP3)
&Curl_handler_pop3,
#if defined(USE_SSL)
&Curl_handler_pop3s,
#endif
#endif
#if !defined(CURL_DISABLE_SMB) && defined(USE_NTLM) && (CURL_SIZEOF_CURL_OFF_T > 4) && (!defined(USE_WINDOWS_SSPI) || defined(USE_WIN32_CRYPTO))
&Curl_handler_smb,
#if defined(USE_SSL)
&Curl_handler_smbs,
#endif
#endif
#if !defined(CURL_DISABLE_SMTP)
&Curl_handler_smtp,
#if defined(USE_SSL)
&Curl_handler_smtps,
#endif
#endif
#if !defined(CURL_DISABLE_RTSP)
&Curl_handler_rtsp,
#endif
#if !defined(CURL_DISABLE_GOPHER)
&Curl_handler_gopher,
#endif
#if defined(USE_LIBRTMP)
&Curl_handler_rtmp,
&Curl_handler_rtmpt,
&Curl_handler_rtmpe,
&Curl_handler_rtmpte,
&Curl_handler_rtmps,
&Curl_handler_rtmpts,
#endif
(struct Curl_handler *) NULL
};
static const struct Curl_handler Curl_handler_dummy = {
"<no protocol>", 
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
ZERO_NULL, 
ZERO_NULL, 
0, 
0, 
PROTOPT_NONE 
};
void Curl_freeset(struct Curl_easy *data)
{
enum dupstring i;
for(i = (enum dupstring)0; i < STRING_LAST; i++) {
Curl_safefree(data->set.str[i]);
}
if(data->change.referer_alloc) {
Curl_safefree(data->change.referer);
data->change.referer_alloc = FALSE;
}
data->change.referer = NULL;
if(data->change.url_alloc) {
Curl_safefree(data->change.url);
data->change.url_alloc = FALSE;
}
data->change.url = NULL;
Curl_mime_cleanpart(&data->set.mimepost);
}
static void up_free(struct Curl_easy *data)
{
struct urlpieces *up = &data->state.up;
Curl_safefree(up->scheme);
Curl_safefree(up->hostname);
Curl_safefree(up->port);
Curl_safefree(up->user);
Curl_safefree(up->password);
Curl_safefree(up->options);
Curl_safefree(up->path);
Curl_safefree(up->query);
curl_url_cleanup(data->state.uh);
data->state.uh = NULL;
}
CURLcode Curl_close(struct Curl_easy **datap)
{
struct Curl_multi *m;
struct Curl_easy *data;
if(!datap || !*datap)
return CURLE_OK;
data = *datap;
*datap = NULL;
Curl_expire_clear(data); 
m = data->multi;
if(m)
curl_multi_remove_handle(data->multi, data);
if(data->multi_easy) {
curl_multi_cleanup(data->multi_easy);
data->multi_easy = NULL;
}
Curl_llist_destroy(&data->state.timeoutlist, NULL);
data->magic = 0; 
if(data->state.rangestringalloc)
free(data->state.range);
Curl_free_request_state(data);
Curl_ssl_close_all(data);
Curl_safefree(data->state.first_host);
Curl_safefree(data->state.scratch);
Curl_ssl_free_certinfo(data);
free(data->req.newurl);
data->req.newurl = NULL;
if(data->change.referer_alloc) {
Curl_safefree(data->change.referer);
data->change.referer_alloc = FALSE;
}
data->change.referer = NULL;
up_free(data);
Curl_safefree(data->state.buffer);
Curl_safefree(data->state.headerbuff);
Curl_safefree(data->state.ulbuf);
Curl_flush_cookies(data, TRUE);
#if defined(USE_ALTSVC)
Curl_altsvc_save(data, data->asi, data->set.str[STRING_ALTSVC]);
Curl_altsvc_cleanup(data->asi);
data->asi = NULL;
#endif
#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_CRYPTO_AUTH)
Curl_http_auth_cleanup_digest(data);
#endif
Curl_safefree(data->info.contenttype);
Curl_safefree(data->info.wouldredirect);
Curl_resolver_cleanup(data->state.resolver);
Curl_http2_cleanup_dependencies(data);
Curl_convert_close(data);
if(data->share) {
Curl_share_lock(data, CURL_LOCK_DATA_SHARE, CURL_LOCK_ACCESS_SINGLE);
data->share->dirty--;
Curl_share_unlock(data, CURL_LOCK_DATA_SHARE);
}
#if !defined(CURL_DISABLE_DOH)
free(data->req.doh.probe[0].serverdoh.memory);
free(data->req.doh.probe[1].serverdoh.memory);
curl_slist_free_all(data->req.doh.headers);
#endif
Curl_wildcard_dtor(&data->wildcard);
Curl_freeset(data);
free(data);
return CURLE_OK;
}
CURLcode Curl_init_userdefined(struct Curl_easy *data)
{
struct UserDefined *set = &data->set;
CURLcode result = CURLE_OK;
set->out = stdout; 
set->in_set = stdin; 
set->err = stderr; 
set->fwrite_func = (curl_write_callback)fwrite;
set->fread_func_set = (curl_read_callback)fread;
set->is_fread_set = 0;
set->is_fwrite_set = 0;
set->seek_func = ZERO_NULL;
set->seek_client = ZERO_NULL;
set->convfromnetwork = ZERO_NULL;
set->convtonetwork = ZERO_NULL;
set->convfromutf8 = ZERO_NULL;
set->filesize = -1; 
set->postfieldsize = -1; 
set->maxredirs = -1; 
set->httpreq = HTTPREQ_GET; 
set->rtspreq = RTSPREQ_OPTIONS; 
#if !defined(CURL_DISABLE_FTP)
set->ftp_use_epsv = TRUE; 
set->ftp_use_eprt = TRUE; 
set->ftp_use_pret = FALSE; 
set->ftp_filemethod = FTPFILE_MULTICWD;
#endif
set->dns_cache_timeout = 60; 
set->general_ssl.max_ssl_sessions = 5;
set->proxyport = 0;
set->proxytype = CURLPROXY_HTTP; 
set->httpauth = CURLAUTH_BASIC; 
set->proxyauth = CURLAUTH_BASIC; 
set->socks5auth = CURLAUTH_BASIC | CURLAUTH_GSSAPI;
set->hide_progress = TRUE; 
Curl_mime_initpart(&set->mimepost, data);
set->ssl.primary.verifypeer = TRUE;
set->ssl.primary.verifyhost = TRUE;
#if defined(USE_TLS_SRP)
set->ssl.authtype = CURL_TLSAUTH_NONE;
#endif
set->ssh_auth_types = CURLSSH_AUTH_DEFAULT; 
set->ssl.primary.sessionid = TRUE; 
set->proxy_ssl = set->ssl;
set->new_file_perms = 0644; 
set->new_directory_perms = 0755; 
set->allowed_protocols = CURLPROTO_ALL;
set->redir_protocols = CURLPROTO_HTTP | CURLPROTO_HTTPS | CURLPROTO_FTP |
CURLPROTO_FTPS;
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
set->socks5_gssapi_nec = FALSE;
#endif
if(Curl_ssl_backend() != CURLSSLBACKEND_SCHANNEL) {
#if defined(CURL_CA_BUNDLE)
result = Curl_setstropt(&set->str[STRING_SSL_CAFILE_ORIG], CURL_CA_BUNDLE);
if(result)
return result;
result = Curl_setstropt(&set->str[STRING_SSL_CAFILE_PROXY],
CURL_CA_BUNDLE);
if(result)
return result;
#endif
#if defined(CURL_CA_PATH)
result = Curl_setstropt(&set->str[STRING_SSL_CAPATH_ORIG], CURL_CA_PATH);
if(result)
return result;
result = Curl_setstropt(&set->str[STRING_SSL_CAPATH_PROXY], CURL_CA_PATH);
if(result)
return result;
#endif
}
set->wildcard_enabled = FALSE;
set->chunk_bgn = ZERO_NULL;
set->chunk_end = ZERO_NULL;
set->tcp_keepalive = FALSE;
set->tcp_keepintvl = 60;
set->tcp_keepidle = 60;
set->tcp_fastopen = FALSE;
set->tcp_nodelay = TRUE;
set->ssl_enable_npn = TRUE;
set->ssl_enable_alpn = TRUE;
set->expect_100_timeout = 1000L; 
set->sep_headers = TRUE; 
set->buffer_size = READBUFFER_SIZE;
set->upload_buffer_size = UPLOADBUFFER_DEFAULT;
set->happy_eyeballs_timeout = CURL_HET_DEFAULT;
set->fnmatch = ZERO_NULL;
set->upkeep_interval_ms = CURL_UPKEEP_INTERVAL_DEFAULT;
set->maxconnects = DEFAULT_CONNCACHE_SIZE; 
set->maxage_conn = 118;
set->http09_allowed = FALSE;
set->httpversion =
#if defined(USE_NGHTTP2)
CURL_HTTP_VERSION_2TLS
#else
CURL_HTTP_VERSION_1_1
#endif
;
Curl_http2_init_userset(set);
return result;
}
CURLcode Curl_open(struct Curl_easy **curl)
{
CURLcode result;
struct Curl_easy *data;
data = calloc(1, sizeof(struct Curl_easy));
if(!data) {
DEBUGF(fprintf(stderr, "Error: calloc of Curl_easy failed\n"));
return CURLE_OUT_OF_MEMORY;
}
data->magic = CURLEASY_MAGIC_NUMBER;
result = Curl_resolver_init(data, &data->state.resolver);
if(result) {
DEBUGF(fprintf(stderr, "Error: resolver_init failed\n"));
free(data);
return result;
}
data->state.buffer = malloc(READBUFFER_SIZE + 1);
if(!data->state.buffer) {
DEBUGF(fprintf(stderr, "Error: malloc of buffer failed\n"));
result = CURLE_OUT_OF_MEMORY;
}
else {
data->state.headerbuff = malloc(HEADERSIZE);
if(!data->state.headerbuff) {
DEBUGF(fprintf(stderr, "Error: malloc of headerbuff failed\n"));
result = CURLE_OUT_OF_MEMORY;
}
else {
result = Curl_init_userdefined(data);
data->state.headersize = HEADERSIZE;
Curl_convert_init(data);
Curl_initinfo(data);
data->state.lastconnect = NULL;
data->progress.flags |= PGRS_HIDE;
data->state.current_speed = -1; 
}
}
if(result) {
Curl_resolver_cleanup(data->state.resolver);
free(data->state.buffer);
free(data->state.headerbuff);
Curl_freeset(data);
free(data);
data = NULL;
}
else
*curl = data;
return result;
}
#if defined(USE_RECV_BEFORE_SEND_WORKAROUND)
static void conn_reset_postponed_data(struct connectdata *conn, int num)
{
struct postponed_data * const psnd = &(conn->postponed[num]);
if(psnd->buffer) {
DEBUGASSERT(psnd->allocated_size > 0);
DEBUGASSERT(psnd->recv_size <= psnd->allocated_size);
DEBUGASSERT(psnd->recv_size ?
(psnd->recv_processed < psnd->recv_size) :
(psnd->recv_processed == 0));
DEBUGASSERT(psnd->bindsock != CURL_SOCKET_BAD);
free(psnd->buffer);
psnd->buffer = NULL;
psnd->allocated_size = 0;
psnd->recv_size = 0;
psnd->recv_processed = 0;
#if defined(DEBUGBUILD)
psnd->bindsock = CURL_SOCKET_BAD; 
#endif 
}
else {
DEBUGASSERT(psnd->allocated_size == 0);
DEBUGASSERT(psnd->recv_size == 0);
DEBUGASSERT(psnd->recv_processed == 0);
DEBUGASSERT(psnd->bindsock == CURL_SOCKET_BAD);
}
}
static void conn_reset_all_postponed_data(struct connectdata *conn)
{
conn_reset_postponed_data(conn, 0);
conn_reset_postponed_data(conn, 1);
}
#else 
#define conn_reset_all_postponed_data(c) do {} while(0)
#endif 
static void conn_shutdown(struct connectdata *conn)
{
if(!conn)
return;
infof(conn->data, "Closing connection %ld\n", conn->connection_id);
DEBUGASSERT(conn->data);
Curl_resolver_cancel(conn);
Curl_ssl_close(conn, FIRSTSOCKET);
Curl_ssl_close(conn, SECONDARYSOCKET);
if(CURL_SOCKET_BAD != conn->sock[SECONDARYSOCKET])
Curl_closesocket(conn, conn->sock[SECONDARYSOCKET]);
if(CURL_SOCKET_BAD != conn->sock[FIRSTSOCKET])
Curl_closesocket(conn, conn->sock[FIRSTSOCKET]);
if(CURL_SOCKET_BAD != conn->tempsock[0])
Curl_closesocket(conn, conn->tempsock[0]);
if(CURL_SOCKET_BAD != conn->tempsock[1])
Curl_closesocket(conn, conn->tempsock[1]);
Curl_conncache_remove_conn(conn->data, conn, TRUE);
}
static void conn_free(struct connectdata *conn)
{
if(!conn)
return;
Curl_free_idnconverted_hostname(&conn->host);
Curl_free_idnconverted_hostname(&conn->conn_to_host);
Curl_free_idnconverted_hostname(&conn->http_proxy.host);
Curl_free_idnconverted_hostname(&conn->socks_proxy.host);
Curl_safefree(conn->user);
Curl_safefree(conn->passwd);
Curl_safefree(conn->sasl_authzid);
Curl_safefree(conn->options);
Curl_safefree(conn->http_proxy.user);
Curl_safefree(conn->socks_proxy.user);
Curl_safefree(conn->http_proxy.passwd);
Curl_safefree(conn->socks_proxy.passwd);
Curl_safefree(conn->allocptr.proxyuserpwd);
Curl_safefree(conn->allocptr.uagent);
Curl_safefree(conn->allocptr.userpwd);
Curl_safefree(conn->allocptr.accept_encoding);
Curl_safefree(conn->allocptr.te);
Curl_safefree(conn->allocptr.rangeline);
Curl_safefree(conn->allocptr.ref);
Curl_safefree(conn->allocptr.host);
Curl_safefree(conn->allocptr.cookiehost);
Curl_safefree(conn->allocptr.rtsp_transport);
Curl_safefree(conn->trailer);
Curl_safefree(conn->host.rawalloc); 
Curl_safefree(conn->conn_to_host.rawalloc); 
Curl_safefree(conn->hostname_resolve);
Curl_safefree(conn->secondaryhostname);
Curl_safefree(conn->http_proxy.host.rawalloc); 
Curl_safefree(conn->socks_proxy.host.rawalloc); 
Curl_safefree(conn->connect_state);
conn_reset_all_postponed_data(conn);
Curl_llist_destroy(&conn->easyq, NULL);
Curl_safefree(conn->localdev);
Curl_free_primary_ssl_config(&conn->ssl_config);
Curl_free_primary_ssl_config(&conn->proxy_ssl_config);
#if defined(USE_UNIX_SOCKETS)
Curl_safefree(conn->unix_domain_socket);
#endif
#if defined(USE_SSL)
Curl_safefree(conn->ssl_extra);
#endif
free(conn); 
}
CURLcode Curl_disconnect(struct Curl_easy *data,
struct connectdata *conn, bool dead_connection)
{
if(!conn)
return CURLE_OK; 
if(!data) {
DEBUGF(infof(data, "DISCONNECT without easy handle, ignoring\n"));
return CURLE_OK;
}
if(CONN_INUSE(conn) && !dead_connection) {
DEBUGF(infof(data, "Curl_disconnect when inuse: %zu\n", CONN_INUSE(conn)));
return CURLE_OK;
}
if(conn->dns_entry != NULL) {
Curl_resolv_unlock(data, conn->dns_entry);
conn->dns_entry = NULL;
}
Curl_hostcache_prune(data); 
#if !defined(CURL_DISABLE_HTTP) && defined(USE_NTLM)
Curl_http_auth_cleanup_ntlm(conn);
#endif
#if !defined(CURL_DISABLE_HTTP) && defined(USE_SPNEGO)
Curl_http_auth_cleanup_negotiate(conn);
#endif
conn->data = data;
if(conn->bits.connect_only)
dead_connection = TRUE;
if(conn->handler->disconnect)
conn->handler->disconnect(conn, dead_connection);
conn_shutdown(conn);
conn_free(conn);
return CURLE_OK;
}
static bool SocketIsDead(curl_socket_t sock)
{
int sval;
bool ret_val = TRUE;
sval = SOCKET_READABLE(sock, 0);
if(sval == 0)
ret_val = FALSE;
return ret_val;
}
static int IsMultiplexingPossible(const struct Curl_easy *handle,
const struct connectdata *conn)
{
int avail = 0;
if((conn->handler->protocol & PROTO_FAMILY_HTTP) &&
(!conn->bits.protoconnstart || !conn->bits.close)) {
if(Curl_multiplex_wanted(handle->multi) &&
(handle->set.httpversion >= CURL_HTTP_VERSION_2))
avail |= CURLPIPE_MULTIPLEX;
}
return avail;
}
#if !defined(CURL_DISABLE_PROXY)
static bool
proxy_info_matches(const struct proxy_info* data,
const struct proxy_info* needle)
{
if((data->proxytype == needle->proxytype) &&
(data->port == needle->port) &&
Curl_safe_strcasecompare(data->host.name, needle->host.name))
return TRUE;
return FALSE;
}
static bool
socks_proxy_info_matches(const struct proxy_info* data,
const struct proxy_info* needle)
{
if(!proxy_info_matches(data, needle))
return FALSE;
if((data->user == NULL) != (needle->user == NULL))
return FALSE;
if(data->user &&
needle->user &&
strcmp(data->user, needle->user) != 0)
return FALSE;
if((data->passwd == NULL) != (needle->passwd == NULL))
return FALSE;
if(data->passwd &&
needle->passwd &&
strcmp(data->passwd, needle->passwd) != 0)
return FALSE;
return TRUE;
}
#else
#define proxy_info_matches(x,y) FALSE
#define socks_proxy_info_matches(x,y) FALSE
#endif
static bool conn_maxage(struct Curl_easy *data,
struct connectdata *conn,
struct curltime now)
{
if(!conn->data) {
timediff_t idletime = Curl_timediff(now, conn->lastused);
idletime /= 1000; 
if(idletime > data->set.maxage_conn) {
infof(data, "Too old connection (%ld seconds), disconnect it\n",
idletime);
return TRUE;
}
}
return FALSE;
}
static bool extract_if_dead(struct connectdata *conn,
struct Curl_easy *data)
{
if(!CONN_INUSE(conn) && !conn->data) {
bool dead;
struct curltime now = Curl_now();
if(conn_maxage(data, conn, now)) {
dead = TRUE;
}
else if(conn->handler->connection_check) {
unsigned int state;
struct Curl_easy *olddata = conn->data;
conn->data = data; 
state = conn->handler->connection_check(conn, CONNCHECK_ISDEAD);
conn->data = olddata;
dead = (state & CONNRESULT_DEAD);
}
else {
dead = SocketIsDead(conn->sock[FIRSTSOCKET]);
}
if(dead) {
infof(data, "Connection %ld seems to be dead!\n", conn->connection_id);
Curl_conncache_remove_conn(data, conn, FALSE);
return TRUE;
}
}
return FALSE;
}
struct prunedead {
struct Curl_easy *data;
struct connectdata *extracted;
};
static int call_extract_if_dead(struct connectdata *conn, void *param)
{
struct prunedead *p = (struct prunedead *)param;
if(extract_if_dead(conn, p->data)) {
p->extracted = conn;
return 1;
}
return 0; 
}
static void prune_dead_connections(struct Curl_easy *data)
{
struct curltime now = Curl_now();
timediff_t elapsed =
Curl_timediff(now, data->state.conn_cache->last_cleanup);
if(elapsed >= 1000L) {
struct prunedead prune;
prune.data = data;
prune.extracted = NULL;
while(Curl_conncache_foreach(data, data->state.conn_cache, &prune,
call_extract_if_dead)) {
(void)Curl_disconnect(data, prune.extracted, TRUE);
}
data->state.conn_cache->last_cleanup = now;
}
}
static bool
ConnectionExists(struct Curl_easy *data,
struct connectdata *needle,
struct connectdata **usethis,
bool *force_reuse,
bool *waitpipe)
{
struct connectdata *check;
struct connectdata *chosen = 0;
bool foundPendingCandidate = FALSE;
bool canmultiplex = IsMultiplexingPossible(data, needle);
struct connectbundle *bundle;
const char *hostbundle;
#if defined(USE_NTLM)
bool wantNTLMhttp = ((data->state.authhost.want &
(CURLAUTH_NTLM | CURLAUTH_NTLM_WB)) &&
(needle->handler->protocol & PROTO_FAMILY_HTTP));
bool wantProxyNTLMhttp = (needle->bits.proxy_user_passwd &&
((data->state.authproxy.want &
(CURLAUTH_NTLM | CURLAUTH_NTLM_WB)) &&
(needle->handler->protocol & PROTO_FAMILY_HTTP)));
#endif
*force_reuse = FALSE;
*waitpipe = FALSE;
bundle = Curl_conncache_find_bundle(needle, data->state.conn_cache,
&hostbundle);
if(bundle) {
struct curl_llist_element *curr;
infof(data, "Found bundle for host %s: %p [%s]\n",
hostbundle, (void *)bundle, (bundle->multiuse == BUNDLE_MULTIPLEX ?
"can multiplex" : "serially"));
if(canmultiplex) {
if(bundle->multiuse == BUNDLE_UNKNOWN) {
if(data->set.pipewait) {
infof(data, "Server doesn't support multiplex yet, wait\n");
*waitpipe = TRUE;
Curl_conncache_unlock(data);
return FALSE; 
}
infof(data, "Server doesn't support multiplex (yet)\n");
canmultiplex = FALSE;
}
if((bundle->multiuse == BUNDLE_MULTIPLEX) &&
!Curl_multiplex_wanted(data->multi)) {
infof(data, "Could multiplex, but not asked to!\n");
canmultiplex = FALSE;
}
if(bundle->multiuse == BUNDLE_NO_MULTIUSE) {
infof(data, "Can not multiplex, even if we wanted to!\n");
canmultiplex = FALSE;
}
}
curr = bundle->conn_list.head;
while(curr) {
bool match = FALSE;
size_t multiplexed = 0;
check = curr->ptr;
curr = curr->next;
if(check->bits.connect_only || check->bits.close)
continue;
if(bundle->multiuse == BUNDLE_MULTIPLEX)
multiplexed = CONN_INUSE(check);
if(canmultiplex) {
;
}
else {
if(multiplexed) {
continue;
}
if(Curl_resolver_asynch()) {
if(!check->ip_addr_str[0]) {
infof(data,
"Connection #%ld is still name resolving, can't reuse\n",
check->connection_id);
continue;
}
}
if(check->sock[FIRSTSOCKET] == CURL_SOCKET_BAD) {
foundPendingCandidate = TRUE;
infof(data, "Connection #%ld isn't open enough, can't reuse\n",
check->connection_id);
continue;
}
}
#if defined(USE_UNIX_SOCKETS)
if(needle->unix_domain_socket) {
if(!check->unix_domain_socket)
continue;
if(strcmp(needle->unix_domain_socket, check->unix_domain_socket))
continue;
if(needle->abstract_unix_socket != check->abstract_unix_socket)
continue;
}
else if(check->unix_domain_socket)
continue;
#endif
if((needle->handler->flags&PROTOPT_SSL) !=
(check->handler->flags&PROTOPT_SSL))
if(get_protocol_family(check->handler->protocol) !=
needle->handler->protocol || !check->tls_upgraded)
continue;
if(needle->bits.httpproxy != check->bits.httpproxy ||
needle->bits.socksproxy != check->bits.socksproxy)
continue;
if(needle->bits.socksproxy &&
!socks_proxy_info_matches(&needle->socks_proxy,
&check->socks_proxy))
continue;
if(needle->bits.conn_to_host != check->bits.conn_to_host)
continue;
if(needle->bits.conn_to_port != check->bits.conn_to_port)
continue;
if(needle->bits.httpproxy) {
if(!proxy_info_matches(&needle->http_proxy, &check->http_proxy))
continue;
if(needle->bits.tunnel_proxy != check->bits.tunnel_proxy)
continue;
if(needle->http_proxy.proxytype == CURLPROXY_HTTPS) {
if(needle->handler->flags&PROTOPT_SSL) {
if(!Curl_ssl_config_matches(&needle->proxy_ssl_config,
&check->proxy_ssl_config))
continue;
if(check->proxy_ssl[FIRSTSOCKET].state != ssl_connection_complete)
continue;
}
else {
if(!Curl_ssl_config_matches(&needle->ssl_config,
&check->ssl_config))
continue;
if(check->ssl[FIRSTSOCKET].state != ssl_connection_complete)
continue;
}
}
}
DEBUGASSERT(!check->data || GOOD_EASY_HANDLE(check->data));
if(!canmultiplex && check->data)
continue;
if(check->data && (check->data->multi != needle->data->multi))
continue;
if(needle->localdev || needle->localport) {
if((check->localport != needle->localport) ||
(check->localportrange != needle->localportrange) ||
(needle->localdev &&
(!check->localdev || strcmp(check->localdev, needle->localdev))))
continue;
}
if(!(needle->handler->flags & PROTOPT_CREDSPERREQUEST)) {
if(strcmp(needle->user, check->user) ||
strcmp(needle->passwd, check->passwd)) {
continue;
}
}
if(!needle->bits.httpproxy || (needle->handler->flags&PROTOPT_SSL) ||
needle->bits.tunnel_proxy) {
if((strcasecompare(needle->handler->scheme, check->handler->scheme) ||
(get_protocol_family(check->handler->protocol) ==
needle->handler->protocol && check->tls_upgraded)) &&
(!needle->bits.conn_to_host || strcasecompare(
needle->conn_to_host.name, check->conn_to_host.name)) &&
(!needle->bits.conn_to_port ||
needle->conn_to_port == check->conn_to_port) &&
strcasecompare(needle->host.name, check->host.name) &&
needle->remote_port == check->remote_port) {
if(needle->handler->flags & PROTOPT_SSL) {
if(!Curl_ssl_config_matches(&needle->ssl_config,
&check->ssl_config)) {
DEBUGF(infof(data,
"Connection #%ld has different SSL parameters, "
"can't reuse\n",
check->connection_id));
continue;
}
if(check->ssl[FIRSTSOCKET].state != ssl_connection_complete) {
foundPendingCandidate = TRUE;
DEBUGF(infof(data,
"Connection #%ld has not started SSL connect, "
"can't reuse\n",
check->connection_id));
continue;
}
}
match = TRUE;
}
}
else {
match = TRUE;
}
if(match) {
#if defined(USE_NTLM)
if(wantNTLMhttp) {
if(strcmp(needle->user, check->user) ||
strcmp(needle->passwd, check->passwd)) {
if(check->http_ntlm_state == NTLMSTATE_NONE)
chosen = check;
continue;
}
}
else if(check->http_ntlm_state != NTLMSTATE_NONE) {
continue;
}
if(wantProxyNTLMhttp) {
if(!check->http_proxy.user || !check->http_proxy.passwd)
continue;
if(strcmp(needle->http_proxy.user, check->http_proxy.user) ||
strcmp(needle->http_proxy.passwd, check->http_proxy.passwd))
continue;
}
else if(check->proxy_ntlm_state != NTLMSTATE_NONE) {
continue;
}
if(wantNTLMhttp || wantProxyNTLMhttp) {
chosen = check;
if((wantNTLMhttp &&
(check->http_ntlm_state != NTLMSTATE_NONE)) ||
(wantProxyNTLMhttp &&
(check->proxy_ntlm_state != NTLMSTATE_NONE))) {
*force_reuse = TRUE;
break;
}
continue;
}
#endif
if(canmultiplex) {
if(!multiplexed) {
chosen = check;
break;
}
#if defined(USE_NGHTTP2)
if(check->bits.multiplex) {
struct http_conn *httpc = &check->proto.httpc;
if(multiplexed >= httpc->settings.max_concurrent_streams) {
infof(data, "MAX_CONCURRENT_STREAMS reached, skip (%zu)\n",
multiplexed);
continue;
}
else if(multiplexed >=
Curl_multi_max_concurrent_streams(needle->data->multi)) {
infof(data, "client side MAX_CONCURRENT_STREAMS reached"
", skip (%zu)\n",
multiplexed);
continue;
}
}
#endif
chosen = check;
infof(data, "Multiplexed connection found!\n");
break;
}
else {
chosen = check;
break;
}
}
}
}
if(chosen) {
chosen->data = data; 
Curl_conncache_unlock(data);
*usethis = chosen;
return TRUE; 
}
Curl_conncache_unlock(data);
if(foundPendingCandidate && data->set.pipewait) {
infof(data,
"Found pending candidate for reuse and CURLOPT_PIPEWAIT is set\n");
*waitpipe = TRUE;
}
return FALSE; 
}
#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
void Curl_verboseconnect(struct connectdata *conn)
{
if(conn->data->set.verbose)
infof(conn->data, "Connected to %s (%s) port %ld (#%ld)\n",
conn->bits.socksproxy ? conn->socks_proxy.host.dispname :
conn->bits.httpproxy ? conn->http_proxy.host.dispname :
conn->bits.conn_to_host ? conn->conn_to_host.dispname :
conn->host.dispname,
conn->ip_addr_str, conn->port, conn->connection_id);
}
#endif
bool Curl_is_ASCII_name(const char *hostname)
{
const unsigned char *ch = (const unsigned char *)hostname;
if(!hostname) 
return TRUE;
while(*ch) {
if(*ch++ & 0x80)
return FALSE;
}
return TRUE;
}
static void strip_trailing_dot(struct hostname *host)
{
size_t len;
if(!host || !host->name)
return;
len = strlen(host->name);
if(len && (host->name[len-1] == '.'))
host->name[len-1] = 0;
}
CURLcode Curl_idnconvert_hostname(struct connectdata *conn,
struct hostname *host)
{
struct Curl_easy *data = conn->data;
#if !defined(USE_LIBIDN2)
(void)data;
(void)conn;
#elif defined(CURL_DISABLE_VERBOSE_STRINGS)
(void)conn;
#endif
host->dispname = host->name;
if(!Curl_is_ASCII_name(host->name)) {
#if defined(USE_LIBIDN2)
if(idn2_check_version(IDN2_VERSION)) {
char *ace_hostname = NULL;
#if IDN2_VERSION_NUMBER >= 0x00140000
int flags = IDN2_NFC_INPUT | IDN2_NONTRANSITIONAL;
#else
int flags = IDN2_NFC_INPUT;
#endif
int rc = idn2_lookup_ul((const char *)host->name, &ace_hostname, flags);
if(rc == IDN2_OK) {
host->encalloc = (char *)ace_hostname;
host->name = host->encalloc;
}
else {
failf(data, "Failed to convert %s to ACE; %s\n", host->name,
idn2_strerror(rc));
return CURLE_URL_MALFORMAT;
}
}
#elif defined(USE_WIN32_IDN)
char *ace_hostname = NULL;
if(curl_win32_idn_to_ascii(host->name, &ace_hostname)) {
host->encalloc = ace_hostname;
host->name = host->encalloc;
}
else {
char buffer[STRERROR_LEN];
failf(data, "Failed to convert %s to ACE; %s\n", host->name,
Curl_winapi_strerror(GetLastError(), buffer, sizeof(buffer)));
return CURLE_URL_MALFORMAT;
}
#else
infof(data, "IDN support not present, can't parse Unicode domains\n");
#endif
}
return CURLE_OK;
}
void Curl_free_idnconverted_hostname(struct hostname *host)
{
#if defined(USE_LIBIDN2)
if(host->encalloc) {
idn2_free(host->encalloc); 
host->encalloc = NULL;
}
#elif defined(USE_WIN32_IDN)
free(host->encalloc); 
host->encalloc = NULL;
#else
(void)host;
#endif
}
static struct connectdata *allocate_conn(struct Curl_easy *data)
{
struct connectdata *conn = calloc(1, sizeof(struct connectdata));
if(!conn)
return NULL;
#if defined(USE_SSL)
{
size_t sslsize = Curl_ssl->sizeof_ssl_backend_data;
char *ssl = calloc(4, sslsize);
if(!ssl) {
free(conn);
return NULL;
}
conn->ssl_extra = ssl;
conn->ssl[0].backend = (void *)ssl;
conn->ssl[1].backend = (void *)(ssl + sslsize);
conn->proxy_ssl[0].backend = (void *)(ssl + 2 * sslsize);
conn->proxy_ssl[1].backend = (void *)(ssl + 3 * sslsize);
}
#endif
conn->handler = &Curl_handler_dummy; 
conn->sock[FIRSTSOCKET] = CURL_SOCKET_BAD; 
conn->sock[SECONDARYSOCKET] = CURL_SOCKET_BAD; 
conn->tempsock[0] = CURL_SOCKET_BAD; 
conn->tempsock[1] = CURL_SOCKET_BAD; 
conn->connection_id = -1; 
conn->port = -1; 
conn->remote_port = -1; 
#if defined(USE_RECV_BEFORE_SEND_WORKAROUND) && defined(DEBUGBUILD)
conn->postponed[0].bindsock = CURL_SOCKET_BAD; 
conn->postponed[1].bindsock = CURL_SOCKET_BAD; 
#endif 
connclose(conn, "Default to force-close");
conn->created = Curl_now();
conn->keepalive = Curl_now();
conn->upkeep_interval_ms = data->set.upkeep_interval_ms;
conn->data = data; 
conn->http_proxy.proxytype = data->set.proxytype;
conn->socks_proxy.proxytype = CURLPROXY_SOCKS4;
#if !defined(CURL_DISABLE_PROXY)
conn->bits.proxy = (data->set.str[STRING_PROXY] &&
*data->set.str[STRING_PROXY]) ? TRUE : FALSE;
conn->bits.httpproxy = (conn->bits.proxy &&
(conn->http_proxy.proxytype == CURLPROXY_HTTP ||
conn->http_proxy.proxytype == CURLPROXY_HTTP_1_0 ||
conn->http_proxy.proxytype == CURLPROXY_HTTPS)) ?
TRUE : FALSE;
conn->bits.socksproxy = (conn->bits.proxy &&
!conn->bits.httpproxy) ? TRUE : FALSE;
if(data->set.str[STRING_PRE_PROXY] && *data->set.str[STRING_PRE_PROXY]) {
conn->bits.proxy = TRUE;
conn->bits.socksproxy = TRUE;
}
conn->bits.proxy_user_passwd =
(data->set.str[STRING_PROXYUSERNAME]) ? TRUE : FALSE;
conn->bits.tunnel_proxy = data->set.tunnel_thru_httpproxy;
#endif 
conn->bits.user_passwd = (data->set.str[STRING_USERNAME]) ? TRUE : FALSE;
#if !defined(CURL_DISABLE_FTP)
conn->bits.ftp_use_epsv = data->set.ftp_use_epsv;
conn->bits.ftp_use_eprt = data->set.ftp_use_eprt;
#endif
conn->ssl_config.verifystatus = data->set.ssl.primary.verifystatus;
conn->ssl_config.verifypeer = data->set.ssl.primary.verifypeer;
conn->ssl_config.verifyhost = data->set.ssl.primary.verifyhost;
conn->proxy_ssl_config.verifystatus =
data->set.proxy_ssl.primary.verifystatus;
conn->proxy_ssl_config.verifypeer = data->set.proxy_ssl.primary.verifypeer;
conn->proxy_ssl_config.verifyhost = data->set.proxy_ssl.primary.verifyhost;
conn->ip_version = data->set.ipver;
conn->bits.connect_only = data->set.connect_only;
conn->transport = TRNSPRT_TCP; 
#if !defined(CURL_DISABLE_HTTP) && defined(USE_NTLM) && defined(NTLM_WB_ENABLED)
conn->ntlm.ntlm_auth_hlpr_socket = CURL_SOCKET_BAD;
conn->proxyntlm.ntlm_auth_hlpr_socket = CURL_SOCKET_BAD;
#endif
Curl_llist_init(&conn->easyq, NULL);
#if defined(HAVE_GSSAPI)
conn->data_prot = PROT_CLEAR;
#endif
if(data->set.str[STRING_DEVICE]) {
conn->localdev = strdup(data->set.str[STRING_DEVICE]);
if(!conn->localdev)
goto error;
}
conn->localportrange = data->set.localportrange;
conn->localport = data->set.localport;
conn->fclosesocket = data->set.fclosesocket;
conn->closesocket_client = data->set.closesocket_client;
conn->lastused = Curl_now(); 
return conn;
error:
Curl_llist_destroy(&conn->easyq, NULL);
free(conn->localdev);
#if defined(USE_SSL)
free(conn->ssl_extra);
#endif
free(conn);
return NULL;
}
const struct Curl_handler *Curl_builtin_scheme(const char *scheme)
{
const struct Curl_handler * const *pp;
const struct Curl_handler *p;
for(pp = protocols; (p = *pp) != NULL; pp++)
if(strcasecompare(p->scheme, scheme))
return p;
return NULL; 
}
static CURLcode findprotocol(struct Curl_easy *data,
struct connectdata *conn,
const char *protostr)
{
const struct Curl_handler *p = Curl_builtin_scheme(protostr);
if(p && 
(data->set.allowed_protocols & p->protocol)) {
if(data->state.this_is_a_follow &&
!(data->set.redir_protocols & p->protocol))
;
else {
conn->handler = conn->given = p;
return CURLE_OK;
}
}
failf(data, "Protocol \"%s\" not supported or disabled in " LIBCURL_NAME,
protostr);
return CURLE_UNSUPPORTED_PROTOCOL;
}
CURLcode Curl_uc_to_curlcode(CURLUcode uc)
{
switch(uc) {
default:
return CURLE_URL_MALFORMAT;
case CURLUE_UNSUPPORTED_SCHEME:
return CURLE_UNSUPPORTED_PROTOCOL;
case CURLUE_OUT_OF_MEMORY:
return CURLE_OUT_OF_MEMORY;
case CURLUE_USER_NOT_ALLOWED:
return CURLE_LOGIN_DENIED;
}
}
static void zonefrom_url(CURLU *uh, struct connectdata *conn)
{
char *zoneid;
CURLUcode uc;
uc = curl_url_get(uh, CURLUPART_ZONEID, &zoneid, 0);
if(!uc && zoneid) {
char *endp;
unsigned long scope = strtoul(zoneid, &endp, 10);
if(!*endp && (scope < UINT_MAX))
conn->scope_id = (unsigned int)scope;
#if defined(HAVE_IF_NAMETOINDEX)
else {
#elif defined(WIN32)
else if(Curl_if_nametoindex) {
#endif
#if defined(HAVE_IF_NAMETOINDEX) || defined(WIN32)
unsigned int scopeidx = 0;
#if defined(WIN32)
scopeidx = Curl_if_nametoindex(zoneid);
#else
scopeidx = if_nametoindex(zoneid);
#endif
if(!scopeidx)
infof(conn->data, "Invalid zoneid: %s; %s\n", zoneid,
strerror(errno));
else
conn->scope_id = scopeidx;
}
#endif 
free(zoneid);
}
}
static CURLcode parseurlandfillconn(struct Curl_easy *data,
struct connectdata *conn)
{
CURLcode result;
CURLU *uh;
CURLUcode uc;
char *hostname;
up_free(data); 
if(data->set.uh) {
uh = data->state.uh = curl_url_dup(data->set.uh);
}
else {
uh = data->state.uh = curl_url();
}
if(!uh)
return CURLE_OUT_OF_MEMORY;
if(data->set.str[STRING_DEFAULT_PROTOCOL] &&
!Curl_is_absolute_url(data->change.url, NULL, MAX_SCHEME_LEN)) {
char *url;
if(data->change.url_alloc)
free(data->change.url);
url = aprintf("%s://%s", data->set.str[STRING_DEFAULT_PROTOCOL],
data->change.url);
if(!url)
return CURLE_OUT_OF_MEMORY;
data->change.url = url;
data->change.url_alloc = TRUE;
}
if(!data->set.uh) {
char *newurl;
uc = curl_url_set(uh, CURLUPART_URL, data->change.url,
CURLU_GUESS_SCHEME |
CURLU_NON_SUPPORT_SCHEME |
(data->set.disallow_username_in_url ?
CURLU_DISALLOW_USER : 0) |
(data->set.path_as_is ? CURLU_PATH_AS_IS : 0));
if(uc) {
DEBUGF(infof(data, "curl_url_set rejected %s\n", data->change.url));
return Curl_uc_to_curlcode(uc);
}
uc = curl_url_get(uh, CURLUPART_URL, &newurl, 0);
if(uc)
return Curl_uc_to_curlcode(uc);
if(data->change.url_alloc)
free(data->change.url);
data->change.url = newurl;
data->change.url_alloc = TRUE;
}
uc = curl_url_get(uh, CURLUPART_SCHEME, &data->state.up.scheme, 0);
if(uc)
return Curl_uc_to_curlcode(uc);
result = findprotocol(data, conn, data->state.up.scheme);
if(result)
return result;
uc = curl_url_get(uh, CURLUPART_USER, &data->state.up.user,
CURLU_URLDECODE);
if(!uc) {
conn->user = strdup(data->state.up.user);
if(!conn->user)
return CURLE_OUT_OF_MEMORY;
conn->bits.user_passwd = TRUE;
}
else if(uc != CURLUE_NO_USER)
return Curl_uc_to_curlcode(uc);
uc = curl_url_get(uh, CURLUPART_PASSWORD, &data->state.up.password,
CURLU_URLDECODE);
if(!uc) {
conn->passwd = strdup(data->state.up.password);
if(!conn->passwd)
return CURLE_OUT_OF_MEMORY;
conn->bits.user_passwd = TRUE;
}
else if(uc != CURLUE_NO_PASSWORD)
return Curl_uc_to_curlcode(uc);
uc = curl_url_get(uh, CURLUPART_OPTIONS, &data->state.up.options,
CURLU_URLDECODE);
if(!uc) {
conn->options = strdup(data->state.up.options);
if(!conn->options)
return CURLE_OUT_OF_MEMORY;
}
else if(uc != CURLUE_NO_OPTIONS)
return Curl_uc_to_curlcode(uc);
uc = curl_url_get(uh, CURLUPART_HOST, &data->state.up.hostname, 0);
if(uc) {
if(!strcasecompare("file", data->state.up.scheme))
return CURLE_OUT_OF_MEMORY;
}
uc = curl_url_get(uh, CURLUPART_PATH, &data->state.up.path, 0);
if(uc)
return Curl_uc_to_curlcode(uc);
uc = curl_url_get(uh, CURLUPART_PORT, &data->state.up.port,
CURLU_DEFAULT_PORT);
if(uc) {
if(!strcasecompare("file", data->state.up.scheme))
return CURLE_OUT_OF_MEMORY;
}
else {
unsigned long port = strtoul(data->state.up.port, NULL, 10);
conn->port = conn->remote_port = curlx_ultous(port);
}
(void)curl_url_get(uh, CURLUPART_QUERY, &data->state.up.query, 0);
hostname = data->state.up.hostname;
if(hostname && hostname[0] == '[') {
size_t hlen;
conn->bits.ipv6_ip = TRUE;
hostname++;
hlen = strlen(hostname);
hostname[hlen - 1] = 0;
zonefrom_url(uh, conn);
}
conn->host.rawalloc = strdup(hostname ? hostname : "");
if(!conn->host.rawalloc)
return CURLE_OUT_OF_MEMORY;
conn->host.name = conn->host.rawalloc;
if(data->set.scope_id)
conn->scope_id = data->set.scope_id;
return CURLE_OK;
}
static CURLcode setup_range(struct Curl_easy *data)
{
struct UrlState *s = &data->state;
s->resume_from = data->set.set_resume_from;
if(s->resume_from || data->set.str[STRING_SET_RANGE]) {
if(s->rangestringalloc)
free(s->range);
if(s->resume_from)
s->range = aprintf("%" CURL_FORMAT_CURL_OFF_T "-", s->resume_from);
else
s->range = strdup(data->set.str[STRING_SET_RANGE]);
s->rangestringalloc = (s->range) ? TRUE : FALSE;
if(!s->range)
return CURLE_OUT_OF_MEMORY;
s->use_range = TRUE; 
}
else
s->use_range = FALSE; 
return CURLE_OK;
}
static CURLcode setup_connection_internals(struct connectdata *conn)
{
const struct Curl_handler * p;
CURLcode result;
p = conn->handler;
if(p->setup_connection) {
result = (*p->setup_connection)(conn);
if(result)
return result;
p = conn->handler; 
}
if(conn->port < 0)
conn->port = p->defport;
return CURLE_OK;
}
void Curl_free_request_state(struct Curl_easy *data)
{
Curl_safefree(data->req.protop);
Curl_safefree(data->req.newurl);
#if !defined(CURL_DISABLE_DOH)
Curl_close(&data->req.doh.probe[0].easy);
Curl_close(&data->req.doh.probe[1].easy);
#endif
}
#if !defined(CURL_DISABLE_PROXY)
static bool check_noproxy(const char *name, const char *no_proxy)
{
if(no_proxy && no_proxy[0]) {
size_t tok_start;
size_t tok_end;
const char *separator = ", ";
size_t no_proxy_len;
size_t namelen;
char *endptr;
if(strcasecompare("*", no_proxy)) {
return TRUE;
}
no_proxy_len = strlen(no_proxy);
if(name[0] == '[') {
endptr = strchr(name, ']');
if(!endptr)
return FALSE;
name++;
namelen = endptr - name;
}
else
namelen = strlen(name);
for(tok_start = 0; tok_start < no_proxy_len; tok_start = tok_end + 1) {
while(tok_start < no_proxy_len &&
strchr(separator, no_proxy[tok_start]) != NULL) {
++tok_start;
}
if(tok_start == no_proxy_len)
break; 
for(tok_end = tok_start; tok_end < no_proxy_len &&
strchr(separator, no_proxy[tok_end]) == NULL; ++tok_end)
;
if(no_proxy[tok_start] == '.')
++tok_start;
if((tok_end - tok_start) <= namelen) {
const char *checkn = name + namelen - (tok_end - tok_start);
if(strncasecompare(no_proxy + tok_start, checkn,
tok_end - tok_start)) {
if((tok_end - tok_start) == namelen || *(checkn - 1) == '.') {
return TRUE;
}
}
} 
} 
} 
return FALSE;
}
#if !defined(CURL_DISABLE_HTTP)
static char *detect_proxy(struct connectdata *conn)
{
char *proxy = NULL;
char proxy_env[128];
const char *protop = conn->handler->scheme;
char *envp = proxy_env;
char *prox;
while(*protop)
*envp++ = (char)tolower((int)*protop++);
strcpy(envp, "_proxy");
prox = curl_getenv(proxy_env);
if(!prox && !strcasecompare("http_proxy", proxy_env)) {
Curl_strntoupper(proxy_env, proxy_env, sizeof(proxy_env));
prox = curl_getenv(proxy_env);
}
envp = proxy_env;
if(prox) {
proxy = prox; 
}
else {
envp = (char *)"all_proxy";
proxy = curl_getenv(envp); 
if(!proxy) {
envp = (char *)"ALL_PROXY";
proxy = curl_getenv(envp);
}
}
if(proxy)
infof(conn->data, "Uses proxy env variable %s == '%s'\n", envp, proxy);
return proxy;
}
#endif 
static CURLcode parse_proxy(struct Curl_easy *data,
struct connectdata *conn, char *proxy,
curl_proxytype proxytype)
{
char *portptr = NULL;
long port = -1;
char *proxyuser = NULL;
char *proxypasswd = NULL;
char *host;
bool sockstype;
CURLUcode uc;
struct proxy_info *proxyinfo;
CURLU *uhp = curl_url();
CURLcode result = CURLE_OK;
char *scheme = NULL;
uc = curl_url_set(uhp, CURLUPART_URL, proxy,
CURLU_NON_SUPPORT_SCHEME|CURLU_GUESS_SCHEME);
if(!uc) {
uc = curl_url_get(uhp, CURLUPART_SCHEME, &scheme, 0);
if(uc) {
result = CURLE_OUT_OF_MEMORY;
goto error;
}
if(strcasecompare("https", scheme))
proxytype = CURLPROXY_HTTPS;
else if(strcasecompare("socks5h", scheme))
proxytype = CURLPROXY_SOCKS5_HOSTNAME;
else if(strcasecompare("socks5", scheme))
proxytype = CURLPROXY_SOCKS5;
else if(strcasecompare("socks4a", scheme))
proxytype = CURLPROXY_SOCKS4A;
else if(strcasecompare("socks4", scheme) ||
strcasecompare("socks", scheme))
proxytype = CURLPROXY_SOCKS4;
else if(strcasecompare("http", scheme))
; 
else {
failf(data, "Unsupported proxy scheme for \'%s\'", proxy);
result = CURLE_COULDNT_CONNECT;
goto error;
}
}
else {
failf(data, "Unsupported proxy syntax in \'%s\'", proxy);
result = CURLE_COULDNT_RESOLVE_PROXY;
goto error;
}
#if defined(USE_SSL)
if(!(Curl_ssl->supports & SSLSUPP_HTTPS_PROXY))
#endif
if(proxytype == CURLPROXY_HTTPS) {
failf(data, "Unsupported proxy \'%s\', libcurl is built without the "
"HTTPS-proxy support.", proxy);
result = CURLE_NOT_BUILT_IN;
goto error;
}
sockstype =
proxytype == CURLPROXY_SOCKS5_HOSTNAME ||
proxytype == CURLPROXY_SOCKS5 ||
proxytype == CURLPROXY_SOCKS4A ||
proxytype == CURLPROXY_SOCKS4;
proxyinfo = sockstype ? &conn->socks_proxy : &conn->http_proxy;
proxyinfo->proxytype = proxytype;
curl_url_get(uhp, CURLUPART_USER, &proxyuser, CURLU_URLDECODE);
curl_url_get(uhp, CURLUPART_PASSWORD, &proxypasswd, CURLU_URLDECODE);
if(proxyuser || proxypasswd) {
Curl_safefree(proxyinfo->user);
proxyinfo->user = proxyuser;
Curl_safefree(proxyinfo->passwd);
if(!proxypasswd) {
proxypasswd = strdup("");
if(!proxypasswd) {
result = CURLE_OUT_OF_MEMORY;
goto error;
}
}
proxyinfo->passwd = proxypasswd;
conn->bits.proxy_user_passwd = TRUE; 
}
curl_url_get(uhp, CURLUPART_PORT, &portptr, 0);
if(portptr) {
port = strtol(portptr, NULL, 10);
free(portptr);
}
else {
if(data->set.proxyport)
port = data->set.proxyport;
else {
if(proxytype == CURLPROXY_HTTPS)
port = CURL_DEFAULT_HTTPS_PROXY_PORT;
else
port = CURL_DEFAULT_PROXY_PORT;
}
}
if(port >= 0) {
proxyinfo->port = port;
if(conn->port < 0 || sockstype || !conn->socks_proxy.host.rawalloc)
conn->port = port;
}
uc = curl_url_get(uhp, CURLUPART_HOST, &host, CURLU_URLDECODE);
if(uc) {
result = CURLE_OUT_OF_MEMORY;
goto error;
}
Curl_safefree(proxyinfo->host.rawalloc);
proxyinfo->host.rawalloc = host;
if(host[0] == '[') {
size_t len = strlen(host);
host[len-1] = 0; 
host++;
zonefrom_url(uhp, conn);
}
proxyinfo->host.name = host;
error:
free(scheme);
curl_url_cleanup(uhp);
return result;
}
static CURLcode parse_proxy_auth(struct Curl_easy *data,
struct connectdata *conn)
{
char proxyuser[MAX_CURL_USER_LENGTH]="";
char proxypasswd[MAX_CURL_PASSWORD_LENGTH]="";
CURLcode result;
if(data->set.str[STRING_PROXYUSERNAME] != NULL) {
strncpy(proxyuser, data->set.str[STRING_PROXYUSERNAME],
MAX_CURL_USER_LENGTH);
proxyuser[MAX_CURL_USER_LENGTH-1] = '\0'; 
}
if(data->set.str[STRING_PROXYPASSWORD] != NULL) {
strncpy(proxypasswd, data->set.str[STRING_PROXYPASSWORD],
MAX_CURL_PASSWORD_LENGTH);
proxypasswd[MAX_CURL_PASSWORD_LENGTH-1] = '\0'; 
}
result = Curl_urldecode(data, proxyuser, 0, &conn->http_proxy.user, NULL,
FALSE);
if(!result)
result = Curl_urldecode(data, proxypasswd, 0, &conn->http_proxy.passwd,
NULL, FALSE);
return result;
}
static CURLcode create_conn_helper_init_proxy(struct connectdata *conn)
{
char *proxy = NULL;
char *socksproxy = NULL;
char *no_proxy = NULL;
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
if(conn->bits.proxy_user_passwd) {
result = parse_proxy_auth(data, conn);
if(result)
goto out;
}
if(data->set.str[STRING_PROXY]) {
proxy = strdup(data->set.str[STRING_PROXY]);
if(NULL == proxy) {
failf(data, "memory shortage");
result = CURLE_OUT_OF_MEMORY;
goto out;
}
}
if(data->set.str[STRING_PRE_PROXY]) {
socksproxy = strdup(data->set.str[STRING_PRE_PROXY]);
if(NULL == socksproxy) {
failf(data, "memory shortage");
result = CURLE_OUT_OF_MEMORY;
goto out;
}
}
if(!data->set.str[STRING_NOPROXY]) {
const char *p = "no_proxy";
no_proxy = curl_getenv(p);
if(!no_proxy) {
p = "NO_PROXY";
no_proxy = curl_getenv(p);
}
if(no_proxy) {
infof(conn->data, "Uses proxy env variable %s == '%s'\n", p, no_proxy);
}
}
if(check_noproxy(conn->host.name, data->set.str[STRING_NOPROXY] ?
data->set.str[STRING_NOPROXY] : no_proxy)) {
Curl_safefree(proxy);
Curl_safefree(socksproxy);
}
#if !defined(CURL_DISABLE_HTTP)
else if(!proxy && !socksproxy)
proxy = detect_proxy(conn);
#endif 
Curl_safefree(no_proxy);
#if defined(USE_UNIX_SOCKETS)
if(proxy && conn->unix_domain_socket) {
free(proxy);
proxy = NULL;
}
#endif
if(proxy && (!*proxy || (conn->handler->flags & PROTOPT_NONETWORK))) {
free(proxy); 
proxy = NULL;
}
if(socksproxy && (!*socksproxy ||
(conn->handler->flags & PROTOPT_NONETWORK))) {
free(socksproxy); 
socksproxy = NULL;
}
if(proxy || socksproxy) {
if(proxy) {
result = parse_proxy(data, conn, proxy, conn->http_proxy.proxytype);
Curl_safefree(proxy); 
if(result)
goto out;
}
if(socksproxy) {
result = parse_proxy(data, conn, socksproxy,
conn->socks_proxy.proxytype);
Curl_safefree(socksproxy);
if(result)
goto out;
}
if(conn->http_proxy.host.rawalloc) {
#if defined(CURL_DISABLE_HTTP)
result = CURLE_UNSUPPORTED_PROTOCOL;
goto out;
#else
if(!(conn->handler->protocol & PROTO_FAMILY_HTTP)) {
if((conn->handler->flags & PROTOPT_PROXY_AS_HTTP) &&
!conn->bits.tunnel_proxy)
conn->handler = &Curl_handler_http;
else
conn->bits.tunnel_proxy = TRUE;
}
conn->bits.httpproxy = TRUE;
#endif
}
else {
conn->bits.httpproxy = FALSE; 
conn->bits.tunnel_proxy = FALSE; 
}
if(conn->socks_proxy.host.rawalloc) {
if(!conn->http_proxy.host.rawalloc) {
if(!conn->socks_proxy.user) {
conn->socks_proxy.user = conn->http_proxy.user;
conn->http_proxy.user = NULL;
Curl_safefree(conn->socks_proxy.passwd);
conn->socks_proxy.passwd = conn->http_proxy.passwd;
conn->http_proxy.passwd = NULL;
}
}
conn->bits.socksproxy = TRUE;
}
else
conn->bits.socksproxy = FALSE; 
}
else {
conn->bits.socksproxy = FALSE;
conn->bits.httpproxy = FALSE;
}
conn->bits.proxy = conn->bits.httpproxy || conn->bits.socksproxy;
if(!conn->bits.proxy) {
conn->bits.proxy = FALSE;
conn->bits.httpproxy = FALSE;
conn->bits.socksproxy = FALSE;
conn->bits.proxy_user_passwd = FALSE;
conn->bits.tunnel_proxy = FALSE;
}
out:
free(socksproxy);
free(proxy);
return result;
}
#endif 
CURLcode Curl_parse_login_details(const char *login, const size_t len,
char **userp, char **passwdp,
char **optionsp)
{
CURLcode result = CURLE_OK;
char *ubuf = NULL;
char *pbuf = NULL;
char *obuf = NULL;
const char *psep = NULL;
const char *osep = NULL;
size_t ulen;
size_t plen;
size_t olen;
if(passwdp) {
psep = strchr(login, ':');
if(psep >= login + len)
psep = NULL;
}
if(optionsp) {
osep = strchr(login, ';');
if(osep >= login + len)
osep = NULL;
}
ulen = (psep ?
(size_t)(osep && psep > osep ? osep - login : psep - login) :
(osep ? (size_t)(osep - login) : len));
plen = (psep ?
(osep && osep > psep ? (size_t)(osep - psep) :
(size_t)(login + len - psep)) - 1 : 0);
olen = (osep ?
(psep && psep > osep ? (size_t)(psep - osep) :
(size_t)(login + len - osep)) - 1 : 0);
if(userp && ulen) {
ubuf = malloc(ulen + 1);
if(!ubuf)
result = CURLE_OUT_OF_MEMORY;
}
if(!result && passwdp && plen) {
pbuf = malloc(plen + 1);
if(!pbuf) {
free(ubuf);
result = CURLE_OUT_OF_MEMORY;
}
}
if(!result && optionsp && olen) {
obuf = malloc(olen + 1);
if(!obuf) {
free(pbuf);
free(ubuf);
result = CURLE_OUT_OF_MEMORY;
}
}
if(!result) {
if(ubuf) {
memcpy(ubuf, login, ulen);
ubuf[ulen] = '\0';
Curl_safefree(*userp);
*userp = ubuf;
}
if(pbuf) {
memcpy(pbuf, psep + 1, plen);
pbuf[plen] = '\0';
Curl_safefree(*passwdp);
*passwdp = pbuf;
}
if(obuf) {
memcpy(obuf, osep + 1, olen);
obuf[olen] = '\0';
Curl_safefree(*optionsp);
*optionsp = obuf;
}
}
return result;
}
static CURLcode parse_remote_port(struct Curl_easy *data,
struct connectdata *conn)
{
if(data->set.use_port && data->state.allow_port) {
char portbuf[16];
CURLUcode uc;
conn->remote_port = (unsigned short)data->set.use_port;
msnprintf(portbuf, sizeof(portbuf), "%d", conn->remote_port);
uc = curl_url_set(data->state.uh, CURLUPART_PORT, portbuf, 0);
if(uc)
return CURLE_OUT_OF_MEMORY;
}
return CURLE_OK;
}
static CURLcode override_login(struct Curl_easy *data,
struct connectdata *conn,
char **userp, char **passwdp, char **optionsp)
{
bool user_changed = FALSE;
bool passwd_changed = FALSE;
CURLUcode uc;
if(data->set.use_netrc == CURL_NETRC_REQUIRED && conn->bits.user_passwd) {
if(*userp) {
Curl_safefree(*userp);
user_changed = TRUE;
}
if(*passwdp) {
Curl_safefree(*passwdp);
passwd_changed = TRUE;
}
conn->bits.user_passwd = FALSE; 
}
if(data->set.str[STRING_USERNAME]) {
free(*userp);
*userp = strdup(data->set.str[STRING_USERNAME]);
if(!*userp)
return CURLE_OUT_OF_MEMORY;
conn->bits.user_passwd = TRUE; 
user_changed = TRUE;
}
if(data->set.str[STRING_PASSWORD]) {
free(*passwdp);
*passwdp = strdup(data->set.str[STRING_PASSWORD]);
if(!*passwdp)
return CURLE_OUT_OF_MEMORY;
conn->bits.user_passwd = TRUE; 
passwd_changed = TRUE;
}
if(data->set.str[STRING_OPTIONS]) {
free(*optionsp);
*optionsp = strdup(data->set.str[STRING_OPTIONS]);
if(!*optionsp)
return CURLE_OUT_OF_MEMORY;
}
conn->bits.netrc = FALSE;
if(data->set.use_netrc != CURL_NETRC_IGNORED &&
(!*userp || !**userp || !*passwdp || !**passwdp)) {
bool netrc_user_changed = FALSE;
bool netrc_passwd_changed = FALSE;
int ret;
ret = Curl_parsenetrc(conn->host.name,
userp, passwdp,
&netrc_user_changed, &netrc_passwd_changed,
data->set.str[STRING_NETRC_FILE]);
if(ret > 0) {
infof(data, "Couldn't find host %s in the .netrc file; using defaults\n",
conn->host.name);
}
else if(ret < 0) {
return CURLE_OUT_OF_MEMORY;
}
else {
conn->bits.netrc = TRUE;
conn->bits.user_passwd = TRUE; 
if(netrc_user_changed) {
user_changed = TRUE;
}
if(netrc_passwd_changed) {
passwd_changed = TRUE;
}
}
}
if(user_changed) {
uc = curl_url_set(data->state.uh, CURLUPART_USER, *userp, 0);
if(uc)
return Curl_uc_to_curlcode(uc);
}
if(passwd_changed) {
uc = curl_url_set(data->state.uh, CURLUPART_PASSWORD, *passwdp, 0);
if(uc)
return Curl_uc_to_curlcode(uc);
}
return CURLE_OK;
}
static CURLcode set_login(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
const char *setuser = CURL_DEFAULT_USER;
const char *setpasswd = CURL_DEFAULT_PASSWORD;
if((conn->handler->flags & PROTOPT_NEEDSPWD) && !conn->bits.user_passwd)
;
else {
setuser = "";
setpasswd = "";
}
if(!conn->user) {
conn->user = strdup(setuser);
if(!conn->user)
return CURLE_OUT_OF_MEMORY;
}
if(!conn->passwd) {
conn->passwd = strdup(setpasswd);
if(!conn->passwd)
result = CURLE_OUT_OF_MEMORY;
}
return result;
}
static CURLcode parse_connect_to_host_port(struct Curl_easy *data,
const char *host,
char **hostname_result,
int *port_result)
{
char *host_dup;
char *hostptr;
char *host_portno;
char *portptr;
int port = -1;
#if defined(CURL_DISABLE_VERBOSE_STRINGS)
(void) data;
#endif
*hostname_result = NULL;
*port_result = -1;
if(!host || !*host)
return CURLE_OK;
host_dup = strdup(host);
if(!host_dup)
return CURLE_OUT_OF_MEMORY;
hostptr = host_dup;
portptr = hostptr;
if(*hostptr == '[') {
#if defined(ENABLE_IPV6)
char *ptr = ++hostptr; 
while(*ptr && (ISXDIGIT(*ptr) || (*ptr == ':') || (*ptr == '.')))
ptr++;
if(*ptr == '%') {
if(strncmp("%25", ptr, 3))
infof(data, "Please URL encode %% as %%25, see RFC 6874.\n");
ptr++;
while(*ptr && (ISALPHA(*ptr) || ISXDIGIT(*ptr) || (*ptr == '-') ||
(*ptr == '.') || (*ptr == '_') || (*ptr == '~')))
ptr++;
}
if(*ptr == ']')
*ptr++ = '\0';
else
infof(data, "Invalid IPv6 address format\n");
portptr = ptr;
#else
failf(data, "Use of IPv6 in *_CONNECT_TO without IPv6 support built-in!");
free(host_dup);
return CURLE_NOT_BUILT_IN;
#endif
}
host_portno = strchr(portptr, ':');
if(host_portno) {
char *endp = NULL;
*host_portno = '\0'; 
host_portno++;
if(*host_portno) {
long portparse = strtol(host_portno, &endp, 10);
if((endp && *endp) || (portparse < 0) || (portparse > 65535)) {
infof(data, "No valid port number in connect to host string (%s)\n",
host_portno);
hostptr = NULL;
port = -1;
}
else
port = (int)portparse; 
}
}
if(hostptr) {
*hostname_result = strdup(hostptr);
if(!*hostname_result) {
free(host_dup);
return CURLE_OUT_OF_MEMORY;
}
}
*port_result = port;
free(host_dup);
return CURLE_OK;
}
static CURLcode parse_connect_to_string(struct Curl_easy *data,
struct connectdata *conn,
const char *conn_to_host,
char **host_result,
int *port_result)
{
CURLcode result = CURLE_OK;
const char *ptr = conn_to_host;
int host_match = FALSE;
int port_match = FALSE;
*host_result = NULL;
*port_result = -1;
if(*ptr == ':') {
host_match = TRUE;
ptr++;
}
else {
size_t hostname_to_match_len;
char *hostname_to_match = aprintf("%s%s%s",
conn->bits.ipv6_ip ? "[" : "",
conn->host.name,
conn->bits.ipv6_ip ? "]" : "");
if(!hostname_to_match)
return CURLE_OUT_OF_MEMORY;
hostname_to_match_len = strlen(hostname_to_match);
host_match = strncasecompare(ptr, hostname_to_match,
hostname_to_match_len);
free(hostname_to_match);
ptr += hostname_to_match_len;
host_match = host_match && *ptr == ':';
ptr++;
}
if(host_match) {
if(*ptr == ':') {
port_match = TRUE;
ptr++;
}
else {
char *ptr_next = strchr(ptr, ':');
if(ptr_next) {
char *endp = NULL;
long port_to_match = strtol(ptr, &endp, 10);
if((endp == ptr_next) && (port_to_match == conn->remote_port)) {
port_match = TRUE;
ptr = ptr_next + 1;
}
}
}
}
if(host_match && port_match) {
result = parse_connect_to_host_port(data, ptr, host_result, port_result);
}
return result;
}
static CURLcode parse_connect_to_slist(struct Curl_easy *data,
struct connectdata *conn,
struct curl_slist *conn_to_host)
{
CURLcode result = CURLE_OK;
char *host = NULL;
int port = -1;
while(conn_to_host && !host && port == -1) {
result = parse_connect_to_string(data, conn, conn_to_host->data,
&host, &port);
if(result)
return result;
if(host && *host) {
conn->conn_to_host.rawalloc = host;
conn->conn_to_host.name = host;
conn->bits.conn_to_host = TRUE;
infof(data, "Connecting to hostname: %s\n", host);
}
else {
conn->bits.conn_to_host = FALSE;
Curl_safefree(host);
}
if(port >= 0) {
conn->conn_to_port = port;
conn->bits.conn_to_port = TRUE;
infof(data, "Connecting to port: %d\n", port);
}
else {
conn->bits.conn_to_port = FALSE;
port = -1;
}
conn_to_host = conn_to_host->next;
}
#if defined(USE_ALTSVC)
if(data->asi && !host && (port == -1) &&
(conn->handler->protocol == CURLPROTO_HTTPS)) {
enum alpnid srcalpnid;
bool hit;
struct altsvc *as;
const int allowed_versions = ( ALPN_h1
#if defined(USE_NGHTTP2)
| ALPN_h2
#endif
#if defined(ENABLE_QUIC)
| ALPN_h3
#endif
) & data->asi->flags;
host = conn->host.rawalloc;
#if defined(USE_NGHTTP2)
srcalpnid = ALPN_h2;
hit = Curl_altsvc_lookup(data->asi,
srcalpnid, host, conn->remote_port, 
&as ,
allowed_versions);
if(!hit)
#endif
{
srcalpnid = ALPN_h1;
hit = Curl_altsvc_lookup(data->asi,
srcalpnid, host, conn->remote_port, 
&as ,
allowed_versions);
}
if(hit) {
char *hostd = strdup((char *)as->dst.host);
if(!hostd)
return CURLE_OUT_OF_MEMORY;
conn->conn_to_host.rawalloc = hostd;
conn->conn_to_host.name = hostd;
conn->bits.conn_to_host = TRUE;
conn->conn_to_port = as->dst.port;
conn->bits.conn_to_port = TRUE;
conn->bits.altused = TRUE;
infof(data, "Alt-svc connecting from [%s]%s:%d to [%s]%s:%d\n",
Curl_alpnid2str(srcalpnid), host, conn->remote_port,
Curl_alpnid2str(as->dst.alpnid), hostd, as->dst.port);
if(srcalpnid != as->dst.alpnid) {
switch(as->dst.alpnid) {
case ALPN_h1:
conn->httpversion = 11;
break;
case ALPN_h2:
conn->httpversion = 20;
break;
case ALPN_h3:
conn->transport = TRNSPRT_QUIC;
conn->httpversion = 30;
break;
default: 
break;
}
}
}
}
#endif
return result;
}
static CURLcode resolve_server(struct Curl_easy *data,
struct connectdata *conn,
bool *async)
{
CURLcode result = CURLE_OK;
timediff_t timeout_ms = Curl_timeleft(data, NULL, TRUE);
DEBUGASSERT(conn);
DEBUGASSERT(data);
if(conn->bits.reuse)
*async = FALSE;
else {
int rc;
struct Curl_dns_entry *hostaddr;
#if defined(USE_UNIX_SOCKETS)
if(conn->unix_domain_socket) {
const char *path = conn->unix_domain_socket;
hostaddr = calloc(1, sizeof(struct Curl_dns_entry));
if(!hostaddr)
result = CURLE_OUT_OF_MEMORY;
else {
bool longpath = FALSE;
hostaddr->addr = Curl_unix2addr(path, &longpath,
conn->abstract_unix_socket);
if(hostaddr->addr)
hostaddr->inuse++;
else {
if(longpath) {
failf(data, "Unix socket path too long: '%s'", path);
result = CURLE_COULDNT_RESOLVE_HOST;
}
else
result = CURLE_OUT_OF_MEMORY;
free(hostaddr);
hostaddr = NULL;
}
}
}
else
#endif
if(!conn->bits.proxy) {
struct hostname *connhost;
if(conn->bits.conn_to_host)
connhost = &conn->conn_to_host;
else
connhost = &conn->host;
if(conn->bits.conn_to_port)
conn->port = conn->conn_to_port;
else
conn->port = conn->remote_port;
conn->hostname_resolve = strdup(connhost->name);
if(!conn->hostname_resolve)
return CURLE_OUT_OF_MEMORY;
rc = Curl_resolv_timeout(conn, conn->hostname_resolve, (int)conn->port,
&hostaddr, timeout_ms);
if(rc == CURLRESOLV_PENDING)
*async = TRUE;
else if(rc == CURLRESOLV_TIMEDOUT)
result = CURLE_OPERATION_TIMEDOUT;
else if(!hostaddr) {
failf(data, "Couldn't resolve host '%s'", connhost->dispname);
result = CURLE_COULDNT_RESOLVE_HOST;
}
}
else {
struct hostname * const host = conn->bits.socksproxy ?
&conn->socks_proxy.host : &conn->http_proxy.host;
conn->hostname_resolve = strdup(host->name);
if(!conn->hostname_resolve)
return CURLE_OUT_OF_MEMORY;
rc = Curl_resolv_timeout(conn, conn->hostname_resolve, (int)conn->port,
&hostaddr, timeout_ms);
if(rc == CURLRESOLV_PENDING)
*async = TRUE;
else if(rc == CURLRESOLV_TIMEDOUT)
result = CURLE_OPERATION_TIMEDOUT;
else if(!hostaddr) {
failf(data, "Couldn't resolve proxy '%s'", host->dispname);
result = CURLE_COULDNT_RESOLVE_PROXY;
}
}
DEBUGASSERT(conn->dns_entry == NULL);
conn->dns_entry = hostaddr;
}
return result;
}
static void reuse_conn(struct connectdata *old_conn,
struct connectdata *conn)
{
Curl_free_idnconverted_hostname(&old_conn->http_proxy.host);
Curl_free_idnconverted_hostname(&old_conn->socks_proxy.host);
free(old_conn->http_proxy.host.rawalloc);
free(old_conn->socks_proxy.host.rawalloc);
Curl_free_primary_ssl_config(&old_conn->ssl_config);
Curl_free_primary_ssl_config(&old_conn->proxy_ssl_config);
conn->data = old_conn->data;
conn->bits.user_passwd = old_conn->bits.user_passwd;
if(conn->bits.user_passwd) {
Curl_safefree(conn->user);
Curl_safefree(conn->passwd);
conn->user = old_conn->user;
conn->passwd = old_conn->passwd;
old_conn->user = NULL;
old_conn->passwd = NULL;
}
conn->bits.proxy_user_passwd = old_conn->bits.proxy_user_passwd;
if(conn->bits.proxy_user_passwd) {
Curl_safefree(conn->http_proxy.user);
Curl_safefree(conn->socks_proxy.user);
Curl_safefree(conn->http_proxy.passwd);
Curl_safefree(conn->socks_proxy.passwd);
conn->http_proxy.user = old_conn->http_proxy.user;
conn->socks_proxy.user = old_conn->socks_proxy.user;
conn->http_proxy.passwd = old_conn->http_proxy.passwd;
conn->socks_proxy.passwd = old_conn->socks_proxy.passwd;
old_conn->http_proxy.user = NULL;
old_conn->socks_proxy.user = NULL;
old_conn->http_proxy.passwd = NULL;
old_conn->socks_proxy.passwd = NULL;
}
Curl_free_idnconverted_hostname(&conn->host);
Curl_free_idnconverted_hostname(&conn->conn_to_host);
Curl_safefree(conn->host.rawalloc);
Curl_safefree(conn->conn_to_host.rawalloc);
conn->host = old_conn->host;
conn->conn_to_host = old_conn->conn_to_host;
conn->conn_to_port = old_conn->conn_to_port;
conn->remote_port = old_conn->remote_port;
Curl_safefree(conn->hostname_resolve);
conn->hostname_resolve = old_conn->hostname_resolve;
old_conn->hostname_resolve = NULL;
Curl_persistconninfo(conn);
conn_reset_all_postponed_data(old_conn); 
conn->bits.reuse = TRUE; 
Curl_safefree(old_conn->user);
Curl_safefree(old_conn->passwd);
Curl_safefree(old_conn->options);
Curl_safefree(old_conn->http_proxy.user);
Curl_safefree(old_conn->socks_proxy.user);
Curl_safefree(old_conn->http_proxy.passwd);
Curl_safefree(old_conn->socks_proxy.passwd);
Curl_safefree(old_conn->localdev);
Curl_llist_destroy(&old_conn->easyq, NULL);
#if defined(USE_UNIX_SOCKETS)
Curl_safefree(old_conn->unix_domain_socket);
#endif
}
static CURLcode create_conn(struct Curl_easy *data,
struct connectdata **in_connect,
bool *async)
{
CURLcode result = CURLE_OK;
struct connectdata *conn;
struct connectdata *conn_temp = NULL;
bool reuse;
bool connections_available = TRUE;
bool force_reuse = FALSE;
bool waitpipe = FALSE;
size_t max_host_connections = Curl_multi_max_host_connections(data->multi);
size_t max_total_connections = Curl_multi_max_total_connections(data->multi);
*async = FALSE;
*in_connect = NULL;
if(!data->change.url) {
result = CURLE_URL_MALFORMAT;
goto out;
}
conn = allocate_conn(data);
if(!conn) {
result = CURLE_OUT_OF_MEMORY;
goto out;
}
*in_connect = conn;
result = parseurlandfillconn(data, conn);
if(result)
goto out;
if(data->set.str[STRING_SASL_AUTHZID]) {
conn->sasl_authzid = strdup(data->set.str[STRING_SASL_AUTHZID]);
if(!conn->sasl_authzid) {
result = CURLE_OUT_OF_MEMORY;
goto out;
}
}
#if defined(USE_UNIX_SOCKETS)
if(data->set.str[STRING_UNIX_SOCKET_PATH]) {
conn->unix_domain_socket = strdup(data->set.str[STRING_UNIX_SOCKET_PATH]);
if(conn->unix_domain_socket == NULL) {
result = CURLE_OUT_OF_MEMORY;
goto out;
}
conn->abstract_unix_socket = data->set.abstract_unix_socket;
}
#endif
#if !defined(CURL_DISABLE_PROXY)
result = create_conn_helper_init_proxy(conn);
if(result)
goto out;
#endif
if((conn->given->flags&PROTOPT_SSL) && conn->bits.httpproxy)
conn->bits.tunnel_proxy = TRUE;
result = parse_remote_port(data, conn);
if(result)
goto out;
result = override_login(data, conn, &conn->user, &conn->passwd,
&conn->options);
if(result)
goto out;
result = set_login(conn); 
if(result)
goto out;
result = parse_connect_to_slist(data, conn, data->set.connect_to);
if(result)
goto out;
result = Curl_idnconvert_hostname(conn, &conn->host);
if(result)
goto out;
if(conn->bits.conn_to_host) {
result = Curl_idnconvert_hostname(conn, &conn->conn_to_host);
if(result)
goto out;
}
if(conn->bits.httpproxy) {
result = Curl_idnconvert_hostname(conn, &conn->http_proxy.host);
if(result)
goto out;
}
if(conn->bits.socksproxy) {
result = Curl_idnconvert_hostname(conn, &conn->socks_proxy.host);
if(result)
goto out;
}
if(conn->bits.conn_to_host &&
strcasecompare(conn->conn_to_host.name, conn->host.name)) {
conn->bits.conn_to_host = FALSE;
}
if(conn->bits.conn_to_port && conn->conn_to_port == conn->remote_port) {
conn->bits.conn_to_port = FALSE;
}
if((conn->bits.conn_to_host || conn->bits.conn_to_port) &&
conn->bits.httpproxy)
conn->bits.tunnel_proxy = TRUE;
result = setup_connection_internals(conn);
if(result)
goto out;
conn->recv[FIRSTSOCKET] = Curl_recv_plain;
conn->send[FIRSTSOCKET] = Curl_send_plain;
conn->recv[SECONDARYSOCKET] = Curl_recv_plain;
conn->send[SECONDARYSOCKET] = Curl_send_plain;
conn->bits.tcp_fastopen = data->set.tcp_fastopen;
#if !defined(CURL_DISABLE_FILE)
if(conn->handler->flags & PROTOPT_NONETWORK) {
bool done;
DEBUGASSERT(conn->handler->connect_it);
Curl_persistconninfo(conn);
result = conn->handler->connect_it(conn, &done);
if(!result) {
conn->bits.tcpconnect[FIRSTSOCKET] = TRUE; 
result = Curl_conncache_add_conn(data->state.conn_cache, conn);
if(result)
goto out;
result = setup_range(data);
if(result) {
DEBUGASSERT(conn->handler->done);
(void)conn->handler->done(conn, result, FALSE);
goto out;
}
Curl_attach_connnection(data, conn);
Curl_setup_transfer(data, -1, -1, FALSE, -1);
}
Curl_init_do(data, conn);
goto out;
}
#endif
data->set.ssl.primary.CApath = data->set.str[STRING_SSL_CAPATH_ORIG];
data->set.proxy_ssl.primary.CApath = data->set.str[STRING_SSL_CAPATH_PROXY];
data->set.ssl.primary.CAfile = data->set.str[STRING_SSL_CAFILE_ORIG];
data->set.proxy_ssl.primary.CAfile = data->set.str[STRING_SSL_CAFILE_PROXY];
data->set.ssl.primary.random_file = data->set.str[STRING_SSL_RANDOM_FILE];
data->set.proxy_ssl.primary.random_file =
data->set.str[STRING_SSL_RANDOM_FILE];
data->set.ssl.primary.egdsocket = data->set.str[STRING_SSL_EGDSOCKET];
data->set.proxy_ssl.primary.egdsocket = data->set.str[STRING_SSL_EGDSOCKET];
data->set.ssl.primary.cipher_list =
data->set.str[STRING_SSL_CIPHER_LIST_ORIG];
data->set.proxy_ssl.primary.cipher_list =
data->set.str[STRING_SSL_CIPHER_LIST_PROXY];
data->set.ssl.primary.cipher_list13 =
data->set.str[STRING_SSL_CIPHER13_LIST_ORIG];
data->set.proxy_ssl.primary.cipher_list13 =
data->set.str[STRING_SSL_CIPHER13_LIST_PROXY];
data->set.ssl.primary.pinned_key =
data->set.str[STRING_SSL_PINNEDPUBLICKEY_ORIG];
data->set.proxy_ssl.primary.pinned_key =
data->set.str[STRING_SSL_PINNEDPUBLICKEY_PROXY];
data->set.ssl.CRLfile = data->set.str[STRING_SSL_CRLFILE_ORIG];
data->set.proxy_ssl.CRLfile = data->set.str[STRING_SSL_CRLFILE_PROXY];
data->set.ssl.issuercert = data->set.str[STRING_SSL_ISSUERCERT_ORIG];
data->set.proxy_ssl.issuercert = data->set.str[STRING_SSL_ISSUERCERT_PROXY];
data->set.ssl.cert = data->set.str[STRING_CERT_ORIG];
data->set.proxy_ssl.cert = data->set.str[STRING_CERT_PROXY];
data->set.ssl.cert_type = data->set.str[STRING_CERT_TYPE_ORIG];
data->set.proxy_ssl.cert_type = data->set.str[STRING_CERT_TYPE_PROXY];
data->set.ssl.key = data->set.str[STRING_KEY_ORIG];
data->set.proxy_ssl.key = data->set.str[STRING_KEY_PROXY];
data->set.ssl.key_type = data->set.str[STRING_KEY_TYPE_ORIG];
data->set.proxy_ssl.key_type = data->set.str[STRING_KEY_TYPE_PROXY];
data->set.ssl.key_passwd = data->set.str[STRING_KEY_PASSWD_ORIG];
data->set.proxy_ssl.key_passwd = data->set.str[STRING_KEY_PASSWD_PROXY];
data->set.ssl.primary.clientcert = data->set.str[STRING_CERT_ORIG];
data->set.proxy_ssl.primary.clientcert = data->set.str[STRING_CERT_PROXY];
#if defined(USE_TLS_SRP)
data->set.ssl.username = data->set.str[STRING_TLSAUTH_USERNAME_ORIG];
data->set.proxy_ssl.username = data->set.str[STRING_TLSAUTH_USERNAME_PROXY];
data->set.ssl.password = data->set.str[STRING_TLSAUTH_PASSWORD_ORIG];
data->set.proxy_ssl.password = data->set.str[STRING_TLSAUTH_PASSWORD_PROXY];
#endif
if(!Curl_clone_primary_ssl_config(&data->set.ssl.primary,
&conn->ssl_config)) {
result = CURLE_OUT_OF_MEMORY;
goto out;
}
if(!Curl_clone_primary_ssl_config(&data->set.proxy_ssl.primary,
&conn->proxy_ssl_config)) {
result = CURLE_OUT_OF_MEMORY;
goto out;
}
prune_dead_connections(data);
DEBUGASSERT(conn->user);
DEBUGASSERT(conn->passwd);
if((data->set.reuse_fresh && !data->state.this_is_a_follow) ||
data->set.connect_only)
reuse = FALSE;
else
reuse = ConnectionExists(data, conn, &conn_temp, &force_reuse, &waitpipe);
if(reuse) {
reuse_conn(conn, conn_temp);
#if defined(USE_SSL)
free(conn->ssl_extra);
#endif
free(conn); 
conn = conn_temp;
*in_connect = conn;
infof(data, "Re-using existing connection! (#%ld) with %s %s\n",
conn->connection_id,
conn->bits.proxy?"proxy":"host",
conn->socks_proxy.host.name ? conn->socks_proxy.host.dispname :
conn->http_proxy.host.name ? conn->http_proxy.host.dispname :
conn->host.dispname);
}
else {
if(conn->handler->flags & PROTOPT_ALPN_NPN) {
if(data->set.ssl_enable_alpn)
conn->bits.tls_enable_alpn = TRUE;
if(data->set.ssl_enable_npn)
conn->bits.tls_enable_npn = TRUE;
}
if(waitpipe)
connections_available = FALSE;
else {
const char *bundlehost;
struct connectbundle *bundle =
Curl_conncache_find_bundle(conn, data->state.conn_cache, &bundlehost);
if(max_host_connections > 0 && bundle &&
(bundle->num_connections >= max_host_connections)) {
struct connectdata *conn_candidate;
conn_candidate = Curl_conncache_extract_bundle(data, bundle);
Curl_conncache_unlock(data);
if(conn_candidate)
(void)Curl_disconnect(data, conn_candidate,
FALSE);
else {
infof(data, "No more connections allowed to host %s: %zu\n",
bundlehost, max_host_connections);
connections_available = FALSE;
}
}
else
Curl_conncache_unlock(data);
}
if(connections_available &&
(max_total_connections > 0) &&
(Curl_conncache_size(data) >= max_total_connections)) {
struct connectdata *conn_candidate;
conn_candidate = Curl_conncache_extract_oldest(data);
if(conn_candidate)
(void)Curl_disconnect(data, conn_candidate,
FALSE);
else {
infof(data, "No connections available in cache\n");
connections_available = FALSE;
}
}
if(!connections_available) {
infof(data, "No connections available.\n");
conn_free(conn);
*in_connect = NULL;
result = CURLE_NO_CONNECTION_AVAILABLE;
goto out;
}
else {
result = Curl_conncache_add_conn(data->state.conn_cache, conn);
if(result)
goto out;
}
#if defined(USE_NTLM)
if((data->state.authhost.picked & (CURLAUTH_NTLM | CURLAUTH_NTLM_WB)) &&
data->state.authhost.done) {
infof(data, "NTLM picked AND auth done set, clear picked!\n");
data->state.authhost.picked = CURLAUTH_NONE;
data->state.authhost.done = FALSE;
}
if((data->state.authproxy.picked & (CURLAUTH_NTLM | CURLAUTH_NTLM_WB)) &&
data->state.authproxy.done) {
infof(data, "NTLM-proxy picked AND auth done set, clear picked!\n");
data->state.authproxy.picked = CURLAUTH_NONE;
data->state.authproxy.done = FALSE;
}
#endif
}
Curl_init_do(data, conn);
result = setup_range(data);
if(result)
goto out;
conn->seek_func = data->set.seek_func;
conn->seek_client = data->set.seek_client;
result = resolve_server(data, conn, async);
strip_trailing_dot(&conn->host);
if(conn->bits.httpproxy)
strip_trailing_dot(&conn->http_proxy.host);
if(conn->bits.socksproxy)
strip_trailing_dot(&conn->socks_proxy.host);
if(conn->bits.conn_to_host)
strip_trailing_dot(&conn->conn_to_host);
out:
return result;
}
CURLcode Curl_setup_conn(struct connectdata *conn,
bool *protocol_done)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
Curl_pgrsTime(data, TIMER_NAMELOOKUP);
if(conn->handler->flags & PROTOPT_NONETWORK) {
*protocol_done = TRUE;
return result;
}
*protocol_done = FALSE; 
conn->bits.proxy_connect_closed = FALSE;
if(data->set.str[STRING_USERAGENT]) {
Curl_safefree(conn->allocptr.uagent);
conn->allocptr.uagent =
aprintf("User-Agent: %s\r\n", data->set.str[STRING_USERAGENT]);
if(!conn->allocptr.uagent)
return CURLE_OUT_OF_MEMORY;
}
data->req.headerbytecount = 0;
#if defined(CURL_DO_LINEEND_CONV)
data->state.crlf_conversions = 0; 
#endif 
conn->now = Curl_now();
if(CURL_SOCKET_BAD == conn->sock[FIRSTSOCKET]) {
conn->bits.tcpconnect[FIRSTSOCKET] = FALSE;
result = Curl_connecthost(conn, conn->dns_entry);
if(result)
return result;
}
else {
Curl_pgrsTime(data, TIMER_CONNECT); 
if(conn->ssl[FIRSTSOCKET].use ||
(conn->handler->protocol & PROTO_FAMILY_SSH))
Curl_pgrsTime(data, TIMER_APPCONNECT); 
conn->bits.tcpconnect[FIRSTSOCKET] = TRUE;
*protocol_done = TRUE;
Curl_updateconninfo(conn, conn->sock[FIRSTSOCKET]);
Curl_verboseconnect(conn);
}
conn->now = Curl_now(); 
return result;
}
CURLcode Curl_connect(struct Curl_easy *data,
bool *asyncp,
bool *protocol_done)
{
CURLcode result;
struct connectdata *conn;
*asyncp = FALSE; 
Curl_free_request_state(data);
memset(&data->req, 0, sizeof(struct SingleRequest));
data->req.maxdownload = -1;
result = create_conn(data, &conn, asyncp);
if(!result) {
if(CONN_INUSE(conn))
*protocol_done = TRUE;
else if(!*asyncp) {
result = Curl_setup_conn(conn, protocol_done);
}
}
if(result == CURLE_NO_CONNECTION_AVAILABLE) {
return result;
}
else if(result && conn) {
Curl_disconnect(data, conn, TRUE);
}
else if(!result && !data->conn)
Curl_attach_connnection(data, conn);
return result;
}
CURLcode Curl_init_do(struct Curl_easy *data, struct connectdata *conn)
{
struct SingleRequest *k = &data->req;
if(conn) {
conn->bits.do_more = FALSE; 
if(data->state.wildcardmatch &&
!(conn->handler->flags & PROTOPT_WILDCARD))
data->state.wildcardmatch = FALSE;
}
data->state.done = FALSE; 
data->state.expect100header = FALSE;
if(data->set.opt_no_body)
data->set.httpreq = HTTPREQ_HEAD;
else if(HTTPREQ_HEAD == data->set.httpreq)
data->set.httpreq = HTTPREQ_GET;
k->start = Curl_now(); 
k->now = k->start; 
k->header = TRUE; 
k->bytecount = 0;
k->buf = data->state.buffer;
k->hbufp = data->state.headerbuff;
k->ignorebody = FALSE;
Curl_speedinit(data);
Curl_pgrsSetUploadCounter(data, 0);
Curl_pgrsSetDownloadCounter(data, 0);
return CURLE_OK;
}
static unsigned int get_protocol_family(unsigned int protocol)
{
unsigned int family;
switch(protocol) {
case CURLPROTO_HTTP:
case CURLPROTO_HTTPS:
family = CURLPROTO_HTTP;
break;
case CURLPROTO_FTP:
case CURLPROTO_FTPS:
family = CURLPROTO_FTP;
break;
case CURLPROTO_SCP:
family = CURLPROTO_SCP;
break;
case CURLPROTO_SFTP:
family = CURLPROTO_SFTP;
break;
case CURLPROTO_TELNET:
family = CURLPROTO_TELNET;
break;
case CURLPROTO_LDAP:
case CURLPROTO_LDAPS:
family = CURLPROTO_LDAP;
break;
case CURLPROTO_DICT:
family = CURLPROTO_DICT;
break;
case CURLPROTO_FILE:
family = CURLPROTO_FILE;
break;
case CURLPROTO_TFTP:
family = CURLPROTO_TFTP;
break;
case CURLPROTO_IMAP:
case CURLPROTO_IMAPS:
family = CURLPROTO_IMAP;
break;
case CURLPROTO_POP3:
case CURLPROTO_POP3S:
family = CURLPROTO_POP3;
break;
case CURLPROTO_SMTP:
case CURLPROTO_SMTPS:
family = CURLPROTO_SMTP;
break;
case CURLPROTO_RTSP:
family = CURLPROTO_RTSP;
break;
case CURLPROTO_RTMP:
case CURLPROTO_RTMPS:
family = CURLPROTO_RTMP;
break;
case CURLPROTO_RTMPT:
case CURLPROTO_RTMPTS:
family = CURLPROTO_RTMPT;
break;
case CURLPROTO_RTMPE:
family = CURLPROTO_RTMPE;
break;
case CURLPROTO_RTMPTE:
family = CURLPROTO_RTMPTE;
break;
case CURLPROTO_GOPHER:
family = CURLPROTO_GOPHER;
break;
case CURLPROTO_SMB:
case CURLPROTO_SMBS:
family = CURLPROTO_SMB;
break;
default:
family = 0;
break;
}
return family;
}
