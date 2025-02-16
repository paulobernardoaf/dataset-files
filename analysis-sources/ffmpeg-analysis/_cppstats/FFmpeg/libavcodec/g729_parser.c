#include "libavutil/avassert.h"
#include "parser.h"
#include "g729.h"
typedef struct G729ParseContext {
ParseContext pc;
int block_size;
int duration;
int remaining;
} G729ParseContext;
static int g729_parse(AVCodecParserContext *s1, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
G729ParseContext *s = s1->priv_data;
ParseContext *pc = &s->pc;
int next;
if (!s->block_size) {
s->block_size = (avctx->bit_rate < 8000) ? G729D_6K4_BLOCK_SIZE : G729_8K_BLOCK_SIZE;
if (avctx->codec_id == AV_CODEC_ID_ACELP_KELVIN)
s->block_size++;
s->block_size *= avctx->channels;
s->duration = avctx->frame_size;
}
if (!s->block_size) {
*poutbuf = buf;
*poutbuf_size = buf_size;
return buf_size;
}
if (!s->remaining)
s->remaining = s->block_size;
if (s->remaining <= buf_size) {
next = s->remaining;
s->remaining = 0;
} else {
next = END_NOT_FOUND;
s->remaining -= buf_size;
}
if (ff_combine_frame(pc, next, &buf, &buf_size) < 0 || !buf_size) {
*poutbuf = NULL;
*poutbuf_size = 0;
return buf_size;
}
s1->duration = s->duration;
*poutbuf = buf;
*poutbuf_size = buf_size;
return next;
}
AVCodecParser ff_g729_parser = {
.codec_ids = { AV_CODEC_ID_G729, AV_CODEC_ID_ACELP_KELVIN },
.priv_data_size = sizeof(G729ParseContext),
.parser_parse = g729_parse,
.parser_close = ff_parse_close,
};
