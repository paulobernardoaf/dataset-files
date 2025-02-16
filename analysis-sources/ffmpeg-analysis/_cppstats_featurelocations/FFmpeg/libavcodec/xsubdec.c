




















#include "libavutil/mathematics.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "get_bits.h"
#include "bytestream.h"

static av_cold int decode_init(AVCodecContext *avctx) {
avctx->pix_fmt = AV_PIX_FMT_PAL8;
return 0;
}

static const uint8_t tc_offsets[9] = { 0, 1, 3, 4, 6, 7, 9, 10, 11 };
static const uint8_t tc_muls[9] = { 10, 6, 10, 6, 10, 10, 10, 10, 1 };

static int64_t parse_timecode(const uint8_t *buf, int64_t packet_time) {
int i;
int64_t ms = 0;
if (buf[2] != ':' || buf[5] != ':' || buf[8] != '.')
return AV_NOPTS_VALUE;
for (i = 0; i < sizeof(tc_offsets); i++) {
uint8_t c = buf[tc_offsets[i]] - '0';
if (c > 9) return AV_NOPTS_VALUE;
ms = (ms + c) * tc_muls[i];
}
return ms - packet_time;
}

static int decode_frame(AVCodecContext *avctx, void *data, int *got_sub_ptr,
AVPacket *avpkt) {
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
AVSubtitle *sub = data;
const uint8_t *buf_end = buf + buf_size;
uint8_t *bitmap;
int w, h, x, y, i, ret;
int64_t packet_time = 0;
GetBitContext gb;
int has_alpha = avctx->codec_tag == MKTAG('D','X','S','A');


if (buf_size < 27 + 7 * 2 + 4 * (3 + has_alpha)) {
av_log(avctx, AV_LOG_ERROR, "coded frame size %d too small\n", buf_size);
return -1;
}


if (buf[0] != '[' || buf[13] != '-' || buf[26] != ']') {
av_log(avctx, AV_LOG_ERROR, "invalid time code\n");
return -1;
}
if (avpkt->pts != AV_NOPTS_VALUE)
packet_time = av_rescale_q(avpkt->pts, AV_TIME_BASE_Q, (AVRational){1, 1000});
sub->start_display_time = parse_timecode(buf + 1, packet_time);
sub->end_display_time = parse_timecode(buf + 14, packet_time);
buf += 27;


w = bytestream_get_le16(&buf);
h = bytestream_get_le16(&buf);
if (av_image_check_size(w, h, 0, avctx) < 0)
return -1;
x = bytestream_get_le16(&buf);
y = bytestream_get_le16(&buf);

bytestream_get_le16(&buf);
bytestream_get_le16(&buf);




bytestream_get_le16(&buf);

if (buf_end - buf < h + 3*4)
return AVERROR_INVALIDDATA;


sub->rects = av_mallocz(sizeof(*sub->rects));
if (!sub->rects)
return AVERROR(ENOMEM);

sub->rects[0] = av_mallocz(sizeof(*sub->rects[0]));
if (!sub->rects[0]) {
av_freep(&sub->rects);
return AVERROR(ENOMEM);
}
sub->rects[0]->x = x; sub->rects[0]->y = y;
sub->rects[0]->w = w; sub->rects[0]->h = h;
sub->rects[0]->type = SUBTITLE_BITMAP;
sub->rects[0]->linesize[0] = w;
sub->rects[0]->data[0] = av_malloc(w * h);
sub->rects[0]->nb_colors = 4;
sub->rects[0]->data[1] = av_mallocz(AVPALETTE_SIZE);
if (!sub->rects[0]->data[0] || !sub->rects[0]->data[1]) {
av_freep(&sub->rects[0]->data[1]);
av_freep(&sub->rects[0]->data[0]);
av_freep(&sub->rects[0]);
av_freep(&sub->rects);
return AVERROR(ENOMEM);
}
sub->num_rects = 1;


for (i = 0; i < sub->rects[0]->nb_colors; i++)
((uint32_t*)sub->rects[0]->data[1])[i] = bytestream_get_be24(&buf);

if (!has_alpha) {

for (i = 1; i < sub->rects[0]->nb_colors; i++)
((uint32_t *)sub->rects[0]->data[1])[i] |= 0xff000000;
} else {
for (i = 0; i < sub->rects[0]->nb_colors; i++)
((uint32_t *)sub->rects[0]->data[1])[i] |= (unsigned)*buf++ << 24;
}

#if FF_API_AVPICTURE
FF_DISABLE_DEPRECATION_WARNINGS
{
AVSubtitleRect *rect;
int j;
rect = sub->rects[0];
for (j = 0; j < 4; j++) {
rect->pict.data[j] = rect->data[j];
rect->pict.linesize[j] = rect->linesize[j];
}
}
FF_ENABLE_DEPRECATION_WARNINGS
#endif


if ((ret = init_get_bits8(&gb, buf, buf_end - buf)) < 0)
return ret;
bitmap = sub->rects[0]->data[0];
for (y = 0; y < h; y++) {

if (y == (h + 1) / 2) bitmap = sub->rects[0]->data[0] + w;
for (x = 0; x < w; ) {
int log2 = ff_log2_tab[show_bits(&gb, 8)];
int run = get_bits(&gb, 14 - 4 * (log2 >> 1));
int color = get_bits(&gb, 2);
run = FFMIN(run, w - x);

if (!run) run = w - x;
memset(bitmap, color, run);
bitmap += run;
x += run;
}

bitmap += w;
align_get_bits(&gb);
}
*got_sub_ptr = 1;
return buf_size;
}

AVCodec ff_xsub_decoder = {
.name = "xsub",
.long_name = NULL_IF_CONFIG_SMALL("XSUB"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_XSUB,
.init = decode_init,
.decode = decode_frame,
};
