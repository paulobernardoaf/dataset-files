#include "libavutil/attributes.h"
#include "libavcodec/mpegvideo.h"
#include "asm.h"
static void dct_unquantize_h263_axp(int16_t *block, int n_coeffs,
uint64_t qscale, uint64_t qadd)
{
uint64_t qmul = qscale << 1;
uint64_t correction = WORD_VEC(qmul * 255 >> 8);
int i;
qadd = WORD_VEC(qadd);
for(i = 0; i <= n_coeffs; block += 4, i += 4) {
uint64_t levels, negmask, zeros, add, sub;
levels = ldq(block);
if (levels == 0)
continue;
#if defined(__alpha_max__)
negmask = maxsw4(levels, -1); 
negmask = minsw4(negmask, 0); 
#else
negmask = cmpbge(WORD_VEC(0x7fff), levels);
negmask &= (negmask >> 1) | (1 << 7);
negmask = zap(-1, negmask);
#endif
zeros = cmpbge(0, levels);
zeros &= zeros >> 1;
levels *= qmul;
levels -= correction & (negmask << 16);
add = qadd & ~negmask;
sub = qadd & negmask;
add = zap(add, zeros);
levels += add;
levels -= sub;
stq(levels, block);
}
}
static void dct_unquantize_h263_intra_axp(MpegEncContext *s, int16_t *block,
int n, int qscale)
{
int n_coeffs;
uint64_t qadd;
int16_t block0 = block[0];
if (!s->h263_aic) {
if (n < 4)
block0 *= s->y_dc_scale;
else
block0 *= s->c_dc_scale;
qadd = (qscale - 1) | 1;
} else {
qadd = 0;
}
if(s->ac_pred)
n_coeffs = 63;
else
n_coeffs = s->inter_scantable.raster_end[s->block_last_index[n]];
dct_unquantize_h263_axp(block, n_coeffs, qscale, qadd);
block[0] = block0;
}
static void dct_unquantize_h263_inter_axp(MpegEncContext *s, int16_t *block,
int n, int qscale)
{
int n_coeffs = s->inter_scantable.raster_end[s->block_last_index[n]];
dct_unquantize_h263_axp(block, n_coeffs, qscale, (qscale - 1) | 1);
}
av_cold void ff_mpv_common_init_axp(MpegEncContext *s)
{
s->dct_unquantize_h263_intra = dct_unquantize_h263_intra_axp;
s->dct_unquantize_h263_inter = dct_unquantize_h263_inter_axp;
}
