#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavcodec/blockdsp.h"
#include "asm.h"
static void clear_blocks_axp(int16_t *blocks) {
uint64_t *p = (uint64_t *) blocks;
int n = sizeof(int16_t) * 6 * 64;
do {
p[0] = 0;
p[1] = 0;
p[2] = 0;
p[3] = 0;
p[4] = 0;
p[5] = 0;
p[6] = 0;
p[7] = 0;
p += 8;
n -= 8 * 8;
} while (n);
}
av_cold void ff_blockdsp_init_alpha(BlockDSPContext *c)
{
c->clear_blocks = clear_blocks_axp;
}
