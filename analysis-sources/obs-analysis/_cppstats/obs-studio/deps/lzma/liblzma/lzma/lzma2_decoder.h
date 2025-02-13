#include "common.h"
extern lzma_ret lzma_lzma2_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);
extern uint64_t lzma_lzma2_decoder_memusage(const void *options);
extern lzma_ret lzma_lzma2_props_decode(
void **options, lzma_allocator *allocator,
const uint8_t *props, size_t props_size);
