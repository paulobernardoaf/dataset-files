#include "avcodec.h"
#include "bsf.h"
#include "bytestream.h"
static int imx_dump_header(AVBSFContext *ctx, AVPacket *out)
{
static const uint8_t imx_header[16] = { 0x06,0x0e,0x2b,0x34,0x01,0x02,0x01,0x01,0x0d,0x01,0x03,0x01,0x05,0x01,0x01,0x00 };
AVPacket *in;
int ret = 0;
uint8_t *out_buf;
ret = ff_bsf_get_packet(ctx, &in);
if (ret < 0)
return ret;
ret = av_new_packet(out, in->size + 20);
if (ret < 0)
goto fail;
out_buf = out->data;
bytestream_put_buffer(&out_buf, imx_header, 16);
bytestream_put_byte(&out_buf, 0x83); 
bytestream_put_be24(&out_buf, in->size);
bytestream_put_buffer(&out_buf, in->data, in->size);
ret = av_packet_copy_props(out, in);
if (ret < 0)
goto fail;
fail:
if (ret < 0)
av_packet_unref(out);
av_packet_free(&in);
return ret;
}
static const enum AVCodecID codec_ids[] = {
AV_CODEC_ID_MPEG2VIDEO, AV_CODEC_ID_NONE,
};
const AVBitStreamFilter ff_imx_dump_header_bsf = {
.name = "imxdump",
.filter = imx_dump_header,
.codec_ids = codec_ids,
};
