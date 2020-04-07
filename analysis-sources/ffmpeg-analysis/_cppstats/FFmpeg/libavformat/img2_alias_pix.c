#include "img2.h"
#include "libavcodec/bytestream.h"
static int alias_pix_read_probe(const AVProbeData *p)
{
const uint8_t *b = p->buf;
const uint8_t *end = b + p->buf_size;
int width = bytestream_get_be16(&b);
int height = bytestream_get_be16(&b);
av_unused int ox = bytestream_get_be16(&b);
av_unused int oy = bytestream_get_be16(&b);
int bpp = bytestream_get_be16(&b);
int x, y;
if (!width || !height)
return 0;
if (bpp != 24 && bpp != 8)
return 0;
for (y=0; y<2 && y<height; y++) {
for (x=0; x<width; ) {
int count = *b++;
if (count == 0 || x + count > width)
return 0;
if (b > end)
return AVPROBE_SCORE_MAX / 8;
b += bpp / 8;
x += count;
}
}
return AVPROBE_SCORE_EXTENSION + 1;
}
static const AVClass image2_alias_pix_class = {
.class_name = "alias_pix demuxer",
.item_name = av_default_item_name,
.option = ff_img_options,
.version = LIBAVUTIL_VERSION_INT,
};
AVInputFormat ff_image2_alias_pix_demuxer = {
.name = "alias_pix",
.long_name = NULL_IF_CONFIG_SMALL("Alias/Wavefront PIX image"),
.priv_data_size = sizeof(VideoDemuxData),
.read_probe = alias_pix_read_probe,
.read_header = ff_img_read_header,
.read_packet = ff_img_read_packet,
.raw_codec_id = AV_CODEC_ID_ALIAS_PIX,
.priv_class = &image2_alias_pix_class,
};
