#include "common.h"
extern lzma_ret lzma_lzma_encoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);
extern uint64_t lzma_lzma_encoder_memusage(const void *options);
extern lzma_ret lzma_lzma_props_encode(const void *options, uint8_t *out);
extern bool lzma_lzma_lclppb_encode(
const lzma_options_lzma *options, uint8_t *byte);
#if defined(LZMA_LZ_ENCODER_H)
extern lzma_ret lzma_lzma_encoder_create(
lzma_coder **coder_ptr, lzma_allocator *allocator,
const lzma_options_lzma *options, lzma_lz_options *lz_options);
extern lzma_ret lzma_lzma_encoder_reset(
lzma_coder *coder, const lzma_options_lzma *options);
extern lzma_ret lzma_lzma_encode(lzma_coder *restrict coder,
lzma_mf *restrict mf, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size,
uint32_t read_limit);
#endif
