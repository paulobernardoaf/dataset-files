











#if !defined(LZMA_DELTA_PRIVATE_H)
#define LZMA_DELTA_PRIVATE_H

#include "delta_common.h"

struct lzma_coder_s {

lzma_next_coder next;


size_t distance;


uint8_t pos;


uint8_t history[LZMA_DELTA_DIST_MAX];
};


extern lzma_ret lzma_delta_coder_init(
lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters);

#endif
