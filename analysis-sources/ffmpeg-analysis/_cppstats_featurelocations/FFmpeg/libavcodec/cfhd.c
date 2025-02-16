
























#include "libavutil/attributes.h"
#include "libavutil/buffer.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"

#include "avcodec.h"
#include "bytestream.h"
#include "get_bits.h"
#include "internal.h"
#include "thread.h"
#include "cfhd.h"

#define ALPHA_COMPAND_DC_OFFSET 256
#define ALPHA_COMPAND_GAIN 9400

enum CFHDParam {
ChannelCount = 12,
SubbandCount = 14,
ImageWidth = 20,
ImageHeight = 21,
LowpassPrecision = 35,
SubbandNumber = 48,
Quantization = 53,
ChannelNumber = 62,
SampleFlags = 68,
BitsPerComponent = 101,
ChannelWidth = 104,
ChannelHeight = 105,
PrescaleShift = 109,
};



static av_cold int cfhd_init(AVCodecContext *avctx)
{
CFHDContext *s = avctx->priv_data;

avctx->bits_per_raw_sample = 10;
s->avctx = avctx;

return ff_cfhd_init_vlcs(s);
}

static void init_plane_defaults(CFHDContext *s)
{
s->subband_num = 0;
s->level = 0;
s->subband_num_actual = 0;
}

static void init_peak_table_defaults(CFHDContext *s)
{
s->peak.level = 0;
s->peak.offset = 0;
memset(&s->peak.base, 0, sizeof(s->peak.base));
}

static void init_frame_defaults(CFHDContext *s)
{
s->coded_width = 0;
s->coded_height = 0;
s->cropped_height = 0;
s->bpc = 10;
s->channel_cnt = 4;
s->subband_cnt = SUBBAND_COUNT;
s->channel_num = 0;
s->lowpass_precision = 16;
s->quantisation = 1;
s->wavelet_depth = 3;
s->pshift = 1;
s->codebook = 0;
s->difference_coding = 0;
s->progressive = 0;
init_plane_defaults(s);
init_peak_table_defaults(s);
}


static inline int dequant_and_decompand(int level, int quantisation, int codebook)
{
if (codebook == 0 || codebook == 1) {
int64_t abslevel = abs(level);
if (level < 264)
return (abslevel + ((768 * abslevel * abslevel * abslevel) / (255 * 255 * 255))) *
FFSIGN(level) * quantisation;
else
return level * quantisation;
} else
return level * quantisation;
}

static inline void difference_coding(int16_t *band, int width, int height)
{

int i,j;
for (i = 0; i < height; i++) {
for (j = 1; j < width; j++) {
band[j] += band[j-1];
}
band += width;
}
}

static inline void peak_table(int16_t *band, Peak *peak, int length)
{
int i;
for (i = 0; i < length; i++)
if (abs(band[i]) > peak->level)
band[i] = bytestream2_get_le16(&peak->base);
}

static inline void process_alpha(int16_t *alpha, int width)
{
int i, channel;
for (i = 0; i < width; i++) {
channel = alpha[i];
channel -= ALPHA_COMPAND_DC_OFFSET;
channel <<= 3;
channel *= ALPHA_COMPAND_GAIN;
channel >>= 16;
channel = av_clip_uintp2(channel, 12);
alpha[i] = channel;
}
}

static inline void process_bayer(AVFrame *frame)
{
const int linesize = frame->linesize[0];
uint16_t *r = (uint16_t *)frame->data[0];
uint16_t *g1 = (uint16_t *)(frame->data[0] + 2);
uint16_t *g2 = (uint16_t *)(frame->data[0] + frame->linesize[0]);
uint16_t *b = (uint16_t *)(frame->data[0] + frame->linesize[0] + 2);
const int mid = 2048;

for (int y = 0; y < frame->height >> 1; y++) {
for (int x = 0; x < frame->width; x += 2) {
int R, G1, G2, B;
int g, rg, bg, gd;

g = r[x];
rg = g1[x];
bg = g2[x];
gd = b[x];
gd -= mid;

R = (rg - mid) * 2 + g;
G1 = g + gd;
G2 = g - gd;
B = (bg - mid) * 2 + g;

R = av_clip_uintp2(R * 16, 16);
G1 = av_clip_uintp2(G1 * 16, 16);
G2 = av_clip_uintp2(G2 * 16, 16);
B = av_clip_uintp2(B * 16, 16);

r[x] = R;
g1[x] = G1;
g2[x] = G2;
b[x] = B;
}

r += linesize;
g1 += linesize;
g2 += linesize;
b += linesize;
}
}

