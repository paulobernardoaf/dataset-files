

























#include "parser.h"
#include "dnxhddata.h"

typedef struct {
ParseContext pc;
int cur_byte;
int remaining;
int w, h;
} DNXHDParserContext;

static int dnxhd_find_frame_end(DNXHDParserContext *dctx,
const uint8_t *buf, int buf_size)
{
ParseContext *pc = &dctx->pc;
uint64_t state = pc->state64;
int pic_found = pc->frame_start_found;
int i = 0;

if (!pic_found) {
for (i = 0; i < buf_size; i++) {
state = (state << 8) | buf[i];
if (ff_dnxhd_check_header_prefix(state & 0xffffffffff00LL) != 0) {
i++;
pic_found = 1;
dctx->cur_byte = 0;
dctx->remaining = 0;
break;
}
}
}

if (pic_found && !dctx->remaining) {
if (!buf_size) 
return 0;
for (; i < buf_size; i++) {
dctx->cur_byte++;
state = (state << 8) | buf[i];

if (dctx->cur_byte == 24) {
dctx->h = (state >> 32) & 0xFFFF;
} else if (dctx->cur_byte == 26) {
dctx->w = (state >> 32) & 0xFFFF;
} else if (dctx->cur_byte == 42) {
int cid = (state >> 32) & 0xFFFFFFFF;
int remaining;

if (cid <= 0)
continue;

remaining = avpriv_dnxhd_get_frame_size(cid);
if (remaining <= 0) {
remaining = avpriv_dnxhd_get_hr_frame_size(cid, dctx->w, dctx->h);
if (remaining <= 0)
continue;
}
remaining += i - 47;
dctx->remaining = remaining;
if (buf_size >= dctx->remaining) {
pc->frame_start_found = 0;
pc->state64 = -1;
dctx->cur_byte = 0;
dctx->remaining = 0;
return remaining;
} else {
dctx->remaining -= buf_size;

state = -1;
dctx->cur_byte += buf_size - i;
break;
}
}
}
} else if (pic_found) {
if (dctx->remaining > buf_size) {
dctx->remaining -= buf_size;
} else {
int remaining = dctx->remaining;

pc->frame_start_found = 0;
pc->state64 = -1;
dctx->cur_byte = 0;
dctx->remaining = 0;
return remaining;
}
}
pc->frame_start_found = pic_found;
pc->state64 = state;
return END_NOT_FOUND;
}

static int dnxhd_parse(AVCodecParserContext *s,
AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
DNXHDParserContext *dctx = s->priv_data;
ParseContext *pc = &dctx->pc;
int next;

if (s->flags & PARSER_FLAG_COMPLETE_FRAMES) {
next = buf_size;
} else {
next = dnxhd_find_frame_end(dctx, buf, buf_size);
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

AVCodecParser ff_dnxhd_parser = {
.codec_ids = { AV_CODEC_ID_DNXHD },
.priv_data_size = sizeof(DNXHDParserContext),
.parser_parse = dnxhd_parse,
.parser_close = ff_parse_close,
};
