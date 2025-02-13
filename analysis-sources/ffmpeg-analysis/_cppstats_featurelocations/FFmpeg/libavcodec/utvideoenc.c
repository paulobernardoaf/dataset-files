

























#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"

#include "avcodec.h"
#include "internal.h"
#include "bswapdsp.h"
#include "bytestream.h"
#include "put_bits.h"
#include "mathops.h"
#include "utvideo.h"
#include "huffman.h"


static int huff_cmp_sym(const void *a, const void *b)
{
const HuffEntry *aa = a, *bb = b;
return aa->sym - bb->sym;
}

static av_cold int utvideo_encode_close(AVCodecContext *avctx)
{
UtvideoContext *c = avctx->priv_data;
int i;

av_freep(&c->slice_bits);
for (i = 0; i < 4; i++)
av_freep(&c->slice_buffer[i]);

return 0;
}

static av_cold int utvideo_encode_init(AVCodecContext *avctx)
{
UtvideoContext *c = avctx->priv_data;
int i, subsampled_height;
uint32_t original_format;

c->avctx = avctx;
c->frame_info_size = 4;
c->slice_stride = FFALIGN(avctx->width, 32);

switch (avctx->pix_fmt) {
case AV_PIX_FMT_GBRP:
c->planes = 3;
avctx->codec_tag = MKTAG('U', 'L', 'R', 'G');
original_format = UTVIDEO_RGB;
break;
case AV_PIX_FMT_GBRAP:
c->planes = 4;
avctx->codec_tag = MKTAG('U', 'L', 'R', 'A');
original_format = UTVIDEO_RGBA;
avctx->bits_per_coded_sample = 32;
break;
case AV_PIX_FMT_YUV420P:
if (avctx->width & 1 || avctx->height & 1) {
av_log(avctx, AV_LOG_ERROR,
"4:2:0 video requires even width and height.\n");
return AVERROR_INVALIDDATA;
}
c->planes = 3;
if (avctx->colorspace == AVCOL_SPC_BT709)
avctx->codec_tag = MKTAG('U', 'L', 'H', '0');
else
avctx->codec_tag = MKTAG('U', 'L', 'Y', '0');
original_format = UTVIDEO_420;
break;
case AV_PIX_FMT_YUV422P:
if (avctx->width & 1) {
av_log(avctx, AV_LOG_ERROR,
"4:2:2 video requires even width.\n");
return AVERROR_INVALIDDATA;
}
c->planes = 3;
if (avctx->colorspace == AVCOL_SPC_BT709)
avctx->codec_tag = MKTAG('U', 'L', 'H', '2');
else
avctx->codec_tag = MKTAG('U', 'L', 'Y', '2');
original_format = UTVIDEO_422;
break;
case AV_PIX_FMT_YUV444P:
c->planes = 3;
if (avctx->colorspace == AVCOL_SPC_BT709)
avctx->codec_tag = MKTAG('U', 'L', 'H', '4');
else
avctx->codec_tag = MKTAG('U', 'L', 'Y', '4');
original_format = UTVIDEO_444;
break;
default:
av_log(avctx, AV_LOG_ERROR, "Unknown pixel format: %d\n",
avctx->pix_fmt);
return AVERROR_INVALIDDATA;
}

ff_bswapdsp_init(&c->bdsp);
ff_llvidencdsp_init(&c->llvidencdsp);

#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS

if (avctx->prediction_method < 0 || avctx->prediction_method > 4) {
av_log(avctx, AV_LOG_WARNING,
"Prediction method %d is not supported in Ut Video.\n",
avctx->prediction_method);
return AVERROR_OPTION_NOT_FOUND;
}

if (avctx->prediction_method == FF_PRED_PLANE) {
av_log(avctx, AV_LOG_ERROR,
"Plane prediction is not supported in Ut Video.\n");
return AVERROR_OPTION_NOT_FOUND;
}


if (avctx->prediction_method)
c->frame_pred = ff_ut_pred_order[avctx->prediction_method];
FF_ENABLE_DEPRECATION_WARNINGS
#endif

if (c->frame_pred == PRED_GRADIENT) {
av_log(avctx, AV_LOG_ERROR, "Gradient prediction is not supported.\n");
return AVERROR_OPTION_NOT_FOUND;
}





if (avctx->slices > 256 || avctx->slices < 0) {
av_log(avctx, AV_LOG_ERROR,
"Slice count %d is not supported in Ut Video (theoretical range is 0-256).\n",
avctx->slices);
return AVERROR(EINVAL);
}


subsampled_height = avctx->height >> av_pix_fmt_desc_get(avctx->pix_fmt)->log2_chroma_h;
if (avctx->slices > subsampled_height) {
av_log(avctx, AV_LOG_ERROR,
"Slice count %d is larger than the subsampling-applied height %d.\n",
avctx->slices, subsampled_height);
return AVERROR(EINVAL);
}


avctx->extradata_size = 16;

avctx->extradata = av_mallocz(avctx->extradata_size +
AV_INPUT_BUFFER_PADDING_SIZE);

if (!avctx->extradata) {
av_log(avctx, AV_LOG_ERROR, "Could not allocate extradata.\n");
utvideo_encode_close(avctx);
return AVERROR(ENOMEM);
}

for (i = 0; i < c->planes; i++) {
c->slice_buffer[i] = av_malloc(c->slice_stride * (avctx->height + 2) +
AV_INPUT_BUFFER_PADDING_SIZE);
if (!c->slice_buffer[i]) {
av_log(avctx, AV_LOG_ERROR, "Cannot allocate temporary buffer 1.\n");
utvideo_encode_close(avctx);
return AVERROR(ENOMEM);
}
}







AV_WB32(avctx->extradata, MKTAG(1, 0, 0, 0xF0));





AV_WL32(avctx->extradata + 4, original_format);


AV_WL32(avctx->extradata + 8, c->frame_info_size);






if (!avctx->slices) {
c->slices = subsampled_height / 120;

if (!c->slices)
c->slices = 1;
else if (c->slices > 256)
c->slices = 256;
} else {
c->slices = avctx->slices;
}


c->compression = COMP_HUFF;








c->flags = (c->slices - 1) << 24;
c->flags |= 0 << 11; 
c->flags |= c->compression;

AV_WL32(avctx->extradata + 12, c->flags);

return 0;
}

