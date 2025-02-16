#include "resample.h"
#define TEMPLATE_RESAMPLE_S16
#include "resample_template.c"
#undef TEMPLATE_RESAMPLE_S16
#define TEMPLATE_RESAMPLE_S32
#include "resample_template.c"
#undef TEMPLATE_RESAMPLE_S32
#define TEMPLATE_RESAMPLE_FLT
#include "resample_template.c"
#undef TEMPLATE_RESAMPLE_FLT
#define TEMPLATE_RESAMPLE_DBL
#include "resample_template.c"
#undef TEMPLATE_RESAMPLE_DBL
void swri_resample_dsp_init(ResampleContext *c)
{
switch(c->format){
case AV_SAMPLE_FMT_S16P:
c->dsp.resample_one = resample_one_int16;
c->dsp.resample_common = resample_common_int16;
c->dsp.resample_linear = resample_linear_int16;
break;
case AV_SAMPLE_FMT_S32P:
c->dsp.resample_one = resample_one_int32;
c->dsp.resample_common = resample_common_int32;
c->dsp.resample_linear = resample_linear_int32;
break;
case AV_SAMPLE_FMT_FLTP:
c->dsp.resample_one = resample_one_float;
c->dsp.resample_common = resample_common_float;
c->dsp.resample_linear = resample_linear_float;
break;
case AV_SAMPLE_FMT_DBLP:
c->dsp.resample_one = resample_one_double;
c->dsp.resample_common = resample_common_double;
c->dsp.resample_linear = resample_linear_double;
break;
}
if (ARCH_X86) swri_resample_dsp_x86_init(c);
else if (ARCH_ARM) swri_resample_dsp_arm_init(c);
else if (ARCH_AARCH64) swri_resample_dsp_aarch64_init(c);
}
