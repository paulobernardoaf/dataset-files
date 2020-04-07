





















#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "pcm.h"
#include "rso.h"

static int rso_read_header(AVFormatContext *s)
{
AVIOContext *pb = s->pb;
int id, rate, bps;
unsigned int size;
enum AVCodecID codec;
AVStream *st;

id = avio_rb16(pb);
size = avio_rb16(pb);
rate = avio_rb16(pb);
avio_rb16(pb); 

codec = ff_codec_get_id(ff_codec_rso_tags, id);

if (codec == AV_CODEC_ID_ADPCM_IMA_WAV) {
avpriv_report_missing_feature(s, "ADPCM in RSO");
return AVERROR_PATCHWELCOME;
}

bps = av_get_bits_per_sample(codec);
if (!bps) {
avpriv_request_sample(s, "Unknown bits per sample");
return AVERROR_PATCHWELCOME;
}


st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

st->duration = (size * 8) / bps;
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_tag = id;
st->codecpar->codec_id = codec;
st->codecpar->channels = 1;
st->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
st->codecpar->sample_rate = rate;
st->codecpar->block_align = 1;

avpriv_set_pts_info(st, 64, 1, rate);

return 0;
}

AVInputFormat ff_rso_demuxer = {
.name = "rso",
.long_name = NULL_IF_CONFIG_SMALL("Lego Mindstorms RSO"),
.extensions = "rso",
.read_header = rso_read_header,
.read_packet = ff_pcm_read_packet,
.read_seek = ff_pcm_read_seek,
.codec_tag = (const AVCodecTag* const []){ff_codec_rso_tags, 0},
};