static void mangle_rgb_planes(uint8_t *dst[4], ptrdiff_t dst_stride,
uint8_t *const src[4], int planes, const int stride[4],
int width, int height)
{
int i, j;
int k = 2 * dst_stride;
const uint8_t *sg = src[0];
const uint8_t *sb = src[1];
const uint8_t *sr = src[2];
const uint8_t *sa = src[3];
unsigned int g;

for (j = 0; j < height; j++) {
if (planes == 3) {
for (i = 0; i < width; i++) {
g = sg[i];
dst[0][k] = g;
g += 0x80;
dst[1][k] = sb[i] - g;
dst[2][k] = sr[i] - g;
k++;
}
} else {
for (i = 0; i < width; i++) {
g = sg[i];
dst[0][k] = g;
g += 0x80;
dst[1][k] = sb[i] - g;
dst[2][k] = sr[i] - g;
dst[3][k] = sa[i];
k++;
}
sa += stride[3];
}
k += dst_stride - width;
sg += stride[0];
sb += stride[1];
sr += stride[2];
}
}

#undef A
#undef B


static void median_predict(UtvideoContext *c, uint8_t *src, uint8_t *dst,
ptrdiff_t stride, int width, int height)
{
int i, j;
int A, B;
uint8_t prev;


prev = 0x80; 
for (i = 0; i < width; i++) {
*dst++ = src[i] - prev;
prev = src[i];
}

if (height == 1)
return;

src += stride;





A = B = 0;


for (j = 1; j < height; j++) {
c->llvidencdsp.sub_median_pred(dst, src - stride, src, width, &A, &B);
dst += width;
src += stride;
}
}


static void count_usage(uint8_t *src, int width,
int height, uint64_t *counts)
{
int i, j;

for (j = 0; j < height; j++) {
for (i = 0; i < width; i++) {
counts[src[i]]++;
}
src += width;
}
}


static void calculate_codes(HuffEntry *he)
{
int last, i;
uint32_t code;

qsort(he, 256, sizeof(*he), ff_ut_huff_cmp_len);

last = 255;
while (he[last].len == 255 && last)
last--;

code = 1;
for (i = last; i >= 0; i--) {
he[i].code = code >> (32 - he[i].len);
code += 0x80000000u >> (he[i].len - 1);
}

qsort(he, 256, sizeof(*he), huff_cmp_sym);
}


