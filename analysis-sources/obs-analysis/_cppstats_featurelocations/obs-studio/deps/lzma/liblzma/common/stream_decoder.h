











#if !defined(LZMA_STREAM_DECODER_H)
#define LZMA_STREAM_DECODER_H

#include "common.h"

extern lzma_ret lzma_stream_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, uint64_t memlimit, uint32_t flags);

#endif
