





















#include "config.h"
#include "libavutil/cpu.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"
#include "libavcodec/fft.h"










#if HAVE_VSX
#include "fft_vsx.h"
#else
void ff_fft_calc_altivec(FFTContext *s, FFTComplex *z);
void ff_fft_calc_interleave_altivec(FFTContext *s, FFTComplex *z);
#endif

#if HAVE_GNU_AS && HAVE_ALTIVEC && (HAVE_BIGENDIAN || HAVE_VSX)
static void imdct_half_altivec(FFTContext *s, FFTSample *output, const FFTSample *input)
{
int j, k;
int n = 1 << s->mdct_bits;
int n4 = n >> 2;
int n8 = n >> 3;
int n32 = n >> 5;
const uint16_t *revtabj = s->revtab;
const uint16_t *revtabk = s->revtab+n4;
const vec_f *tcos = (const vec_f*)(s->tcos+n8);
const vec_f *tsin = (const vec_f*)(s->tsin+n8);
const vec_f *pin = (const vec_f*)(input+n4);
vec_f *pout = (vec_f*)(output+n4);


k = n32-1;
do {
vec_f cos,sin,cos0,sin0,cos1,sin1,re,im,r0,i0,r1,i1,a,b,c,d;
#define CMULA(p,o0,o1,o2,o3)a = pin[ k*2+p]; b = pin[-k*2-p-1]; re = vec_perm(a, b, vcprm(0,2,s0,s2)); im = vec_perm(a, b, vcprm(s3,s1,3,1)); cos = vec_perm(cos0, cos1, vcprm(o0,o1,s##o2,s##o3)); sin = vec_perm(sin0, sin1, vcprm(o0,o1,s##o2,s##o3));r##p = im*cos - re*sin;i##p = re*cos + im*sin;








#define STORE2(v,dst)j = dst;vec_ste(v, 0, output+j*2);vec_ste(v, 4, output+j*2);



#define STORE8(p)a = vec_perm(r##p, i##p, vcprm(0,s0,0,s0));b = vec_perm(r##p, i##p, vcprm(1,s1,1,s1));c = vec_perm(r##p, i##p, vcprm(2,s2,2,s2));d = vec_perm(r##p, i##p, vcprm(3,s3,3,s3));STORE2(a, revtabk[ p*2-4]);STORE2(b, revtabk[ p*2-3]);STORE2(c, revtabj[-p*2+2]);STORE2(d, revtabj[-p*2+3]);









cos0 = tcos[k];
sin0 = tsin[k];
cos1 = tcos[-k-1];
sin1 = tsin[-k-1];
CMULA(0, 0,1,2,3);
CMULA(1, 2,3,0,1);
STORE8(0);
STORE8(1);
revtabj += 4;
revtabk -= 4;
k--;
} while(k >= 0);

#if HAVE_VSX
ff_fft_calc_vsx(s, (FFTComplex*)output);
#else
ff_fft_calc_altivec(s, (FFTComplex*)output);
#endif


j = -n32;
k = n32-1;
do {
vec_f cos,sin,re,im,a,b,c,d;
#define CMULB(d0,d1,o)re = pout[o*2];im = pout[o*2+1];cos = tcos[o];sin = tsin[o];d0 = im*sin - re*cos;d1 = re*sin + im*cos;







CMULB(a,b,j);
CMULB(c,d,k);
pout[2*j] = vec_perm(a, d, vcprm(0,s3,1,s2));
pout[2*j+1] = vec_perm(a, d, vcprm(2,s1,3,s0));
pout[2*k] = vec_perm(c, b, vcprm(0,s3,1,s2));
pout[2*k+1] = vec_perm(c, b, vcprm(2,s1,3,s0));
j++;
k--;
} while(k >= 0);
}

static void imdct_calc_altivec(FFTContext *s, FFTSample *output, const FFTSample *input)
{
int k;
int n = 1 << s->mdct_bits;
int n4 = n >> 2;
int n16 = n >> 4;
vec_u32 sign = {1U<<31,1U<<31,1U<<31,1U<<31};
vec_u32 *p0 = (vec_u32*)(output+n4);
vec_u32 *p1 = (vec_u32*)(output+n4*3);

imdct_half_altivec(s, output + n4, input);

for (k = 0; k < n16; k++) {
vec_u32 a = p0[k] ^ sign;
vec_u32 b = p1[-k-1];
p0[-k-1] = vec_perm(a, a, vcprm(3,2,1,0));
p1[k] = vec_perm(b, b, vcprm(3,2,1,0));
}
}
#endif 

av_cold void ff_fft_init_ppc(FFTContext *s)
{
#if HAVE_GNU_AS && HAVE_ALTIVEC && (HAVE_BIGENDIAN || HAVE_VSX)
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

#if HAVE_VSX
s->fft_calc = ff_fft_calc_interleave_vsx;
#else
s->fft_calc = ff_fft_calc_interleave_altivec;
#endif
if (s->mdct_bits >= 5) {
s->imdct_calc = imdct_calc_altivec;
s->imdct_half = imdct_half_altivec;
}
#endif 
}
