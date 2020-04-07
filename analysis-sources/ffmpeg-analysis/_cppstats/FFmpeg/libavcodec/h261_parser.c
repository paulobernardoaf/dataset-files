#include "parser.h"
static int h261_find_frame_end(ParseContext *pc, AVCodecContext *avctx,
const uint8_t *buf, int buf_size)
{
int vop_found, i, j;
uint32_t state;
vop_found = pc->frame_start_found;
state = pc->state;
for (i = 0; i < buf_size && !vop_found; i++) {
state = (state << 8) | buf[i];
for (j = 0; j < 8; j++) {
if (((state >> j) & 0xFFFFF0) == 0x000100) {
vop_found = 1;
break;
}
}
}
if (vop_found) {
for (; i < buf_size; i++) {
state = (state << 8) | buf[i];
for (j = 0; j < 8; j++) {
if (((state >> j) & 0xFFFFF0) == 0x000100) {
pc->frame_start_found = 0;
pc->state = (state >> (3 * 8)) + 0xFF00;
return i - 2;
}
}
}
}
pc->frame_start_found = vop_found;
pc->state = state;
return END_NOT_FOUND;
}
static int h261_parse(AVCodecParserContext *s,
AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
ParseContext *pc = s->priv_data;
int next;
if (s->flags & PARSER_FLAG_COMPLETE_FRAMES) {
next = buf_size;
} else {
next = h261_find_frame_end(pc, avctx, buf, buf_size);
if (ff_combine_frame(pc, next, &buf, &buf_size) < 0) {
*poutbuf = NULL;
*poutbuf_size = 0;
return buf_size;
}
}
*poutbuf = buf;
*poutbuf_size = buf_size;
return next;
}
AVCodecParser ff_h261_parser = {
.codec_ids = { AV_CODEC_ID_H261 },
.priv_data_size = sizeof(ParseContext),
.parser_parse = h261_parse,
.parser_close = ff_parse_close,
};
