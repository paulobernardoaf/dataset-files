




















#include "libavutil/avassert.h"
#include "avformat.h"
#include "internal.h"
#include "rawdec.h"

static int g722_read_header(AVFormatContext *s)
{
AVStream *st;

st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_G722;
st->codecpar->sample_rate = 16000;
st->codecpar->channels = 1;

st->codecpar->bits_per_coded_sample =
av_get_bits_per_sample(st->codecpar->codec_id);

av_assert0(st->codecpar->bits_per_coded_sample > 0);

avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
return 0;
}

FF_RAW_DEMUXER_CLASS(g722)
AVInputFormat ff_g722_demuxer = {
.name = "g722",
.long_name = NULL_IF_CONFIG_SMALL("raw G.722"),
.read_header = g722_read_header,
.read_packet = ff_raw_read_partial_packet,
.flags = AVFMT_GENERIC_INDEX,
.extensions = "g722,722",
.raw_codec_id = AV_CODEC_ID_ADPCM_G722,
.priv_data_size = sizeof(FFRawDemuxerContext),
.priv_class = &g722_demuxer_class,};
