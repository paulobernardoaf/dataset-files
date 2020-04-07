#include "common.h"
#define LZMA2_CHUNK_MAX (UINT32_C(1) << 16)
#define LZMA2_UNCOMPRESSED_MAX (UINT32_C(1) << 21)
#define LZMA2_HEADER_MAX 6
#define LZMA2_HEADER_UNCOMPRESSED 3
extern lzma_ret lzma_lzma2_encoder_init(
lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters);
extern uint64_t lzma_lzma2_encoder_memusage(const void *options);
extern lzma_ret lzma_lzma2_props_encode(const void *options, uint8_t *out);
