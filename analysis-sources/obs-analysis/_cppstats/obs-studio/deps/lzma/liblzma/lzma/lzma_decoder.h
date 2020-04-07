#include "common.h"
extern lzma_ret lzma_lzma_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);
extern uint64_t lzma_lzma_decoder_memusage(const void *options);
extern lzma_ret lzma_lzma_props_decode(
void **options, lzma_allocator *allocator,
const uint8_t *props, size_t props_size);
extern bool lzma_lzma_lclppb_decode(
lzma_options_lzma *options, uint8_t byte);
#if defined(LZMA_LZ_DECODER_H)
extern lzma_ret lzma_lzma_decoder_create(
lzma_lz_decoder *lz, lzma_allocator *allocator,
const void *opt, lzma_lz_options *lz_options);
extern uint64_t lzma_lzma_decoder_memusage_nocheck(const void *options);
#endif
