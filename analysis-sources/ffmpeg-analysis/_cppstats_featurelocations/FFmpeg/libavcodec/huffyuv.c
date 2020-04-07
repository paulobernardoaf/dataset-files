





























#include <stdint.h>

#include "libavutil/mem.h"

#include "avcodec.h"
#include "bswapdsp.h"
#include "huffyuv.h"

int ff_huffyuv_generate_bits_table(uint32_t *dst, const uint8_t *len_table, int n)
{
int len, index;
uint32_t bits = 0;

for (len = 32; len > 0; len--) {
for (index = 0; index < n; index++) {
if (len_table[index] == len)
dst[index] = bits++;
}
if (bits & 1) {
av_log(NULL, AV_LOG_ERROR, "Error generating huffman table\n");
return -1;
}
bits >>= 1;
}
return 0;
}

av_cold int ff_huffyuv_alloc_temp(HYuvContext *s)
{
int i;

for (i=0; i<3; i++) {
s->temp[i]= av_malloc(4*s->width + 16);
if (!s->temp[i])
return AVERROR(ENOMEM);
s->temp16[i] = (uint16_t*)s->temp[i];
}
return 0;
}

av_cold void ff_huffyuv_common_init(AVCodecContext *avctx)
{
HYuvContext *s = avctx->priv_data;

s->avctx = avctx;
s->flags = avctx->flags;

ff_bswapdsp_init(&s->bdsp);

s->width = avctx->width;
s->height = avctx->height;

av_assert1(s->width > 0 && s->height > 0);
}

av_cold void ff_huffyuv_common_end(HYuvContext *s)
{
int i;

for(i = 0; i < 3; i++) {
av_freep(&s->temp[i]);
s->temp16[i] = NULL;
}
}
