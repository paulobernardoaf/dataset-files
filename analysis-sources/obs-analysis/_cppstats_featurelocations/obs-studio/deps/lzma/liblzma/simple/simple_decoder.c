











#include "simple_decoder.h"


extern lzma_ret
lzma_simple_props_decode(void **options, lzma_allocator *allocator,
const uint8_t *props, size_t props_size)
{
if (props_size == 0)
return LZMA_OK;

if (props_size != 4)
return LZMA_OPTIONS_ERROR;

lzma_options_bcj *opt = lzma_alloc(
sizeof(lzma_options_bcj), allocator);
if (opt == NULL)
return LZMA_MEM_ERROR;

opt->start_offset = unaligned_read32le(props);


if (opt->start_offset == 0)
lzma_free(opt, allocator);
else
*options = opt;

return LZMA_OK;
}