static inline void filter(int16_t *output, ptrdiff_t out_stride,
int16_t *low, ptrdiff_t low_stride,
int16_t *high, ptrdiff_t high_stride,
int len, int clip)
{
int16_t tmp;
int i;

for (i = 0; i < len; i++) {
if (i == 0) {
tmp = (11*low[0*low_stride] - 4*low[1*low_stride] + low[2*low_stride] + 4) >> 3;
output[(2*i+0)*out_stride] = (tmp + high[0*high_stride]) >> 1;
if (clip)
output[(2*i+0)*out_stride] = av_clip_uintp2_c(output[(2*i+0)*out_stride], clip);

tmp = ( 5*low[0*low_stride] + 4*low[1*low_stride] - low[2*low_stride] + 4) >> 3;
output[(2*i+1)*out_stride] = (tmp - high[0*high_stride]) >> 1;
if (clip)
output[(2*i+1)*out_stride] = av_clip_uintp2_c(output[(2*i+1)*out_stride], clip);
} else if (i == len-1) {
tmp = ( 5*low[i*low_stride] + 4*low[(i-1)*low_stride] - low[(i-2)*low_stride] + 4) >> 3;
output[(2*i+0)*out_stride] = (tmp + high[i*high_stride]) >> 1;
if (clip)
output[(2*i+0)*out_stride] = av_clip_uintp2_c(output[(2*i+0)*out_stride], clip);

tmp = (11*low[i*low_stride] - 4*low[(i-1)*low_stride] + low[(i-2)*low_stride] + 4) >> 3;
output[(2*i+1)*out_stride] = (tmp - high[i*high_stride]) >> 1;
if (clip)
output[(2*i+1)*out_stride] = av_clip_uintp2_c(output[(2*i+1)*out_stride], clip);
} else {
tmp = (low[(i-1)*low_stride] - low[(i+1)*low_stride] + 4) >> 3;
output[(2*i+0)*out_stride] = (tmp + low[i*low_stride] + high[i*high_stride]) >> 1;
if (clip)
output[(2*i+0)*out_stride] = av_clip_uintp2_c(output[(2*i+0)*out_stride], clip);

tmp = (low[(i+1)*low_stride] - low[(i-1)*low_stride] + 4) >> 3;
output[(2*i+1)*out_stride] = (tmp + low[i*low_stride] - high[i*high_stride]) >> 1;
if (clip)
output[(2*i+1)*out_stride] = av_clip_uintp2_c(output[(2*i+1)*out_stride], clip);
}
}
}

static inline void interlaced_vertical_filter(int16_t *output, int16_t *low, int16_t *high,
int width, int linesize, int plane)
{
int i;
int16_t even, odd;
for (i = 0; i < width; i++) {
even = (low[i] - high[i])/2;
odd = (low[i] + high[i])/2;
output[i] = av_clip_uintp2(even, 10);
output[i + linesize] = av_clip_uintp2(odd, 10);
}
}
static void horiz_filter(int16_t *output, int16_t *low, int16_t *high,
int width)
{
filter(output, 1, low, 1, high, 1, width, 0);
}

static void horiz_filter_clip(int16_t *output, int16_t *low, int16_t *high,
int width, int clip)
{
filter(output, 1, low, 1, high, 1, width, clip);
}

static void horiz_filter_clip_bayer(int16_t *output, int16_t *low, int16_t *high,
int width, int clip)
{
filter(output, 2, low, 1, high, 1, width, clip);
}

static void vert_filter(int16_t *output, ptrdiff_t out_stride,
int16_t *low, ptrdiff_t low_stride,
int16_t *high, ptrdiff_t high_stride, int len)
{
filter(output, out_stride, low, low_stride, high, high_stride, len, 0);
}

static void free_buffers(CFHDContext *s)
{
int i, j;

for (i = 0; i < FF_ARRAY_ELEMS(s->plane); i++) {
av_freep(&s->plane[i].idwt_buf);
av_freep(&s->plane[i].idwt_tmp);

for (j = 0; j < 9; j++)
s->plane[i].subband[j] = NULL;

for (j = 0; j < 8; j++)
s->plane[i].l_h[j] = NULL;
}
s->a_height = 0;
s->a_width = 0;
}

