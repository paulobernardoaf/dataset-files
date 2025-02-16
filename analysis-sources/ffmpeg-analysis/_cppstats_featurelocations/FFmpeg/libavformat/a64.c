




















#include "libavcodec/avcodec.h"
#include "libavcodec/bytestream.h"
#include "avformat.h"
#include "rawenc.h"

static int a64_write_header(AVFormatContext *s)
{
AVCodecParameters *par = s->streams[0]->codecpar;
uint8_t header[5] = {
0x00, 
0x40, 
0x00, 
0x00, 
0x00 
};

if (par->extradata_size < 4) {
av_log(s, AV_LOG_ERROR, "Missing extradata\n");
return AVERROR_INVALIDDATA;
}

switch (par->codec_id) {
case AV_CODEC_ID_A64_MULTI:
header[2] = 0x00;
header[3] = AV_RB32(par->extradata+0);
header[4] = 2;
break;
case AV_CODEC_ID_A64_MULTI5:
header[2] = 0x01;
header[3] = AV_RB32(par->extradata+0);
header[4] = 3;
break;
default:
return AVERROR_INVALIDDATA;
}
avio_write(s->pb, header, 2);
return 0;
}

AVOutputFormat ff_a64_muxer = {
.name = "a64",
.long_name = NULL_IF_CONFIG_SMALL("a64 - video for Commodore 64"),
.extensions = "a64, A64",
.video_codec = AV_CODEC_ID_A64_MULTI,
.write_header = a64_write_header,
.write_packet = ff_raw_write_packet,
};
