#include "config.h"
#include "libavutil/attributes.h"
#include "wmv2dsp_mips.h"
#if HAVE_MMI
static av_cold void wmv2dsp_init_mmi(WMV2DSPContext *c)
{
c->idct_add = ff_wmv2_idct_add_mmi;
c->idct_put = ff_wmv2_idct_put_mmi;
}
#endif 
av_cold void ff_wmv2dsp_init_mips(WMV2DSPContext *c)
{
#if HAVE_MMI
wmv2dsp_init_mmi(c);
#endif 
}
