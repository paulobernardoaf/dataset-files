#include "simple_coder.h"
extern lzma_ret lzma_simple_props_decode(
void **options, lzma_allocator *allocator,
const uint8_t *props, size_t props_size);
