



























#include "parser.h"

typedef struct MJPEGParserContext{
ParseContext pc;
int size;
}MJPEGParserContext;





static int find_frame_end(MJPEGParserContext *m, const uint8_t *buf, int buf_size){
ParseContext *pc= &m->pc;
int vop_found, i;
uint32_t state;

vop_found= pc->frame_start_found;
state= pc->state;

i=0;
if(!vop_found){
for(i=0; i<buf_size;){
state= (state<<8) | buf[i];
if(state>=0xFFC00000 && state<=0xFFFEFFFF){
if(state>=0xFFD8FFC0 && state<=0xFFD8FFFF){
i++;
vop_found=1;
break;
}else if(state<0xFFD00000 || state>0xFFD9FFFF){
m->size= (state&0xFFFF)-1;
}
}
if(m->size>0){
int size= FFMIN(buf_size-i, m->size);
i+=size;
m->size-=size;
state=0;
continue;
}else
i++;
}
}

if(vop_found){

if (buf_size == 0)
return 0;
for(; i<buf_size;){
state= (state<<8) | buf[i];
if(state>=0xFFC00000 && state<=0xFFFEFFFF){
if(state>=0xFFD8FFC0 && state<=0xFFD8FFFF){
pc->frame_start_found=0;
pc->state=0;
return i-3;
} else if(state<0xFFD00000 || state>0xFFD9FFFF){
m->size= (state&0xFFFF)-1;
if (m->size >= 0x8000)
m->size = 0;
}
}
if(m->size>0){
int size= FFMIN(buf_size-i, m->size);
i+=size;
m->size-=size;
state=0;
continue;
}else
i++;
}
}
pc->frame_start_found= vop_found;
pc->state= state;
return END_NOT_FOUND;
}

static int jpeg_parse(AVCodecParserContext *s,
AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
MJPEGParserContext *m = s->priv_data;
ParseContext *pc = &m->pc;
int next;

if(s->flags & PARSER_FLAG_COMPLETE_FRAMES){
next= buf_size;
}else{
next= find_frame_end(m, buf, buf_size);

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


AVCodecParser ff_mjpeg_parser = {
.codec_ids = { AV_CODEC_ID_MJPEG, AV_CODEC_ID_JPEGLS },
.priv_data_size = sizeof(MJPEGParserContext),
.parser_parse = jpeg_parse,
.parser_close = ff_parse_close,
};