static int alloc_buffers(AVCodecContext *avctx)
{
CFHDContext *s = avctx->priv_data;
int i, j, ret, planes;
int chroma_x_shift, chroma_y_shift;
unsigned k;

if (s->coded_format == AV_PIX_FMT_BAYER_RGGB16) {
s->coded_width *= 2;
s->coded_height *= 2;
}

if ((ret = ff_set_dimensions(avctx, s->coded_width, s->coded_height)) < 0)
return ret;
avctx->pix_fmt = s->coded_format;

if ((ret = av_pix_fmt_get_chroma_sub_sample(s->coded_format,
&chroma_x_shift,
&chroma_y_shift)) < 0)
return ret;
planes = av_pix_fmt_count_planes(s->coded_format);
if (s->coded_format == AV_PIX_FMT_BAYER_RGGB16) {
planes = 4;
chroma_x_shift = 1;
chroma_y_shift = 1;
}

for (i = 0; i < planes; i++) {
int w8, h8, w4, h4, w2, h2;
int width = i ? avctx->width >> chroma_x_shift : avctx->width;
int height = i ? avctx->height >> chroma_y_shift : avctx->height;
ptrdiff_t stride = FFALIGN(width / 8, 8) * 8;
if (chroma_y_shift)
height = FFALIGN(height / 8, 2) * 8;
s->plane[i].width = width;
s->plane[i].height = height;
s->plane[i].stride = stride;

w8 = FFALIGN(s->plane[i].width / 8, 8);
h8 = height / 8;
w4 = w8 * 2;
h4 = h8 * 2;
w2 = w4 * 2;
h2 = h4 * 2;

s->plane[i].idwt_buf =
av_mallocz_array(height * stride, sizeof(*s->plane[i].idwt_buf));
s->plane[i].idwt_tmp =
av_malloc_array(height * stride, sizeof(*s->plane[i].idwt_tmp));
if (!s->plane[i].idwt_buf || !s->plane[i].idwt_tmp)
return AVERROR(ENOMEM);

s->plane[i].subband[0] = s->plane[i].idwt_buf;
s->plane[i].subband[1] = s->plane[i].idwt_buf + 2 * w8 * h8;
s->plane[i].subband[2] = s->plane[i].idwt_buf + 1 * w8 * h8;
s->plane[i].subband[3] = s->plane[i].idwt_buf + 3 * w8 * h8;
s->plane[i].subband[4] = s->plane[i].idwt_buf + 2 * w4 * h4;
s->plane[i].subband[5] = s->plane[i].idwt_buf + 1 * w4 * h4;
s->plane[i].subband[6] = s->plane[i].idwt_buf + 3 * w4 * h4;
s->plane[i].subband[7] = s->plane[i].idwt_buf + 2 * w2 * h2;
s->plane[i].subband[8] = s->plane[i].idwt_buf + 1 * w2 * h2;
s->plane[i].subband[9] = s->plane[i].idwt_buf + 3 * w2 * h2;

for (j = 0; j < DWT_LEVELS; j++) {
for (k = 0; k < FF_ARRAY_ELEMS(s->plane[i].band[j]); k++) {
s->plane[i].band[j][k].a_width = w8 << j;
s->plane[i].band[j][k].a_height = h8 << j;
}
}


s->plane[i].l_h[0] = s->plane[i].idwt_tmp;
s->plane[i].l_h[1] = s->plane[i].idwt_tmp + 2 * w8 * h8;

s->plane[i].l_h[3] = s->plane[i].idwt_tmp;
s->plane[i].l_h[4] = s->plane[i].idwt_tmp + 2 * w4 * h4;

s->plane[i].l_h[6] = s->plane[i].idwt_tmp;
s->plane[i].l_h[7] = s->plane[i].idwt_tmp + 2 * w2 * h2;
}

s->a_height = s->coded_height;
s->a_width = s->coded_width;
s->a_format = s->coded_format;

return 0;
}

