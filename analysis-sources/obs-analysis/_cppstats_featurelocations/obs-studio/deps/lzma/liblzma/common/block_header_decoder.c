











#include "common.h"
#include "check.h"


static void
free_properties(lzma_block *block, lzma_allocator *allocator)
{



for (size_t i = 0; i < LZMA_FILTERS_MAX; ++i) {
lzma_free(block->filters[i].options, allocator);
block->filters[i].id = LZMA_VLI_UNKNOWN;
block->filters[i].options = NULL;
}

return;
}


extern LZMA_API(lzma_ret)
lzma_block_header_decode(lzma_block *block,
lzma_allocator *allocator, const uint8_t *in)
{







for (size_t i = 0; i <= LZMA_FILTERS_MAX; ++i) {
block->filters[i].id = LZMA_VLI_UNKNOWN;
block->filters[i].options = NULL;
}


block->version = 0;



if (lzma_block_header_size_decode(in[0]) != block->header_size
|| (unsigned int)(block->check) > LZMA_CHECK_ID_MAX)
return LZMA_PROG_ERROR;


const size_t in_size = block->header_size - 4;


if (lzma_crc32(in, in_size, 0) != unaligned_read32le(in + in_size))
return LZMA_DATA_ERROR;


if (in[1] & 0x3C)
return LZMA_OPTIONS_ERROR;


size_t in_pos = 2;


if (in[1] & 0x40) {
return_if_error(lzma_vli_decode(&block->compressed_size,
NULL, in, &in_pos, in_size));



if (lzma_block_unpadded_size(block) == 0)
return LZMA_DATA_ERROR;
} else {
block->compressed_size = LZMA_VLI_UNKNOWN;
}


if (in[1] & 0x80)
return_if_error(lzma_vli_decode(&block->uncompressed_size,
NULL, in, &in_pos, in_size));
else
block->uncompressed_size = LZMA_VLI_UNKNOWN;


const size_t filter_count = (in[1] & 3) + 1;
for (size_t i = 0; i < filter_count; ++i) {
const lzma_ret ret = lzma_filter_flags_decode(
&block->filters[i], allocator,
in, &in_pos, in_size);
if (ret != LZMA_OK) {
free_properties(block, allocator);
return ret;
}
}


while (in_pos < in_size) {
if (in[in_pos++] != 0x00) {
free_properties(block, allocator);



return LZMA_OPTIONS_ERROR;
}
}

return LZMA_OK;
}
