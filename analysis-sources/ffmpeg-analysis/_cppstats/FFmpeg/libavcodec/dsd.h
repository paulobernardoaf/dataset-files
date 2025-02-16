#include "libavcodec/internal.h"
#include "libavcodec/mathops.h"
#include "avcodec.h"
#define HTAPS 48 
#define FIFOSIZE 16 
#define FIFOMASK (FIFOSIZE - 1) 
#if FIFOSIZE * 8 < HTAPS * 2
#error "FIFOSIZE too small"
#endif
typedef struct DSDContext {
uint8_t buf[FIFOSIZE];
unsigned pos;
} DSDContext;
void ff_init_dsd_data(void);
void ff_dsd2pcm_translate(DSDContext* s, size_t samples, int lsbf,
const uint8_t *src, ptrdiff_t src_stride,
float *dst, ptrdiff_t dst_stride);
