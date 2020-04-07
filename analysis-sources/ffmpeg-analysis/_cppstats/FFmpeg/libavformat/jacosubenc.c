#include "avformat.h"
#include "rawenc.h"
static int jacosub_write_header(AVFormatContext *s)
{
const AVCodecParameters *par = s->streams[0]->codecpar;
if (par->extradata_size) {
avio_write(s->pb, par->extradata, par->extradata_size - 1);
}
return 0;
}
AVOutputFormat ff_jacosub_muxer = {
.name = "jacosub",
.long_name = NULL_IF_CONFIG_SMALL("JACOsub subtitle format"),
.mime_type = "text/x-jacosub",
.extensions = "jss,js",
.write_header = jacosub_write_header,
.write_packet = ff_raw_write_packet,
.flags = AVFMT_TS_NONSTRICT,
.subtitle_codec = AV_CODEC_ID_JACOSUB,
};
