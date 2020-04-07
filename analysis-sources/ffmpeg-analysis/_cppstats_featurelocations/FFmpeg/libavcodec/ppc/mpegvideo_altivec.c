






















#include <stdlib.h>
#include <stdio.h>

#include "config.h"

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"

#include "libavcodec/mpegvideo.h"

#if HAVE_ALTIVEC



static void dct_unquantize_h263_altivec(MpegEncContext *s,
int16_t *block, int n, int qscale)
{
int i, level, qmul, qadd;
int nCoeffs;

qadd = (qscale - 1) | 1;
qmul = qscale << 1;

if (s->mb_intra) {
if (!s->h263_aic) {
if (n < 4)
block[0] = block[0] * s->y_dc_scale;
else
block[0] = block[0] * s->c_dc_scale;
}else
qadd = 0;
i = 1;
nCoeffs= 63; 
} else {
i = 0;
av_assert2(s->block_last_index[n]>=0);
nCoeffs= s->intra_scantable.raster_end[ s->block_last_index[n] ];
}

{
register const vector signed short vczero = (const vector signed short)vec_splat_s16(0);
DECLARE_ALIGNED(16, short, qmul8) = qmul;
DECLARE_ALIGNED(16, short, qadd8) = qadd;
register vector signed short blockv, qmulv, qaddv, nqaddv, temp1;
register vector bool short blockv_null, blockv_neg;
register short backup_0 = block[0];
register int j = 0;

qmulv = vec_splat((vec_s16)vec_lde(0, &qmul8), 0);
qaddv = vec_splat((vec_s16)vec_lde(0, &qadd8), 0);
nqaddv = vec_sub(vczero, qaddv);



for(; (j + 7) <= nCoeffs ; j+=8) {
blockv = vec_ld(j << 1, block);
blockv_neg = vec_cmplt(blockv, vczero);
blockv_null = vec_cmpeq(blockv, vczero);

temp1 = vec_sel(qaddv, nqaddv, blockv_neg);

temp1 = vec_mladd(blockv, qmulv, temp1);

blockv = vec_sel(temp1, blockv, blockv_null);
vec_st(blockv, j << 1, block);
}





for(; j <= nCoeffs ; j++) {
level = block[j];
if (level) {
if (level < 0) {
level = level * qmul - qadd;
} else {
level = level * qmul + qadd;
}
block[j] = level;
}
}

if (i == 1) {

block[0] = backup_0;
}
}
}

#endif 

av_cold void ff_mpv_common_init_ppc(MpegEncContext *s)
{
#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

if ((s->avctx->dct_algo == FF_DCT_AUTO) ||
(s->avctx->dct_algo == FF_DCT_ALTIVEC)) {
s->dct_unquantize_h263_intra = dct_unquantize_h263_altivec;
s->dct_unquantize_h263_inter = dct_unquantize_h263_altivec;
}
#endif 
}
