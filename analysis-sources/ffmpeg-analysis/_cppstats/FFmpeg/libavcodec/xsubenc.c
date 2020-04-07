#include "avcodec.h"
#include "bytestream.h"
#include "put_bits.h"
#define PADDING 0
#define PADDING_COLOR 0
static void put_xsub_rle(PutBitContext *pb, int len, int color)
{
if (len <= 255)
put_bits(pb, 2 + ((ff_log2_tab[len] >> 1) << 2), len);
else
put_bits(pb, 14, 0);
put_bits(pb, 2, color);
}
static int xsub_encode_rle(PutBitContext *pb, const uint8_t *bitmap,
int linesize, int w, int h)
{
int x0, x1, y, len, color = PADDING_COLOR;
for (y = 0; y < h; y++) {
x0 = 0;
while (x0 < w) {
if (pb->size_in_bits - put_bits_count(pb) < 7*8)
return -1;
x1 = x0;
color = bitmap[x1++] & 3;
while (x1 < w && (bitmap[x1] & 3) == color)
x1++;
len = x1 - x0;
if (PADDING && x0 == 0) {
if (color == PADDING_COLOR) {
len += PADDING;
x0 -= PADDING;
} else
put_xsub_rle(pb, PADDING, PADDING_COLOR);
}
if (x1 == w && color == PADDING_COLOR) {
len += PADDING + (w&1);
} else
len = FFMIN(len, 255);
put_xsub_rle(pb, len, color);
x0 += len;
}
if (color != PADDING_COLOR && (PADDING + (w&1)))
put_xsub_rle(pb, PADDING + (w&1), PADDING_COLOR);
avpriv_align_put_bits(pb);
bitmap += linesize;
}
return 0;
}
static int make_tc(uint64_t ms, int *tc)
{
static const int tc_divs[3] = { 1000, 60, 60 };
int i;
for (i=0; i<3; i++) {
tc[i] = ms % tc_divs[i];
ms /= tc_divs[i];
}
tc[3] = ms;
return ms > 99;
}
static int xsub_encode(AVCodecContext *avctx, unsigned char *buf,
int bufsize, const AVSubtitle *h)
{
uint64_t startTime = h->pts / 1000; 
uint64_t endTime = startTime + h->end_display_time - h->start_display_time;
int start_tc[4], end_tc[4];
uint8_t *hdr = buf + 27; 
uint8_t *rlelenptr;
uint16_t width, height;
int i;
PutBitContext pb;
if (bufsize < 27 + 7*2 + 4*3) {
av_log(avctx, AV_LOG_ERROR, "Buffer too small for XSUB header.\n");
return -1;
}
if (h->num_rects != 1)
av_log(avctx, AV_LOG_WARNING, "Only single rects supported (%d in subtitle.)\n", h->num_rects);
#if FF_API_AVPICTURE
FF_DISABLE_DEPRECATION_WARNINGS
if (!h->rects[0]->data[0]) {
AVSubtitleRect *rect = h->rects[0];
int j;
for (j = 0; j < 4; j++) {
rect->data[j] = rect->pict.data[j];
rect->linesize[j] = rect->pict.linesize[j];
}
}
FF_ENABLE_DEPRECATION_WARNINGS
#endif
if (!h->rects[0]->data[0] || !h->rects[0]->data[1]) {
av_log(avctx, AV_LOG_WARNING, "No subtitle bitmap available.\n");
return -1;
}
if (h->rects[0]->nb_colors > 4)
av_log(avctx, AV_LOG_WARNING, "No more than 4 subtitle colors supported (%d found.)\n", h->rects[0]->nb_colors);
if (((uint32_t *)h->rects[0]->data[1])[0] & 0xff000000)
av_log(avctx, AV_LOG_WARNING, "Color index 0 is not transparent. Transparency will be messed up.\n");
if (make_tc(startTime, start_tc) || make_tc(endTime, end_tc)) {
av_log(avctx, AV_LOG_WARNING, "Time code >= 100 hours.\n");
return -1;
}
snprintf(buf, 28,
"[%02d:%02d:%02d.%03d-%02d:%02d:%02d.%03d]",
start_tc[3], start_tc[2], start_tc[1], start_tc[0],
end_tc[3], end_tc[2], end_tc[1], end_tc[0]);
width = FFALIGN(h->rects[0]->w, 2) + PADDING * 2;
height = FFALIGN(h->rects[0]->h, 2);
bytestream_put_le16(&hdr, width);
bytestream_put_le16(&hdr, height);
bytestream_put_le16(&hdr, h->rects[0]->x);
bytestream_put_le16(&hdr, h->rects[0]->y);
bytestream_put_le16(&hdr, h->rects[0]->x + width -1);
bytestream_put_le16(&hdr, h->rects[0]->y + height -1);
rlelenptr = hdr; 
hdr+=2;
for (i=0; i<4; i++)
bytestream_put_be24(&hdr, ((uint32_t *)h->rects[0]->data[1])[i]);
init_put_bits(&pb, hdr, bufsize - (hdr - buf) - 2);
if (xsub_encode_rle(&pb, h->rects[0]->data[0],
h->rects[0]->linesize[0] * 2,
h->rects[0]->w, (h->rects[0]->h + 1) >> 1))
return -1;
bytestream_put_le16(&rlelenptr, put_bits_count(&pb) >> 3); 
if (xsub_encode_rle(&pb, h->rects[0]->data[0] + h->rects[0]->linesize[0],
h->rects[0]->linesize[0] * 2,
h->rects[0]->w, h->rects[0]->h >> 1))
return -1;
if (h->rects[0]->h & 1) {
put_xsub_rle(&pb, h->rects[0]->w, PADDING_COLOR);
avpriv_align_put_bits(&pb);
}
flush_put_bits(&pb);
return hdr - buf + put_bits_count(&pb)/8;
}
static av_cold int xsub_encoder_init(AVCodecContext *avctx)
{
if (!avctx->codec_tag)
avctx->codec_tag = MKTAG('D','X','S','B');
avctx->bits_per_coded_sample = 4;
return 0;
}
AVCodec ff_xsub_encoder = {
.name = "xsub",
.long_name = NULL_IF_CONFIG_SMALL("DivX subtitles (XSUB)"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_XSUB,
.init = xsub_encoder_init,
.encode_sub = xsub_encode,
};
