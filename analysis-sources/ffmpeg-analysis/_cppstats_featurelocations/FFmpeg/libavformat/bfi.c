



























#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"

typedef struct BFIContext {
int nframes;
int audio_frame;
int video_frame;
int video_size;
int avflag;
} BFIContext;

static int bfi_probe(const AVProbeData * p)
{

if (AV_RL32(p->buf) == MKTAG('B', 'F', '&', 'I'))
return AVPROBE_SCORE_MAX;
else
return 0;
}

static int bfi_read_header(AVFormatContext * s)
{
BFIContext *bfi = s->priv_data;
AVIOContext *pb = s->pb;
AVStream *vstream;
AVStream *astream;
int ret, fps, chunk_header;


vstream = avformat_new_stream(s, NULL);
if (!vstream)
return AVERROR(ENOMEM);


astream = avformat_new_stream(s, NULL);
if (!astream)
return AVERROR(ENOMEM);


avio_skip(pb, 8);
chunk_header = avio_rl32(pb);
bfi->nframes = avio_rl32(pb);
avio_rl32(pb);
avio_rl32(pb);
avio_rl32(pb);
fps = avio_rl32(pb);
avio_skip(pb, 12);
vstream->codecpar->width = avio_rl32(pb);
vstream->codecpar->height = avio_rl32(pb);


avio_skip(pb, 8);
ret = ff_get_extradata(s, vstream->codecpar, pb, 768);
if (ret < 0)
return ret;

astream->codecpar->sample_rate = avio_rl32(pb);
if (astream->codecpar->sample_rate <= 0) {
av_log(s, AV_LOG_ERROR, "Invalid sample rate %d\n", astream->codecpar->sample_rate);
return AVERROR_INVALIDDATA;
}


avpriv_set_pts_info(vstream, 32, 1, fps);
vstream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
vstream->codecpar->codec_id = AV_CODEC_ID_BFI;
vstream->codecpar->format = AV_PIX_FMT_PAL8;
vstream->nb_frames =
vstream->duration = bfi->nframes;


astream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
astream->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
astream->codecpar->channels = 1;
astream->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
astream->codecpar->bits_per_coded_sample = 8;
astream->codecpar->bit_rate =
(int64_t)astream->codecpar->sample_rate * astream->codecpar->bits_per_coded_sample;
avio_seek(pb, chunk_header - 3, SEEK_SET);
avpriv_set_pts_info(astream, 64, 1, astream->codecpar->sample_rate);
return 0;
}


static int bfi_read_packet(AVFormatContext * s, AVPacket * pkt)
{
BFIContext *bfi = s->priv_data;
AVIOContext *pb = s->pb;
int ret, audio_offset, video_offset, chunk_size, audio_size = 0;
if (bfi->nframes == 0 || avio_feof(pb)) {
return AVERROR_EOF;
}


if (!bfi->avflag) {
uint32_t state = 0;
while(state != MKTAG('S','A','V','I')){
if (avio_feof(pb))
return AVERROR(EIO);
state = 256*state + avio_r8(pb);
}

chunk_size = avio_rl32(pb);
avio_rl32(pb);
audio_offset = avio_rl32(pb);
avio_rl32(pb);
video_offset = avio_rl32(pb);
audio_size = video_offset - audio_offset;
bfi->video_size = chunk_size - video_offset;
if (audio_size < 0 || bfi->video_size < 0) {
av_log(s, AV_LOG_ERROR, "Invalid audio/video offsets or chunk size\n");
return AVERROR_INVALIDDATA;
}


ret = av_get_packet(pb, pkt, audio_size);
if (ret < 0)
return ret;

pkt->pts = bfi->audio_frame;
bfi->audio_frame += ret;
} else if (bfi->video_size > 0) {


ret = av_get_packet(pb, pkt, bfi->video_size);
if (ret < 0)
return ret;

pkt->pts = bfi->video_frame;
bfi->video_frame += ret / bfi->video_size;


bfi->nframes--;
} else {

ret = AVERROR(EAGAIN);
}

bfi->avflag = !bfi->avflag;
pkt->stream_index = bfi->avflag;
return ret;
}

AVInputFormat ff_bfi_demuxer = {
.name = "bfi",
.long_name = NULL_IF_CONFIG_SMALL("Brute Force & Ignorance"),
.priv_data_size = sizeof(BFIContext),
.read_probe = bfi_probe,
.read_header = bfi_read_header,
.read_packet = bfi_read_packet,
};
