
























#include <stdint.h>
#include "parser.h"

#define LATM_HEADER 0x56e000 
#define LATM_MASK 0xFFE000 
#define LATM_SIZE_MASK 0x001FFF 

typedef struct LATMParseContext{
ParseContext pc;
int count;
} LATMParseContext;





static int latm_find_frame_end(AVCodecParserContext *s1, const uint8_t *buf,
int buf_size)
{
LATMParseContext *s = s1->priv_data;
ParseContext *pc = &s->pc;
int pic_found, i;
uint32_t state;

pic_found = pc->frame_start_found;
state = pc->state;

if (!pic_found) {
for (i = 0; i < buf_size; i++) {
state = (state<<8) | buf[i];
if ((state & LATM_MASK) == LATM_HEADER) {
i++;
s->count = -i;
pic_found = 1;
break;
}
}
}

if (pic_found) {

if (buf_size == 0)
return 0;
if ((state & LATM_SIZE_MASK) - s->count <= buf_size) {
pc->frame_start_found = 0;
pc->state = -1;
return (state & LATM_SIZE_MASK) - s->count;
}
}

s->count += buf_size;
pc->frame_start_found = pic_found;
pc->state = state;

return END_NOT_FOUND;
}

static int latm_parse(AVCodecParserContext *s1, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
LATMParseContext *s = s1->priv_data;
ParseContext *pc = &s->pc;
int next;

if (s1->flags & PARSER_FLAG_COMPLETE_FRAMES) {
next = buf_size;
} else {
next = latm_find_frame_end(s1, buf, buf_size);

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

AVCodecParser ff_aac_latm_parser = {
.codec_ids = { AV_CODEC_ID_AAC_LATM },
.priv_data_size = sizeof(LATMParseContext),
.parser_parse = latm_parse,
.parser_close = ff_parse_close
};
