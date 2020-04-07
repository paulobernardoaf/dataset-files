#include "filter_decoder.h"
extern LZMA_API(lzma_ret)
lzma_filter_flags_decode(
lzma_filter *filter, lzma_allocator *allocator,
const uint8_t *in, size_t *in_pos, size_t in_size)
{
filter->options = NULL;
return_if_error(lzma_vli_decode(&filter->id, NULL,
in, in_pos, in_size));
if (filter->id >= LZMA_FILTER_RESERVED_START)
return LZMA_DATA_ERROR;
lzma_vli props_size;
return_if_error(lzma_vli_decode(&props_size, NULL,
in, in_pos, in_size));
if (in_size - *in_pos < props_size)
return LZMA_DATA_ERROR;
const lzma_ret ret = lzma_properties_decode(
filter, allocator, in + *in_pos, props_size);
*in_pos += props_size;
return ret;
}
