
















#if !defined(AVFILTER_TRANSPOSE_H)
#define AVFILTER_TRANSPOSE_H

#include <stddef.h>
#include <stdint.h>

enum PassthroughType {
TRANSPOSE_PT_TYPE_NONE,
TRANSPOSE_PT_TYPE_LANDSCAPE,
TRANSPOSE_PT_TYPE_PORTRAIT,
};

enum TransposeDir {
TRANSPOSE_CCLOCK_FLIP,
TRANSPOSE_CLOCK,
TRANSPOSE_CCLOCK,
TRANSPOSE_CLOCK_FLIP,
TRANSPOSE_REVERSAL, 
TRANSPOSE_HFLIP,
TRANSPOSE_VFLIP,
};

typedef struct TransVtable {
void (*transpose_8x8)(uint8_t *src, ptrdiff_t src_linesize,
uint8_t *dst, ptrdiff_t dst_linesize);
void (*transpose_block)(uint8_t *src, ptrdiff_t src_linesize,
uint8_t *dst, ptrdiff_t dst_linesize,
int w, int h);
} TransVtable;

void ff_transpose_init_x86(TransVtable *v, int pixstep);

#endif
