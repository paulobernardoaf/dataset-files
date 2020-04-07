




















#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "pcm.h"

static int fwse_probe(const AVProbeData *p)
{
if (AV_RL32(p->buf) != MKTAG('F','W','S','E'))
return 0;
if (AV_RL32(p->buf+4) != 2 && AV_RL32(p->buf+4) != 3)
return 0;
if (AV_RL32(p->buf+16) != 1 && AV_RL32(p->buf+16) != 2)
return 0;

return AVPROBE_SCORE_MAX / 4 * 3;
}

static int fwse_read_header(AVFormatContext *s)
{
unsigned start_offset, version;
AVIOContext *pb = s->pb;
AVCodecParameters *par;
AVStream *st;

avio_skip(pb, 4);
version = avio_rl32(pb);
if (version != 2 && version != 3)
return AVERROR_INVALIDDATA;
avio_skip(pb, 4);
start_offset = avio_rl32(pb);

st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

par = st->codecpar;
par->codec_type = AVMEDIA_TYPE_AUDIO;
par->codec_id = AV_CODEC_ID_ADPCM_IMA_MTF;
par->format = AV_SAMPLE_FMT_S16;
par->channels = avio_rl32(pb);
if (par->channels != 1 && par->channels != 2)
return AVERROR_INVALIDDATA;
if (par->channels == 1)
par->channel_layout = AV_CH_LAYOUT_MONO;
else if (par->channels == 2)
par->channel_layout = AV_CH_LAYOUT_STEREO;
st->duration = avio_rl32(pb);
par->sample_rate = avio_rl32(pb);
if (par->sample_rate <= 0 || par->sample_rate > INT_MAX)
return AVERROR_INVALIDDATA;

par->block_align = 1;
avio_skip(pb, start_offset - avio_tell(pb));

avpriv_set_pts_info(st, 64, 1, par->sample_rate);

return 0;
}

AVInputFormat ff_fwse_demuxer = {
.name = "fwse",
.long_name = NULL_IF_CONFIG_SMALL("Capcom's MT Framework sound"),
.read_probe = fwse_probe,
.read_header = fwse_read_header,
.read_packet = ff_pcm_read_packet,
.extensions = "fwse",
};
