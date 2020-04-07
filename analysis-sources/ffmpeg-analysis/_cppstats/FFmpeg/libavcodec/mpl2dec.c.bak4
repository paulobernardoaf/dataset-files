


























#include "avcodec.h"
#include "ass.h"
#include "libavutil/bprint.h"

static int mpl2_event_to_ass(AVBPrint *buf, const char *p)
{
if (*p == ' ')
p++;

while (*p) {
int got_style = 0;

while (*p && strchr("/\\_", *p)) {
if (*p == '/') av_bprintf(buf, "{\\i1}");
else if (*p == '\\') av_bprintf(buf, "{\\b1}");
else if (*p == '_') av_bprintf(buf, "{\\u1}");
got_style = 1;
p++;
}

while (*p && *p != '|') {
if (*p != '\r' && *p != '\n')
av_bprint_chars(buf, *p, 1);
p++;
}

if (*p == '|') {
if (got_style)
av_bprintf(buf, "{\\r}");
av_bprintf(buf, "\\N");
p++;
}
}

return 0;
}

static int mpl2_decode_frame(AVCodecContext *avctx, void *data,
int *got_sub_ptr, AVPacket *avpkt)
{
int ret = 0;
AVBPrint buf;
AVSubtitle *sub = data;
const char *ptr = avpkt->data;
FFASSDecoderContext *s = avctx->priv_data;

av_bprint_init(&buf, 0, AV_BPRINT_SIZE_UNLIMITED);
if (ptr && avpkt->size > 0 && *ptr && !mpl2_event_to_ass(&buf, ptr))
ret = ff_ass_add_rect(sub, buf.str, s->readorder++, 0, NULL, NULL);
av_bprint_finalize(&buf, NULL);
if (ret < 0)
return ret;
*got_sub_ptr = sub->num_rects > 0;
return avpkt->size;
}

AVCodec ff_mpl2_decoder = {
.name = "mpl2",
.long_name = NULL_IF_CONFIG_SMALL("MPL2 subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_MPL2,
.decode = mpl2_decode_frame,
.init = ff_ass_subtitle_header_default,
.flush = ff_ass_decoder_flush,
.priv_data_size = sizeof(FFASSDecoderContext),
};
