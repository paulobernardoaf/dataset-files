#include "../mpegvideo.h"
void ff_fill_block16_msa(uint8_t *src, uint8_t val, ptrdiff_t stride, int height);
void ff_fill_block8_msa(uint8_t *src, uint8_t val, ptrdiff_t stride, int height);
void ff_clear_block_msa(int16_t *block);
void ff_clear_blocks_msa(int16_t *block);
void ff_fill_block16_mmi(uint8_t *block, uint8_t value, ptrdiff_t line_size, int h);
void ff_fill_block8_mmi(uint8_t *block, uint8_t value, ptrdiff_t line_size, int h);
void ff_clear_block_mmi(int16_t *block);
void ff_clear_blocks_mmi(int16_t *block);
