

























#include "parser.h"
#include "h263_parser.h"

int ff_h263_find_frame_end(ParseContext *pc, const uint8_t *buf, int buf_size){
int vop_found, i;
uint32_t state;

vop_found= pc->frame_start_found;
state= pc->state;

i=0;
if(!vop_found){
for(i=0; i<buf_size; i++){
state= (state<<8) | buf[i];
if(state>>(32-22) == 0x20){
i++;
vop_found=1;
break;
}
}
}

if(vop_found){
for(; i<buf_size; i++){
state= (state<<8) | buf[i];
if(state>>(32-22) == 0x20){
pc->frame_start_found=0;
pc->state=-1;
return i-3;
}
}
}
pc->frame_start_found= vop_found;
pc->state= state;

return END_NOT_FOUND;
}

static int h263_parse(AVCodecParserContext *s,
AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
ParseContext *pc = s->priv_data;
int next;

if (s->flags & PARSER_FLAG_COMPLETE_FRAMES) {
next = buf_size;
} else {
next= ff_h263_find_frame_end(pc, buf, buf_size);

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

AVCodecParser ff_h263_parser = {
.codec_ids = { AV_CODEC_ID_H263 },
.priv_data_size = sizeof(ParseContext),
.parser_parse = h263_parse,
.parser_close = ff_parse_close,
};
