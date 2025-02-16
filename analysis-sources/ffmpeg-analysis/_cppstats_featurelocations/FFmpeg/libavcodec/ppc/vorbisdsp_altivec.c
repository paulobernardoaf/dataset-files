



















#include "config.h"

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"

#include "libavcodec/vorbisdsp.h"

#if HAVE_ALTIVEC
static void vorbis_inverse_coupling_altivec(float *mag, float *ang,
intptr_t blocksize)
{
int i;
vector float m, a;
vector bool int t0, t1;
const vector unsigned int v_31 = 
vec_add(vec_add(vec_splat_u32(15),vec_splat_u32(15)),vec_splat_u32(1));
for (i = 0; i < blocksize; i += 4) {
m = vec_ld(0, mag+i);
a = vec_ld(0, ang+i);
t0 = vec_cmple(m, (vector float)vec_splat_u32(0));
t1 = vec_cmple(a, (vector float)vec_splat_u32(0));
a = vec_xor(a, (vector float) vec_sl((vector unsigned int)t0, v_31));
t0 = (vector bool int)vec_and(a, t1);
t1 = (vector bool int)vec_andc(a, t1);
a = vec_sub(m, (vector float)t1);
m = vec_add(m, (vector float)t0);
vec_stl(a, 0, ang+i);
vec_stl(m, 0, mag+i);
}
}
#endif 

av_cold void ff_vorbisdsp_init_ppc(VorbisDSPContext *c)
{
#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

c->vorbis_inverse_coupling = vorbis_inverse_coupling_altivec;
#endif 
}
