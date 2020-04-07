#include "bit_depth_template.c"
static void FUNCC(ff_h264_add_pixels4)(uint8_t *_dst, int16_t *_src, int stride)
{
int i;
pixel *dst = (pixel *) _dst;
dctcoef *src = (dctcoef *) _src;
stride /= sizeof(pixel);
for (i = 0; i < 4; i++) {
dst[0] += (unsigned)src[0];
dst[1] += (unsigned)src[1];
dst[2] += (unsigned)src[2];
dst[3] += (unsigned)src[3];
dst += stride;
src += 4;
}
memset(_src, 0, sizeof(dctcoef) * 16);
}
static void FUNCC(ff_h264_add_pixels8)(uint8_t *_dst, int16_t *_src, int stride)
{
int i;
pixel *dst = (pixel *) _dst;
dctcoef *src = (dctcoef *) _src;
stride /= sizeof(pixel);
for (i = 0; i < 8; i++) {
dst[0] += (unsigned)src[0];
dst[1] += (unsigned)src[1];
dst[2] += (unsigned)src[2];
dst[3] += (unsigned)src[3];
dst[4] += (unsigned)src[4];
dst[5] += (unsigned)src[5];
dst[6] += (unsigned)src[6];
dst[7] += (unsigned)src[7];
dst += stride;
src += 8;
}
memset(_src, 0, sizeof(dctcoef) * 64);
}
