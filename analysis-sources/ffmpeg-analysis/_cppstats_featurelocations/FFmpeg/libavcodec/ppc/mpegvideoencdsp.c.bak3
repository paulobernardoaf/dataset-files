

















#include "config.h"

#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"

#include "libavcodec/mpegvideoencdsp.h"

#if HAVE_ALTIVEC

#if HAVE_VSX
static int pix_norm1_altivec(uint8_t *pix, int line_size)
{
int i, s = 0;
const vector unsigned int zero =
(const vector unsigned int) vec_splat_u32(0);
vector unsigned int sv = (vector unsigned int) vec_splat_u32(0);
vector signed int sum;

for (i = 0; i < 16; i++) {




vector unsigned char pixv = vec_vsx_ld(0, pix);


sv = vec_msum(pixv, pixv, sv);

pix += line_size;
}

sum = vec_sums((vector signed int) sv, (vector signed int) zero);
sum = vec_splat(sum, 3);
vec_ste(sum, 0, &s);
return s;
}
#else
static int pix_norm1_altivec(uint8_t *pix, int line_size)
{
int i, s = 0;
const vector unsigned int zero =
(const vector unsigned int) vec_splat_u32(0);
vector unsigned char perm = vec_lvsl(0, pix);
vector unsigned int sv = (vector unsigned int) vec_splat_u32(0);
vector signed int sum;

for (i = 0; i < 16; i++) {

vector unsigned char pixl = vec_ld(0, pix);
vector unsigned char pixr = vec_ld(15, pix);
vector unsigned char pixv = vec_perm(pixl, pixr, perm);


sv = vec_msum(pixv, pixv, sv);

pix += line_size;
}

sum = vec_sums((vector signed int) sv, (vector signed int) zero);
sum = vec_splat(sum, 3);
vec_ste(sum, 0, &s);

return s;
}
#endif 

#if HAVE_VSX
static int pix_sum_altivec(uint8_t *pix, int line_size)
{
int i, s;
const vector unsigned int zero =
(const vector unsigned int) vec_splat_u32(0);
vector unsigned int sad = (vector unsigned int) vec_splat_u32(0);
vector signed int sumdiffs;

for (i = 0; i < 16; i++) {




vector unsigned char t1 = vec_vsx_ld(0, pix);


sad = vec_sum4s(t1, sad);

pix += line_size;
}


sumdiffs = vec_sums((vector signed int) sad, (vector signed int) zero);
sumdiffs = vec_splat(sumdiffs, 3);
vec_ste(sumdiffs, 0, &s);
return s;
}
#else
static int pix_sum_altivec(uint8_t *pix, int line_size)
{
int i, s;
const vector unsigned int zero =
(const vector unsigned int) vec_splat_u32(0);
vector unsigned char perm = vec_lvsl(0, pix);
vector unsigned int sad = (vector unsigned int) vec_splat_u32(0);
vector signed int sumdiffs;

for (i = 0; i < 16; i++) {

vector unsigned char pixl = vec_ld(0, pix);
vector unsigned char pixr = vec_ld(15, pix);
vector unsigned char t1 = vec_perm(pixl, pixr, perm);


sad = vec_sum4s(t1, sad);

pix += line_size;
}


sumdiffs = vec_sums((vector signed int) sad, (vector signed int) zero);
sumdiffs = vec_splat(sumdiffs, 3);
vec_ste(sumdiffs, 0, &s);

return s;
}

#endif 

#endif 

av_cold void ff_mpegvideoencdsp_init_ppc(MpegvideoEncDSPContext *c,
AVCodecContext *avctx)
{
#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

c->pix_norm1 = pix_norm1_altivec;
c->pix_sum = pix_sum_altivec;
#endif 
}
