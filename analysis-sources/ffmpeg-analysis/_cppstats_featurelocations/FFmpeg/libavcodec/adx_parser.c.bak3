


























#include "libavutil/intreadwrite.h"
#include "parser.h"
#include "adx.h"

typedef struct ADXParseContext {
ParseContext pc;
int header_size;
int block_size;
int remaining;
} ADXParseContext;

static int adx_parse(AVCodecParserContext *s1,
AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
ADXParseContext *s = s1->priv_data;
ParseContext *pc = &s->pc;
int next = END_NOT_FOUND;
int i;
uint64_t state = pc->state64;

if (!s->header_size) {
for (i = 0; i < buf_size; i++) {
state = (state << 8) | buf[i];

if ((state & 0xFFFF0000FFFFFF00) == 0x8000000003120400ULL) {
int channels = state & 0xFF;
int header_size = ((state >> 32) & 0xFFFF) + 4;
if (channels > 0 && header_size >= 8) {
s->header_size = header_size;
s->block_size = BLOCK_SIZE * channels;
s->remaining = i - 7 + s->header_size + s->block_size;
break;
}
}
}
pc->state64 = state;
}

if (s->header_size) {
if (!s->remaining)
s->remaining = s->block_size;
if (s->remaining <= buf_size) {
next = s->remaining;
s->remaining = 0;
} else
s->remaining -= buf_size;
}

if (ff_combine_frame(pc, next, &buf, &buf_size) < 0 || !buf_size) {
*poutbuf = NULL;
*poutbuf_size = 0;
return buf_size;
}

s1->duration = BLOCK_SAMPLES;

*poutbuf = buf;
*poutbuf_size = buf_size;
return next;
}

AVCodecParser ff_adx_parser = {
.codec_ids = { AV_CODEC_ID_ADPCM_ADX },
.priv_data_size = sizeof(ADXParseContext),
.parser_parse = adx_parse,
.parser_close = ff_parse_close,
};
