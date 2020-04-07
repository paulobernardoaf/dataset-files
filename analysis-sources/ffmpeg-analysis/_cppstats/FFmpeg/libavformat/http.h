#include "url.h"
#define HTTP_HEADERS_SIZE 4096
void ff_http_init_auth_state(URLContext *dest, const URLContext *src);
int ff_http_get_shutdown_status(URLContext *h);
int ff_http_do_new_request(URLContext *h, const char *uri);
int ff_http_do_new_request2(URLContext *h, const char *uri, AVDictionary **options);
int ff_http_averror(int status_code, int default_averror);
