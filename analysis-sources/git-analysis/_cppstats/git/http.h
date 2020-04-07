#include "cache.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include "strbuf.h"
#include "remote.h"
#include "url.h"
#undef USE_CURL_MULTI
#if LIBCURL_VERSION_NUM >= 0x071000
#define USE_CURL_MULTI
#define DEFAULT_MAX_REQUESTS 5
#endif
#if LIBCURL_VERSION_NUM < 0x070704
#define curl_global_cleanup() do { } while (0)
#endif
#if LIBCURL_VERSION_NUM < 0x070800
#define curl_global_init(a) do { } while (0)
#elif LIBCURL_VERSION_NUM >= 0x070c00
#define curl_global_init(a) curl_global_init_mem(a, xmalloc, free, xrealloc, xstrdup, xcalloc)
#endif
#if (LIBCURL_VERSION_NUM < 0x070c04) || (LIBCURL_VERSION_NUM == 0x071000)
#define NO_CURL_EASY_DUPHANDLE
#endif
#if LIBCURL_VERSION_NUM < 0x070a03
#define CURLE_HTTP_RETURNED_ERROR CURLE_HTTP_NOT_FOUND
#endif
#if LIBCURL_VERSION_NUM < 0x070c03
#define NO_CURL_IOCTL
#endif
#if !defined(CURLOPT_USE_SSL) && defined(CURLOPT_FTP_SSL)
#define CURLOPT_USE_SSL CURLOPT_FTP_SSL
#define CURLUSESSL_TRY CURLFTPSSL_TRY
#endif
struct slot_results {
CURLcode curl_result;
long http_code;
long auth_avail;
long http_connectcode;
};
struct active_request_slot {
CURL *curl;
int in_use;
CURLcode curl_result;
long http_code;
int *finished;
struct slot_results *results;
void *callback_data;
void (*callback_func)(void *data);
struct active_request_slot *next;
};
struct buffer {
struct strbuf buf;
size_t posn;
};
size_t fread_buffer(char *ptr, size_t eltsize, size_t nmemb, void *strbuf);
size_t fwrite_buffer(char *ptr, size_t eltsize, size_t nmemb, void *strbuf);
size_t fwrite_null(char *ptr, size_t eltsize, size_t nmemb, void *strbuf);
#if !defined(NO_CURL_IOCTL)
curlioerr ioctl_buffer(CURL *handle, int cmd, void *clientp);
#endif
struct active_request_slot *get_active_slot(void);
int start_active_slot(struct active_request_slot *slot);
void run_active_slot(struct active_request_slot *slot);
void finish_all_active_slots(void);
int run_one_slot(struct active_request_slot *slot,
struct slot_results *results);
#if defined(USE_CURL_MULTI)
void fill_active_slots(void);
void add_fill_function(void *data, int (*fill)(void *));
void step_active_slots(void);
#endif
void http_init(struct remote *remote, const char *url,
int proactive_auth);
void http_cleanup(void);
struct curl_slist *http_copy_default_headers(void);
extern long int git_curl_ipresolve;
extern int active_requests;
extern int http_is_verbose;
extern ssize_t http_post_buffer;
extern struct credential http_auth;
extern char curl_errorstr[CURL_ERROR_SIZE];
enum http_follow_config {
HTTP_FOLLOW_NONE,
HTTP_FOLLOW_ALWAYS,
HTTP_FOLLOW_INITIAL
};
extern enum http_follow_config http_follow_config;
static inline int missing__target(int code, int result)
{
return 
(result == CURLE_FILE_COULDNT_READ_FILE) ||
(code == 404 && result == CURLE_HTTP_RETURNED_ERROR) ||
(code == 550 && result == CURLE_FTP_COULDNT_RETR_FILE)
;
}
#define missing_target(a) missing__target((a)->http_code, (a)->curl_result)
void normalize_curl_result(CURLcode *result, long http_code, char *errorstr,
size_t errorlen);
void append_remote_object_url(struct strbuf *buf, const char *url,
const char *hex,
int only_two_digit_prefix);
char *get_remote_object_url(const char *url, const char *hex,
int only_two_digit_prefix);
struct http_get_options {
unsigned no_cache:1,
initial_request:1;
struct strbuf *content_type;
struct strbuf *charset;
struct strbuf *effective_url;
struct strbuf *base_url;
struct string_list *extra_headers;
};
#define HTTP_OK 0
#define HTTP_MISSING_TARGET 1
#define HTTP_ERROR 2
#define HTTP_START_FAILED 3
#define HTTP_REAUTH 4
#define HTTP_NOAUTH 5
int http_get_strbuf(const char *url, struct strbuf *result, struct http_get_options *options);
int http_fetch_ref(const char *base, struct ref *ref);
int http_get_info_packs(const char *base_url,
struct packed_git **packs_head);
struct http_pack_request {
char *url;
struct packed_git *target;
struct packed_git **lst;
FILE *packfile;
struct strbuf tmpfile;
struct active_request_slot *slot;
};
struct http_pack_request *new_http_pack_request(
struct packed_git *target, const char *base_url);
int finish_http_pack_request(struct http_pack_request *preq);
void release_http_pack_request(struct http_pack_request *preq);
struct http_object_request {
char *url;
struct strbuf tmpfile;
int localfile;
CURLcode curl_result;
char errorstr[CURL_ERROR_SIZE];
long http_code;
struct object_id oid;
struct object_id real_oid;
git_hash_ctx c;
git_zstream stream;
int zret;
int rename;
struct active_request_slot *slot;
};
struct http_object_request *new_http_object_request(
const char *base_url, const struct object_id *oid);
void process_http_object_request(struct http_object_request *freq);
int finish_http_object_request(struct http_object_request *freq);
void abort_http_object_request(struct http_object_request *freq);
void release_http_object_request(struct http_object_request *freq);
void setup_curl_trace(CURL *handle);
