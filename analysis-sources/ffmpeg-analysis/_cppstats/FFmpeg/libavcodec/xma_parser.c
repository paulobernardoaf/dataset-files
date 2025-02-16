#include "parser.h"
typedef struct XMAParserContext{
int skip_packets;
} XMAParserContext;
static int xma_parse(AVCodecParserContext *s1, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
XMAParserContext *s = s1->priv_data;
if (buf_size % 2048 == 0) {
int duration = 0, packet, nb_packets = buf_size / 2048;
for (packet = 0; packet < nb_packets; packet++) {
if (s->skip_packets == 0) {
duration += buf[packet * 2048] * 128;
s->skip_packets = buf[packet * 2048 + 3] + 1;
}
s->skip_packets--;
}
s1->duration = duration;
s1->key_frame = !!duration;
}
*poutbuf = buf;
*poutbuf_size = buf_size;
return buf_size;
}
AVCodecParser ff_xma_parser = {
.codec_ids = { AV_CODEC_ID_XMA2 },
.priv_data_size = sizeof(XMAParserContext),
.parser_parse = xma_parse,
};
