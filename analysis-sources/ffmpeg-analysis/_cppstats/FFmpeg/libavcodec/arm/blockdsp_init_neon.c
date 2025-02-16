#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavcodec/blockdsp.h"
#include "blockdsp_arm.h"
void ff_clear_block_neon(int16_t *block);
void ff_clear_blocks_neon(int16_t *blocks);
av_cold void ff_blockdsp_init_neon(BlockDSPContext *c)
{
c->clear_block = ff_clear_block_neon;
c->clear_blocks = ff_clear_blocks_neon;
}
