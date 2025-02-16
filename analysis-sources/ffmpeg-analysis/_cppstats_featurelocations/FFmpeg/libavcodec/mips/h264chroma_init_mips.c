




















#include "h264chroma_mips.h"

#if HAVE_MSA
static av_cold void h264chroma_init_msa(H264ChromaContext *c, int bit_depth)
{
const int high_bit_depth = bit_depth > 8;

if (!high_bit_depth) {
c->put_h264_chroma_pixels_tab[0] = ff_put_h264_chroma_mc8_msa;
c->put_h264_chroma_pixels_tab[1] = ff_put_h264_chroma_mc4_msa;
c->put_h264_chroma_pixels_tab[2] = ff_put_h264_chroma_mc2_msa;

c->avg_h264_chroma_pixels_tab[0] = ff_avg_h264_chroma_mc8_msa;
c->avg_h264_chroma_pixels_tab[1] = ff_avg_h264_chroma_mc4_msa;
c->avg_h264_chroma_pixels_tab[2] = ff_avg_h264_chroma_mc2_msa;
}
}
#endif 

#if HAVE_MMI
static av_cold void h264chroma_init_mmi(H264ChromaContext *c, int bit_depth)
{
int high_bit_depth = bit_depth > 8;

if (!high_bit_depth) {
c->put_h264_chroma_pixels_tab[0] = ff_put_h264_chroma_mc8_mmi;
c->avg_h264_chroma_pixels_tab[0] = ff_avg_h264_chroma_mc8_mmi;
c->put_h264_chroma_pixels_tab[1] = ff_put_h264_chroma_mc4_mmi;
c->avg_h264_chroma_pixels_tab[1] = ff_avg_h264_chroma_mc4_mmi;
}
}
#endif 

av_cold void ff_h264chroma_init_mips(H264ChromaContext *c, int bit_depth)
{
#if HAVE_MMI
h264chroma_init_mmi(c, bit_depth);
#endif 
#if HAVE_MSA
h264chroma_init_msa(c, bit_depth);
#endif 
}
