#include <stdint.h>
#include "snappy-c.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "bytestream.h"
#include "hap.h"
#include "internal.h"
#include "texturedsp.h"
#define HAP_MAX_CHUNKS 64
enum HapHeaderLength {
HAP_HDR_SHORT = 4,
HAP_HDR_LONG = 8,
};
static int compress_texture(AVCodecContext *avctx, uint8_t *out, int out_length, const AVFrame *f)
{
HapContext *ctx = avctx->priv_data;
int i, j;
if (ctx->tex_size > out_length)
return AVERROR_BUFFER_TOO_SMALL;
for (j = 0; j < avctx->height; j += 4) {
for (i = 0; i < avctx->width; i += 4) {
uint8_t *p = f->data[0] + i * 4 + j * f->linesize[0];
const int step = ctx->tex_fun(out, f->linesize[0], p);
out += step;
}
}
return 0;
}
static void hap_write_section_header(PutByteContext *pbc,
enum HapHeaderLength header_length,
int section_length,
enum HapSectionType section_type)
{
bytestream2_put_le24(pbc, header_length == HAP_HDR_LONG ? 0 : section_length);
bytestream2_put_byte(pbc, section_type);
if (header_length == HAP_HDR_LONG) {
bytestream2_put_le32(pbc, section_length);
}
}
static int hap_compress_frame(AVCodecContext *avctx, uint8_t *dst)
{
HapContext *ctx = avctx->priv_data;
int i, final_size = 0;
for (i = 0; i < ctx->chunk_count; i++) {
HapChunk *chunk = &ctx->chunks[i];
uint8_t *chunk_src, *chunk_dst;
int ret;
if (i == 0) {
chunk->compressed_offset = 0;
} else {
chunk->compressed_offset = ctx->chunks[i-1].compressed_offset
+ ctx->chunks[i-1].compressed_size;
}
chunk->uncompressed_size = ctx->tex_size / ctx->chunk_count;
chunk->uncompressed_offset = i * chunk->uncompressed_size;
chunk->compressed_size = ctx->max_snappy;
chunk_src = ctx->tex_buf + chunk->uncompressed_offset;
chunk_dst = dst + chunk->compressed_offset;
ret = snappy_compress(chunk_src, chunk->uncompressed_size,
chunk_dst, &chunk->compressed_size);
if (ret != SNAPPY_OK) {
av_log(avctx, AV_LOG_ERROR, "Snappy compress error.\n");
return AVERROR_BUG;
}
if (chunk->compressed_size >= chunk->uncompressed_size) {
av_log(avctx, AV_LOG_VERBOSE,
"Snappy buffer bigger than uncompressed (%"SIZE_SPECIFIER" >= %"SIZE_SPECIFIER" bytes).\n",
chunk->compressed_size, chunk->uncompressed_size);
memcpy(chunk_dst, chunk_src, chunk->uncompressed_size);
chunk->compressor = HAP_COMP_NONE;
chunk->compressed_size = chunk->uncompressed_size;
} else {
chunk->compressor = HAP_COMP_SNAPPY;
}
final_size += chunk->compressed_size;
}
return final_size;
}
static int hap_decode_instructions_length(HapContext *ctx)
{
return (5 * ctx->chunk_count) + 8;
}
static int hap_header_length(HapContext *ctx)
{
int length = HAP_HDR_LONG;
if (ctx->chunk_count > 1) {
length += HAP_HDR_SHORT + hap_decode_instructions_length(ctx);
}
return length;
}
static void hap_write_frame_header(HapContext *ctx, uint8_t *dst, int frame_length)
{
PutByteContext pbc;
int i;
bytestream2_init_writer(&pbc, dst, frame_length);
if (ctx->chunk_count == 1) {
hap_write_section_header(&pbc, HAP_HDR_LONG, frame_length - 8,
ctx->chunks[0].compressor | ctx->opt_tex_fmt);
} else {
hap_write_section_header(&pbc, HAP_HDR_LONG, frame_length - 8,
HAP_COMP_COMPLEX | ctx->opt_tex_fmt);
hap_write_section_header(&pbc, HAP_HDR_SHORT, hap_decode_instructions_length(ctx),
HAP_ST_DECODE_INSTRUCTIONS);
hap_write_section_header(&pbc, HAP_HDR_SHORT, ctx->chunk_count,
HAP_ST_COMPRESSOR_TABLE);
for (i = 0; i < ctx->chunk_count; i++) {
bytestream2_put_byte(&pbc, ctx->chunks[i].compressor >> 4);
}
hap_write_section_header(&pbc, HAP_HDR_SHORT, ctx->chunk_count * 4,
HAP_ST_SIZE_TABLE);
for (i = 0; i < ctx->chunk_count; i++) {
bytestream2_put_le32(&pbc, ctx->chunks[i].compressed_size);
}
}
}
static int hap_encode(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *frame, int *got_packet)
{
HapContext *ctx = avctx->priv_data;
int header_length = hap_header_length(ctx);
int final_data_size, ret;
int pktsize = FFMAX(ctx->tex_size, ctx->max_snappy * ctx->chunk_count) + header_length;
ret = ff_alloc_packet2(avctx, pkt, pktsize, header_length);
if (ret < 0)
return ret;
if (ctx->opt_compressor == HAP_COMP_NONE) {
ret = compress_texture(avctx, pkt->data + header_length, pkt->size - header_length, frame);
if (ret < 0)
return ret;
ctx->chunks[0].compressor = HAP_COMP_NONE;
final_data_size = ctx->tex_size;
} else {
ret = compress_texture(avctx, ctx->tex_buf, ctx->tex_size, frame);
if (ret < 0)
return ret;
final_data_size = hap_compress_frame(avctx, pkt->data + header_length);
if (final_data_size < 0)
return final_data_size;
}
hap_write_frame_header(ctx, pkt->data, final_data_size + header_length);
av_shrink_packet(pkt, final_data_size + header_length);
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}
static av_cold int hap_init(AVCodecContext *avctx)
{
HapContext *ctx = avctx->priv_data;
int ratio;
int corrected_chunk_count;
int ret = av_image_check_size(avctx->width, avctx->height, 0, avctx);
if (ret < 0) {
av_log(avctx, AV_LOG_ERROR, "Invalid video size %dx%d.\n",
avctx->width, avctx->height);
return ret;
}
if (avctx->width % 4 || avctx->height % 4) {
av_log(avctx, AV_LOG_ERROR, "Video size %dx%d is not multiple of 4.\n",
avctx->width, avctx->height);
return AVERROR_INVALIDDATA;
}
ff_texturedspenc_init(&ctx->dxtc);
switch (ctx->opt_tex_fmt) {
case HAP_FMT_RGBDXT1:
ratio = 8;
avctx->codec_tag = MKTAG('H', 'a', 'p', '1');
avctx->bits_per_coded_sample = 24;
ctx->tex_fun = ctx->dxtc.dxt1_block;
break;
case HAP_FMT_RGBADXT5:
ratio = 4;
avctx->codec_tag = MKTAG('H', 'a', 'p', '5');
avctx->bits_per_coded_sample = 32;
ctx->tex_fun = ctx->dxtc.dxt5_block;
break;
case HAP_FMT_YCOCGDXT5:
ratio = 4;
avctx->codec_tag = MKTAG('H', 'a', 'p', 'Y');
avctx->bits_per_coded_sample = 24;
ctx->tex_fun = ctx->dxtc.dxt5ys_block;
break;
default:
av_log(avctx, AV_LOG_ERROR, "Invalid format %02X\n", ctx->opt_tex_fmt);
return AVERROR_INVALIDDATA;
}
ctx->tex_size = FFALIGN(avctx->width, TEXTURE_BLOCK_W) *
FFALIGN(avctx->height, TEXTURE_BLOCK_H) * 4 / ratio;
switch (ctx->opt_compressor) {
case HAP_COMP_NONE:
corrected_chunk_count = 1;
ctx->max_snappy = ctx->tex_size;
ctx->tex_buf = NULL;
break;
case HAP_COMP_SNAPPY:
corrected_chunk_count = av_clip(ctx->opt_chunk_count, 1, HAP_MAX_CHUNKS);
while ((ctx->tex_size / (64 / ratio)) % corrected_chunk_count != 0) {
corrected_chunk_count--;
}
ctx->max_snappy = snappy_max_compressed_length(ctx->tex_size / corrected_chunk_count);
ctx->tex_buf = av_malloc(ctx->tex_size);
if (!ctx->tex_buf) {
return AVERROR(ENOMEM);
}
break;
default:
av_log(avctx, AV_LOG_ERROR, "Invalid compresor %02X\n", ctx->opt_compressor);
return AVERROR_INVALIDDATA;
}
if (corrected_chunk_count != ctx->opt_chunk_count) {
av_log(avctx, AV_LOG_INFO, "%d chunks requested but %d used.\n",
ctx->opt_chunk_count, corrected_chunk_count);
}
ret = ff_hap_set_chunk_count(ctx, corrected_chunk_count, 1);
if (ret != 0)
return ret;
return 0;
}
static av_cold int hap_close(AVCodecContext *avctx)
{
HapContext *ctx = avctx->priv_data;
ff_hap_free_context(ctx);
return 0;
}
#define OFFSET(x) offsetof(HapContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "format", NULL, OFFSET(opt_tex_fmt), AV_OPT_TYPE_INT, { .i64 = HAP_FMT_RGBDXT1 }, HAP_FMT_RGBDXT1, HAP_FMT_YCOCGDXT5, FLAGS, "format" },
{ "hap", "Hap 1 (DXT1 textures)", 0, AV_OPT_TYPE_CONST, { .i64 = HAP_FMT_RGBDXT1 }, 0, 0, FLAGS, "format" },
{ "hap_alpha", "Hap Alpha (DXT5 textures)", 0, AV_OPT_TYPE_CONST, { .i64 = HAP_FMT_RGBADXT5 }, 0, 0, FLAGS, "format" },
{ "hap_q", "Hap Q (DXT5-YCoCg textures)", 0, AV_OPT_TYPE_CONST, { .i64 = HAP_FMT_YCOCGDXT5 }, 0, 0, FLAGS, "format" },
{ "chunks", "chunk count", OFFSET(opt_chunk_count), AV_OPT_TYPE_INT, {.i64 = 1 }, 1, HAP_MAX_CHUNKS, FLAGS, },
{ "compressor", "second-stage compressor", OFFSET(opt_compressor), AV_OPT_TYPE_INT, { .i64 = HAP_COMP_SNAPPY }, HAP_COMP_NONE, HAP_COMP_SNAPPY, FLAGS, "compressor" },
{ "none", "None", 0, AV_OPT_TYPE_CONST, { .i64 = HAP_COMP_NONE }, 0, 0, FLAGS, "compressor" },
{ "snappy", "Snappy", 0, AV_OPT_TYPE_CONST, { .i64 = HAP_COMP_SNAPPY }, 0, 0, FLAGS, "compressor" },
{ NULL },
};
static const AVClass hapenc_class = {
.class_name = "Hap encoder",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
AVCodec ff_hap_encoder = {
.name = "hap",
.long_name = NULL_IF_CONFIG_SMALL("Vidvox Hap"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_HAP,
.priv_data_size = sizeof(HapContext),
.priv_class = &hapenc_class,
.init = hap_init,
.encode2 = hap_encode,
.close = hap_close,
.pix_fmts = (const enum AVPixelFormat[]) {
AV_PIX_FMT_RGBA, AV_PIX_FMT_NONE,
},
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE |
FF_CODEC_CAP_INIT_CLEANUP,
};
