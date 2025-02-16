#include "libavcodec/get_bits.h"
#include "libavcodec/put_bits.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/mpeg4audio.h"
#include "libavutil/opt.h"
#include "avformat.h"
#include "internal.h"
#include "rawenc.h"
#define MAX_EXTRADATA_SIZE 1024
typedef struct LATMContext {
AVClass *av_class;
int off;
int channel_conf;
int object_type;
int counter;
int mod;
uint8_t buffer[0x1fff + MAX_EXTRADATA_SIZE + 1024];
} LATMContext;
static const AVOption options[] = {
{"smc-interval", "StreamMuxConfig interval.",
offsetof(LATMContext, mod), AV_OPT_TYPE_INT, {.i64 = 0x0014}, 0x0001, 0xffff, AV_OPT_FLAG_ENCODING_PARAM},
{NULL},
};
static const AVClass latm_muxer_class = {
.class_name = "LATM/LOAS muxer",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
static int latm_decode_extradata(AVFormatContext *s, uint8_t *buf, int size)
{
LATMContext *ctx = s->priv_data;
MPEG4AudioConfig m4ac;
if (size > MAX_EXTRADATA_SIZE) {
av_log(s, AV_LOG_ERROR, "Extradata is larger than currently supported.\n");
return AVERROR_INVALIDDATA;
}
ctx->off = avpriv_mpeg4audio_get_config2(&m4ac, buf, size, 1, s);
if (ctx->off < 0)
return ctx->off;
if (ctx->object_type == AOT_ALS && (ctx->off & 7)) {
av_log(s, AV_LOG_ERROR, "BUG: ALS offset is not byte-aligned\n");
return AVERROR_INVALIDDATA;
}
if (m4ac.object_type > AOT_SBR && m4ac.object_type != AOT_ALS) {
av_log(s, AV_LOG_ERROR, "Muxing MPEG-4 AOT %d in LATM is not supported\n", m4ac.object_type);
return AVERROR_INVALIDDATA;
}
ctx->channel_conf = m4ac.chan_config;
ctx->object_type = m4ac.object_type;
return 0;
}
static int latm_write_header(AVFormatContext *s)
{
AVCodecParameters *par = s->streams[0]->codecpar;
if (par->codec_id == AV_CODEC_ID_AAC_LATM)
return 0;
if (par->codec_id != AV_CODEC_ID_AAC && par->codec_id != AV_CODEC_ID_MP4ALS) {
av_log(s, AV_LOG_ERROR, "Only AAC, LATM and ALS are supported\n");
return AVERROR(EINVAL);
}
if (par->extradata_size > 0 &&
latm_decode_extradata(s, par->extradata, par->extradata_size) < 0)
return AVERROR_INVALIDDATA;
return 0;
}
static void latm_write_frame_header(AVFormatContext *s, PutBitContext *bs)
{
LATMContext *ctx = s->priv_data;
AVCodecParameters *par = s->streams[0]->codecpar;
int header_size;
put_bits(bs, 1, !!ctx->counter);
if (!ctx->counter) {
put_bits(bs, 1, 0); 
put_bits(bs, 1, 1); 
put_bits(bs, 6, 0); 
put_bits(bs, 4, 0); 
put_bits(bs, 3, 0); 
if (ctx->object_type == AOT_ALS) {
header_size = par->extradata_size-(ctx->off >> 3);
avpriv_copy_bits(bs, &par->extradata[ctx->off >> 3], header_size);
} else {
avpriv_copy_bits(bs, par->extradata, ctx->off + 3);
if (!ctx->channel_conf) {
GetBitContext gb;
int ret = init_get_bits8(&gb, par->extradata, par->extradata_size);
av_assert0(ret >= 0); 
skip_bits_long(&gb, ctx->off + 3);
ff_copy_pce_data(bs, &gb);
}
}
put_bits(bs, 3, 0); 
put_bits(bs, 8, 0xff); 
put_bits(bs, 1, 0); 
put_bits(bs, 1, 0); 
}
ctx->counter++;
ctx->counter %= ctx->mod;
}
static int latm_write_packet(AVFormatContext *s, AVPacket *pkt)
{
LATMContext *ctx = s->priv_data;
AVCodecParameters *par = s->streams[0]->codecpar;
AVIOContext *pb = s->pb;
PutBitContext bs;
int i, len;
uint8_t loas_header[] = "\x56\xe0\x00";
if (par->codec_id == AV_CODEC_ID_AAC_LATM)
return ff_raw_write_packet(s, pkt);
if (!par->extradata) {
if(pkt->size > 2 && pkt->data[0] == 0x56 && (pkt->data[1] >> 4) == 0xe &&
(AV_RB16(pkt->data + 1) & 0x1FFF) + 3 == pkt->size)
return ff_raw_write_packet(s, pkt);
else {
uint8_t *side_data;
int side_data_size = 0, ret;
side_data = av_packet_get_side_data(pkt, AV_PKT_DATA_NEW_EXTRADATA,
&side_data_size);
if (side_data_size) {
if (latm_decode_extradata(s, side_data, side_data_size) < 0)
return AVERROR_INVALIDDATA;
ret = ff_alloc_extradata(par, side_data_size);
if (ret < 0)
return ret;
memcpy(par->extradata, side_data, side_data_size);
} else
return AVERROR_INVALIDDATA;
}
}
if (pkt->size > 0x1fff)
goto too_large;
init_put_bits(&bs, ctx->buffer, pkt->size+1024+MAX_EXTRADATA_SIZE);
latm_write_frame_header(s, &bs);
for (i = 0; i <= pkt->size-255; i+=255)
put_bits(&bs, 8, 255);
put_bits(&bs, 8, pkt->size-i);
if (pkt->size && (pkt->data[0] & 0xe1) == 0x81) {
put_bits(&bs, 8, pkt->data[0] & 0xfe);
avpriv_copy_bits(&bs, pkt->data + 1, 8*pkt->size - 8);
} else
avpriv_copy_bits(&bs, pkt->data, 8*pkt->size);
avpriv_align_put_bits(&bs);
flush_put_bits(&bs);
len = put_bits_count(&bs) >> 3;
if (len > 0x1fff)
goto too_large;
loas_header[1] |= (len >> 8) & 0x1f;
loas_header[2] |= len & 0xff;
avio_write(pb, loas_header, 3);
avio_write(pb, ctx->buffer, len);
return 0;
too_large:
av_log(s, AV_LOG_ERROR, "LATM packet size larger than maximum size 0x1fff\n");
return AVERROR_INVALIDDATA;
}
static int latm_check_bitstream(struct AVFormatContext *s, const AVPacket *pkt)
{
int ret = 1;
AVStream *st = s->streams[pkt->stream_index];
if (st->codecpar->codec_id == AV_CODEC_ID_AAC) {
if (pkt->size > 2 && (AV_RB16(pkt->data) & 0xfff0) == 0xfff0)
ret = ff_stream_add_bitstream_filter(st, "aac_adtstoasc", NULL);
}
return ret;
}
AVOutputFormat ff_latm_muxer = {
.name = "latm",
.long_name = NULL_IF_CONFIG_SMALL("LOAS/LATM"),
.mime_type = "audio/MP4A-LATM",
.extensions = "latm,loas",
.priv_data_size = sizeof(LATMContext),
.audio_codec = AV_CODEC_ID_AAC,
.video_codec = AV_CODEC_ID_NONE,
.write_header = latm_write_header,
.write_packet = latm_write_packet,
.priv_class = &latm_muxer_class,
.check_bitstream= latm_check_bitstream,
.flags = AVFMT_NOTIMESTAMPS,
};
