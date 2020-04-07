#include "voc.h"
#include "internal.h"
typedef struct voc_enc_context {
int param_written;
} VocEncContext;
static int voc_write_header(AVFormatContext *s)
{
AVIOContext *pb = s->pb;
AVCodecParameters *par = s->streams[0]->codecpar;
const int header_size = 26;
const int version = 0x0114;
if (s->nb_streams != 1
|| s->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
return AVERROR_PATCHWELCOME;
if (!par->codec_tag && par->codec_id != AV_CODEC_ID_PCM_U8) {
av_log(s, AV_LOG_ERROR, "unsupported codec\n");
return AVERROR(EINVAL);
}
avio_write(pb, ff_voc_magic, sizeof(ff_voc_magic) - 1);
avio_wl16(pb, header_size);
avio_wl16(pb, version);
avio_wl16(pb, ~version + 0x1234);
return 0;
}
static int voc_write_packet(AVFormatContext *s, AVPacket *pkt)
{
VocEncContext *voc = s->priv_data;
AVCodecParameters *par = s->streams[0]->codecpar;
AVIOContext *pb = s->pb;
if (!voc->param_written) {
if (par->codec_tag > 3) {
avio_w8(pb, VOC_TYPE_NEW_VOICE_DATA);
avio_wl24(pb, pkt->size + 12);
avio_wl32(pb, par->sample_rate);
avio_w8(pb, par->bits_per_coded_sample);
avio_w8(pb, par->channels);
avio_wl16(pb, par->codec_tag);
avio_wl32(pb, 0);
} else {
if (s->streams[0]->codecpar->channels > 1) {
avio_w8(pb, VOC_TYPE_EXTENDED);
avio_wl24(pb, 4);
avio_wl16(pb, 65536-(256000000 + par->sample_rate*par->channels/2)/(par->sample_rate*par->channels));
avio_w8(pb, par->codec_tag);
avio_w8(pb, par->channels - 1);
}
avio_w8(pb, VOC_TYPE_VOICE_DATA);
avio_wl24(pb, pkt->size + 2);
avio_w8(pb, 256 - (1000000 + par->sample_rate/2) / par->sample_rate);
avio_w8(pb, par->codec_tag);
}
voc->param_written = 1;
} else {
avio_w8(pb, VOC_TYPE_VOICE_DATA_CONT);
avio_wl24(pb, pkt->size);
}
avio_write(pb, pkt->data, pkt->size);
return 0;
}
static int voc_write_trailer(AVFormatContext *s)
{
avio_w8(s->pb, 0);
return 0;
}
AVOutputFormat ff_voc_muxer = {
.name = "voc",
.long_name = NULL_IF_CONFIG_SMALL("Creative Voice"),
.mime_type = "audio/x-voc",
.extensions = "voc",
.priv_data_size = sizeof(VocEncContext),
.audio_codec = AV_CODEC_ID_PCM_S16LE,
.video_codec = AV_CODEC_ID_NONE,
.write_header = voc_write_header,
.write_packet = voc_write_packet,
.write_trailer = voc_write_trailer,
.codec_tag = (const AVCodecTag* const []){ ff_voc_codec_tags, 0 },
.flags = AVFMT_NOTIMESTAMPS,
};