static int cfhd_decode(AVCodecContext *avctx, void *data, int *got_frame,
AVPacket *avpkt)
{
CFHDContext *s = avctx->priv_data;
GetByteContext gb;
ThreadFrame frame = { .f = data };
AVFrame *pic = data;
int ret = 0, i, j, planes, plane, got_buffer = 0;
int16_t *coeff_data;

s->coded_format = AV_PIX_FMT_YUV422P10;
init_frame_defaults(s);
planes = av_pix_fmt_count_planes(s->coded_format);

bytestream2_init(&gb, avpkt->data, avpkt->size);

while (bytestream2_get_bytes_left(&gb) > 4) {

uint16_t tagu = bytestream2_get_be16(&gb);
int16_t tag = (int16_t)tagu;
int8_t tag8 = (int8_t)(tagu >> 8);
uint16_t abstag = abs(tag);
int8_t abs_tag8 = abs(tag8);
uint16_t data = bytestream2_get_be16(&gb);
if (abs_tag8 >= 0x60 && abs_tag8 <= 0x6f) {
av_log(avctx, AV_LOG_DEBUG, "large len %x\n", ((tagu & 0xff) << 16) | data);
} else if (tag == SampleFlags) {
av_log(avctx, AV_LOG_DEBUG, "Progressive?%"PRIu16"\n", data);
s->progressive = data & 0x0001;
} else if (tag == ImageWidth) {
av_log(avctx, AV_LOG_DEBUG, "Width %"PRIu16"\n", data);
s->coded_width = data;
} else if (tag == ImageHeight) {
av_log(avctx, AV_LOG_DEBUG, "Height %"PRIu16"\n", data);
s->coded_height = data;
} else if (tag == 101) {
av_log(avctx, AV_LOG_DEBUG, "Bits per component: %"PRIu16"\n", data);
if (data < 1 || data > 31) {
av_log(avctx, AV_LOG_ERROR, "Bits per component %d is invalid\n", data);
ret = AVERROR(EINVAL);
break;
}
s->bpc = data;
} else if (tag == ChannelCount) {
av_log(avctx, AV_LOG_DEBUG, "Channel Count: %"PRIu16"\n", data);
s->channel_cnt = data;
if (data > 4) {
av_log(avctx, AV_LOG_ERROR, "Channel Count of %"PRIu16" is unsupported\n", data);
ret = AVERROR_PATCHWELCOME;
break;
}
} else if (tag == SubbandCount) {
av_log(avctx, AV_LOG_DEBUG, "Subband Count: %"PRIu16"\n", data);
if (data != SUBBAND_COUNT) {
av_log(avctx, AV_LOG_ERROR, "Subband Count of %"PRIu16" is unsupported\n", data);
ret = AVERROR_PATCHWELCOME;
break;
}
} else if (tag == ChannelNumber) {
s->channel_num = data;
av_log(avctx, AV_LOG_DEBUG, "Channel number %"PRIu16"\n", data);
if (s->channel_num >= planes) {
av_log(avctx, AV_LOG_ERROR, "Invalid channel number\n");
ret = AVERROR(EINVAL);
break;
}
init_plane_defaults(s);
} else if (tag == SubbandNumber) {
if (s->subband_num != 0 && data == 1) 
s->level++;
av_log(avctx, AV_LOG_DEBUG, "Subband number %"PRIu16"\n", data);
s->subband_num = data;
if (s->level >= DWT_LEVELS) {
av_log(avctx, AV_LOG_ERROR, "Invalid level\n");
ret = AVERROR(EINVAL);
break;
}
if (s->subband_num > 3) {
av_log(avctx, AV_LOG_ERROR, "Invalid subband number\n");
ret = AVERROR(EINVAL);
break;
}
} else if (tag == 51) {
av_log(avctx, AV_LOG_DEBUG, "Subband number actual %"PRIu16"\n", data);
s->subband_num_actual = data;
if (s->subband_num_actual >= 10) {
av_log(avctx, AV_LOG_ERROR, "Invalid subband number actual\n");
ret = AVERROR(EINVAL);
break;
}
} else if (tag == LowpassPrecision)
av_log(avctx, AV_LOG_DEBUG, "Lowpass precision bits: %"PRIu16"\n", data);
else if (tag == Quantization) {
s->quantisation = data;
av_log(avctx, AV_LOG_DEBUG, "Quantisation: %"PRIu16"\n", data);
} else if (tag == PrescaleShift) {
s->prescale_shift[0] = (data >> 0) & 0x7;
s->prescale_shift[1] = (data >> 3) & 0x7;
s->prescale_shift[2] = (data >> 6) & 0x7;
av_log(avctx, AV_LOG_DEBUG, "Prescale shift (VC-5): %x\n", data);
} else if (tag == 27) {
av_log(avctx, AV_LOG_DEBUG, "Lowpass width %"PRIu16"\n", data);
if (data < 3 || data > s->plane[s->channel_num].band[0][0].a_width) {
av_log(avctx, AV_LOG_ERROR, "Invalid lowpass width\n");
ret = AVERROR(EINVAL);
break;
}
s->plane[s->channel_num].band[0][0].width = data;
s->plane[s->channel_num].band[0][0].stride = data;
} else if (tag == 28) {
av_log(avctx, AV_LOG_DEBUG, "Lowpass height %"PRIu16"\n", data);
if (data < 3 || data > s->plane[s->channel_num].band[0][0].a_height) {
av_log(avctx, AV_LOG_ERROR, "Invalid lowpass height\n");
ret = AVERROR(EINVAL);
break;
}
s->plane[s->channel_num].band[0][0].height = data;
} else if (tag == 1)
av_log(avctx, AV_LOG_DEBUG, "Sample type? %"PRIu16"\n", data);
else if (tag == 10) {
if (data != 0) {
avpriv_report_missing_feature(avctx, "Transform type of %"PRIu16, data);
ret = AVERROR_PATCHWELCOME;
break;
}
av_log(avctx, AV_LOG_DEBUG, "Transform-type? %"PRIu16"\n", data);
} else if (abstag >= 0x4000 && abstag <= 0x40ff) {
if (abstag == 0x4001)
s->peak.level = 0;
av_log(avctx, AV_LOG_DEBUG, "Small chunk length %d %s\n", data * 4, tag < 0 ? "optional" : "required");
bytestream2_skipu(&gb, data * 4);
} else if (tag == 23) {
av_log(avctx, AV_LOG_DEBUG, "Skip frame\n");
avpriv_report_missing_feature(avctx, "Skip frame");
ret = AVERROR_PATCHWELCOME;
break;
} else if (tag == 2) {
av_log(avctx, AV_LOG_DEBUG, "tag=2 header - skipping %i tag/value pairs\n", data);
if (data > bytestream2_get_bytes_left(&gb) / 4) {
av_log(avctx, AV_LOG_ERROR, "too many tag/value pairs (%d)\n", data);
ret = AVERROR_INVALIDDATA;
break;
}
for (i = 0; i < data; i++) {
uint16_t tag2 = bytestream2_get_be16(&gb);
uint16_t val2 = bytestream2_get_be16(&gb);
av_log(avctx, AV_LOG_DEBUG, "Tag/Value = %x %x\n", tag2, val2);
}
} else if (tag == 41) {
av_log(avctx, AV_LOG_DEBUG, "Highpass width %i channel %i level %i subband %i\n", data, s->channel_num, s->level, s->subband_num);
if (data < 3) {
av_log(avctx, AV_LOG_ERROR, "Invalid highpass width\n");
ret = AVERROR(EINVAL);
break;
}
s->plane[s->channel_num].band[s->level][s->subband_num].width = data;
s->plane[s->channel_num].band[s->level][s->subband_num].stride = FFALIGN(data, 8);
} else if (tag == 42) {
av_log(avctx, AV_LOG_DEBUG, "Highpass height %i\n", data);
if (data < 3) {
av_log(avctx, AV_LOG_ERROR, "Invalid highpass height\n");
ret = AVERROR(EINVAL);
break;
}
s->plane[s->channel_num].band[s->level][s->subband_num].height = data;
} else if (tag == 49) {
av_log(avctx, AV_LOG_DEBUG, "Highpass width2 %i\n", data);
if (data < 3) {
av_log(avctx, AV_LOG_ERROR, "Invalid highpass width2\n");
ret = AVERROR(EINVAL);
break;
}
s->plane[s->channel_num].band[s->level][s->subband_num].width = data;
s->plane[s->channel_num].band[s->level][s->subband_num].stride = FFALIGN(data, 8);
} else if (tag == 50) {
av_log(avctx, AV_LOG_DEBUG, "Highpass height2 %i\n", data);
if (data < 3) {
av_log(avctx, AV_LOG_ERROR, "Invalid highpass height2\n");
ret = AVERROR(EINVAL);
break;
}
s->plane[s->channel_num].band[s->level][s->subband_num].height = data;
} else if (tag == 71) {
s->codebook = data;
av_log(avctx, AV_LOG_DEBUG, "Codebook %i\n", s->codebook);
} else if (tag == 72) {
s->codebook = data & 0xf;
s->difference_coding = (data >> 4) & 1;
av_log(avctx, AV_LOG_DEBUG, "Other codebook? %i\n", s->codebook);
} else if (tag == 70) {
av_log(avctx, AV_LOG_DEBUG, "Subsampling or bit-depth flag? %i\n", data);
if (!(data == 10 || data == 12)) {
av_log(avctx, AV_LOG_ERROR, "Invalid bits per channel\n");
ret = AVERROR(EINVAL);
break;
}
s->bpc = data;
} else if (tag == 84) {
av_log(avctx, AV_LOG_DEBUG, "Sample format? %i\n", data);
if (data == 1) {
s->coded_format = AV_PIX_FMT_YUV422P10;
} else if (data == 2) {
s->coded_format = AV_PIX_FMT_BAYER_RGGB16;
} else if (data == 3) {
s->coded_format = AV_PIX_FMT_GBRP12;
} else if (data == 4) {
s->coded_format = AV_PIX_FMT_GBRAP12;
} else {
avpriv_report_missing_feature(avctx, "Sample format of %"PRIu16, data);
ret = AVERROR_PATCHWELCOME;
break;
}
planes = data == 2 ? 4 : av_pix_fmt_count_planes(s->coded_format);
} else if (tag == -85) {
av_log(avctx, AV_LOG_DEBUG, "Cropped height %"PRIu16"\n", data);
s->cropped_height = data;
} else if (tag == -75) {
s->peak.offset &= ~0xffff;
s->peak.offset |= (data & 0xffff);
s->peak.base = gb;
s->peak.level = 0;
} else if (tag == -76) {
s->peak.offset &= 0xffff;
s->peak.offset |= (data & 0xffffU)<<16;
s->peak.base = gb;
s->peak.level = 0;
} else if (tag == -74 && s->peak.offset) {
s->peak.level = data;
bytestream2_seek(&s->peak.base, s->peak.offset - 4, SEEK_CUR);
} else
av_log(avctx, AV_LOG_DEBUG, "Unknown tag %i data %x\n", tag, data);


if (tag == 4 && data == 0x1a4a && s->coded_width && s->coded_height &&
s->coded_format != AV_PIX_FMT_NONE) {
if (s->a_width != s->coded_width || s->a_height != s->coded_height ||
s->a_format != s->coded_format) {
free_buffers(s);
if ((ret = alloc_buffers(avctx)) < 0) {
free_buffers(s);
return ret;
}
}
ret = ff_set_dimensions(avctx, s->coded_width, s->coded_height);
if (ret < 0)
return ret;
if (s->cropped_height) {
unsigned height = s->cropped_height << (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16);
if (avctx->height < height)
return AVERROR_INVALIDDATA;
avctx->height = height;
}
frame.f->width =
frame.f->height = 0;

if ((ret = ff_thread_get_buffer(avctx, &frame, 0)) < 0)
return ret;

s->coded_width = 0;
s->coded_height = 0;
s->coded_format = AV_PIX_FMT_NONE;
got_buffer = 1;
}
coeff_data = s->plane[s->channel_num].subband[s->subband_num_actual];


if (tag == 4 && data == 0xf0f && s->a_width && s->a_height) {
int lowpass_height = s->plane[s->channel_num].band[0][0].height;
int lowpass_width = s->plane[s->channel_num].band[0][0].width;
int lowpass_a_height = s->plane[s->channel_num].band[0][0].a_height;
int lowpass_a_width = s->plane[s->channel_num].band[0][0].a_width;

if (!got_buffer) {
av_log(avctx, AV_LOG_ERROR, "No end of header tag found\n");
ret = AVERROR(EINVAL);
goto end;
}

if (lowpass_height > lowpass_a_height || lowpass_width > lowpass_a_width ||
lowpass_a_width * lowpass_a_height * sizeof(int16_t) > bytestream2_get_bytes_left(&gb)) {
av_log(avctx, AV_LOG_ERROR, "Too many lowpass coefficients\n");
ret = AVERROR(EINVAL);
goto end;
}

av_log(avctx, AV_LOG_DEBUG, "Start of lowpass coeffs component %d height:%d, width:%d\n", s->channel_num, lowpass_height, lowpass_width);
for (i = 0; i < lowpass_height; i++) {
for (j = 0; j < lowpass_width; j++)
coeff_data[j] = bytestream2_get_be16u(&gb);

coeff_data += lowpass_width;
}


bytestream2_seek(&gb, bytestream2_tell(&gb) & 3, SEEK_CUR);


if (lowpass_height & 1) {
memcpy(&coeff_data[lowpass_height * lowpass_width],
&coeff_data[(lowpass_height - 1) * lowpass_width],
lowpass_width * sizeof(*coeff_data));
}

av_log(avctx, AV_LOG_DEBUG, "Lowpass coefficients %d\n", lowpass_width * lowpass_height);
}

if (tag == 55 && s->subband_num_actual != 255 && s->a_width && s->a_height) {
int highpass_height = s->plane[s->channel_num].band[s->level][s->subband_num].height;
int highpass_width = s->plane[s->channel_num].band[s->level][s->subband_num].width;
int highpass_a_width = s->plane[s->channel_num].band[s->level][s->subband_num].a_width;
int highpass_a_height = s->plane[s->channel_num].band[s->level][s->subband_num].a_height;
int highpass_stride = s->plane[s->channel_num].band[s->level][s->subband_num].stride;
int expected;
int a_expected = highpass_a_height * highpass_a_width;
int level, run, coeff;
int count = 0, bytes;

if (!got_buffer) {
av_log(avctx, AV_LOG_ERROR, "No end of header tag found\n");
ret = AVERROR(EINVAL);
goto end;
}

if (highpass_height > highpass_a_height || highpass_width > highpass_a_width || a_expected < highpass_height * (uint64_t)highpass_stride) {
av_log(avctx, AV_LOG_ERROR, "Too many highpass coefficients\n");
ret = AVERROR(EINVAL);
goto end;
}
expected = highpass_height * highpass_stride;

av_log(avctx, AV_LOG_DEBUG, "Start subband coeffs plane %i level %i codebook %i expected %i\n", s->channel_num, s->level, s->codebook, expected);

init_get_bits(&s->gb, gb.buffer, bytestream2_get_bytes_left(&gb) * 8);
{
OPEN_READER(re, &s->gb);
if (!s->codebook) {
while (1) {
UPDATE_CACHE(re, &s->gb);
GET_RL_VLC(level, run, re, &s->gb, s->table_9_rl_vlc,
VLC_BITS, 3, 1);


if (level == 64)
break;

count += run;

if (count > expected)
break;

coeff = dequant_and_decompand(level, s->quantisation, 0);
for (i = 0; i < run; i++)
*coeff_data++ = coeff;
}
} else {
while (1) {
UPDATE_CACHE(re, &s->gb);
GET_RL_VLC(level, run, re, &s->gb, s->table_18_rl_vlc,
VLC_BITS, 3, 1);


if (level == 255 && run == 2)
break;

count += run;

if (count > expected)
break;

coeff = dequant_and_decompand(level, s->quantisation, s->codebook);
for (i = 0; i < run; i++)
*coeff_data++ = coeff;
}
}
CLOSE_READER(re, &s->gb);
}

if (count > expected) {
av_log(avctx, AV_LOG_ERROR, "Escape codeword not found, probably corrupt data\n");
ret = AVERROR(EINVAL);
goto end;
}
if (s->peak.level)
peak_table(coeff_data - count, &s->peak, count);
if (s->difference_coding)
difference_coding(s->plane[s->channel_num].subband[s->subband_num_actual], highpass_width, highpass_height);

bytes = FFALIGN(AV_CEIL_RSHIFT(get_bits_count(&s->gb), 3), 4);
if (bytes > bytestream2_get_bytes_left(&gb)) {
av_log(avctx, AV_LOG_ERROR, "Bitstream overread error\n");
ret = AVERROR(EINVAL);
goto end;
} else
bytestream2_seek(&gb, bytes, SEEK_CUR);

av_log(avctx, AV_LOG_DEBUG, "End subband coeffs %i extra %i\n", count, count - expected);
s->codebook = 0;


if (highpass_height & 1) {
memcpy(&coeff_data[highpass_height * highpass_stride],
&coeff_data[(highpass_height - 1) * highpass_stride],
highpass_stride * sizeof(*coeff_data));
}
}
}

if (!s->a_width || !s->a_height || s->a_format == AV_PIX_FMT_NONE ||
s->coded_width || s->coded_height || s->coded_format != AV_PIX_FMT_NONE) {
av_log(avctx, AV_LOG_ERROR, "Invalid dimensions\n");
ret = AVERROR(EINVAL);
goto end;
}

if (!got_buffer) {
av_log(avctx, AV_LOG_ERROR, "No end of header tag found\n");
ret = AVERROR(EINVAL);
goto end;
}

planes = av_pix_fmt_count_planes(avctx->pix_fmt);
if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16) {
if (!s->progressive)
return AVERROR_INVALIDDATA;
planes = 4;
}

