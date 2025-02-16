#include "parser.h"
#include "g723_1.h"
typedef struct G723_1ParseContext {
ParseContext pc;
} G723_1ParseContext;
static int g723_1_parse(AVCodecParserContext *s1, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
G723_1ParseContext *s = s1->priv_data;
ParseContext *pc = &s->pc;
int next = END_NOT_FOUND;
if (buf_size > 0)
next = frame_size[buf[0] & 3] * FFMAX(1, avctx->channels);
if (ff_combine_frame(pc, next, &buf, &buf_size) < 0 || !buf_size) {
*poutbuf = NULL;
*poutbuf_size = 0;
return buf_size;
}
s1->duration = 240;
*poutbuf = buf;
*poutbuf_size = buf_size;
return next;
}
AVCodecParser ff_g723_1_parser = {
.codec_ids = { AV_CODEC_ID_G723_1 },
.priv_data_size = sizeof(G723_1ParseContext),
.parser_parse = g723_1_parse,
.parser_close = ff_parse_close,
};
