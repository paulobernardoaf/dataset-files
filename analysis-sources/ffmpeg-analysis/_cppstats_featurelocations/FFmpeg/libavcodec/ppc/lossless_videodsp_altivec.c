





















#include "config.h"

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"

#include "libavcodec/lossless_videodsp.h"

#if HAVE_ALTIVEC
static void add_bytes_altivec(uint8_t *dst, uint8_t *src, ptrdiff_t w)
{
register int i;
register vector unsigned char vdst, vsrc;


for (i = 0; i + 15 < w; i += 16) {
vdst = vec_ld(i, (unsigned char *) dst);
vsrc = vec_ld(i, (unsigned char *) src);
vdst = vec_add(vsrc, vdst);
vec_st(vdst, i, (unsigned char *) dst);
}

for (; i < w; i++)
dst[i] = src[i];
}
#endif 

av_cold void ff_llviddsp_init_ppc(LLVidDSPContext *c)
{
#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

c->add_bytes = add_bytes_altivec;
#endif 
}
