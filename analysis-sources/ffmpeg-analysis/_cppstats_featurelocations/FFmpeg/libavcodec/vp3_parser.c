



















#include "parser.h"

static int parse(AVCodecParserContext *s,
AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
if (avctx->codec_id == AV_CODEC_ID_THEORA)
s->pict_type = (buf[0] & 0x40) ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_I;
else
s->pict_type = (buf[0] & 0x80) ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_I;

*poutbuf = buf;
*poutbuf_size = buf_size;
return buf_size;
}

AVCodecParser ff_vp3_parser = {
.codec_ids = {
AV_CODEC_ID_THEORA, AV_CODEC_ID_VP3,
AV_CODEC_ID_VP6, AV_CODEC_ID_VP6F, AV_CODEC_ID_VP6A
},
.parser_parse = parse,
};
