#include "avformat.h"
#include "internal.h"
typedef struct CdataDemuxContext {
unsigned int channels;
unsigned int audio_pts;
} CdataDemuxContext;
static int cdata_probe(const AVProbeData *p)
{
const uint8_t *b = p->buf;
if (b[0] == 0x04 && (b[1] == 0x00 || b[1] == 0x04 || b[1] == 0x0C || b[1] == 0x14))
return AVPROBE_SCORE_MAX/8;
return 0;
}
static int cdata_read_header(AVFormatContext *s)
{
CdataDemuxContext *cdata = s->priv_data;
AVIOContext *pb = s->pb;
unsigned int sample_rate, header;
AVStream *st;
int64_t channel_layout = 0;
header = avio_rb16(pb);
switch (header) {
case 0x0400: cdata->channels = 1; break;
case 0x0404: cdata->channels = 2; break;
case 0x040C: cdata->channels = 4; channel_layout = AV_CH_LAYOUT_QUAD; break;
case 0x0414: cdata->channels = 6; channel_layout = AV_CH_LAYOUT_5POINT1_BACK; break;
default:
av_log(s, AV_LOG_INFO, "unknown header 0x%04x\n", header);
return -1;
};
sample_rate = avio_rb16(pb);
avio_skip(pb, (avio_r8(pb) & 0x20) ? 15 : 11);
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_tag = 0; 
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_EA_XAS;
st->codecpar->channels = cdata->channels;
st->codecpar->channel_layout = channel_layout;
st->codecpar->sample_rate = sample_rate;
avpriv_set_pts_info(st, 64, 1, sample_rate);
cdata->audio_pts = 0;
return 0;
}
static int cdata_read_packet(AVFormatContext *s, AVPacket *pkt)
{
CdataDemuxContext *cdata = s->priv_data;
int packet_size = 76*cdata->channels;
int ret = av_get_packet(s->pb, pkt, packet_size);
if (ret < 0)
return ret;
pkt->pts = cdata->audio_pts++;
return 0;
}
AVInputFormat ff_ea_cdata_demuxer = {
.name = "ea_cdata",
.long_name = NULL_IF_CONFIG_SMALL("Electronic Arts cdata"),
.priv_data_size = sizeof(CdataDemuxContext),
.read_probe = cdata_probe,
.read_header = cdata_read_header,
.read_packet = cdata_read_packet,
.extensions = "cdata",
};
