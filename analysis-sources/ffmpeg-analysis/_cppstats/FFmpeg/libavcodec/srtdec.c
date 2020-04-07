#include "libavutil/avstring.h"
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/parseutils.h"
#include "avcodec.h"
#include "ass.h"
#include "htmlsubtitles.h"
static int srt_to_ass(AVCodecContext *avctx, AVBPrint *dst,
const char *in, int x1, int y1, int x2, int y2)
{
if (x1 >= 0 && y1 >= 0) {
if (x2 >= 0 && y2 >= 0 && (x2 != x1 || y2 != y1) && x2 >= x1 && y2 >= y1) {
const int cx = x1 + (x2 - x1)/2;
const int cy = y1 + (y2 - y1)/2;
const int scaled_x = cx * (int64_t)ASS_DEFAULT_PLAYRESX / 720;
const int scaled_y = cy * (int64_t)ASS_DEFAULT_PLAYRESY / 480;
av_bprintf(dst, "{\\an5}{\\pos(%d,%d)}", scaled_x, scaled_y);
} else {
const int scaled_x = x1 * (int64_t)ASS_DEFAULT_PLAYRESX / 720;
const int scaled_y = y1 * (int64_t)ASS_DEFAULT_PLAYRESY / 480;
av_bprintf(dst, "{\\an1}{\\pos(%d,%d)}", scaled_x, scaled_y);
}
}
return ff_htmlmarkup_to_ass(avctx, dst, in);
}
static int srt_decode_frame(AVCodecContext *avctx,
void *data, int *got_sub_ptr, AVPacket *avpkt)
{
AVSubtitle *sub = data;
AVBPrint buffer;
int x1 = -1, y1 = -1, x2 = -1, y2 = -1;
int size, ret;
const uint8_t *p = av_packet_get_side_data(avpkt, AV_PKT_DATA_SUBTITLE_POSITION, &size);
FFASSDecoderContext *s = avctx->priv_data;
if (p && size == 16) {
x1 = AV_RL32(p );
y1 = AV_RL32(p + 4);
x2 = AV_RL32(p + 8);
y2 = AV_RL32(p + 12);
}
if (avpkt->size <= 0)
return avpkt->size;
av_bprint_init(&buffer, 0, AV_BPRINT_SIZE_UNLIMITED);
ret = srt_to_ass(avctx, &buffer, avpkt->data, x1, y1, x2, y2);
if (ret >= 0)
ret = ff_ass_add_rect(sub, buffer.str, s->readorder++, 0, NULL, NULL);
av_bprint_finalize(&buffer, NULL);
if (ret < 0)
return ret;
*got_sub_ptr = sub->num_rects > 0;
return avpkt->size;
}
#if CONFIG_SRT_DECODER
AVCodec ff_srt_decoder = {
.name = "srt",
.long_name = NULL_IF_CONFIG_SMALL("SubRip subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_SUBRIP,
.init = ff_ass_subtitle_header_default,
.decode = srt_decode_frame,
.flush = ff_ass_decoder_flush,
.priv_data_size = sizeof(FFASSDecoderContext),
};
#endif
#if CONFIG_SUBRIP_DECODER
AVCodec ff_subrip_decoder = {
.name = "subrip",
.long_name = NULL_IF_CONFIG_SMALL("SubRip subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_SUBRIP,
.init = ff_ass_subtitle_header_default,
.decode = srt_decode_frame,
.flush = ff_ass_decoder_flush,
.priv_data_size = sizeof(FFASSDecoderContext),
};
#endif