static int write_huff_codes(uint8_t *src, uint8_t *dst, int dst_size,
int width, int height, HuffEntry *he)
{
PutBitContext pb;
int i, j;
int count;

init_put_bits(&pb, dst, dst_size);


for (j = 0; j < height; j++) {
for (i = 0; i < width; i++)
put_bits(&pb, he[src[i]].len, he[src[i]].code);

src += width;
}


count = put_bits_count(&pb) & 0x1F;

if (count)
put_bits(&pb, 32 - count, 0);


count = put_bits_count(&pb);


flush_put_bits(&pb);

return count;
}

static int encode_plane(AVCodecContext *avctx, uint8_t *src,
uint8_t *dst, ptrdiff_t stride, int plane_no,
int width, int height, PutByteContext *pb)
{
UtvideoContext *c = avctx->priv_data;
uint8_t lengths[256];
uint64_t counts[256] = { 0 };

HuffEntry he[256];

uint32_t offset = 0, slice_len = 0;
const int cmask = ~(!plane_no && avctx->pix_fmt == AV_PIX_FMT_YUV420P);
int i, sstart, send = 0;
int symbol;
int ret;


switch (c->frame_pred) {
case PRED_NONE:
for (i = 0; i < c->slices; i++) {
sstart = send;
send = height * (i + 1) / c->slices & cmask;
av_image_copy_plane(dst + sstart * width, width,
src + sstart * stride, stride,
width, send - sstart);
}
break;
case PRED_LEFT:
for (i = 0; i < c->slices; i++) {
sstart = send;
send = height * (i + 1) / c->slices & cmask;
c->llvidencdsp.sub_left_predict(dst + sstart * width, src + sstart * stride, stride, width, send - sstart);
}
break;
case PRED_MEDIAN:
for (i = 0; i < c->slices; i++) {
sstart = send;
send = height * (i + 1) / c->slices & cmask;
median_predict(c, src + sstart * stride, dst + sstart * width,
stride, width, send - sstart);
}
break;
default:
av_log(avctx, AV_LOG_ERROR, "Unknown prediction mode: %d\n",
c->frame_pred);
return AVERROR_OPTION_NOT_FOUND;
}


count_usage(dst, width, height, counts);


for (symbol = 0; symbol < 256; symbol++) {

if (counts[symbol]) {

if (counts[symbol] == width * (int64_t)height) {




for (i = 0; i < 256; i++) {
if (i == symbol)
bytestream2_put_byte(pb, 0);
else
bytestream2_put_byte(pb, 0xFF);
}


for (i = 0; i < c->slices; i++)
bytestream2_put_le32(pb, 0);


return 0;
}
break;
}
}


if ((ret = ff_huff_gen_len_table(lengths, counts, 256, 1)) < 0)
return ret;






for (i = 0; i < 256; i++) {
bytestream2_put_byte(pb, lengths[i]);

he[i].len = lengths[i];
he[i].sym = i;
}


calculate_codes(he);

send = 0;
for (i = 0; i < c->slices; i++) {
sstart = send;
send = height * (i + 1) / c->slices & cmask;





offset += write_huff_codes(dst + sstart * width, c->slice_bits,
width * height + 4, width,
send - sstart, he) >> 3;

slice_len = offset - slice_len;


c->bdsp.bswap_buf((uint32_t *) c->slice_bits,
(uint32_t *) c->slice_bits,
slice_len >> 2);


bytestream2_put_le32(pb, offset);


bytestream2_seek_p(pb, 4 * (c->slices - i - 1) +
offset - slice_len, SEEK_CUR);


bytestream2_put_buffer(pb, c->slice_bits, slice_len);


bytestream2_seek_p(pb, -4 * (c->slices - i - 1) - offset,
SEEK_CUR);

slice_len = offset;
}


bytestream2_seek_p(pb, offset, SEEK_CUR);

return 0;
}

