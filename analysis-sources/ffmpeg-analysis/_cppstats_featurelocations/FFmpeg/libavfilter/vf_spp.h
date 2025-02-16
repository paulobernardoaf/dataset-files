




















#if !defined(AVFILTER_SPP_H)
#define AVFILTER_SPP_H

#include "libavcodec/avcodec.h"
#include "libavcodec/avdct.h"
#include "avfilter.h"

#define MAX_LEVEL 6 

typedef struct SPPContext {
const AVClass *av_class;

int log2_count;
int qp;
int mode;
int qscale_type;
int temp_linesize;
uint8_t *src;
uint16_t *temp;
AVCodecContext *avctx;
AVDCT *dct;
int8_t *non_b_qp_table;
int non_b_qp_alloc_size;
int use_bframe_qp;
int hsub, vsub;

void (*store_slice)(uint8_t *dst, const int16_t *src,
int dst_stride, int src_stride,
int width, int height, int log2_scale,
const uint8_t dither[8][8]);

void (*requantize)(int16_t dst[64], const int16_t src[64],
int qp, const uint8_t *permutation);
} SPPContext;

void ff_spp_init_x86(SPPContext *s);

#endif 
