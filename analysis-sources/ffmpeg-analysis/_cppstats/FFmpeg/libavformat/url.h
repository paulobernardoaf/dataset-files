#include "avio.h"
#include "libavformat/version.h"
#include "libavutil/dict.h"
#include "libavutil/log.h"
#define URL_PROTOCOL_FLAG_NESTED_SCHEME 1 
#define URL_PROTOCOL_FLAG_NETWORK 2 
extern const AVClass ffurl_context_class;
typedef struct URLContext {
const AVClass *av_class; 
const struct URLProtocol *prot;
void *priv_data;
char *filename; 
int flags;
int max_packet_size; 
int is_streamed; 
int is_connected;
AVIOInterruptCB interrupt_callback;
int64_t rw_timeout; 
const char *protocol_whitelist;
const char *protocol_blacklist;
int min_packet_size; 
} URLContext;
typedef struct URLProtocol {
const char *name;
int (*url_open)( URLContext *h, const char *url, int flags);
int (*url_open2)(URLContext *h, const char *url, int flags, AVDictionary **options);
int (*url_accept)(URLContext *s, URLContext **c);
int (*url_handshake)(URLContext *c);
int (*url_read)( URLContext *h, unsigned char *buf, int size);
int (*url_write)(URLContext *h, const unsigned char *buf, int size);
int64_t (*url_seek)( URLContext *h, int64_t pos, int whence);
int (*url_close)(URLContext *h);
int (*url_read_pause)(URLContext *h, int pause);
int64_t (*url_read_seek)(URLContext *h, int stream_index,
int64_t timestamp, int flags);
int (*url_get_file_handle)(URLContext *h);
int (*url_get_multi_file_handle)(URLContext *h, int **handles,
int *numhandles);
int (*url_get_short_seek)(URLContext *h);
int (*url_shutdown)(URLContext *h, int flags);
int priv_data_size;
const AVClass *priv_data_class;
int flags;
int (*url_check)(URLContext *h, int mask);
int (*url_open_dir)(URLContext *h);
int (*url_read_dir)(URLContext *h, AVIODirEntry **next);
int (*url_close_dir)(URLContext *h);
int (*url_delete)(URLContext *h);
int (*url_move)(URLContext *h_src, URLContext *h_dst);
const char *default_whitelist;
} URLProtocol;
int ffurl_alloc(URLContext **puc, const char *filename, int flags,
const AVIOInterruptCB *int_cb);
int ffurl_connect(URLContext *uc, AVDictionary **options);
int ffurl_open_whitelist(URLContext **puc, const char *filename, int flags,
const AVIOInterruptCB *int_cb, AVDictionary **options,
const char *whitelist, const char* blacklist,
URLContext *parent);
int ffurl_open(URLContext **puc, const char *filename, int flags,
const AVIOInterruptCB *int_cb, AVDictionary **options);
int ffurl_accept(URLContext *s, URLContext **c);
int ffurl_handshake(URLContext *c);
int ffurl_read(URLContext *h, unsigned char *buf, int size);
int ffurl_read_complete(URLContext *h, unsigned char *buf, int size);
int ffurl_write(URLContext *h, const unsigned char *buf, int size);
int64_t ffurl_seek(URLContext *h, int64_t pos, int whence);
int ffurl_closep(URLContext **h);
int ffurl_close(URLContext *h);
int64_t ffurl_size(URLContext *h);
int ffurl_get_file_handle(URLContext *h);
int ffurl_get_multi_file_handle(URLContext *h, int **handles, int *numhandles);
int ffurl_get_short_seek(URLContext *h);
int ffurl_shutdown(URLContext *h, int flags);
int ff_check_interrupt(AVIOInterruptCB *cb);
int ff_udp_set_remote_url(URLContext *h, const char *uri);
int ff_udp_get_local_port(URLContext *h);
int ff_url_join(char *str, int size, const char *proto,
const char *authorization, const char *hostname,
int port, const char *fmt, ...) av_printf_format(7, 8);
void ff_make_absolute_url(char *buf, int size, const char *base,
const char *rel);
AVIODirEntry *ff_alloc_dir_entry(void);
const AVClass *ff_urlcontext_child_class_next(const AVClass *prev);
const URLProtocol **ffurl_get_protocols(const char *whitelist,
const char *blacklist);
