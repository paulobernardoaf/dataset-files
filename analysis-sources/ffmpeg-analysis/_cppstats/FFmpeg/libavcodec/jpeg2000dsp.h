#include <stdint.h>
#include "jpeg2000dwt.h"
typedef struct Jpeg2000DSPContext {
void (*mct_decode[FF_DWT_NB])(void *src0, void *src1, void *src2, int csize);
} Jpeg2000DSPContext;
void ff_jpeg2000dsp_init(Jpeg2000DSPContext *c);
void ff_jpeg2000dsp_init_x86(Jpeg2000DSPContext *c);
