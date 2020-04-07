#include "avcodec.h"
#include "ass.h"
#include "libavutil/bprint.h"
static int subviewer_event_to_ass(AVBPrint *buf, const char *p)
{
while (*p) {
if (!strncmp(p, "[br]", 4)) {
av_bprintf(buf, "\\N");
p += 4;
} else {
if (p[0] == '\n' && p[1])
av_bprintf(buf, "\\N");
else if (*p != '\n' && *p != '\r')
av_bprint_chars(buf, *p, 1);
p++;
}
}
return 0;
}
static int subviewer_decode_frame(AVCodecContext *avctx,
void *data, int *got_sub_ptr, AVPacket *avpkt)
{
int ret = 0;
AVSubtitle *sub = data;
const char *ptr = avpkt->data;
FFASSDecoderContext *s = avctx->priv_data;
AVBPrint buf;
av_bprint_init(&buf, 0, AV_BPRINT_SIZE_UNLIMITED);
if (ptr && avpkt->size > 0 && !subviewer_event_to_ass(&buf, ptr))
ret = ff_ass_add_rect(sub, buf.str, s->readorder++, 0, NULL, NULL);
av_bprint_finalize(&buf, NULL);
if (ret < 0)
return ret;
*got_sub_ptr = sub->num_rects > 0;
return avpkt->size;
}
AVCodec ff_subviewer_decoder = {
.name = "subviewer",
.long_name = NULL_IF_CONFIG_SMALL("SubViewer subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_SUBVIEWER,
.decode = subviewer_decode_frame,
.init = ff_ass_subtitle_header_default,
.flush = ff_ass_decoder_flush,
.priv_data_size = sizeof(FFASSDecoderContext),
};
