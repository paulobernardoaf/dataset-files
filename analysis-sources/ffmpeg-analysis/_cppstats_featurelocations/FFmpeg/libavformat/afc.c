




















#include "libavutil/channel_layout.h"
#include "avformat.h"
#include "internal.h"

typedef struct AFCDemuxContext {
int64_t data_end;
} AFCDemuxContext;

static int afc_read_header(AVFormatContext *s)
{
AFCDemuxContext *c = s->priv_data;
AVStream *st;
int ret;

st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_AFC;
st->codecpar->channels = 2;
st->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;

if ((ret = ff_alloc_extradata(st->codecpar, 1)) < 0)
return ret;
st->codecpar->extradata[0] = 8 * st->codecpar->channels;

c->data_end = avio_rb32(s->pb) + 32LL;
st->duration = avio_rb32(s->pb);
st->codecpar->sample_rate = avio_rb16(s->pb);
avio_skip(s->pb, 22);
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);

return 0;
}

static int afc_read_packet(AVFormatContext *s, AVPacket *pkt)
{
AFCDemuxContext *c = s->priv_data;
int64_t size;
int ret;

size = FFMIN(c->data_end - avio_tell(s->pb), 18 * 128);
if (size <= 0)
return AVERROR_EOF;

ret = av_get_packet(s->pb, pkt, size);
pkt->stream_index = 0;
return ret;
}

AVInputFormat ff_afc_demuxer = {
.name = "afc",
.long_name = NULL_IF_CONFIG_SMALL("AFC"),
.priv_data_size = sizeof(AFCDemuxContext),
.read_header = afc_read_header,
.read_packet = afc_read_packet,
.extensions = "afc",
.flags = AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK,
};
