#include "delta_common.h"
extern lzma_ret lzma_delta_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);
extern lzma_ret lzma_delta_props_decode(
void **options, lzma_allocator *allocator,
const uint8_t *props, size_t props_size);
