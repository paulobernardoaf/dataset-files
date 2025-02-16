#include "libavutil/avstring.h"
#include "libavutil/mem.h"
#include "avformat.h"
#include "url.h"
#define AV_CAT_SEPARATOR "|"
struct concat_nodes {
URLContext *uc; 
int64_t size; 
};
struct concat_data {
struct concat_nodes *nodes; 
size_t length; 
size_t current; 
uint64_t total_size;
};
static av_cold int concat_close(URLContext *h)
{
int err = 0;
size_t i;
struct concat_data *data = h->priv_data;
struct concat_nodes *nodes = data->nodes;
for (i = 0; i != data->length; i++)
err |= ffurl_close(nodes[i].uc);
av_freep(&data->nodes);
return err < 0 ? -1 : 0;
}
static av_cold int concat_open(URLContext *h, const char *uri, int flags)
{
char *node_uri = NULL;
int err = 0;
int64_t size, total_size = 0;
size_t len, i;
URLContext *uc;
struct concat_data *data = h->priv_data;
struct concat_nodes *nodes;
if (!av_strstart(uri, "concat:", &uri)) {
av_log(h, AV_LOG_ERROR, "URL %s lacks prefix\n", uri);
return AVERROR(EINVAL);
}
for (i = 0, len = 1; uri[i]; i++) {
if (uri[i] == *AV_CAT_SEPARATOR) {
if (++len == UINT_MAX / sizeof(*nodes)) {
av_freep(&h->priv_data);
return AVERROR(ENAMETOOLONG);
}
}
}
if (!(nodes = av_realloc(NULL, sizeof(*nodes) * len)))
return AVERROR(ENOMEM);
else
data->nodes = nodes;
if (!*uri)
err = AVERROR(ENOENT);
for (i = 0; *uri; i++) {
len = strcspn(uri, AV_CAT_SEPARATOR);
if ((err = av_reallocp(&node_uri, len + 1)) < 0)
break;
av_strlcpy(node_uri, uri, len + 1);
uri += len + strspn(uri + len, AV_CAT_SEPARATOR);
err = ffurl_open_whitelist(&uc, node_uri, flags,
&h->interrupt_callback, NULL, h->protocol_whitelist, h->protocol_blacklist, h);
if (err < 0)
break;
if ((size = ffurl_size(uc)) < 0) {
ffurl_close(uc);
err = AVERROR(ENOSYS);
break;
}
nodes[i].uc = uc;
nodes[i].size = size;
total_size += size;
}
av_free(node_uri);
data->length = i;
if (err < 0)
concat_close(h);
else if (!(nodes = av_realloc(nodes, data->length * sizeof(*nodes)))) {
concat_close(h);
err = AVERROR(ENOMEM);
} else
data->nodes = nodes;
data->total_size = total_size;
return err;
}
static int concat_read(URLContext *h, unsigned char *buf, int size)
{
int result, total = 0;
struct concat_data *data = h->priv_data;
struct concat_nodes *nodes = data->nodes;
size_t i = data->current;
while (size > 0) {
result = ffurl_read(nodes[i].uc, buf, size);
if (result == AVERROR_EOF) {
if (i + 1 == data->length ||
ffurl_seek(nodes[++i].uc, 0, SEEK_SET) < 0)
break;
result = 0;
}
if (result < 0)
return total ? total : result;
total += result;
buf += result;
size -= result;
}
data->current = i;
return total ? total : result;
}
static int64_t concat_seek(URLContext *h, int64_t pos, int whence)
{
int64_t result;
struct concat_data *data = h->priv_data;
struct concat_nodes *nodes = data->nodes;
size_t i;
if ((whence & AVSEEK_SIZE))
return data->total_size;
switch (whence) {
case SEEK_END:
for (i = data->length - 1; i && pos < -nodes[i].size; i--)
pos += nodes[i].size;
break;
case SEEK_CUR:
for (i = 0; i != data->current; i++)
pos += nodes[i].size;
pos += ffurl_seek(nodes[i].uc, 0, SEEK_CUR);
whence = SEEK_SET;
case SEEK_SET:
for (i = 0; i != data->length - 1 && pos >= nodes[i].size; i++)
pos -= nodes[i].size;
break;
default:
return AVERROR(EINVAL);
}
result = ffurl_seek(nodes[i].uc, pos, whence);
if (result >= 0) {
data->current = i;
while (i)
result += nodes[--i].size;
}
return result;
}
const URLProtocol ff_concat_protocol = {
.name = "concat",
.url_open = concat_open,
.url_read = concat_read,
.url_seek = concat_seek,
.url_close = concat_close,
.priv_data_size = sizeof(struct concat_data),
.default_whitelist = "concat,file,subfile",
};