for (plane = 0; plane < planes && !ret; plane++) {

int lowpass_height = s->plane[plane].band[0][0].height;
int lowpass_width = s->plane[plane].band[0][0].width;
int highpass_stride = s->plane[plane].band[0][1].stride;
int act_plane = plane == 1 ? 2 : plane == 2 ? 1 : plane;
ptrdiff_t dst_linesize;
int16_t *low, *high, *output, *dst;

if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16) {
act_plane = 0;
dst_linesize = pic->linesize[act_plane];
} else {
dst_linesize = pic->linesize[act_plane] / 2;
}

if (lowpass_height > s->plane[plane].band[0][0].a_height || lowpass_width > s->plane[plane].band[0][0].a_width ||
!highpass_stride || s->plane[plane].band[0][1].width > s->plane[plane].band[0][1].a_width) {
av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
ret = AVERROR(EINVAL);
goto end;
}

av_log(avctx, AV_LOG_DEBUG, "Decoding level 1 plane %i %i %i %i\n", plane, lowpass_height, lowpass_width, highpass_stride);

low = s->plane[plane].subband[0];
high = s->plane[plane].subband[2];
output = s->plane[plane].l_h[0];
for (i = 0; i < lowpass_width; i++) {
vert_filter(output, lowpass_width, low, lowpass_width, high, highpass_stride, lowpass_height);
low++;
high++;
output++;
}

