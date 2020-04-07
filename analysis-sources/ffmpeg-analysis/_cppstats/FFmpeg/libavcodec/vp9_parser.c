#include "libavutil/intreadwrite.h"
#include "libavcodec/get_bits.h"
#include "parser.h"
static int parse(AVCodecParserContext *ctx,
AVCodecContext *avctx,
const uint8_t **out_data, int *out_size,
const uint8_t *data, int size)
{
GetBitContext gb;
int res, profile, keyframe;
*out_data = data;
*out_size = size;
if (!size || (res = init_get_bits8(&gb, data, size)) < 0)
return size; 
get_bits(&gb, 2); 
profile = get_bits1(&gb);
profile |= get_bits1(&gb) << 1;
if (profile == 3) profile += get_bits1(&gb);
if (profile > 3)
return size;
avctx->profile = profile;
if (get_bits1(&gb)) {
keyframe = 0;
} else {
keyframe = !get_bits1(&gb);
}
if (!keyframe) {
ctx->pict_type = AV_PICTURE_TYPE_P;
ctx->key_frame = 0;
} else {
ctx->pict_type = AV_PICTURE_TYPE_I;
ctx->key_frame = 1;
}
return size;
}
AVCodecParser ff_vp9_parser = {
.codec_ids = { AV_CODEC_ID_VP9 },
.parser_parse = parse,
};
