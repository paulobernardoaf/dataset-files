#include <stdio.h>
#include "libavutil/avstring.h"
#include "libavutil/md5.h"
#include "libavutil/mem.h"
#include "libavutil/error.h"
#include "avformat.h"
#include "avio.h"
#include "url.h"
struct MD5Context {
struct AVMD5 *md5;
};
static int md5_open(URLContext *h, const char *filename, int flags)
{
struct MD5Context *c = h->priv_data;
if (!(flags & AVIO_FLAG_WRITE))
return AVERROR(EINVAL);
c->md5 = av_md5_alloc();
if (!c->md5)
return AVERROR(ENOMEM);
av_md5_init(c->md5);
return 0;
}
static int md5_write(URLContext *h, const unsigned char *buf, int size)
{
struct MD5Context *c = h->priv_data;
av_md5_update(c->md5, buf, size);
return size;
}
static int md5_close(URLContext *h)
{
struct MD5Context *c = h->priv_data;
const char *filename = h->filename;
uint8_t md5[16], buf[64];
URLContext *out;
int i, err = 0;
av_md5_final(c->md5, md5);
for (i = 0; i < sizeof(md5); i++)
snprintf(buf + i*2, 3, "%02x", md5[i]);
buf[i*2] = '\n';
av_strstart(filename, "md5:", &filename);
if (*filename) {
err = ffurl_open_whitelist(&out, filename, AVIO_FLAG_WRITE,
&h->interrupt_callback, NULL,
h->protocol_whitelist, h->protocol_blacklist, h);
if (err)
return err;
err = ffurl_write(out, buf, i*2+1);
ffurl_close(out);
} else {
if (fwrite(buf, 1, i*2+1, stdout) < i*2+1)
err = AVERROR(errno);
}
av_freep(&c->md5);
return err;
}
const URLProtocol ff_md5_protocol = {
.name = "md5",
.url_open = md5_open,
.url_write = md5_write,
.url_close = md5_close,
.priv_data_size = sizeof(struct MD5Context),
};
