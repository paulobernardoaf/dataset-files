




















#include "config.h"
#include "libavutil/attributes.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/vp3dsp.h"
#include "vp3dsp_mips.h"

#if HAVE_MSA
static av_cold void vp3dsp_init_msa(VP3DSPContext *c, int flags)
{
c->put_no_rnd_pixels_l2 = ff_put_no_rnd_pixels_l2_msa;

c->idct_add = ff_vp3_idct_add_msa;
c->idct_put = ff_vp3_idct_put_msa;
c->idct_dc_add = ff_vp3_idct_dc_add_msa;
c->v_loop_filter = ff_vp3_v_loop_filter_msa;
c->h_loop_filter = ff_vp3_h_loop_filter_msa;
}
#endif 

#if HAVE_MMI
static av_cold void vp3dsp_init_mmi(VP3DSPContext *c, int flags)
{
c->put_no_rnd_pixels_l2 = ff_put_no_rnd_pixels_l2_mmi;

c->idct_add = ff_vp3_idct_add_mmi;
c->idct_put = ff_vp3_idct_put_mmi;
c->idct_dc_add = ff_vp3_idct_dc_add_mmi;
}
#endif 

av_cold void ff_vp3dsp_init_mips(VP3DSPContext *c, int flags)
{
#if HAVE_MMI
vp3dsp_init_mmi(c, flags);
#endif 
#if HAVE_MSA
vp3dsp_init_msa(c, flags);
#endif 
}