static int utvideo_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *pic, int *got_packet)
{
UtvideoContext *c = avctx->priv_data;
PutByteContext pb;

uint32_t frame_info;

uint8_t *dst;

int width = avctx->width, height = avctx->height;
int i, ret = 0;


ret = ff_alloc_packet2(avctx, pkt, (256 + 4 * c->slices + width * height) *
c->planes + 4, 0);

if (ret < 0)
return ret;

dst = pkt->data;

bytestream2_init_writer(&pb, dst, pkt->size);

av_fast_padded_malloc(&c->slice_bits, &c->slice_bits_size, width * height + 4);

if (!c->slice_bits) {
av_log(avctx, AV_LOG_ERROR, "Cannot allocate temporary buffer 2.\n");
return AVERROR(ENOMEM);
}


if (avctx->pix_fmt == AV_PIX_FMT_GBRAP || avctx->pix_fmt == AV_PIX_FMT_GBRP)
mangle_rgb_planes(c->slice_buffer, c->slice_stride, pic->data,
c->planes, pic->linesize, width, height);


switch (avctx->pix_fmt) {
case AV_PIX_FMT_GBRP:
case AV_PIX_FMT_GBRAP:
for (i = 0; i < c->planes; i++) {
ret = encode_plane(avctx, c->slice_buffer[i] + 2 * c->slice_stride,
c->slice_buffer[i], c->slice_stride, i,
width, height, &pb);

if (ret) {
av_log(avctx, AV_LOG_ERROR, "Error encoding plane %d.\n", i);
return ret;
}
}
break;
case AV_PIX_FMT_YUV444P:
for (i = 0; i < c->planes; i++) {
ret = encode_plane(avctx, pic->data[i], c->slice_buffer[0],
pic->linesize[i], i, width, height, &pb);

if (ret) {
av_log(avctx, AV_LOG_ERROR, "Error encoding plane %d.\n", i);
return ret;
}
}
break;
case AV_PIX_FMT_YUV422P:
for (i = 0; i < c->planes; i++) {
ret = encode_plane(avctx, pic->data[i], c->slice_buffer[0],
pic->linesize[i], i, width >> !!i, height, &pb);

if (ret) {
av_log(avctx, AV_LOG_ERROR, "Error encoding plane %d.\n", i);
return ret;
}
}
break;
case AV_PIX_FMT_YUV420P:
for (i = 0; i < c->planes; i++) {
ret = encode_plane(avctx, pic->data[i], c->slice_buffer[0],
pic->linesize[i], i, width >> !!i, height >> !!i,
&pb);

if (ret) {
av_log(avctx, AV_LOG_ERROR, "Error encoding plane %d.\n", i);
return ret;
}
}
break;
default:
av_log(avctx, AV_LOG_ERROR, "Unknown pixel format: %d\n",
avctx->pix_fmt);
return AVERROR_INVALIDDATA;
}






frame_info = c->frame_pred << 8;
bytestream2_put_le32(&pb, frame_info);





#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
avctx->coded_frame->key_frame = 1;
avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

pkt->size = bytestream2_tell_p(&pb);
pkt->flags |= AV_PKT_FLAG_KEY;


*got_packet = 1;

return 0;
}

#define OFFSET(x) offsetof(UtvideoContext, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "pred", "Prediction method", OFFSET(frame_pred), AV_OPT_TYPE_INT, { .i64 = PRED_LEFT }, PRED_NONE, PRED_MEDIAN, VE, "pred" },
{ "none", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = PRED_NONE }, INT_MIN, INT_MAX, VE, "pred" },
{ "left", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = PRED_LEFT }, INT_MIN, INT_MAX, VE, "pred" },
{ "gradient", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = PRED_GRADIENT }, INT_MIN, INT_MAX, VE, "pred" },
{ "median", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = PRED_MEDIAN }, INT_MIN, INT_MAX, VE, "pred" },

{ NULL},
};

static const AVClass utvideo_class = {
.class_name = "utvideo",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_utvideo_encoder = {
.name = "utvideo",
.long_name = NULL_IF_CONFIG_SMALL("Ut Video"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_UTVIDEO,
.priv_data_size = sizeof(UtvideoContext),
.priv_class = &utvideo_class,
.init = utvideo_encode_init,
.encode2 = utvideo_encode_frame,
.close = utvideo_encode_close,
.capabilities = AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_INTRA_ONLY,
.pix_fmts = (const enum AVPixelFormat[]) {
AV_PIX_FMT_GBRP, AV_PIX_FMT_GBRAP, AV_PIX_FMT_YUV422P,
AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV444P, AV_PIX_FMT_NONE
},
};
