











#if !defined(LZMA_SIMPLE_CODER_H)
#define LZMA_SIMPLE_CODER_H

#include "common.h"


extern lzma_ret lzma_simple_x86_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);

extern lzma_ret lzma_simple_x86_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);


extern lzma_ret lzma_simple_powerpc_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);

extern lzma_ret lzma_simple_powerpc_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);


extern lzma_ret lzma_simple_ia64_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);

extern lzma_ret lzma_simple_ia64_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);


extern lzma_ret lzma_simple_arm_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);

extern lzma_ret lzma_simple_arm_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);


extern lzma_ret lzma_simple_armthumb_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);

extern lzma_ret lzma_simple_armthumb_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);


extern lzma_ret lzma_simple_sparc_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);

extern lzma_ret lzma_simple_sparc_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);

#endif
