











#if !defined(LZMA_ALONE_DECODER_H)
#define LZMA_ALONE_DECODER_H

#include "common.h"


extern lzma_ret lzma_alone_decoder_init(
lzma_next_coder *next, lzma_allocator *allocator,
uint64_t memlimit, bool picky);

#endif
