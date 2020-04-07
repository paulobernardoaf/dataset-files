




















#if !defined(AVCODEC_H264QPEL_H)
#define AVCODEC_H264QPEL_H

#include "qpeldsp.h"

typedef struct H264QpelContext {
qpel_mc_func put_h264_qpel_pixels_tab[4][16];
qpel_mc_func avg_h264_qpel_pixels_tab[4][16];
} H264QpelContext;

void ff_h264qpel_init(H264QpelContext *c, int bit_depth);

void ff_h264qpel_init_aarch64(H264QpelContext *c, int bit_depth);
void ff_h264qpel_init_arm(H264QpelContext *c, int bit_depth);
void ff_h264qpel_init_ppc(H264QpelContext *c, int bit_depth);
void ff_h264qpel_init_x86(H264QpelContext *c, int bit_depth);
void ff_h264qpel_init_mips(H264QpelContext *c, int bit_depth);

#endif 
