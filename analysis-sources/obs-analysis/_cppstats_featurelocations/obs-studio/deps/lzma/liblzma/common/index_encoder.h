











#if !defined(LZMA_INDEX_ENCODER_H)
#define LZMA_INDEX_ENCODER_H

#include "common.h"


extern lzma_ret lzma_index_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_index *i);


#endif
