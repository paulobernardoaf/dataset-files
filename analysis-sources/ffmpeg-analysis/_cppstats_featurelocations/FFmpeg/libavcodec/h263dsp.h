

















#if !defined(AVCODEC_H263DSP_H)
#define AVCODEC_H263DSP_H

#include <stdint.h>

extern const uint8_t ff_h263_loop_filter_strength[32];

typedef struct H263DSPContext {
void (*h263_h_loop_filter)(uint8_t *src, int stride, int qscale);
void (*h263_v_loop_filter)(uint8_t *src, int stride, int qscale);
} H263DSPContext;

void ff_h263dsp_init(H263DSPContext *ctx);
void ff_h263dsp_init_x86(H263DSPContext *ctx);
void ff_h263dsp_init_mips(H263DSPContext *ctx);

#endif 
