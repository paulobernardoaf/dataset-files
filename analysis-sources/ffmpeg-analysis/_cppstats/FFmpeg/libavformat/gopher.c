#include "libavutil/avstring.h"
#include "avformat.h"
#include "internal.h"
#include "network.h"
#include "url.h"
typedef struct GopherContext {
URLContext *hd;
} GopherContext;
static int gopher_write(URLContext *h, const uint8_t *buf, int size)
{
GopherContext *s = h->priv_data;
return ffurl_write(s->hd, buf, size);
}
static int gopher_connect(URLContext *h, const char *path)
{
char buffer[1024];
if (!*path) return AVERROR(EINVAL);
switch (*++path) {
case '5':
case '9':
path = strchr(path, '/');
if (!path) return AVERROR(EINVAL);
break;
default:
av_log(h, AV_LOG_WARNING,
"Gopher protocol type '%c' not supported yet!\n",
*path);
return AVERROR(EINVAL);
}
snprintf(buffer, sizeof(buffer), "%s\r\n", path);
if (gopher_write(h, buffer, strlen(buffer)) < 0)
return AVERROR(EIO);
return 0;
}
static int gopher_close(URLContext *h)
{
GopherContext *s = h->priv_data;
if (s->hd) {
ffurl_close(s->hd);
s->hd = NULL;
}
return 0;
}
static int gopher_open(URLContext *h, const char *uri, int flags)
{
GopherContext *s = h->priv_data;
char hostname[1024], auth[1024], path[1024], buf[1024];
int port, err;
h->is_streamed = 1;
av_url_split(NULL, 0, auth, sizeof(auth), hostname, sizeof(hostname), &port,
path, sizeof(path), uri);
if (port < 0)
port = 70;
ff_url_join(buf, sizeof(buf), "tcp", NULL, hostname, port, NULL);
s->hd = NULL;
err = ffurl_open_whitelist(&s->hd, buf, AVIO_FLAG_READ_WRITE,
&h->interrupt_callback, NULL, h->protocol_whitelist, h->protocol_blacklist, h);
if (err < 0)
goto fail;
if ((err = gopher_connect(h, path)) < 0)
goto fail;
return 0;
fail:
gopher_close(h);
return err;
}
static int gopher_read(URLContext *h, uint8_t *buf, int size)
{
GopherContext *s = h->priv_data;
int len = ffurl_read(s->hd, buf, size);
return len;
}
const URLProtocol ff_gopher_protocol = {
.name = "gopher",
.url_open = gopher_open,
.url_read = gopher_read,
.url_write = gopher_write,
.url_close = gopher_close,
.priv_data_size = sizeof(GopherContext),
.flags = URL_PROTOCOL_FLAG_NETWORK,
};
