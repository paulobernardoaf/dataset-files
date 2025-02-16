#include "parser.h"
#define SLICE_MAX_START_CODE 0x000001af
#define ISPIC(x) ((x) == 0xB3 || (x) == 0xB6)
#define ISUNIT(x) ((x) == 0xB0 || (x) == 0xB1 || (x) == 0xB2 || ISPIC(x))
static int avs2_find_frame_end(ParseContext *pc, const uint8_t *buf, int buf_size)
{
int pic_found = pc->frame_start_found;
uint32_t state = pc->state;
int cur = 0;
if (!pic_found) {
for (; cur < buf_size; ++cur) {
state = (state<<8) | buf[cur];
if (ISUNIT(buf[cur])){
++cur;
pic_found = 1;
break;
}
}
}
if (pic_found) {
if (!buf_size)
return END_NOT_FOUND;
for (; cur < buf_size; ++cur) {
state = (state << 8) | buf[cur];
if ((state & 0xFFFFFF00) == 0x100 && state > SLICE_MAX_START_CODE) {
pc->frame_start_found = 0;
pc->state = -1;
return cur - 3;
}
}
}
pc->frame_start_found = pic_found;
pc->state = state;
return END_NOT_FOUND;
}
static int avs2_parse(AVCodecParserContext *s, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
ParseContext *pc = s->priv_data;
int next;
if (s->flags & PARSER_FLAG_COMPLETE_FRAMES) {
next = buf_size;
} else {
next = avs2_find_frame_end(pc, buf, buf_size);
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
AVCodecParser ff_avs2_parser = {
.codec_ids = { AV_CODEC_ID_AVS2 },
.priv_data_size = sizeof(ParseContext),
.parser_parse = avs2_parse,
.parser_close = ff_parse_close,
.split = ff_mpeg4video_split,
};
