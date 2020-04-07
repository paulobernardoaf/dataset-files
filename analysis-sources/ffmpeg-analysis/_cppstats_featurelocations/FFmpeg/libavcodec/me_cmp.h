

















#if !defined(AVCODEC_ME_CMP_H)
#define AVCODEC_ME_CMP_H

#include <stdint.h>

#include "avcodec.h"

extern const uint32_t ff_square_tab[512];















struct MpegEncContext;





typedef int (*me_cmp_func)(struct MpegEncContext *c,
uint8_t *blk1 ,
uint8_t *blk2 , ptrdiff_t stride,
int h);

typedef struct MECmpContext {
int (*sum_abs_dctelem)(int16_t *block );

me_cmp_func sad[6]; 
me_cmp_func sse[6];
me_cmp_func hadamard8_diff[6];
me_cmp_func dct_sad[6];
me_cmp_func quant_psnr[6];
me_cmp_func bit[6];
me_cmp_func rd[6];
me_cmp_func vsad[6];
me_cmp_func vsse[6];
me_cmp_func nsse[6];
me_cmp_func w53[6];
me_cmp_func w97[6];
me_cmp_func dct_max[6];
me_cmp_func dct264_sad[6];

me_cmp_func me_pre_cmp[6];
me_cmp_func me_cmp[6];
me_cmp_func me_sub_cmp[6];
me_cmp_func mb_cmp[6];
me_cmp_func ildct_cmp[6]; 
me_cmp_func frame_skip_cmp[6]; 

me_cmp_func pix_abs[2][4];
me_cmp_func median_sad[6];
} MECmpContext;

int ff_check_alignment(void);

void ff_me_cmp_init(MECmpContext *c, AVCodecContext *avctx);
void ff_me_cmp_init_alpha(MECmpContext *c, AVCodecContext *avctx);
void ff_me_cmp_init_arm(MECmpContext *c, AVCodecContext *avctx);
void ff_me_cmp_init_ppc(MECmpContext *c, AVCodecContext *avctx);
void ff_me_cmp_init_x86(MECmpContext *c, AVCodecContext *avctx);
void ff_me_cmp_init_mips(MECmpContext *c, AVCodecContext *avctx);

void ff_set_cmp(MECmpContext *c, me_cmp_func *cmp, int type);

void ff_dsputil_init_dwt(MECmpContext *c);

#endif 
