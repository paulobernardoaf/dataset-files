




















#include "libavutil/avstring.h"
#include "libavutil/opt.h"
#include "avformat.h"
#include "avio_internal.h"
#include "tee_common.h"

typedef struct ChildContext {
URLContext *url_context;
} ChildContext;

typedef struct TeeContext {
const AVClass *class;
int child_count;
ChildContext *child;
} TeeContext;

static const AVOption tee_options[] = {
{ NULL }
};

static const AVClass tee_class = {
.class_name = "tee",
.item_name = av_default_item_name,
.option = tee_options,
.version = LIBAVUTIL_VERSION_INT,
};

static const char *const child_delim = "|";

static int tee_write(URLContext *h, const unsigned char *buf, int size)
{
TeeContext *c = h->priv_data;
int i;
int main_ret = size;

for (i=0; i<c->child_count; i++) {
int ret = ffurl_write(c->child[i].url_context, buf, size);
if (ret < 0)
main_ret = ret;
}
return main_ret;
}

static int tee_close(URLContext *h)
{
TeeContext *c = h->priv_data;
int i;
int main_ret = 0;

for (i=0; i<c->child_count; i++) {
int ret = ffurl_closep(&c->child[i].url_context);
if (ret < 0)
main_ret = ret;
}

av_freep(&c->child);
c->child_count = 0;
return main_ret;
}

static int tee_open(URLContext *h, const char *filename, int flags)
{
TeeContext *c = h->priv_data;
int ret, i;

av_strstart(filename, "tee:", &filename);

if (flags & AVIO_FLAG_READ)
return AVERROR(ENOSYS);

while (*filename) {
char *child_string = av_get_token(&filename, child_delim);
char *child_name = NULL;
void *tmp;
AVDictionary *options = NULL;
if (!child_string) {
ret = AVERROR(ENOMEM);
goto fail;
}

tmp = av_realloc_array(c->child, c->child_count + 1, sizeof(*c->child));
if (!tmp) {
ret = AVERROR(ENOMEM);
goto loop_fail;
}
c->child = tmp;
memset(&c->child[c->child_count], 0, sizeof(c->child[c->child_count]));

ret = ff_tee_parse_slave_options(h, child_string, &options, &child_name);
if (ret < 0)
goto loop_fail;

ret = ffurl_open_whitelist(&c->child[c->child_count].url_context, child_name, flags,
&h->interrupt_callback, &options,
h->protocol_whitelist, h->protocol_blacklist,
h);
loop_fail:
av_freep(&child_string);
av_dict_free(&options);
if (ret < 0)
goto fail;
c->child_count++;

if (strspn(filename, child_delim))
filename++;
}

h->is_streamed = 0;
for (i=0; i<c->child_count; i++) {
h->is_streamed |= c->child[i].url_context->is_streamed;
}

return 0;
fail:
tee_close(h);
return ret;
}
const URLProtocol ff_tee_protocol = {
.name = "tee",
.url_open = tee_open,
.url_write = tee_write,
.url_close = tee_close,
.priv_data_size = sizeof(TeeContext),
.priv_data_class = &tee_class,
.default_whitelist = "crypto,file,http,https,httpproxy,rtmp,tcp,tls"
};