low = s->plane[plane].subband[1];
high = s->plane[plane].subband[3];
output = s->plane[plane].l_h[1];

for (i = 0; i < lowpass_width; i++) {

vert_filter(output, lowpass_width, low, highpass_stride, high, highpass_stride, lowpass_height);
low++;
high++;
output++;
}

low = s->plane[plane].l_h[0];
high = s->plane[plane].l_h[1];
output = s->plane[plane].subband[0];
for (i = 0; i < lowpass_height * 2; i++) {
horiz_filter(output, low, high, lowpass_width);
low += lowpass_width;
high += lowpass_width;
output += lowpass_width * 2;
}
if (s->bpc == 12) {
output = s->plane[plane].subband[0];
for (i = 0; i < lowpass_height * 2; i++) {
for (j = 0; j < lowpass_width * 2; j++)
output[j] *= 4;

output += lowpass_width * 2;
}
}


lowpass_height = s->plane[plane].band[1][1].height;
lowpass_width = s->plane[plane].band[1][1].width;
highpass_stride = s->plane[plane].band[1][1].stride;

if (lowpass_height > s->plane[plane].band[1][1].a_height || lowpass_width > s->plane[plane].band[1][1].a_width ||
!highpass_stride || s->plane[plane].band[1][1].width > s->plane[plane].band[1][1].a_width) {
av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
ret = AVERROR(EINVAL);
goto end;
}

