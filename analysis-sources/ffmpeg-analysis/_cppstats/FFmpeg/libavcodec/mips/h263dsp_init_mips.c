#include "h263dsp_mips.h"
#if HAVE_MSA
static av_cold void h263dsp_init_msa(H263DSPContext *c)
{
c->h263_h_loop_filter = ff_h263_h_loop_filter_msa;
c->h263_v_loop_filter = ff_h263_v_loop_filter_msa;
}
#endif 
av_cold void ff_h263dsp_init_mips(H263DSPContext *c)
{
#if HAVE_MSA
h263dsp_init_msa(c);
#endif 
}
