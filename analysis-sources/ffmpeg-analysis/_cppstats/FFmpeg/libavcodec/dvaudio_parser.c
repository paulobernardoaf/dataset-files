#include "parser.h"
#include "dvaudio.h"
static int dvaudio_parse(AVCodecParserContext *s1, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
if (buf_size >= 248)
s1->duration = dv_get_audio_sample_count(buf + 244, avctx->block_align == 8640);
*poutbuf = buf;
*poutbuf_size = buf_size;
return buf_size;
}
AVCodecParser ff_dvaudio_parser = {
.codec_ids = { AV_CODEC_ID_DVAUDIO },
.parser_parse = dvaudio_parse,
};
