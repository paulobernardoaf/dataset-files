




















#include "avformat.h"
#include "avio_internal.h"
#include "rtpenc_chain.h"
#include "rtp.h"
#include "libavutil/opt.h"

int ff_rtp_chain_mux_open(AVFormatContext **out, AVFormatContext *s,
AVStream *st, URLContext *handle, int packet_size,
int idx)
{
AVFormatContext *rtpctx = NULL;
int ret;
ff_const59 AVOutputFormat *rtp_format = av_guess_format("rtp", NULL, NULL);
uint8_t *rtpflags;
AVDictionary *opts = NULL;

if (!rtp_format) {
ret = AVERROR(ENOSYS);
goto fail;
}


rtpctx = avformat_alloc_context();
if (!rtpctx) {
ret = AVERROR(ENOMEM);
goto fail;
}

rtpctx->oformat = rtp_format;
if (!avformat_new_stream(rtpctx, NULL)) {
ret = AVERROR(ENOMEM);
goto fail;
}

rtpctx->interrupt_callback = s->interrupt_callback;

rtpctx->max_delay = s->max_delay;

rtpctx->streams[0]->sample_aspect_ratio = st->sample_aspect_ratio;
rtpctx->flags |= s->flags & AVFMT_FLAG_BITEXACT;
rtpctx->strict_std_compliance = s->strict_std_compliance;


if (st->id < RTP_PT_PRIVATE)
rtpctx->streams[0]->id =
ff_rtp_get_payload_type(s, st->codecpar, idx);
else
rtpctx->streams[0]->id = st->id;


if (av_opt_get(s, "rtpflags", AV_OPT_SEARCH_CHILDREN, &rtpflags) >= 0)
av_dict_set(&opts, "rtpflags", rtpflags, AV_DICT_DONT_STRDUP_VAL);


rtpctx->start_time_realtime = s->start_time_realtime;

avcodec_parameters_copy(rtpctx->streams[0]->codecpar, st->codecpar);
rtpctx->streams[0]->time_base = st->time_base;

if (handle) {
ret = ffio_fdopen(&rtpctx->pb, handle);
if (ret < 0)
ffurl_close(handle);
} else
ret = ffio_open_dyn_packet_buf(&rtpctx->pb, packet_size);
if (!ret)
ret = avformat_write_header(rtpctx, &opts);
av_dict_free(&opts);

if (ret) {
if (handle && rtpctx->pb) {
avio_closep(&rtpctx->pb);
} else if (rtpctx->pb) {
ffio_free_dyn_buf(&rtpctx->pb);
}
avformat_free_context(rtpctx);
return ret;
}

*out = rtpctx;
return 0;

fail:
avformat_free_context(rtpctx);
if (handle)
ffurl_close(handle);
return ret;
}
