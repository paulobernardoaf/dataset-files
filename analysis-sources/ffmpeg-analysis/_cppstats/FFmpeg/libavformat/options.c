#include "avformat.h"
#include "avio_internal.h"
#include "internal.h"
#include "libavutil/internal.h"
#include "libavutil/opt.h"
FF_DISABLE_DEPRECATION_WARNINGS
#include "options_table.h"
FF_ENABLE_DEPRECATION_WARNINGS
static const char* format_to_name(void* ptr)
{
AVFormatContext* fc = (AVFormatContext*) ptr;
if(fc->iformat) return fc->iformat->name;
else if(fc->oformat) return fc->oformat->name;
else return "NULL";
}
static void *format_child_next(void *obj, void *prev)
{
AVFormatContext *s = obj;
if (!prev && s->priv_data &&
((s->iformat && s->iformat->priv_class) ||
s->oformat && s->oformat->priv_class))
return s->priv_data;
if (s->pb && s->pb->av_class && prev != s->pb)
return s->pb;
return NULL;
}
static const AVClass *format_child_class_next(const AVClass *prev)
{
AVInputFormat *ifmt = NULL;
AVOutputFormat *ofmt = NULL;
if (!prev)
return &ff_avio_class;
while ((ifmt = av_iformat_next(ifmt)))
if (ifmt->priv_class == prev)
break;
if (!ifmt)
while ((ofmt = av_oformat_next(ofmt)))
if (ofmt->priv_class == prev)
break;
if (!ofmt)
while (ifmt = av_iformat_next(ifmt))
if (ifmt->priv_class)
return ifmt->priv_class;
while (ofmt = av_oformat_next(ofmt))
if (ofmt->priv_class)
return ofmt->priv_class;
return NULL;
}
static AVClassCategory get_category(void *ptr)
{
AVFormatContext* s = ptr;
if(s->iformat) return AV_CLASS_CATEGORY_DEMUXER;
else return AV_CLASS_CATEGORY_MUXER;
}
static const AVClass av_format_context_class = {
.class_name = "AVFormatContext",
.item_name = format_to_name,
.option = avformat_options,
.version = LIBAVUTIL_VERSION_INT,
.child_next = format_child_next,
.child_class_next = format_child_class_next,
.category = AV_CLASS_CATEGORY_MUXER,
.get_category = get_category,
};
static int io_open_default(AVFormatContext *s, AVIOContext **pb,
const char *url, int flags, AVDictionary **options)
{
int loglevel;
if (!strcmp(url, s->url) ||
s->iformat && !strcmp(s->iformat->name, "image2") ||
s->oformat && !strcmp(s->oformat->name, "image2")
) {
loglevel = AV_LOG_DEBUG;
} else
loglevel = AV_LOG_INFO;
av_log(s, loglevel, "Opening \'%s\' for %s\n", url, flags & AVIO_FLAG_WRITE ? "writing" : "reading");
#if FF_API_OLD_OPEN_CALLBACKS
FF_DISABLE_DEPRECATION_WARNINGS
if (s->open_cb)
return s->open_cb(s, pb, url, flags, &s->interrupt_callback, options);
FF_ENABLE_DEPRECATION_WARNINGS
#endif
return ffio_open_whitelist(pb, url, flags, &s->interrupt_callback, options, s->protocol_whitelist, s->protocol_blacklist);
}
static void io_close_default(AVFormatContext *s, AVIOContext *pb)
{
avio_close(pb);
}
static void avformat_get_context_defaults(AVFormatContext *s)
{
memset(s, 0, sizeof(AVFormatContext));
s->av_class = &av_format_context_class;
s->io_open = io_open_default;
s->io_close = io_close_default;
av_opt_set_defaults(s);
}
AVFormatContext *avformat_alloc_context(void)
{
AVFormatContext *ic;
AVFormatInternal *internal;
ic = av_malloc(sizeof(AVFormatContext));
if (!ic) return ic;
internal = av_mallocz(sizeof(*internal));
if (!internal) {
av_free(ic);
return NULL;
}
avformat_get_context_defaults(ic);
ic->internal = internal;
ic->internal->offset = AV_NOPTS_VALUE;
ic->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
ic->internal->shortest_end = AV_NOPTS_VALUE;
return ic;
}
enum AVDurationEstimationMethod av_fmt_ctx_get_duration_estimation_method(const AVFormatContext* ctx)
{
return ctx->duration_estimation_method;
}
const AVClass *avformat_get_class(void)
{
return &av_format_context_class;
}
