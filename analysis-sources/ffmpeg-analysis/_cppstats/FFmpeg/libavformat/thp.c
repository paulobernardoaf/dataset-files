#include "libavutil/intreadwrite.h"
#include "libavutil/intfloat.h"
#include "avformat.h"
#include "internal.h"
typedef struct ThpDemuxContext {
int version;
unsigned first_frame;
unsigned first_framesz;
unsigned last_frame;
int compoff;
unsigned framecnt;
AVRational fps;
unsigned frame;
int64_t next_frame;
unsigned next_framesz;
int video_stream_index;
int audio_stream_index;
int compcount;
unsigned char components[16];
AVStream* vst;
int has_audio;
unsigned audiosize;
} ThpDemuxContext;
static int thp_probe(const AVProbeData *p)
{
double d;
if (AV_RL32(p->buf) != MKTAG('T', 'H', 'P', '\0'))
return 0;
d = av_int2float(AV_RB32(p->buf + 16));
if (d < 0.1 || d > 1000 || isnan(d))
return AVPROBE_SCORE_MAX/4;
return AVPROBE_SCORE_MAX;
}
static int thp_read_header(AVFormatContext *s)
{
ThpDemuxContext *thp = s->priv_data;
AVStream *st;
AVIOContext *pb = s->pb;
int64_t fsize= avio_size(pb);
int i;
avio_rb32(pb); 
thp->version = avio_rb32(pb);
avio_rb32(pb); 
avio_rb32(pb); 
thp->fps = av_d2q(av_int2float(avio_rb32(pb)), INT_MAX);
thp->framecnt = avio_rb32(pb);
thp->first_framesz = avio_rb32(pb);
pb->maxsize = avio_rb32(pb);
if(fsize>0 && (!pb->maxsize || fsize < pb->maxsize))
pb->maxsize= fsize;
thp->compoff = avio_rb32(pb);
avio_rb32(pb); 
thp->first_frame = avio_rb32(pb);
thp->last_frame = avio_rb32(pb);
thp->next_framesz = thp->first_framesz;
thp->next_frame = thp->first_frame;
avio_seek (pb, thp->compoff, SEEK_SET);
thp->compcount = avio_rb32(pb);
avio_read(pb, thp->components, 16);
for (i = 0; i < thp->compcount; i++) {
if (thp->components[i] == 0) {
if (thp->vst)
break;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
avpriv_set_pts_info(st, 64, thp->fps.den, thp->fps.num);
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = AV_CODEC_ID_THP;
st->codecpar->codec_tag = 0; 
st->codecpar->width = avio_rb32(pb);
st->codecpar->height = avio_rb32(pb);
st->codecpar->sample_rate = av_q2d(thp->fps);
st->nb_frames =
st->duration = thp->framecnt;
thp->vst = st;
thp->video_stream_index = st->index;
if (thp->version == 0x11000)
avio_rb32(pb); 
} else if (thp->components[i] == 1) {
if (thp->has_audio != 0)
break;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_THP;
st->codecpar->codec_tag = 0; 
st->codecpar->channels = avio_rb32(pb); 
st->codecpar->sample_rate = avio_rb32(pb); 
st->duration = avio_rb32(pb);
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
thp->audio_stream_index = st->index;
thp->has_audio = 1;
}
}
return 0;
}
static int thp_read_packet(AVFormatContext *s,
AVPacket *pkt)
{
ThpDemuxContext *thp = s->priv_data;
AVIOContext *pb = s->pb;
unsigned int size;
int ret;
if (thp->audiosize == 0) {
if (thp->frame >= thp->framecnt)
return AVERROR_EOF;
avio_seek(pb, thp->next_frame, SEEK_SET);
thp->next_frame += FFMAX(thp->next_framesz, 1);
thp->next_framesz = avio_rb32(pb);
avio_rb32(pb); 
size = avio_rb32(pb); 
if (thp->has_audio)
thp->audiosize = avio_rb32(pb); 
else
thp->frame++;
ret = av_get_packet(pb, pkt, size);
if (ret < 0)
return ret;
if (ret != size) {
return AVERROR(EIO);
}
pkt->stream_index = thp->video_stream_index;
} else {
ret = av_get_packet(pb, pkt, thp->audiosize);
if (ret < 0)
return ret;
if (ret != thp->audiosize) {
return AVERROR(EIO);
}
pkt->stream_index = thp->audio_stream_index;
if (thp->audiosize >= 8)
pkt->duration = AV_RB32(&pkt->data[4]);
thp->audiosize = 0;
thp->frame++;
}
return 0;
}
AVInputFormat ff_thp_demuxer = {
.name = "thp",
.long_name = NULL_IF_CONFIG_SMALL("THP"),
.priv_data_size = sizeof(ThpDemuxContext),
.read_probe = thp_probe,
.read_header = thp_read_header,
.read_packet = thp_read_packet
};
