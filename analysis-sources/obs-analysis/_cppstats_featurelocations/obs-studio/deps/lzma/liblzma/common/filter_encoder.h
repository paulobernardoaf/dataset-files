











#if !defined(LZMA_FILTER_ENCODER_H)
#define LZMA_FILTER_ENCODER_H

#include "common.h"






extern lzma_ret lzma_raw_encoder_init(
lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter *filters);

#endif
