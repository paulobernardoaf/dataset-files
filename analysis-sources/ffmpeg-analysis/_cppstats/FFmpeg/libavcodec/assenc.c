#include <string.h>
#include "avcodec.h"
#include "ass.h"
#include "libavutil/avstring.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"
typedef struct {
int id; 
} ASSEncodeContext;
static av_cold int ass_encode_init(AVCodecContext *avctx)
{
avctx->extradata = av_malloc(avctx->subtitle_header_size + 1);
if (!avctx->extradata)
return AVERROR(ENOMEM);
memcpy(avctx->extradata, avctx->subtitle_header, avctx->subtitle_header_size);
avctx->extradata_size = avctx->subtitle_header_size;
avctx->extradata[avctx->extradata_size] = 0;
return 0;
}
static int ass_encode_frame(AVCodecContext *avctx,
unsigned char *buf, int bufsize,
const AVSubtitle *sub)
{
ASSEncodeContext *s = avctx->priv_data;
int i, len, total_len = 0;
for (i=0; i<sub->num_rects; i++) {
char ass_line[2048];
const char *ass = sub->rects[i]->ass;
long int layer;
char *p;
if (sub->rects[i]->type != SUBTITLE_ASS) {
av_log(avctx, AV_LOG_ERROR, "Only SUBTITLE_ASS type supported.\n");
return AVERROR(EINVAL);
}
#if FF_API_ASS_TIMING
if (!strncmp(ass, "Dialogue: ", 10)) {
if (i > 0) {
av_log(avctx, AV_LOG_ERROR, "ASS encoder supports only one "
"ASS rectangle field.\n");
return AVERROR_INVALIDDATA;
}
ass += 10; 
layer = strtol(ass, &p, 10);
#define SKIP_ENTRY(ptr) do { char *sep = strchr(ptr, ','); if (sep) ptr = sep + 1; } while (0)
SKIP_ENTRY(p); 
SKIP_ENTRY(p); 
SKIP_ENTRY(p); 
snprintf(ass_line, sizeof(ass_line), "%d,%ld,%s", ++s->id, layer, p);
ass_line[strcspn(ass_line, "\r\n")] = 0;
ass = ass_line;
}
#endif
len = av_strlcpy(buf+total_len, ass, bufsize-total_len);
if (len > bufsize-total_len-1) {
av_log(avctx, AV_LOG_ERROR, "Buffer too small for ASS event.\n");
return AVERROR(EINVAL);
}
total_len += len;
}
return total_len;
}
#if CONFIG_SSA_ENCODER
AVCodec ff_ssa_encoder = {
.name = "ssa",
.long_name = NULL_IF_CONFIG_SMALL("ASS (Advanced SubStation Alpha) subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_ASS,
.init = ass_encode_init,
.encode_sub = ass_encode_frame,
.priv_data_size = sizeof(ASSEncodeContext),
};
#endif
#if CONFIG_ASS_ENCODER
AVCodec ff_ass_encoder = {
.name = "ass",
.long_name = NULL_IF_CONFIG_SMALL("ASS (Advanced SubStation Alpha) subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_ASS,
.init = ass_encode_init,
.encode_sub = ass_encode_frame,
.priv_data_size = sizeof(ASSEncodeContext),
};
#endif
