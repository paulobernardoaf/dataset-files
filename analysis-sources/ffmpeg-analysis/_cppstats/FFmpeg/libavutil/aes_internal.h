#include "mem.h"
#include <stdint.h>
typedef union {
uint64_t u64[2];
uint32_t u32[4];
uint8_t u8x4[4][4];
uint8_t u8[16];
} av_aes_block;
typedef struct AVAES {
DECLARE_ALIGNED(16, av_aes_block, round_key)[15];
DECLARE_ALIGNED(16, av_aes_block, state)[2];
int rounds;
void (*crypt)(struct AVAES *a, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int rounds);
} AVAES;
