




















#include "libavutil/channel_layout.h"

#include "libavcodec/flac.h"

#include "avformat.h"
#include "flacenc.h"

int ff_flac_write_header(AVIOContext *pb, const uint8_t *extradata,
int extradata_size, int last_block)
{
uint8_t header[8] = {
0x66, 0x4C, 0x61, 0x43, 0x00, 0x00, 0x00, 0x22
};

header[4] = last_block ? 0x80 : 0x00;

if (extradata_size < FLAC_STREAMINFO_SIZE)
return AVERROR_INVALIDDATA;


avio_write(pb, header, 8);


avio_write(pb, extradata, FLAC_STREAMINFO_SIZE);

return 0;
}

int ff_flac_is_native_layout(uint64_t channel_layout)
{
if (channel_layout == AV_CH_LAYOUT_MONO ||
channel_layout == AV_CH_LAYOUT_STEREO ||
channel_layout == AV_CH_LAYOUT_SURROUND ||
channel_layout == AV_CH_LAYOUT_QUAD ||
channel_layout == AV_CH_LAYOUT_5POINT0 ||
channel_layout == AV_CH_LAYOUT_5POINT1 ||
channel_layout == AV_CH_LAYOUT_6POINT1 ||
channel_layout == AV_CH_LAYOUT_7POINT1)
return 1;
return 0;
}
