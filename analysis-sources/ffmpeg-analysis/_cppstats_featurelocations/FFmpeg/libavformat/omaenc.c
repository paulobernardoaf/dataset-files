





















#include "avformat.h"
#include "avio_internal.h"
#include "id3v2.h"
#include "internal.h"
#include "oma.h"
#include "rawenc.h"

static av_cold int oma_write_header(AVFormatContext *s)
{
int i;
AVCodecParameters *par;
int srate_index;
int isjointstereo;

par = s->streams[0]->codecpar;


for (srate_index = 0; ; srate_index++) {
if (ff_oma_srate_tab[srate_index] == 0) {
av_log(s, AV_LOG_ERROR, "Sample rate %d not supported in OpenMG audio\n",
par->sample_rate);
return AVERROR(EINVAL);
}

if (ff_oma_srate_tab[srate_index] * 100 == par->sample_rate)
break;
}


ff_id3v2_write_simple(s, 3, ID3v2_EA3_MAGIC);

ffio_wfourcc(s->pb, "EA3\0");
avio_w8(s->pb, EA3_HEADER_SIZE >> 7);
avio_w8(s->pb, EA3_HEADER_SIZE & 0x7F);
avio_wl16(s->pb, 0xFFFF); 
for (i = 0; i < 6; i++)
avio_wl32(s->pb, 0); 

switch (par->codec_tag) {
case OMA_CODECID_ATRAC3:
if (par->channels != 2) {
av_log(s, AV_LOG_ERROR, "ATRAC3 in OMA is only supported with 2 channels\n");
return AVERROR(EINVAL);
}
if (par->extradata_size == 14) 
isjointstereo = par->extradata[6] != 0;
else if(par->extradata_size == 10) 
isjointstereo = par->extradata[8] == 0x12;
else {
av_log(s, AV_LOG_ERROR, "ATRAC3: Unsupported extradata size\n");
return AVERROR(EINVAL);
}
avio_wb32(s->pb, (OMA_CODECID_ATRAC3 << 24) |
(isjointstereo << 17) |
(srate_index << 13) |
(par->block_align/8));
break;
case OMA_CODECID_ATRAC3P:
avio_wb32(s->pb, (OMA_CODECID_ATRAC3P << 24) |
(srate_index << 13) |
(par->channels << 10) |
(par->block_align/8 - 1));
break;
default:
av_log(s, AV_LOG_ERROR, "unsupported codec tag %s for write\n",
av_fourcc2str(par->codec_tag));
return AVERROR(EINVAL);
}
for (i = 0; i < (EA3_HEADER_SIZE - 36)/4; i++)
avio_wl32(s->pb, 0); 

return 0;
}

AVOutputFormat ff_oma_muxer = {
.name = "oma",
.long_name = NULL_IF_CONFIG_SMALL("Sony OpenMG audio"),
.mime_type = "audio/x-oma",
.extensions = "oma",
.audio_codec = AV_CODEC_ID_ATRAC3,
.write_header = oma_write_header,
.write_packet = ff_raw_write_packet,
.codec_tag = (const AVCodecTag* const []){ff_oma_codec_tags, 0},
.flags = AVFMT_NOTIMESTAMPS,
};
