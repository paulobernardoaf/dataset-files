#include "delta_common.h"
extern lzma_ret lzma_delta_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);
extern lzma_ret lzma_delta_props_encode(const void *options, uint8_t *out);
