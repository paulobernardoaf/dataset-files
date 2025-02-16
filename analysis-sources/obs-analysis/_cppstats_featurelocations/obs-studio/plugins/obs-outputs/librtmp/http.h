#if !defined(__RTMP_HTTP_H__)
#define __RTMP_HTTP_H__























typedef enum
{
HTTPRES_OK, 
HTTPRES_OK_NOT_MODIFIED, 
HTTPRES_NOT_FOUND, 
HTTPRES_BAD_REQUEST, 
HTTPRES_SERVER_ERROR, 
HTTPRES_REDIRECTED, 
HTTPRES_LOST_CONNECTION 
} HTTPResult;

struct HTTP_ctx
{
char *date;
int size;
int status;
void *data;
};

typedef size_t (HTTP_read_callback)(void *ptr, size_t size, size_t nmemb, void *stream);

HTTPResult HTTP_get(struct HTTP_ctx *http, const char *url, HTTP_read_callback *cb);

#endif
