#include "tool_setup.h"
#include "tool_sdecls.h"
#include "tool_metalink.h"
#include "tool_urlglob.h"
#include "tool_formparse.h"
typedef enum {
ERR_NONE,
ERR_BINARY_TERMINAL = 1, 
ERR_LAST
} curl_error;
struct GlobalConfig;
struct State {
struct getout *urlnode;
URLGlob *inglob;
URLGlob *urls;
char *outfiles;
char *httpgetfields;
char *uploadfile;
unsigned long infilenum; 
unsigned long up; 
unsigned long urlnum; 
unsigned long li;
};
struct OperationConfig {
bool remote_time;
char *random_file;
char *egd_file;
char *useragent;
char *cookie; 
char *cookiejar; 
char *cookiefile; 
char *altsvc; 
bool cookiesession; 
bool encoding; 
bool tr_encoding; 
unsigned long authtype; 
bool use_resume;
bool resume_from_current;
bool disable_epsv;
bool disable_eprt;
bool ftp_pret;
long proto;
bool proto_present;
long proto_redir;
bool proto_redir_present;
char *proto_default;
curl_off_t resume_from;
char *postfields;
curl_off_t postfieldsize;
char *referer;
double timeout;
double connecttimeout;
long maxredirs;
curl_off_t max_filesize;
char *headerfile;
char *ftpport;
char *iface;
long localport;
long localportrange;
unsigned short porttouse;
char *range;
long low_speed_limit;
long low_speed_time;
char *dns_servers; 
char *dns_interface; 
char *dns_ipv4_addr; 
char *dns_ipv6_addr; 
char *userpwd;
char *login_options;
char *tls_username;
char *tls_password;
char *tls_authtype;
char *proxy_tls_username;
char *proxy_tls_password;
char *proxy_tls_authtype;
char *proxyuserpwd;
char *proxy;
int proxyver; 
char *noproxy;
char *mail_from;
struct curl_slist *mail_rcpt;
char *mail_auth;
bool mail_rcpt_allowfails; 
char *sasl_authzid; 
bool sasl_ir; 
bool proxytunnel;
bool ftp_append; 
bool use_ascii; 
bool autoreferer; 
bool failonerror; 
bool show_headers; 
bool no_body; 
bool dirlistonly; 
bool followlocation; 
bool unrestricted_auth; 
bool netrc_opt;
bool netrc;
char *netrc_file;
struct getout *url_list; 
struct getout *url_last; 
struct getout *url_get; 
struct getout *url_out; 
struct getout *url_ul; 
char *doh_url;
char *cipher_list;
char *proxy_cipher_list;
char *cipher13_list;
char *proxy_cipher13_list;
char *cert;
char *proxy_cert;
char *cert_type;
char *proxy_cert_type;
char *cacert;
char *proxy_cacert;
char *capath;
char *proxy_capath;
char *crlfile;
char *proxy_crlfile;
char *pinnedpubkey;
char *proxy_pinnedpubkey;
char *key;
char *proxy_key;
char *key_type;
char *proxy_key_type;
char *key_passwd;
char *proxy_key_passwd;
char *pubkey;
char *hostpubmd5;
char *engine;
char *etag_save_file;
char *etag_compare_file;
bool crlf;
char *customrequest;
char *krblevel;
char *request_target;
long httpversion;
bool http09_allowed;
bool nobuffer;
bool readbusy; 
bool globoff;
bool use_httpget;
bool insecure_ok; 
bool proxy_insecure_ok; 
bool terminal_binary_ok;
bool verifystatus;
bool create_dirs;
bool ftp_create_dirs;
bool ftp_skip_ip;
bool proxynegotiate;
bool proxyntlm;
bool proxydigest;
bool proxybasic;
bool proxyanyauth;
char *writeout; 
struct curl_slist *quote;
struct curl_slist *postquote;
struct curl_slist *prequote;
long ssl_version;
long ssl_version_max;
long proxy_ssl_version;
long ip_version;
curl_TimeCond timecond;
curl_off_t condtime;
struct curl_slist *headers;
struct curl_slist *proxyheaders;
tool_mime *mimeroot;
tool_mime *mimecurrent;
curl_mime *mimepost;
struct curl_slist *telnet_options;
struct curl_slist *resolve;
struct curl_slist *connect_to;
HttpReq httpreq;
curl_off_t sendpersecond; 
curl_off_t recvpersecond; 
bool ftp_ssl;
bool ftp_ssl_reqd;
bool ftp_ssl_control;
bool ftp_ssl_ccc;
int ftp_ssl_ccc_mode;
char *preproxy;
int socks5_gssapi_nec; 
unsigned long socks5_auth;
char *proxy_service_name; 
char *service_name; 
bool tcp_nodelay;
bool tcp_fastopen;
long req_retry; 
bool retry_connrefused; 
long retry_delay; 
long retry_maxtime; 
char *ftp_account; 
char *ftp_alternative_to_user; 
int ftp_filemethod;
long tftp_blksize; 
bool tftp_no_options; 
bool ignorecl; 
bool disable_sessionid;
bool raw;
bool post301;
bool post302;
bool post303;
bool nokeepalive; 
long alivetime;
bool content_disposition; 
int default_node_flags; 
bool xattr; 
long gssapi_delegation;
bool ssl_allow_beast; 
bool proxy_ssl_allow_beast; 
bool ssl_no_revoke; 
bool ssl_revoke_best_effort; 
bool use_metalink; 
metalinkfile *metalinkfile_list; 
metalinkfile *metalinkfile_last; 
char *oauth_bearer; 
bool nonpn; 
bool noalpn; 
char *unix_socket_path; 
bool abstract_unix_socket; 
bool falsestart;
bool path_as_is;
double expect100timeout;
bool suppress_connect_headers; 
curl_error synthetic_error; 
bool ssh_compression; 
long happy_eyeballs_timeout_ms; 
bool haproxy_protocol; 
bool disallow_username_in_url; 
struct GlobalConfig *global;
struct OperationConfig *prev;
struct OperationConfig *next; 
struct State state; 
};
struct GlobalConfig {
int showerror; 
bool mute; 
bool noprogress; 
bool isatty; 
FILE *errors; 
bool errors_fopened; 
char *trace_dump; 
FILE *trace_stream;
bool trace_fopened;
trace tracetype;
bool tracetime; 
int progressmode; 
char *libcurl; 
bool fail_early; 
bool styled_output; 
#if defined(CURLDEBUG)
bool test_event_based;
#endif
bool parallel;
long parallel_max;
bool parallel_connect;
struct OperationConfig *first;
struct OperationConfig *current;
struct OperationConfig *last; 
};
void config_init(struct OperationConfig *config);
void config_free(struct OperationConfig *config);