av_log(avctx, AV_LOG_DEBUG, "Level 2 plane %i %i %i %i\n", plane, lowpass_height, lowpass_width, highpass_stride);

low = s->plane[plane].subband[0];
high = s->plane[plane].subband[5];
output = s->plane[plane].l_h[3];
for (i = 0; i < lowpass_width; i++) {
vert_filter(output, lowpass_width, low, lowpass_width, high, highpass_stride, lowpass_height);
low++;
high++;
output++;
}

low = s->plane[plane].subband[4];
high = s->plane[plane].subband[6];
output = s->plane[plane].l_h[4];
for (i = 0; i < lowpass_width; i++) {
vert_filter(output, lowpass_width, low, highpass_stride, high, highpass_stride, lowpass_height);
low++;
high++;
output++;
}

low = s->plane[plane].l_h[3];
high = s->plane[plane].l_h[4];
output = s->plane[plane].subband[0];
for (i = 0; i < lowpass_height * 2; i++) {
horiz_filter(output, low, high, lowpass_width);
low += lowpass_width;
high += lowpass_width;
output += lowpass_width * 2;
}

output = s->plane[plane].subband[0];
for (i = 0; i < lowpass_height * 2; i++) {
for (j = 0; j < lowpass_width * 2; j++)
output[j] *= 4;

output += lowpass_width * 2;
}


