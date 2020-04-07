





















#include "config.h"
#include "libavutil/attributes.h"
#include "jpeg2000dsp.h"



static const float f_ict_params[4] = {
1.402f,
0.34413f,
0.71414f,
1.772f
};

static const int i_ict_params[4] = {
91881,
22553,
46802,
116130
};

static void ict_float(void *_src0, void *_src1, void *_src2, int csize)
{
float *src0 = _src0, *src1 = _src1, *src2 = _src2;
float i0f, i1f, i2f;
int i;

for (i = 0; i < csize; i++) {
i0f = *src0 + (f_ict_params[0] * *src2);
i1f = *src0 - (f_ict_params[1] * *src1)
- (f_ict_params[2] * *src2);
i2f = *src0 + (f_ict_params[3] * *src1);
*src0++ = i0f;
*src1++ = i1f;
*src2++ = i2f;
}
}

static void ict_int(void *_src0, void *_src1, void *_src2, int csize)
{
int32_t *src0 = _src0, *src1 = _src1, *src2 = _src2;
int32_t i0, i1, i2;
int i;

for (i = 0; i < csize; i++) {
i0 = *src0 + *src2 + ((int)((26345U * *src2) + (1 << 15)) >> 16);
i1 = *src0 - ((int)(((unsigned)i_ict_params[1] * *src1) + (1 << 15)) >> 16)
- ((int)(((unsigned)i_ict_params[2] * *src2) + (1 << 15)) >> 16);
i2 = *src0 + (2 * *src1) + ((int)((-14942U * *src1) + (1 << 15)) >> 16);
*src0++ = i0;
*src1++ = i1;
*src2++ = i2;
}
}

static void rct_int(void *_src0, void *_src1, void *_src2, int csize)
{
int32_t *src0 = _src0, *src1 = _src1, *src2 = _src2;
int32_t i0, i1, i2;
int i;

for (i = 0; i < csize; i++) {
i1 = *src0 - (*src2 + *src1 >> 2);
i0 = i1 + *src2;
i2 = i1 + *src1;
*src0++ = i0;
*src1++ = i1;
*src2++ = i2;
}
}

av_cold void ff_jpeg2000dsp_init(Jpeg2000DSPContext *c)
{
c->mct_decode[FF_DWT97] = ict_float;
c->mct_decode[FF_DWT53] = rct_int;
c->mct_decode[FF_DWT97_INT] = ict_int;

if (ARCH_X86)
ff_jpeg2000dsp_init_x86(c);
}
