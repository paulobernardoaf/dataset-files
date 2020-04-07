#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "lzw.h"
#include "gif.h"
#define GIF_TRANSPARENT_COLOR 0x00ffffff
typedef struct GifState {
const AVClass *class;
AVFrame *frame;
int screen_width;
int screen_height;
int has_global_palette;
int bits_per_pixel;
uint32_t bg_color;
int background_color_index;
int transparent_color_index;
int color_resolution;
uint8_t *idx_line;
int idx_line_size;
int gce_prev_disposal;
int gce_disposal;
int gce_l, gce_t, gce_w, gce_h;
uint32_t * stored_img;
int stored_img_size;
int stored_bg_color;
GetByteContext gb;
LZWState *lzw;
uint32_t global_palette[256];
uint32_t local_palette[256];
AVCodecContext *avctx;
int keyframe;
int keyframe_ok;
int trans_color; 
} GifState;
static void gif_read_palette(GifState *s, uint32_t *pal, int nb)
{
int i;
for (i = 0; i < nb; i++, pal++)
*pal = (0xffu << 24) | bytestream2_get_be24u(&s->gb);
}
static void gif_fill(AVFrame *picture, uint32_t color)
{
uint32_t *p = (uint32_t *)picture->data[0];
uint32_t *p_end = p + (picture->linesize[0] / sizeof(uint32_t)) * picture->height;
for (; p < p_end; p++)
*p = color;
}
static void gif_fill_rect(AVFrame *picture, uint32_t color, int l, int t, int w, int h)
{
const int linesize = picture->linesize[0] / sizeof(uint32_t);
const uint32_t *py = (uint32_t *)picture->data[0] + t * linesize;
const uint32_t *pr, *pb = py + h * linesize;
uint32_t *px;
for (; py < pb; py += linesize) {
px = (uint32_t *)py + l;
pr = px + w;
for (; px < pr; px++)
*px = color;
}
}
static void gif_copy_img_rect(const uint32_t *src, uint32_t *dst,
int linesize, int l, int t, int w, int h)
{
const int y_start = t * linesize;
const uint32_t *src_px,
*src_py = src + y_start,
*dst_py = dst + y_start;
const uint32_t *src_pb = src_py + h * linesize;
uint32_t *dst_px;
for (; src_py < src_pb; src_py += linesize, dst_py += linesize) {
src_px = src_py + l;
dst_px = (uint32_t *)dst_py + l;
memcpy(dst_px, src_px, w * sizeof(uint32_t));
}
}
static int gif_read_image(GifState *s, AVFrame *frame)
{
int left, top, width, height, bits_per_pixel, code_size, flags, pw;
int is_interleaved, has_local_palette, y, pass, y1, linesize, pal_size, lzwed_len;
uint32_t *ptr, *pal, *px, *pr, *ptr1;
int ret;
uint8_t *idx;
if (bytestream2_get_bytes_left(&s->gb) < 9)
return AVERROR_INVALIDDATA;
left = bytestream2_get_le16u(&s->gb);
top = bytestream2_get_le16u(&s->gb);
width = bytestream2_get_le16u(&s->gb);
height = bytestream2_get_le16u(&s->gb);
flags = bytestream2_get_byteu(&s->gb);
is_interleaved = flags & 0x40;
has_local_palette = flags & 0x80;
bits_per_pixel = (flags & 0x07) + 1;
ff_dlog(s->avctx, "image x=%d y=%d w=%d h=%d\n", left, top, width, height);
if (has_local_palette) {
pal_size = 1 << bits_per_pixel;
if (bytestream2_get_bytes_left(&s->gb) < pal_size * 3)
return AVERROR_INVALIDDATA;
gif_read_palette(s, s->local_palette, pal_size);
pal = s->local_palette;
} else {
if (!s->has_global_palette) {
av_log(s->avctx, AV_LOG_ERROR, "picture doesn't have either global or local palette.\n");
return AVERROR_INVALIDDATA;
}
pal = s->global_palette;
}
if (s->keyframe) {
if (s->transparent_color_index == -1 && s->has_global_palette) {
gif_fill(frame, s->bg_color);
} else {
gif_fill(frame, s->trans_color);
}
}
if (!width || width > s->screen_width) {
av_log(s->avctx, AV_LOG_WARNING, "Invalid image width: %d, truncating.\n", width);
width = s->screen_width;
}
if (left >= s->screen_width) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid left position: %d.\n", left);
return AVERROR_INVALIDDATA;
}
if (!height || height > s->screen_height) {
av_log(s->avctx, AV_LOG_WARNING, "Invalid image height: %d, truncating.\n", height);
height = s->screen_height;
}
if (top >= s->screen_height) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid top position: %d.\n", top);
return AVERROR_INVALIDDATA;
}
if (left + width > s->screen_width) {
pw = s->screen_width - left;
av_log(s->avctx, AV_LOG_WARNING, "Image too wide by %d, truncating.\n",
left + width - s->screen_width);
} else {
pw = width;
}
if (top + height > s->screen_height) {
av_log(s->avctx, AV_LOG_WARNING, "Image too high by %d, truncating.\n",
top + height - s->screen_height);
height = s->screen_height - top;
}
if (s->gce_prev_disposal == GCE_DISPOSAL_BACKGROUND) {
gif_fill_rect(frame, s->stored_bg_color, s->gce_l, s->gce_t, s->gce_w, s->gce_h);
} else if (s->gce_prev_disposal == GCE_DISPOSAL_RESTORE) {
gif_copy_img_rect(s->stored_img, (uint32_t *)frame->data[0],
frame->linesize[0] / sizeof(uint32_t), s->gce_l, s->gce_t, s->gce_w, s->gce_h);
}
s->gce_prev_disposal = s->gce_disposal;
if (s->gce_disposal != GCE_DISPOSAL_NONE) {
s->gce_l = left; s->gce_t = top;
s->gce_w = pw; s->gce_h = height;
if (s->gce_disposal == GCE_DISPOSAL_BACKGROUND) {
if (s->transparent_color_index >= 0)
s->stored_bg_color = s->trans_color;
else
s->stored_bg_color = s->bg_color;
} else if (s->gce_disposal == GCE_DISPOSAL_RESTORE) {
av_fast_malloc(&s->stored_img, &s->stored_img_size, frame->linesize[0] * frame->height);
if (!s->stored_img)
return AVERROR(ENOMEM);
gif_copy_img_rect((uint32_t *)frame->data[0], s->stored_img,
frame->linesize[0] / sizeof(uint32_t), left, top, pw, height);
}
}
if (bytestream2_get_bytes_left(&s->gb) < 2)
return AVERROR_INVALIDDATA;
code_size = bytestream2_get_byteu(&s->gb);
if ((ret = ff_lzw_decode_init(s->lzw, code_size, s->gb.buffer,
bytestream2_get_bytes_left(&s->gb), FF_LZW_GIF)) < 0) {
av_log(s->avctx, AV_LOG_ERROR, "LZW init failed\n");
return ret;
}
linesize = frame->linesize[0] / sizeof(uint32_t);
ptr1 = (uint32_t *)frame->data[0] + top * linesize + left;
ptr = ptr1;
pass = 0;
y1 = 0;
for (y = 0; y < height; y++) {
int count = ff_lzw_decode(s->lzw, s->idx_line, width);
if (count != width) {
if (count)
av_log(s->avctx, AV_LOG_ERROR, "LZW decode failed\n");
goto decode_tail;
}
pr = ptr + pw;
for (px = ptr, idx = s->idx_line; px < pr; px++, idx++) {
if (*idx != s->transparent_color_index)
*px = pal[*idx];
}
if (is_interleaved) {
switch(pass) {
default:
case 0:
case 1:
y1 += 8;
ptr += linesize * 8;
break;
case 2:
y1 += 4;
ptr += linesize * 4;
break;
case 3:
y1 += 2;
ptr += linesize * 2;
break;
}
while (y1 >= height) {
y1 = 4 >> pass;
ptr = ptr1 + linesize * y1;
pass++;
}
} else {
ptr += linesize;
}
}
decode_tail:
lzwed_len = ff_lzw_decode_tail(s->lzw);
bytestream2_skipu(&s->gb, lzwed_len);
s->transparent_color_index = -1;
s->gce_disposal = GCE_DISPOSAL_NONE;
return 0;
}
static int gif_read_extension(GifState *s)
{
int ext_code, ext_len, gce_flags, gce_transparent_index;
if (bytestream2_get_bytes_left(&s->gb) < 2)
return AVERROR_INVALIDDATA;
ext_code = bytestream2_get_byteu(&s->gb);
ext_len = bytestream2_get_byteu(&s->gb);
ff_dlog(s->avctx, "ext_code=0x%x len=%d\n", ext_code, ext_len);
switch(ext_code) {
case GIF_GCE_EXT_LABEL:
if (ext_len != 4)
goto discard_ext;
if (bytestream2_get_bytes_left(&s->gb) < 5)
return AVERROR_INVALIDDATA;
gce_flags = bytestream2_get_byteu(&s->gb);
bytestream2_skipu(&s->gb, 2); 
gce_transparent_index = bytestream2_get_byteu(&s->gb);
if (gce_flags & 0x01)
s->transparent_color_index = gce_transparent_index;
else
s->transparent_color_index = -1;
s->gce_disposal = (gce_flags >> 2) & 0x7;
ff_dlog(s->avctx, "gce_flags=%x tcolor=%d disposal=%d\n",
gce_flags,
s->transparent_color_index, s->gce_disposal);
if (s->gce_disposal > 3) {
s->gce_disposal = GCE_DISPOSAL_NONE;
ff_dlog(s->avctx, "invalid value in gce_disposal (%d). Using default value of 0.\n", ext_len);
}
ext_len = bytestream2_get_byteu(&s->gb);
break;
}
discard_ext:
while (ext_len) {
if (bytestream2_get_bytes_left(&s->gb) < ext_len + 1)
return AVERROR_INVALIDDATA;
bytestream2_skipu(&s->gb, ext_len);
ext_len = bytestream2_get_byteu(&s->gb);
ff_dlog(s->avctx, "ext_len1=%d\n", ext_len);
}
return 0;
}
static int gif_read_header1(GifState *s)
{
uint8_t sig[6];
int v, n;
int background_color_index;
if (bytestream2_get_bytes_left(&s->gb) < 13)
return AVERROR_INVALIDDATA;
bytestream2_get_bufferu(&s->gb, sig, 6);
if (memcmp(sig, gif87a_sig, 6) &&
memcmp(sig, gif89a_sig, 6))
return AVERROR_INVALIDDATA;
s->transparent_color_index = -1;
s->screen_width = bytestream2_get_le16u(&s->gb);
s->screen_height = bytestream2_get_le16u(&s->gb);
v = bytestream2_get_byteu(&s->gb);
s->color_resolution = ((v & 0x70) >> 4) + 1;
s->has_global_palette = (v & 0x80);
s->bits_per_pixel = (v & 0x07) + 1;
background_color_index = bytestream2_get_byteu(&s->gb);
n = bytestream2_get_byteu(&s->gb);
if (n) {
s->avctx->sample_aspect_ratio.num = n + 15;
s->avctx->sample_aspect_ratio.den = 64;
}
ff_dlog(s->avctx, "screen_w=%d screen_h=%d bpp=%d global_palette=%d\n",
s->screen_width, s->screen_height, s->bits_per_pixel,
s->has_global_palette);
if (s->has_global_palette) {
s->background_color_index = background_color_index;
n = 1 << s->bits_per_pixel;
if (bytestream2_get_bytes_left(&s->gb) < n * 3)
return AVERROR_INVALIDDATA;
gif_read_palette(s, s->global_palette, n);
s->bg_color = s->global_palette[s->background_color_index];
} else
s->background_color_index = -1;
return 0;
}
static int gif_parse_next_image(GifState *s, AVFrame *frame)
{
while (bytestream2_get_bytes_left(&s->gb) > 0) {
int code = bytestream2_get_byte(&s->gb);
int ret;
av_log(s->avctx, AV_LOG_DEBUG, "code=%02x '%c'\n", code, code);
switch (code) {
case GIF_IMAGE_SEPARATOR:
return gif_read_image(s, frame);
case GIF_EXTENSION_INTRODUCER:
if ((ret = gif_read_extension(s)) < 0)
return ret;
break;
case GIF_TRAILER:
return AVERROR_EOF;
default:
return AVERROR_INVALIDDATA;
}
}
return AVERROR_EOF;
}
static av_cold int gif_decode_init(AVCodecContext *avctx)
{
GifState *s = avctx->priv_data;
s->avctx = avctx;
avctx->pix_fmt = AV_PIX_FMT_RGB32;
s->frame = av_frame_alloc();
if (!s->frame)
return AVERROR(ENOMEM);
ff_lzw_decode_open(&s->lzw);
if (!s->lzw)
return AVERROR(ENOMEM);
return 0;
}
static int gif_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
GifState *s = avctx->priv_data;
int ret;
bytestream2_init(&s->gb, avpkt->data, avpkt->size);
s->frame->pts = avpkt->pts;
#if FF_API_PKT_PTS
FF_DISABLE_DEPRECATION_WARNINGS
s->frame->pkt_pts = avpkt->pts;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
s->frame->pkt_dts = avpkt->dts;
s->frame->pkt_duration = avpkt->duration;
if (avpkt->size >= 6) {
s->keyframe = memcmp(avpkt->data, gif87a_sig, 6) == 0 ||
memcmp(avpkt->data, gif89a_sig, 6) == 0;
} else {
s->keyframe = 0;
}
if (s->keyframe) {
s->keyframe_ok = 0;
s->gce_prev_disposal = GCE_DISPOSAL_NONE;
if ((ret = gif_read_header1(s)) < 0)
return ret;
if ((ret = ff_set_dimensions(avctx, s->screen_width, s->screen_height)) < 0)
return ret;
av_frame_unref(s->frame);
if ((ret = ff_get_buffer(avctx, s->frame, 0)) < 0)
return ret;
av_fast_malloc(&s->idx_line, &s->idx_line_size, s->screen_width);
if (!s->idx_line)
return AVERROR(ENOMEM);
s->frame->pict_type = AV_PICTURE_TYPE_I;
s->frame->key_frame = 1;
s->keyframe_ok = 1;
} else {
if (!s->keyframe_ok) {
av_log(avctx, AV_LOG_ERROR, "cannot decode frame without keyframe\n");
return AVERROR_INVALIDDATA;
}
if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
return ret;
s->frame->pict_type = AV_PICTURE_TYPE_P;
s->frame->key_frame = 0;
}
ret = gif_parse_next_image(s, s->frame);
if (ret < 0)
return ret;
if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;
*got_frame = 1;
return bytestream2_tell(&s->gb);
}
static av_cold int gif_decode_close(AVCodecContext *avctx)
{
GifState *s = avctx->priv_data;
ff_lzw_decode_close(&s->lzw);
av_frame_free(&s->frame);
av_freep(&s->idx_line);
av_freep(&s->stored_img);
return 0;
}
static const AVOption options[] = {
{ "trans_color", "color value (ARGB) that is used instead of transparent color",
offsetof(GifState, trans_color), AV_OPT_TYPE_INT,
{.i64 = GIF_TRANSPARENT_COLOR}, 0, 0xffffffff,
AV_OPT_FLAG_DECODING_PARAM|AV_OPT_FLAG_VIDEO_PARAM },
{ NULL },
};
static const AVClass decoder_class = {
.class_name = "gif decoder",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
.category = AV_CLASS_CATEGORY_DECODER,
};
AVCodec ff_gif_decoder = {
.name = "gif",
.long_name = NULL_IF_CONFIG_SMALL("GIF (Graphics Interchange Format)"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_GIF,
.priv_data_size = sizeof(GifState),
.init = gif_decode_init,
.close = gif_decode_close,
.decode = gif_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE |
FF_CODEC_CAP_INIT_CLEANUP,
.priv_class = &decoder_class,
};
