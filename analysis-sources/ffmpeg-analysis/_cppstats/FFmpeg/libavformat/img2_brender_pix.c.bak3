




















#include "img2.h"
#include "libavutil/intreadwrite.h"

static int brender_read_probe(const AVProbeData *p)
{
static const uint8_t brender_magic[16] = {
0,0,0,0x12,0,0,0,8,0,0,0,2,0,0,0,2
};

if (memcmp(p->buf, brender_magic, sizeof(brender_magic)))
return 0;

if (AV_RB32(p->buf+16) != 0x03 &&
AV_RB32(p->buf+16) != 0x3D)
return 0;

return AVPROBE_SCORE_MAX-10;
}

static const AVClass image2_brender_pix_class = {
.class_name = "brender_pix demuxer",
.item_name = av_default_item_name,
.option = ff_img_options,
.version = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_image2_brender_pix_demuxer = {
.name = "brender_pix",
.long_name = NULL_IF_CONFIG_SMALL("BRender PIX image"),
.priv_data_size = sizeof(VideoDemuxData),
.read_probe = brender_read_probe,
.read_header = ff_img_read_header,
.read_packet = ff_img_read_packet,
.raw_codec_id = AV_CODEC_ID_BRENDER_PIX,
.priv_class = &image2_brender_pix_class,
};
