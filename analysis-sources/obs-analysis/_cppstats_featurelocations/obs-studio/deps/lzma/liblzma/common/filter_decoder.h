











#if !defined(LZMA_FILTER_DECODER_H)
#define LZMA_FILTER_DECODER_H

#include "common.h"


extern lzma_ret lzma_raw_decoder_init(
lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter *options);

#endif
