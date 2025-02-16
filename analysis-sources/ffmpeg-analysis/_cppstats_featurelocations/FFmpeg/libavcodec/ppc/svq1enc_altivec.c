



















#include "config.h"

#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"

#include "libavcodec/svq1enc.h"

#if HAVE_ALTIVEC
static int ssd_int8_vs_int16_altivec(const int8_t *pix1, const int16_t *pix2,
intptr_t size)
{
int i, size16 = size >> 4;
vector signed char vpix1;
vector signed short vpix2, vdiff, vpix1l, vpix1h;
union {
vector signed int vscore;
int32_t score[4];
} u = { .vscore = vec_splat_s32(0) };

while (size16) {



vpix1 = vec_unaligned_load(pix1);
vpix2 = vec_unaligned_load(pix2);
pix2 += 8;

vpix1h = vec_unpackh(vpix1);
vdiff = vec_sub(vpix1h, vpix2);
vpix1l = vec_unpackl(vpix1);

vpix2 = vec_unaligned_load(pix2);
u.vscore = vec_msum(vdiff, vdiff, u.vscore);
vdiff = vec_sub(vpix1l, vpix2);
u.vscore = vec_msum(vdiff, vdiff, u.vscore);
pix1 += 16;
pix2 += 8;
size16--;
}
u.vscore = vec_sums(u.vscore, vec_splat_s32(0));

size %= 16;
for (i = 0; i < size; i++)
u.score[3] += (pix1[i] - pix2[i]) * (pix1[i] - pix2[i]);

return u.score[3];
}
#endif 

av_cold void ff_svq1enc_init_ppc(SVQ1EncContext *c)
{
#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

c->ssd_int8_vs_int16 = ssd_int8_vs_int16_altivec;
#endif 
}
