#include <stdint.h>
#include "libavutil/rational.h"
#include "avcodec.h"
#include "bytestream.h"
#include "canopus.h"
int ff_canopus_parse_info_tag(AVCodecContext *avctx,
const uint8_t *src, size_t size)
{
GetByteContext gbc;
int par_x, par_y, field_order;
bytestream2_init(&gbc, src, size);
bytestream2_skip(&gbc, 8); 
par_x = bytestream2_get_le32(&gbc);
par_y = bytestream2_get_le32(&gbc);
if (par_x && par_y)
av_reduce(&avctx->sample_aspect_ratio.num,
&avctx->sample_aspect_ratio.den,
par_x, par_y, 255);
if (size == 0x18)
return 0;
bytestream2_skip(&gbc, 16); 
bytestream2_skip(&gbc, 8); 
field_order = bytestream2_get_le32(&gbc);
switch (field_order) {
case 0: avctx->field_order = AV_FIELD_TT; break;
case 1: avctx->field_order = AV_FIELD_BB; break;
case 2: avctx->field_order = AV_FIELD_PROGRESSIVE; break;
}
return 0;
}
