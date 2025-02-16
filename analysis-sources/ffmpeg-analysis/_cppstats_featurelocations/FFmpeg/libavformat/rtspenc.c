




















#include "avformat.h"

#if HAVE_POLL_H
#include <poll.h>
#endif
#include "network.h"
#include "os_support.h"
#include "rtsp.h"
#include "internal.h"
#include "avio_internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/avstring.h"
#include "libavutil/time.h"
#include "url.h"

#define SDP_MAX_SIZE 16384

static const AVClass rtsp_muxer_class = {
.class_name = "RTSP muxer",
.item_name = av_default_item_name,
.option = ff_rtsp_options,
.version = LIBAVUTIL_VERSION_INT,
};

int ff_rtsp_setup_output_streams(AVFormatContext *s, const char *addr)
{
RTSPState *rt = s->priv_data;
RTSPMessageHeader reply1, *reply = &reply1;
int i;
char *sdp;
AVFormatContext sdp_ctx, *ctx_array[1];
char url[1024];

if (s->start_time_realtime == 0 || s->start_time_realtime == AV_NOPTS_VALUE)
s->start_time_realtime = av_gettime();


sdp = av_mallocz(SDP_MAX_SIZE);
if (!sdp)
return AVERROR(ENOMEM);












sdp_ctx = *s;
sdp_ctx.url = url;
ff_url_join(url, sizeof(url),
"rtsp", NULL, addr, -1, NULL);
ctx_array[0] = &sdp_ctx;
if (av_sdp_create(ctx_array, 1, sdp, SDP_MAX_SIZE)) {
av_free(sdp);
return AVERROR_INVALIDDATA;
}
av_log(s, AV_LOG_VERBOSE, "SDP:\n%s\n", sdp);
ff_rtsp_send_cmd_with_content(s, "ANNOUNCE", rt->control_uri,
"Content-Type: application/sdp\r\n",
reply, NULL, sdp, strlen(sdp));
av_free(sdp);
if (reply->status_code != RTSP_STATUS_OK)
return ff_rtsp_averror(reply->status_code, AVERROR_INVALIDDATA);


for (i = 0; i < s->nb_streams; i++) {
RTSPStream *rtsp_st;

rtsp_st = av_mallocz(sizeof(RTSPStream));
if (!rtsp_st)
return AVERROR(ENOMEM);
dynarray_add(&rt->rtsp_streams, &rt->nb_rtsp_streams, rtsp_st);

rtsp_st->stream_index = i;

av_strlcpy(rtsp_st->control_url, rt->control_uri, sizeof(rtsp_st->control_url));

av_strlcatf(rtsp_st->control_url, sizeof(rtsp_st->control_url),
"/streamid=%d", i);
}

return 0;
}

static int rtsp_write_record(AVFormatContext *s)
{
RTSPState *rt = s->priv_data;
RTSPMessageHeader reply1, *reply = &reply1;
char cmd[1024];

snprintf(cmd, sizeof(cmd),
"Range: npt=0.000-\r\n");
ff_rtsp_send_cmd(s, "RECORD", rt->control_uri, cmd, reply, NULL);
if (reply->status_code != RTSP_STATUS_OK)
return ff_rtsp_averror(reply->status_code, -1);
rt->state = RTSP_STATE_STREAMING;
return 0;
}

static int rtsp_write_header(AVFormatContext *s)
{
int ret;

ret = ff_rtsp_connect(s);
if (ret)
return ret;

if (rtsp_write_record(s) < 0) {
ff_rtsp_close_streams(s);
ff_rtsp_close_connections(s);
return AVERROR_INVALIDDATA;
}
return 0;
}

int ff_rtsp_tcp_write_packet(AVFormatContext *s, RTSPStream *rtsp_st)
{
RTSPState *rt = s->priv_data;
AVFormatContext *rtpctx = rtsp_st->transport_priv;
uint8_t *buf, *ptr;
int size;
uint8_t *interleave_header, *interleaved_packet;

size = avio_close_dyn_buf(rtpctx->pb, &buf);
rtpctx->pb = NULL;
ptr = buf;
while (size > 4) {
uint32_t packet_len = AV_RB32(ptr);
int id;





interleaved_packet = interleave_header = ptr;
ptr += 4;
size -= 4;
if (packet_len > size || packet_len < 2)
break;
if (RTP_PT_IS_RTCP(ptr[1]))
id = rtsp_st->interleaved_max; 
else
id = rtsp_st->interleaved_min; 
interleave_header[0] = '$';
interleave_header[1] = id;
AV_WB16(interleave_header + 2, packet_len);
ffurl_write(rt->rtsp_hd_out, interleaved_packet, 4 + packet_len);
ptr += packet_len;
size -= packet_len;
}
av_free(buf);
return ffio_open_dyn_packet_buf(&rtpctx->pb, RTSP_TCP_MAX_PACKET_SIZE);
}

static int rtsp_write_packet(AVFormatContext *s, AVPacket *pkt)
{
RTSPState *rt = s->priv_data;
RTSPStream *rtsp_st;
int n;
struct pollfd p = {ffurl_get_file_handle(rt->rtsp_hd), POLLIN, 0};
AVFormatContext *rtpctx;
int ret;

while (1) {
n = poll(&p, 1, 0);
if (n <= 0)
break;
if (p.revents & POLLIN) {
RTSPMessageHeader reply;





ret = ff_rtsp_read_reply(s, &reply, NULL, 1, NULL);
if (ret < 0)
return AVERROR(EPIPE);
if (ret == 1)
ff_rtsp_skip_packet(s);

if (rt->state != RTSP_STATE_STREAMING)
return AVERROR(EPIPE);
}
}

if (pkt->stream_index < 0 || pkt->stream_index >= rt->nb_rtsp_streams)
return AVERROR_INVALIDDATA;
rtsp_st = rt->rtsp_streams[pkt->stream_index];
rtpctx = rtsp_st->transport_priv;

ret = ff_write_chained(rtpctx, 0, pkt, s, 0);




if (!ret && rt->lower_transport == RTSP_LOWER_TRANSPORT_TCP)
ret = ff_rtsp_tcp_write_packet(s, rtsp_st);
return ret;
}

static int rtsp_write_close(AVFormatContext *s)
{
RTSPState *rt = s->priv_data;




ff_rtsp_undo_setup(s, 1);

ff_rtsp_send_cmd_async(s, "TEARDOWN", rt->control_uri, NULL);

ff_rtsp_close_streams(s);
ff_rtsp_close_connections(s);
ff_network_close();
return 0;
}

AVOutputFormat ff_rtsp_muxer = {
.name = "rtsp",
.long_name = NULL_IF_CONFIG_SMALL("RTSP output"),
.priv_data_size = sizeof(RTSPState),
.audio_codec = AV_CODEC_ID_AAC,
.video_codec = AV_CODEC_ID_MPEG4,
.write_header = rtsp_write_header,
.write_packet = rtsp_write_packet,
.write_trailer = rtsp_write_close,
.flags = AVFMT_NOFILE | AVFMT_GLOBALHEADER,
.priv_class = &rtsp_muxer_class,
};
