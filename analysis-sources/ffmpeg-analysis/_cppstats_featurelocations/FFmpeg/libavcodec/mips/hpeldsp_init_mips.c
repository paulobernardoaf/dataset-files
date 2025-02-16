




















#include "../hpeldsp.h"
#include "libavcodec/mips/hpeldsp_mips.h"

#if HAVE_MSA
static void ff_hpeldsp_init_msa(HpelDSPContext *c, int flags)
{
c->put_pixels_tab[0][0] = ff_put_pixels16_msa;
c->put_pixels_tab[0][1] = ff_put_pixels16_x2_msa;
c->put_pixels_tab[0][2] = ff_put_pixels16_y2_msa;
c->put_pixels_tab[0][3] = ff_put_pixels16_xy2_msa;

c->put_pixels_tab[1][0] = ff_put_pixels8_msa;
c->put_pixels_tab[1][1] = ff_put_pixels8_x2_msa;
c->put_pixels_tab[1][2] = ff_put_pixels8_y2_msa;
c->put_pixels_tab[1][3] = ff_put_pixels8_xy2_msa;

c->put_pixels_tab[2][1] = ff_put_pixels4_x2_msa;
c->put_pixels_tab[2][2] = ff_put_pixels4_y2_msa;
c->put_pixels_tab[2][3] = ff_put_pixels4_xy2_msa;

c->put_no_rnd_pixels_tab[0][0] = ff_put_pixels16_msa;
c->put_no_rnd_pixels_tab[0][1] = ff_put_no_rnd_pixels16_x2_msa;
c->put_no_rnd_pixels_tab[0][2] = ff_put_no_rnd_pixels16_y2_msa;
c->put_no_rnd_pixels_tab[0][3] = ff_put_no_rnd_pixels16_xy2_msa;

c->put_no_rnd_pixels_tab[1][0] = ff_put_pixels8_msa;
c->put_no_rnd_pixels_tab[1][1] = ff_put_no_rnd_pixels8_x2_msa;
c->put_no_rnd_pixels_tab[1][2] = ff_put_no_rnd_pixels8_y2_msa;
c->put_no_rnd_pixels_tab[1][3] = ff_put_no_rnd_pixels8_xy2_msa;

c->avg_pixels_tab[0][0] = ff_avg_pixels16_msa;
c->avg_pixels_tab[0][1] = ff_avg_pixels16_x2_msa;
c->avg_pixels_tab[0][2] = ff_avg_pixels16_y2_msa;
c->avg_pixels_tab[0][3] = ff_avg_pixels16_xy2_msa;

c->avg_pixels_tab[1][0] = ff_avg_pixels8_msa;
c->avg_pixels_tab[1][1] = ff_avg_pixels8_x2_msa;
c->avg_pixels_tab[1][2] = ff_avg_pixels8_y2_msa;
c->avg_pixels_tab[1][3] = ff_avg_pixels8_xy2_msa;

c->avg_pixels_tab[2][0] = ff_avg_pixels4_msa;
c->avg_pixels_tab[2][1] = ff_avg_pixels4_x2_msa;
c->avg_pixels_tab[2][2] = ff_avg_pixels4_y2_msa;
c->avg_pixels_tab[2][3] = ff_avg_pixels4_xy2_msa;
}
#endif 

#if HAVE_MMI
static void ff_hpeldsp_init_mmi(HpelDSPContext *c, int flags)
{
c->put_pixels_tab[0][0] = ff_put_pixels16_8_mmi;
c->put_pixels_tab[0][1] = ff_put_pixels16_x2_8_mmi;
c->put_pixels_tab[0][2] = ff_put_pixels16_y2_8_mmi;
c->put_pixels_tab[0][3] = ff_put_pixels16_xy2_8_mmi;

c->put_pixels_tab[1][0] = ff_put_pixels8_8_mmi;
c->put_pixels_tab[1][1] = ff_put_pixels8_x2_8_mmi;
c->put_pixels_tab[1][2] = ff_put_pixels8_y2_8_mmi;
c->put_pixels_tab[1][3] = ff_put_pixels8_xy2_8_mmi;

c->put_pixels_tab[2][0] = ff_put_pixels4_8_mmi;
c->put_pixels_tab[2][1] = ff_put_pixels4_x2_8_mmi;
c->put_pixels_tab[2][2] = ff_put_pixels4_y2_8_mmi;
c->put_pixels_tab[2][3] = ff_put_pixels4_xy2_8_mmi;

c->put_no_rnd_pixels_tab[0][0] = ff_put_pixels16_8_mmi;
c->put_no_rnd_pixels_tab[0][1] = ff_put_no_rnd_pixels16_x2_8_mmi;
c->put_no_rnd_pixels_tab[0][2] = ff_put_no_rnd_pixels16_y2_8_mmi;
c->put_no_rnd_pixels_tab[0][3] = ff_put_no_rnd_pixels16_xy2_8_mmi;

c->put_no_rnd_pixels_tab[1][0] = ff_put_pixels8_8_mmi;
c->put_no_rnd_pixels_tab[1][1] = ff_put_no_rnd_pixels8_x2_8_mmi;
c->put_no_rnd_pixels_tab[1][2] = ff_put_no_rnd_pixels8_y2_8_mmi;
c->put_no_rnd_pixels_tab[1][3] = ff_put_no_rnd_pixels8_xy2_8_mmi;

c->avg_pixels_tab[0][0] = ff_avg_pixels16_8_mmi;
c->avg_pixels_tab[0][1] = ff_avg_pixels16_x2_8_mmi;
c->avg_pixels_tab[0][2] = ff_avg_pixels16_y2_8_mmi;
c->avg_pixels_tab[0][3] = ff_avg_pixels16_xy2_8_mmi;

c->avg_pixels_tab[1][0] = ff_avg_pixels8_8_mmi;
c->avg_pixels_tab[1][1] = ff_avg_pixels8_x2_8_mmi;
c->avg_pixels_tab[1][2] = ff_avg_pixels8_y2_8_mmi;
c->avg_pixels_tab[1][3] = ff_avg_pixels8_xy2_8_mmi;

c->avg_pixels_tab[2][0] = ff_avg_pixels4_8_mmi;
c->avg_pixels_tab[2][1] = ff_avg_pixels4_x2_8_mmi;
c->avg_pixels_tab[2][2] = ff_avg_pixels4_y2_8_mmi;
c->avg_pixels_tab[2][3] = ff_avg_pixels4_xy2_8_mmi;
}
#endif 

void ff_hpeldsp_init_mips(HpelDSPContext *c, int flags)
{
#if HAVE_MMI
ff_hpeldsp_init_mmi(c, flags);
#endif 
#if HAVE_MSA
ff_hpeldsp_init_msa(c, flags);
#endif 
}
