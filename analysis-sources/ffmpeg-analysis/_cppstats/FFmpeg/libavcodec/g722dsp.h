#include <stdint.h>
typedef struct G722DSPContext {
void (*apply_qmf)(const int16_t *prev_samples, int xout[2]);
} G722DSPContext;
void ff_g722dsp_init(G722DSPContext *c);
void ff_g722dsp_init_arm(G722DSPContext *c);
void ff_g722dsp_init_x86(G722DSPContext *c);
