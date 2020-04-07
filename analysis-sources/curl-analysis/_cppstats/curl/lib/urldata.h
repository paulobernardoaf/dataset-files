#include "curl_setup.h"
#define PORT_FTP 21
#define PORT_FTPS 990
#define PORT_TELNET 23
#define PORT_HTTP 80
#define PORT_HTTPS 443
#define PORT_DICT 2628
#define PORT_LDAP 389
#define PORT_LDAPS 636
#define PORT_TFTP 69
#define PORT_SSH 22
#define PORT_IMAP 143
#define PORT_IMAPS 993
#define PORT_POP3 110
#define PORT_POP3S 995
#define PORT_SMB 445
#define PORT_SMBS 445
#define PORT_SMTP 25
#define PORT_SMTPS 465 
#define PORT_RTSP 554
#define PORT_RTMP 1935
#define PORT_RTMPT PORT_HTTP
#define PORT_RTMPS PORT_HTTPS
#define PORT_GOPHER 70
#define DICT_MATCH "/MATCH:"
#define DICT_MATCH2 "/M:"
#define DICT_MATCH3 "/FIND:"
#define DICT_DEFINE "/DEFINE:"
#define DICT_DEFINE2 "/D:"
#define DICT_DEFINE3 "/LOOKUP:"
#define CURL_DEFAULT_USER "anonymous"
#define CURL_DEFAULT_PASSWORD "ftp@example.com"
#define PROTO_FAMILY_HTTP (CURLPROTO_HTTP|CURLPROTO_HTTPS)
#define PROTO_FAMILY_FTP (CURLPROTO_FTP|CURLPROTO_FTPS)
#define PROTO_FAMILY_POP3 (CURLPROTO_POP3|CURLPROTO_POP3S)
#define PROTO_FAMILY_SMB (CURLPROTO_SMB|CURLPROTO_SMBS)
#define PROTO_FAMILY_SMTP (CURLPROTO_SMTP|CURLPROTO_SMTPS)
#define PROTO_FAMILY_SSH (CURLPROTO_SCP|CURLPROTO_SFTP)
#define DEFAULT_CONNCACHE_SIZE 5
#define MAX_IPADR_LEN sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")
#define RESP_TIMEOUT (120*1000)
#define CURL_MAX_INPUT_LENGTH 8000000
#include "cookie.h"
#include "psl.h"
#include "formdata.h"
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(HAVE_NETINET_IN6_H)
#include <netinet/in6.h>
#endif
#include "timeval.h"
#include <curl/curl.h>
#include "http_chunks.h" 
#include "hostip.h"
#include "hash.h"
#include "splay.h"
typedef ssize_t (Curl_send)(struct connectdata *conn, 
int sockindex, 
const void *buf, 
size_t len, 
CURLcode *err); 
typedef ssize_t (Curl_recv)(struct connectdata *conn, 
int sockindex, 
char *buf, 
size_t len, 
CURLcode *err); 
#include "mime.h"
#include "imap.h"
#include "pop3.h"
#include "smtp.h"
#include "ftp.h"
#include "file.h"
#include "vssh/ssh.h"
#include "http.h"
#include "rtsp.h"
#include "smb.h"
#include "wildcard.h"
#include "multihandle.h"
#include "quic.h"
#if defined(HAVE_GSSAPI)
#if defined(HAVE_GSSGNU)
#include <gss.h>
#elif defined HAVE_GSSAPI_GSSAPI_H
#include <gssapi/gssapi.h>
#else
#include <gssapi.h>
#endif
#if defined(HAVE_GSSAPI_GSSAPI_GENERIC_H)
#include <gssapi/gssapi_generic.h>
#endif
#endif
#if defined(HAVE_LIBSSH2_H)
#include <libssh2.h>
#include <libssh2_sftp.h>
#endif 
#define HEADERSIZE 256
#define CURLEASY_MAGIC_NUMBER 0xc0dedbadU
#define GOOD_EASY_HANDLE(x) ((x) && ((x)->magic == CURLEASY_MAGIC_NUMBER))
#if defined(_MSC_VER)
typedef bool bit;
#define BIT(x) bool x
#else
typedef unsigned int bit;
#define BIT(x) bit x:1
#endif
#if defined(HAVE_GSSAPI)
struct krb5buffer {
void *data;
size_t size;
size_t index;
BIT(eof_flag);
};
enum protection_level {
PROT_NONE, 
PROT_CLEAR,
PROT_SAFE,
PROT_CONFIDENTIAL,
PROT_PRIVATE,
PROT_CMD,
PROT_LAST 
};
#endif
typedef enum {
ssl_connect_1,
ssl_connect_2,
ssl_connect_2_reading,
ssl_connect_2_writing,
ssl_connect_3,
ssl_connect_done
} ssl_connect_state;
typedef enum {
ssl_connection_none,
ssl_connection_negotiating,
ssl_connection_complete
} ssl_connection_state;
struct ssl_backend_data;
struct ssl_connect_data {
ssl_connection_state state;
ssl_connect_state connecting_state;
#if defined(USE_SSL)
struct ssl_backend_data *backend;
#endif
BIT(use);
};
struct ssl_primary_config {
long version; 
long version_max; 
char *CApath; 
char *CAfile; 
char *clientcert;
char *random_file; 
char *egdsocket; 
char *cipher_list; 
char *cipher_list13; 
char *pinned_key;
BIT(verifypeer); 
BIT(verifyhost); 
BIT(verifystatus); 
BIT(sessionid); 
};
struct ssl_config_data {
struct ssl_primary_config primary;
long certverifyresult; 
char *CRLfile; 
char *issuercert;
curl_ssl_ctx_callback fsslctx; 
void *fsslctxp; 
char *cert; 
char *cert_type; 
char *key; 
char *key_type; 
char *key_passwd; 
#if defined(USE_TLS_SRP)
char *username; 
char *password; 
enum CURL_TLSAUTH authtype; 
#endif
BIT(certinfo); 
BIT(falsestart);
BIT(enable_beast); 
BIT(no_revoke); 
BIT(no_partialchain); 
BIT(revoke_best_effort); 
};
struct ssl_general_config {
size_t max_ssl_sessions; 
};
struct curl_ssl_session {
char *name; 
char *conn_to_host; 
const char *scheme; 
void *sessionid; 
size_t idsize; 
long age; 
int remote_port; 
int conn_to_port; 
struct ssl_primary_config ssl_config; 
};
#if defined(USE_WINDOWS_SSPI)
#include "curl_sspi.h"
#endif
struct digestdata {
#if defined(USE_WINDOWS_SSPI)
BYTE *input_token;
size_t input_token_len;
CtxtHandle *http_context;
char *user;
char *passwd;
#else
char *nonce;
char *cnonce;
char *realm;
int algo;
char *opaque;
char *qop;
char *algorithm;
int nc; 
BIT(stale); 
BIT(userhash);
#endif
};
typedef enum {
NTLMSTATE_NONE,
NTLMSTATE_TYPE1,
NTLMSTATE_TYPE2,
NTLMSTATE_TYPE3,
NTLMSTATE_LAST
} curlntlm;
typedef enum {
GSS_AUTHNONE,
GSS_AUTHRECV,
GSS_AUTHSENT,
GSS_AUTHDONE,
GSS_AUTHSUCC
} curlnegotiate;
#if defined(CURL_DOES_CONVERSIONS) && defined(HAVE_ICONV)
#include <iconv.h>
#endif
#if defined(USE_KERBEROS5)
struct kerberos5data {
#if defined(USE_WINDOWS_SSPI)
CredHandle *credentials;
CtxtHandle *context;
TCHAR *spn;
SEC_WINNT_AUTH_IDENTITY identity;
SEC_WINNT_AUTH_IDENTITY *p_identity;
size_t token_max;
BYTE *output_token;
#else
gss_ctx_id_t context;
gss_name_t spn;
#endif
};
#endif
#if defined(USE_NTLM)
struct ntlmdata {
#if defined(USE_WINDOWS_SSPI)
#if defined(SECPKG_ATTR_ENDPOINT_BINDINGS)
CtxtHandle *sslContext;
#endif
CredHandle *credentials;
CtxtHandle *context;
SEC_WINNT_AUTH_IDENTITY identity;
SEC_WINNT_AUTH_IDENTITY *p_identity;
size_t token_max;
BYTE *output_token;
BYTE *input_token;
size_t input_token_len;
TCHAR *spn;
#else
unsigned int flags;
unsigned char nonce[8];
void *target_info; 
unsigned int target_info_len;
#if defined(NTLM_WB_ENABLED)
curl_socket_t ntlm_auth_hlpr_socket;
pid_t ntlm_auth_hlpr_pid;
char *challenge; 
char *response; 
#endif
#endif
};
#endif
#if defined(USE_SPNEGO)
struct negotiatedata {
#if defined(HAVE_GSSAPI)
OM_uint32 status;
gss_ctx_id_t context;
gss_name_t spn;
gss_buffer_desc output_token;
#else
#if defined(USE_WINDOWS_SSPI)
#if defined(SECPKG_ATTR_ENDPOINT_BINDINGS)
CtxtHandle *sslContext;
#endif
DWORD status;
CredHandle *credentials;
CtxtHandle *context;
SEC_WINNT_AUTH_IDENTITY identity;
SEC_WINNT_AUTH_IDENTITY *p_identity;
TCHAR *spn;
size_t token_max;
BYTE *output_token;
size_t output_token_length;
#endif
#endif
BIT(noauthpersist);
BIT(havenoauthpersist);
BIT(havenegdata);
BIT(havemultiplerequests);
};
#endif
struct ConnectBits {
bool proxy_ssl_connected[2]; 
bool tcpconnect[2]; 
BIT(close); 
BIT(reuse); 
BIT(altused); 
BIT(conn_to_host); 
BIT(conn_to_port); 
BIT(proxy); 
BIT(httpproxy); 
BIT(socksproxy); 
BIT(user_passwd); 
BIT(proxy_user_passwd); 
BIT(ipv6_ip); 
BIT(ipv6); 
BIT(do_more); 
BIT(protoconnstart);
BIT(retry); 
BIT(tunnel_proxy); 
BIT(authneg); 
BIT(rewindaftersend);
#if !defined(CURL_DISABLE_FTP)
BIT(ftp_use_epsv); 
BIT(ftp_use_eprt); 
BIT(ftp_use_data_ssl); 
#endif
BIT(netrc); 
BIT(userpwd_in_url); 
BIT(proxy_connect_closed); 
BIT(bound); 
BIT(type_set); 
BIT(multiplex); 
BIT(tcp_fastopen); 
BIT(tls_enable_npn); 
BIT(tls_enable_alpn); 
BIT(connect_only);
};
struct hostname {
char *rawalloc; 
char *encalloc; 
char *name; 
const char *dispname; 
};
#define KEEP_NONE 0
#define KEEP_RECV (1<<0) 
#define KEEP_SEND (1<<1) 
#define KEEP_RECV_HOLD (1<<2) 
#define KEEP_SEND_HOLD (1<<3) 
#define KEEP_RECV_PAUSE (1<<4) 
#define KEEP_SEND_PAUSE (1<<5) 
#define KEEP_RECVBITS (KEEP_RECV | KEEP_RECV_HOLD | KEEP_RECV_PAUSE)
#define KEEP_SENDBITS (KEEP_SEND | KEEP_SEND_HOLD | KEEP_SEND_PAUSE)
struct Curl_async {
char *hostname;
int port;
struct Curl_dns_entry *dns;
int status; 
void *os_specific; 
BIT(done); 
};
#define FIRSTSOCKET 0
#define SECONDARYSOCKET 1
typedef CURLcode (*Curl_do_more_func)(struct connectdata *, int *);
typedef CURLcode (*Curl_done_func)(struct connectdata *, CURLcode, bool);
enum expect100 {
EXP100_SEND_DATA, 
EXP100_AWAITING_CONTINUE, 
EXP100_SENDING_REQUEST, 
EXP100_FAILED 
};
enum upgrade101 {
UPGR101_INIT, 
UPGR101_REQUESTED, 
UPGR101_RECEIVED, 
UPGR101_WORKING 
};
enum doh_slots {
DOH_PROBE_SLOT_IPADDR_V4 = 0, 
DOH_PROBE_SLOT_IPADDR_V6 = 1, 
DOH_PROBE_SLOTS
};
struct dohresponse {
unsigned char *memory;
size_t size;
};
struct dnsprobe {
CURL *easy;
int dnstype;
unsigned char dohbuffer[512];
size_t dohlen;
struct dohresponse serverdoh;
};
struct dohdata {
struct curl_slist *headers;
struct dnsprobe probe[DOH_PROBE_SLOTS];
unsigned int pending; 
const char *host;
int port;
};
struct SingleRequest {
curl_off_t size; 
curl_off_t maxdownload; 
curl_off_t bytecount; 
curl_off_t writebytecount; 
curl_off_t headerbytecount; 
curl_off_t deductheadercount; 
struct curltime start; 
struct curltime now; 
enum {
HEADER_NORMAL, 
HEADER_PARTHEADER, 
HEADER_ALLBAD 
} badheader; 
int headerline; 
char *hbufp; 
size_t hbuflen;
char *str; 
char *str_start; 
char *end_ptr; 
char *p; 
curl_off_t offset; 
int httpcode; 
struct curltime start100; 
enum expect100 exp100; 
enum upgrade101 upgr101; 
struct contenc_writer_s *writer_stack; 
time_t timeofdoc;
long bodywrites;
char *buf;
int keepon;
char *location; 
char *newurl; 
ssize_t upload_present;
char *upload_fromhere;
void *protop; 
#if !defined(CURL_DISABLE_DOH)
struct dohdata doh; 
#endif
BIT(header); 
BIT(content_range); 
BIT(upload_done); 
BIT(ignorebody); 
BIT(http_bodyless); 
BIT(chunk); 
BIT(upload_chunky); 
BIT(getheader); 
BIT(forbidchunk); 
};
struct Curl_handler {
const char *scheme; 
CURLcode (*setup_connection)(struct connectdata *);
CURLcode (*do_it)(struct connectdata *, bool *done);
Curl_done_func done;
Curl_do_more_func do_more;
CURLcode (*connect_it)(struct connectdata *, bool *done);
CURLcode (*connecting)(struct connectdata *, bool *done);
CURLcode (*doing)(struct connectdata *, bool *done);
int (*proto_getsock)(struct connectdata *conn,
curl_socket_t *socks);
int (*doing_getsock)(struct connectdata *conn,
curl_socket_t *socks);
int (*domore_getsock)(struct connectdata *conn,
curl_socket_t *socks);
int (*perform_getsock)(const struct connectdata *conn,
curl_socket_t *socks);
CURLcode (*disconnect)(struct connectdata *, bool dead_connection);
CURLcode (*readwrite)(struct Curl_easy *data, struct connectdata *conn,
ssize_t *nread, bool *readmore);
unsigned int (*connection_check)(struct connectdata *conn,
unsigned int checks_to_perform);
long defport; 
unsigned int protocol; 
unsigned int flags; 
};
#define PROTOPT_NONE 0 
#define PROTOPT_SSL (1<<0) 
#define PROTOPT_DUAL (1<<1) 
#define PROTOPT_CLOSEACTION (1<<2) 
#define PROTOPT_DIRLOCK (1<<3)
#define PROTOPT_NONETWORK (1<<4) 
#define PROTOPT_NEEDSPWD (1<<5) 
#define PROTOPT_NOURLQUERY (1<<6) 
#define PROTOPT_CREDSPERREQUEST (1<<7) 
#define PROTOPT_ALPN_NPN (1<<8) 
#define PROTOPT_STREAM (1<<9) 
#define PROTOPT_URLOPTIONS (1<<10) 
#define PROTOPT_PROXY_AS_HTTP (1<<11) 
#define PROTOPT_WILDCARD (1<<12) 
#define CONNCHECK_NONE 0 
#define CONNCHECK_ISDEAD (1<<0) 
#define CONNCHECK_KEEPALIVE (1<<1) 
#define CONNRESULT_NONE 0 
#define CONNRESULT_DEAD (1<<0) 
#if defined(USE_RECV_BEFORE_SEND_WORKAROUND)
struct postponed_data {
char *buffer; 
size_t allocated_size; 
size_t recv_size; 
size_t recv_processed; 
#if defined(DEBUGBUILD)
curl_socket_t bindsock;
#endif 
};
#endif 
struct proxy_info {
struct hostname host;
long port;
curl_proxytype proxytype; 
char *user; 
char *passwd; 
};
#define CONNECT_BUFFER_SIZE 16384
struct http_connect_state {
char connect_buffer[CONNECT_BUFFER_SIZE];
int perline; 
int keepon;
char *line_start;
char *ptr; 
curl_off_t cl; 
enum {
TUNNEL_INIT, 
TUNNEL_CONNECT, 
TUNNEL_COMPLETE 
} tunnel_state;
BIT(chunked_encoding);
BIT(close_connection);
};
struct ldapconninfo;
enum connect_t {
CONNECT_INIT,
CONNECT_SOCKS_INIT, 
CONNECT_SOCKS_SEND, 
CONNECT_SOCKS_READ_INIT, 
CONNECT_SOCKS_READ, 
CONNECT_GSSAPI_INIT, 
CONNECT_AUTH_INIT, 
CONNECT_AUTH_SEND, 
CONNECT_AUTH_READ, 
CONNECT_REQ_INIT, 
CONNECT_RESOLVING, 
CONNECT_RESOLVED, 
CONNECT_RESOLVE_REMOTE, 
CONNECT_REQ_SEND, 
CONNECT_REQ_SENDING, 
CONNECT_REQ_READ, 
CONNECT_REQ_READ_MORE, 
CONNECT_DONE 
};
#define SOCKS_STATE(x) (((x) >= CONNECT_SOCKS_INIT) && ((x) < CONNECT_DONE))
#define SOCKS_REQUEST_BUFSIZE 600 
struct connstate {
enum connect_t state;
unsigned char socksreq[SOCKS_REQUEST_BUFSIZE];
ssize_t outstanding; 
unsigned char *outp; 
};
struct connectdata {
struct Curl_easy *data;
struct connstate cnnct;
struct curl_llist_element bundle_node; 
struct Curl_chunker chunk;
curl_closesocket_callback fclosesocket; 
void *closesocket_client;
#define CONN_INUSE(c) ((c)->easyq.size)
long connection_id; 
struct Curl_dns_entry *dns_entry;
Curl_addrinfo *ip_addr;
Curl_addrinfo *tempaddr[2]; 
char ip_addr_str[MAX_IPADR_LEN];
unsigned int scope_id; 
enum {
TRNSPRT_TCP = 3,
TRNSPRT_UDP = 4,
TRNSPRT_QUIC = 5
} transport;
#if defined(ENABLE_QUIC)
struct quicsocket hequic[2]; 
struct quicsocket *quic;
#endif
struct hostname host;
char *hostname_resolve; 
char *secondaryhostname; 
struct hostname conn_to_host; 
struct proxy_info socks_proxy;
struct proxy_info http_proxy;
long port; 
int remote_port; 
int conn_to_port; 
unsigned short secondary_port; 
char primary_ip[MAX_IPADR_LEN];
long primary_port;
char local_ip[MAX_IPADR_LEN];
long local_port;
char *user; 
char *passwd; 
char *options; 
char *sasl_authzid; 
int httpversion; 
int rtspversion; 
struct curltime now; 
struct curltime created; 
struct curltime lastused; 
curl_socket_t sock[2]; 
curl_socket_t tempsock[2]; 
int tempfamily[2]; 
Curl_recv *recv[2];
Curl_send *send[2];
#if defined(USE_RECV_BEFORE_SEND_WORKAROUND)
struct postponed_data postponed[2]; 
#endif 
struct ssl_connect_data ssl[2]; 
struct ssl_connect_data proxy_ssl[2]; 
#if defined(USE_SSL)
void *ssl_extra; 
#endif
struct ssl_primary_config ssl_config;
struct ssl_primary_config proxy_ssl_config;
struct ConnectBits bits; 
struct curltime connecttime;
int num_addr; 
timediff_t timeoutms_per_addr; 
const struct Curl_handler *handler; 
const struct Curl_handler *given; 
long ip_version; 
struct curltime keepalive;
long upkeep_interval_ms; 
curl_socket_t sockfd; 
curl_socket_t writesockfd; 
struct dynamically_allocated_data {
char *proxyuserpwd;
char *uagent;
char *accept_encoding;
char *userpwd;
char *rangeline;
char *ref;
char *host;
char *cookiehost;
char *rtsp_transport;
char *te; 
} allocptr;
#if defined(HAVE_GSSAPI)
BIT(sec_complete); 
enum protection_level command_prot;
enum protection_level data_prot;
enum protection_level request_data_prot;
size_t buffer_size;
struct krb5buffer in_buffer;
void *app_data;
const struct Curl_sec_client_mech *mech;
struct sockaddr_in local_addr;
#endif
#if defined(USE_KERBEROS5) 
struct kerberos5data krb5; 
#endif 
struct curl_llist easyq; 
curl_seek_callback seek_func; 
void *seek_client; 
#if defined(USE_WINDOWS_SSPI) && defined(SECPKG_ATTR_ENDPOINT_BINDINGS)
CtxtHandle *sslContext;
#endif
#if defined(USE_NTLM)
curlntlm http_ntlm_state;
curlntlm proxy_ntlm_state;
struct ntlmdata ntlm; 
struct ntlmdata proxyntlm; 
#endif
#if defined(USE_SPNEGO)
curlnegotiate http_negotiate_state;
curlnegotiate proxy_negotiate_state;
struct negotiatedata negotiate; 
struct negotiatedata proxyneg; 
#endif
struct Curl_async async;
char *trailer; 
int trlMax; 
int trlPos; 
union {
struct ftp_conn ftpc;
struct http_conn httpc;
struct ssh_conn sshc;
struct tftp_state_data *tftpc;
struct imap_conn imapc;
struct pop3_conn pop3c;
struct smtp_conn smtpc;
struct rtsp_conn rtspc;
struct smb_conn smbc;
void *rtmp;
struct ldapconninfo *ldapc;
} proto;
int cselect_bits; 
int waitfor; 
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
int socks5_gssapi_enctype;
#endif
char *localdev;
unsigned short localport;
int localportrange;
struct http_connect_state *connect_state; 
struct connectbundle *bundle; 
int negnpn; 
int retrycount; 
#if defined(USE_UNIX_SOCKETS)
char *unix_domain_socket;
BIT(abstract_unix_socket);
#endif
BIT(tls_upgraded);
BIT(readchannel_inuse); 
BIT(writechannel_inuse); 
BIT(sock_accepted); 
BIT(parallel_connect); 
};
struct PureInfo {
int httpcode; 
int httpproxycode; 
int httpversion; 
time_t filetime; 
curl_off_t header_size; 
curl_off_t request_size; 
unsigned long proxyauthavail; 
unsigned long httpauthavail; 
long numconnects; 
char *contenttype; 
char *wouldredirect; 
curl_off_t retry_after; 
char conn_primary_ip[MAX_IPADR_LEN];
long conn_primary_port;
char conn_local_ip[MAX_IPADR_LEN];
long conn_local_port;
const char *conn_scheme;
unsigned int conn_protocol;
struct curl_certinfo certs; 
BIT(timecond); 
};
struct Progress {
time_t lastshow; 
curl_off_t size_dl; 
curl_off_t size_ul; 
curl_off_t downloaded; 
curl_off_t uploaded; 
curl_off_t current_speed; 
int width; 
int flags; 
timediff_t timespent;
curl_off_t dlspeed;
curl_off_t ulspeed;
timediff_t t_nslookup;
timediff_t t_connect;
timediff_t t_appconnect;
timediff_t t_pretransfer;
timediff_t t_starttransfer;
timediff_t t_redirect;
struct curltime start;
struct curltime t_startsingle;
struct curltime t_startop;
struct curltime t_acceptdata;
struct curltime ul_limit_start;
curl_off_t ul_limit_size;
struct curltime dl_limit_start;
curl_off_t dl_limit_size;
#define CURR_TIME (5 + 1) 
curl_off_t speeder[ CURR_TIME ];
struct curltime speeder_time[ CURR_TIME ];
int speeder_c;
BIT(callback); 
BIT(is_t_startransfer_set);
};
typedef enum {
HTTPREQ_NONE, 
HTTPREQ_GET,
HTTPREQ_POST,
HTTPREQ_POST_FORM, 
HTTPREQ_POST_MIME, 
HTTPREQ_PUT,
HTTPREQ_HEAD,
HTTPREQ_OPTIONS,
HTTPREQ_LAST 
} Curl_HttpReq;
typedef enum {
RTSPREQ_NONE, 
RTSPREQ_OPTIONS,
RTSPREQ_DESCRIBE,
RTSPREQ_ANNOUNCE,
RTSPREQ_SETUP,
RTSPREQ_PLAY,
RTSPREQ_PAUSE,
RTSPREQ_TEARDOWN,
RTSPREQ_GET_PARAMETER,
RTSPREQ_SET_PARAMETER,
RTSPREQ_RECORD,
RTSPREQ_RECEIVE,
RTSPREQ_LAST 
} Curl_RtspReq;
#define MAX_CURL_USER_LENGTH 256
#define MAX_CURL_PASSWORD_LENGTH 256
struct auth {
unsigned long want; 
unsigned long picked;
unsigned long avail; 
BIT(done); 
BIT(multipass); 
BIT(iestyle); 
};
struct Curl_http2_dep {
struct Curl_http2_dep *next;
struct Curl_easy *data;
};
struct tempbuf {
char *buf; 
size_t len; 
int type; 
};
typedef enum {
EXPIRE_100_TIMEOUT,
EXPIRE_ASYNC_NAME,
EXPIRE_CONNECTTIMEOUT,
EXPIRE_DNS_PER_NAME,
EXPIRE_HAPPY_EYEBALLS_DNS, 
EXPIRE_HAPPY_EYEBALLS,
EXPIRE_MULTI_PENDING,
EXPIRE_RUN_NOW,
EXPIRE_SPEEDCHECK,
EXPIRE_TIMEOUT,
EXPIRE_TOOFAST,
EXPIRE_QUIC,
EXPIRE_LAST 
} expire_id;
typedef enum {
TRAILERS_NONE,
TRAILERS_INITIALIZED,
TRAILERS_SENDING,
TRAILERS_DONE
} trailers_state;
struct time_node {
struct curl_llist_element list;
struct curltime time;
expire_id eid;
};
struct urlpieces {
char *scheme;
char *hostname;
char *port;
char *user;
char *password;
char *options;
char *path;
char *query;
};
struct UrlState {
struct conncache *conn_cache;
struct curltime keeps_speed; 
struct connectdata *lastconnect; 
char *headerbuff; 
size_t headersize; 
char *buffer; 
char *ulbuf; 
curl_off_t current_speed; 
char *first_host; 
int first_remote_port; 
struct curl_ssl_session *session; 
long sessionage; 
unsigned int tempcount; 
struct tempbuf tempwrite[3]; 
char *scratch; 
int os_errno; 
#if defined(HAVE_SIGNAL)
void (*prev_signal)(int sig);
#endif
struct digestdata digest; 
struct digestdata proxydigest; 
struct auth authhost; 
struct auth authproxy; 
void *resolver; 
#if defined(USE_OPENSSL)
void *engine;
#endif 
struct curltime expiretime; 
struct Curl_tree timenode; 
struct curl_llist timeoutlist; 
struct time_node expires[EXPIRE_LAST]; 
char *most_recent_ftp_entrypath;
int httpversion; 
#if !defined(WIN32) && !defined(MSDOS) && !defined(__EMX__) && !defined(__SYMBIAN32__)
#define CURL_DO_LINEEND_CONV
BIT(prev_block_had_trailing_cr);
curl_off_t crlf_conversions;
#endif
char *range; 
curl_off_t resume_from; 
long rtsp_next_client_CSeq; 
long rtsp_next_server_CSeq; 
long rtsp_CSeq_recv; 
curl_off_t infilesize; 
size_t drain; 
curl_read_callback fread_func; 
void *in; 
struct Curl_easy *stream_depends_on;
int stream_weight;
CURLU *uh; 
struct urlpieces up;
#if !defined(CURL_DISABLE_HTTP)
size_t trailers_bytes_sent;
Curl_send_buffer *trailers_buf; 
#endif
trailers_state trailers_state; 
#if defined(CURLDEBUG)
BIT(conncache_lock);
#endif
BIT(multi_owned_by_easy);
BIT(this_is_a_follow); 
BIT(refused_stream); 
BIT(errorbuf); 
BIT(allow_port); 
BIT(authproblem); 
BIT(ftp_trying_alternative);
BIT(wildcardmatch); 
BIT(expect100header); 
BIT(disableexpect); 
BIT(use_range);
BIT(rangestringalloc); 
BIT(done); 
BIT(stream_depends_e); 
BIT(previouslypending); 
BIT(cookie_engine);
};
struct DynamicStatic {
char *url; 
char *referer; 
struct curl_slist *cookielist; 
struct curl_slist *resolve; 
BIT(url_alloc); 
BIT(referer_alloc); 
BIT(wildcard_resolve); 
};
struct Curl_multi; 
enum dupstring {
STRING_CERT_ORIG, 
STRING_CERT_PROXY, 
STRING_CERT_TYPE_ORIG, 
STRING_CERT_TYPE_PROXY, 
STRING_COOKIE, 
STRING_COOKIEJAR, 
STRING_CUSTOMREQUEST, 
STRING_DEFAULT_PROTOCOL, 
STRING_DEVICE, 
STRING_ENCODING, 
STRING_FTP_ACCOUNT, 
STRING_FTP_ALTERNATIVE_TO_USER, 
STRING_FTPPORT, 
STRING_KEY_ORIG, 
STRING_KEY_PROXY, 
STRING_KEY_PASSWD_ORIG, 
STRING_KEY_PASSWD_PROXY, 
STRING_KEY_TYPE_ORIG, 
STRING_KEY_TYPE_PROXY, 
STRING_KRB_LEVEL, 
STRING_NETRC_FILE, 
STRING_PROXY, 
STRING_PRE_PROXY, 
STRING_SET_RANGE, 
STRING_SET_REFERER, 
STRING_SET_URL, 
STRING_SSL_CAPATH_ORIG, 
STRING_SSL_CAPATH_PROXY, 
STRING_SSL_CAFILE_ORIG, 
STRING_SSL_CAFILE_PROXY, 
STRING_SSL_PINNEDPUBLICKEY_ORIG, 
STRING_SSL_PINNEDPUBLICKEY_PROXY, 
STRING_SSL_CIPHER_LIST_ORIG, 
STRING_SSL_CIPHER_LIST_PROXY, 
STRING_SSL_CIPHER13_LIST_ORIG, 
STRING_SSL_CIPHER13_LIST_PROXY, 
STRING_SSL_EGDSOCKET, 
STRING_SSL_RANDOM_FILE, 
STRING_USERAGENT, 
STRING_SSL_CRLFILE_ORIG, 
STRING_SSL_CRLFILE_PROXY, 
STRING_SSL_ISSUERCERT_ORIG, 
STRING_SSL_ISSUERCERT_PROXY, 
STRING_SSL_ENGINE, 
STRING_USERNAME, 
STRING_PASSWORD, 
STRING_OPTIONS, 
STRING_PROXYUSERNAME, 
STRING_PROXYPASSWORD, 
STRING_NOPROXY, 
STRING_RTSP_SESSION_ID, 
STRING_RTSP_STREAM_URI, 
STRING_RTSP_TRANSPORT, 
STRING_SSH_PRIVATE_KEY, 
STRING_SSH_PUBLIC_KEY, 
STRING_SSH_HOST_PUBLIC_KEY_MD5, 
STRING_SSH_KNOWNHOSTS, 
STRING_PROXY_SERVICE_NAME, 
STRING_SERVICE_NAME, 
STRING_MAIL_FROM,
STRING_MAIL_AUTH,
STRING_TLSAUTH_USERNAME_ORIG, 
STRING_TLSAUTH_USERNAME_PROXY, 
STRING_TLSAUTH_PASSWORD_ORIG, 
STRING_TLSAUTH_PASSWORD_PROXY, 
STRING_BEARER, 
STRING_UNIX_SOCKET_PATH, 
STRING_TARGET, 
STRING_DOH, 
STRING_ALTSVC, 
STRING_SASL_AUTHZID, 
STRING_TEMP_URL, 
STRING_DNS_SERVERS,
STRING_DNS_INTERFACE,
STRING_DNS_LOCAL_IP4,
STRING_DNS_LOCAL_IP6,
STRING_LASTZEROTERMINATED,
STRING_COPYPOSTFIELDS, 
STRING_LAST 
};
typedef int (*multidone_func)(struct Curl_easy *easy, CURLcode result);
struct UserDefined {
FILE *err; 
void *debugdata; 
char *errorbuffer; 
long proxyport; 
void *out; 
void *in_set; 
void *writeheader; 
void *rtp_out; 
long use_port; 
unsigned long httpauth; 
unsigned long proxyauth; 
unsigned long socks5auth;
long followlocation; 
long maxredirs; 
int keep_post; 
void *postfields; 
curl_seek_callback seek_func; 
curl_off_t postfieldsize; 
unsigned short localport; 
int localportrange; 
curl_write_callback fwrite_func; 
curl_write_callback fwrite_header; 
curl_write_callback fwrite_rtp; 
curl_read_callback fread_func_set; 
curl_progress_callback fprogress; 
curl_xferinfo_callback fxferinfo; 
curl_debug_callback fdebug; 
curl_ioctl_callback ioctl_func; 
curl_sockopt_callback fsockopt; 
void *sockopt_client; 
curl_opensocket_callback fopensocket; 
void *opensocket_client;
curl_closesocket_callback fclosesocket; 
void *closesocket_client;
void *seek_client; 
curl_conv_callback convfromnetwork;
curl_conv_callback convtonetwork;
curl_conv_callback convfromutf8;
void *progress_client; 
void *ioctl_client; 
long timeout; 
long connecttimeout; 
long accepttimeout; 
long happy_eyeballs_timeout; 
long server_response_timeout; 
long maxage_conn; 
long tftp_blksize; 
curl_off_t filesize; 
long low_speed_limit; 
long low_speed_time; 
curl_off_t max_send_speed; 
curl_off_t max_recv_speed; 
curl_off_t set_resume_from; 
struct curl_slist *headers; 
struct curl_slist *proxyheaders; 
struct curl_httppost *httppost; 
curl_mimepart mimepost; 
struct curl_slist *quote; 
struct curl_slist *postquote; 
struct curl_slist *prequote; 
struct curl_slist *source_quote; 
struct curl_slist *source_prequote; 
struct curl_slist *source_postquote; 
struct curl_slist *telnet_options; 
struct curl_slist *resolve; 
struct curl_slist *connect_to; 
curl_TimeCond timecondition; 
time_t timevalue; 
Curl_HttpReq httpreq; 
long httpversion; 
struct ssl_config_data ssl; 
struct ssl_config_data proxy_ssl; 
struct ssl_general_config general_ssl; 
curl_proxytype proxytype; 
long dns_cache_timeout; 
long buffer_size; 
size_t upload_buffer_size; 
void *private_data; 
struct curl_slist *http200aliases; 
long ipver; 
curl_off_t max_filesize; 
#if !defined(CURL_DISABLE_FTP)
curl_ftpfile ftp_filemethod; 
curl_ftpauth ftpsslauth; 
curl_ftpccc ftp_ccc; 
#endif
int ftp_create_missing_dirs; 
curl_sshkeycallback ssh_keyfunc; 
void *ssh_keyfunc_userp; 
enum CURL_NETRC_OPTION
use_netrc; 
curl_usessl use_ssl; 
long new_file_perms; 
long new_directory_perms; 
long ssh_auth_types; 
char *str[STRING_LAST]; 
unsigned int scope_id; 
long allowed_protocols;
long redir_protocols;
struct curl_slist *mail_rcpt; 
Curl_RtspReq rtspreq; 
long rtspversion; 
curl_chunk_bgn_callback chunk_bgn; 
curl_chunk_end_callback chunk_end; 
curl_fnmatch_callback fnmatch; 
void *fnmatch_data;
long gssapi_delegation; 
long tcp_keepidle; 
long tcp_keepintvl; 
size_t maxconnects; 
long expect_100_timeout; 
struct Curl_easy *stream_depends_on;
int stream_weight;
struct Curl_http2_dep *stream_dependents;
curl_resolver_start_callback resolver_start; 
void *resolver_start_client; 
long upkeep_interval_ms; 
multidone_func fmultidone;
struct Curl_easy *dohfor; 
CURLU *uh; 
void *trailer_data; 
curl_trailer_callback trailer_callback; 
BIT(is_fread_set); 
BIT(is_fwrite_set); 
BIT(free_referer); 
BIT(tftp_no_options); 
BIT(sep_headers); 
BIT(cookiesession); 
BIT(crlf); 
BIT(strip_path_slash); 
BIT(ssh_compression); 
BIT(get_filetime); 
BIT(tunnel_thru_httpproxy); 
BIT(prefer_ascii); 
BIT(ftp_append); 
BIT(ftp_list_only); 
#if !defined(CURL_DISABLE_FTP)
BIT(ftp_use_port); 
BIT(ftp_use_epsv); 
BIT(ftp_use_eprt); 
BIT(ftp_use_pret); 
BIT(ftp_skip_ip); 
#endif
BIT(hide_progress); 
BIT(http_fail_on_error); 
BIT(http_keep_sending_on_error); 
BIT(http_follow_location); 
BIT(http_transfer_encoding); 
BIT(allow_auth_to_other_hosts);
BIT(include_header); 
BIT(http_set_referer); 
BIT(http_auto_referer); 
BIT(opt_no_body); 
BIT(upload); 
BIT(verbose); 
BIT(krb); 
BIT(reuse_forbid); 
BIT(reuse_fresh); 
BIT(no_signal); 
BIT(tcp_nodelay); 
BIT(ignorecl); 
BIT(connect_only); 
BIT(http_te_skip); 
BIT(http_ce_skip); 
BIT(proxy_transfer_mode); 
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
BIT(socks5_gssapi_nec); 
#endif
BIT(sasl_ir); 
BIT(wildcard_enabled); 
BIT(tcp_keepalive); 
BIT(tcp_fastopen); 
BIT(ssl_enable_npn); 
BIT(ssl_enable_alpn);
BIT(path_as_is); 
BIT(pipewait); 
BIT(suppress_connect_headers); 
BIT(dns_shuffle_addresses); 
BIT(stream_depends_e); 
BIT(haproxyprotocol); 
BIT(abstract_unix_socket);
BIT(disallow_username_in_url); 
BIT(doh); 
BIT(doh_get); 
BIT(http09_allowed); 
BIT(mail_rcpt_allowfails); 
};
struct Names {
struct curl_hash *hostcache;
enum {
HCACHE_NONE, 
HCACHE_MULTI, 
HCACHE_SHARED 
} hostcachetype;
};
struct Curl_easy {
struct Curl_easy *next;
struct Curl_easy *prev;
struct connectdata *conn;
struct curl_llist_element connect_queue;
struct curl_llist_element conn_queue; 
CURLMstate mstate; 
CURLcode result; 
struct Curl_message msg; 
curl_socket_t sockets[MAX_SOCKSPEREASYHANDLE];
int actions[MAX_SOCKSPEREASYHANDLE]; 
int numsocks;
struct Names dns;
struct Curl_multi *multi; 
struct Curl_multi *multi_easy; 
struct Curl_share *share; 
#if defined(USE_LIBPSL)
struct PslCache *psl; 
#endif
struct SingleRequest req; 
struct UserDefined set; 
struct DynamicStatic change; 
struct CookieInfo *cookies; 
#if defined(USE_ALTSVC)
struct altsvcinfo *asi; 
#endif
struct Progress progress; 
struct UrlState state; 
#if !defined(CURL_DISABLE_FTP)
struct WildcardData wildcard; 
#endif
struct PureInfo info; 
struct curl_tlssessioninfo tsi; 
#if defined(CURL_DOES_CONVERSIONS) && defined(HAVE_ICONV)
iconv_t outbound_cd; 
iconv_t inbound_cd; 
iconv_t utf8_cd; 
#endif 
unsigned int magic; 
};
#define LIBCURL_NAME "libcurl"