lowpass_height = s->plane[plane].band[2][1].height;
lowpass_width = s->plane[plane].band[2][1].width;
highpass_stride = s->plane[plane].band[2][1].stride;

if (lowpass_height > s->plane[plane].band[2][1].a_height || lowpass_width > s->plane[plane].band[2][1].a_width ||
!highpass_stride || s->plane[plane].band[2][1].width > s->plane[plane].band[2][1].a_width) {
av_log(avctx, AV_LOG_ERROR, "Invalid plane dimensions\n");
ret = AVERROR(EINVAL);
goto end;
}

av_log(avctx, AV_LOG_DEBUG, "Level 3 plane %i %i %i %i\n", plane, lowpass_height, lowpass_width, highpass_stride);
if (s->progressive) {
low = s->plane[plane].subband[0];
high = s->plane[plane].subband[8];
output = s->plane[plane].l_h[6];
for (i = 0; i < lowpass_width; i++) {
vert_filter(output, lowpass_width, low, lowpass_width, high, highpass_stride, lowpass_height);
low++;
high++;
output++;
}

low = s->plane[plane].subband[7];
high = s->plane[plane].subband[9];
output = s->plane[plane].l_h[7];
for (i = 0; i < lowpass_width; i++) {
vert_filter(output, lowpass_width, low, highpass_stride, high, highpass_stride, lowpass_height);
low++;
high++;
output++;
}

dst = (int16_t *)pic->data[act_plane];
if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16) {
if (plane & 1)
dst++;
if (plane > 1)
dst += pic->linesize[act_plane] >> 1;
}
low = s->plane[plane].l_h[6];
high = s->plane[plane].l_h[7];

if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16 &&
(lowpass_height * 2 > avctx->coded_height / 2 ||
lowpass_width * 2 > avctx->coded_width / 2 )
) {
ret = AVERROR_INVALIDDATA;
goto end;
}

for (i = 0; i < lowpass_height * 2; i++) {
if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16)
horiz_filter_clip_bayer(dst, low, high, lowpass_width, s->bpc);
else
horiz_filter_clip(dst, low, high, lowpass_width, s->bpc);
if (avctx->pix_fmt == AV_PIX_FMT_GBRAP12 && act_plane == 3)
process_alpha(dst, lowpass_width * 2);
low += lowpass_width;
high += lowpass_width;
dst += dst_linesize;
}
} else {
av_log(avctx, AV_LOG_DEBUG, "interlaced frame ? %d", pic->interlaced_frame);
pic->interlaced_frame = 1;
low = s->plane[plane].subband[0];
high = s->plane[plane].subband[7];
output = s->plane[plane].l_h[6];
for (i = 0; i < lowpass_height; i++) {
horiz_filter(output, low, high, lowpass_width);
low += lowpass_width;
high += lowpass_width;
output += lowpass_width * 2;
}

low = s->plane[plane].subband[8];
high = s->plane[plane].subband[9];
output = s->plane[plane].l_h[7];
for (i = 0; i < lowpass_height; i++) {
horiz_filter(output, low, high, lowpass_width);
low += lowpass_width;
high += lowpass_width;
output += lowpass_width * 2;
}

dst = (int16_t *)pic->data[act_plane];
low = s->plane[plane].l_h[6];
high = s->plane[plane].l_h[7];
for (i = 0; i < lowpass_height; i++) {
interlaced_vertical_filter(dst, low, high, lowpass_width * 2, pic->linesize[act_plane]/2, act_plane);
low += lowpass_width * 2;
high += lowpass_width * 2;
dst += pic->linesize[act_plane];
}
}
}


if (avctx->pix_fmt == AV_PIX_FMT_BAYER_RGGB16)
process_bayer(pic);
end:
if (ret < 0)
return ret;

*got_frame = 1;
return avpkt->size;
}

static av_cold int cfhd_close(AVCodecContext *avctx)
{
CFHDContext *s = avctx->priv_data;

free_buffers(s);

if (!avctx->internal->is_copy) {
ff_free_vlc(&s->vlc_9);
ff_free_vlc(&s->vlc_18);
}

return 0;
}

AVCodec ff_cfhd_decoder = {
.name = "cfhd",
.long_name = NULL_IF_CONFIG_SMALL("Cineform HD"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_CFHD,
.priv_data_size = sizeof(CFHDContext),
.init = cfhd_init,
.close = cfhd_close,
.decode = cfhd_decode,
.capabilities = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE | FF_CODEC_CAP_INIT_CLEANUP,
};
