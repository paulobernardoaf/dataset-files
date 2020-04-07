

























#include "parser.h"
#include "libavutil/intreadwrite.h"

typedef struct RV34ParseContext {
ParseContext pc;
int64_t key_dts;
int key_pts;
} RV34ParseContext;

static const int rv_to_av_frame_type[4] = {
AV_PICTURE_TYPE_I, AV_PICTURE_TYPE_I, AV_PICTURE_TYPE_P, AV_PICTURE_TYPE_B,
};

static int rv34_parse(AVCodecParserContext *s,
AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
RV34ParseContext *pc = s->priv_data;
int type, pts, hdr;

if (buf_size < 13 + *buf * 8) {
*poutbuf = buf;
*poutbuf_size = buf_size;
return buf_size;
}

hdr = AV_RB32(buf + 9 + *buf * 8);
if (avctx->codec_id == AV_CODEC_ID_RV30) {
type = (hdr >> 27) & 3;
pts = (hdr >> 7) & 0x1FFF;
} else {
type = (hdr >> 29) & 3;
pts = (hdr >> 6) & 0x1FFF;
}

if (type != 3 && s->pts != AV_NOPTS_VALUE) {
pc->key_dts = s->pts;
pc->key_pts = pts;
} else {
if (type != 3)
s->pts = pc->key_dts + ((pts - pc->key_pts) & 0x1FFF);
else
s->pts = pc->key_dts - ((pc->key_pts - pts) & 0x1FFF);
}
s->pict_type = rv_to_av_frame_type[type];

*poutbuf = buf;
*poutbuf_size = buf_size;
return buf_size;
}

#if CONFIG_RV30_PARSER
AVCodecParser ff_rv30_parser = {
.codec_ids = { AV_CODEC_ID_RV30 },
.priv_data_size = sizeof(RV34ParseContext),
.parser_parse = rv34_parse,
};
#endif

#if CONFIG_RV40_PARSER
AVCodecParser ff_rv40_parser = {
.codec_ids = { AV_CODEC_ID_RV40 },
.priv_data_size = sizeof(RV34ParseContext),
.parser_parse = rv34_parse,
};
#endif
