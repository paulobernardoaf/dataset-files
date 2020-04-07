#include "index.h"
#define INDEX_BOUND ((1 + 1 + 2 * LZMA_VLI_BYTES_MAX + 4 + 3) & ~3)
#define HEADERS_BOUND (2 * LZMA_STREAM_HEADER_SIZE + INDEX_BOUND)
extern LZMA_API(size_t)
lzma_stream_buffer_bound(size_t uncompressed_size)
{
const size_t block_bound = lzma_block_buffer_bound(uncompressed_size);
if (block_bound == 0)
return 0;
if (my_min(SIZE_MAX, LZMA_VLI_MAX) - block_bound < HEADERS_BOUND)
return 0;
return block_bound + HEADERS_BOUND;
}
extern LZMA_API(lzma_ret)
lzma_stream_buffer_encode(lzma_filter *filters, lzma_check check,
lzma_allocator *allocator, const uint8_t *in, size_t in_size,
uint8_t *out, size_t *out_pos_ptr, size_t out_size)
{
if (filters == NULL || (unsigned int)(check) > LZMA_CHECK_ID_MAX
|| (in == NULL && in_size != 0) || out == NULL
|| out_pos_ptr == NULL || *out_pos_ptr > out_size)
return LZMA_PROG_ERROR;
if (!lzma_check_is_supported(check))
return LZMA_UNSUPPORTED_CHECK;
size_t out_pos = *out_pos_ptr;
if (out_size - out_pos <= 2 * LZMA_STREAM_HEADER_SIZE)
return LZMA_BUF_ERROR;
out_size -= LZMA_STREAM_HEADER_SIZE;
lzma_stream_flags stream_flags = {
.version = 0,
.check = check,
};
if (lzma_stream_header_encode(&stream_flags, out + out_pos)
!= LZMA_OK)
return LZMA_PROG_ERROR;
out_pos += LZMA_STREAM_HEADER_SIZE;
lzma_block block = {
.version = 0,
.check = check,
.filters = filters,
};
if (in_size > 0)
return_if_error(lzma_block_buffer_encode(&block, allocator,
in, in_size, out, &out_pos, out_size));
{
lzma_index *i = lzma_index_init(allocator);
if (i == NULL)
return LZMA_MEM_ERROR;
lzma_ret ret = LZMA_OK;
if (in_size > 0)
ret = lzma_index_append(i, allocator,
lzma_block_unpadded_size(&block),
block.uncompressed_size);
if (ret == LZMA_OK) {
ret = lzma_index_buffer_encode(
i, out, &out_pos, out_size);
stream_flags.backward_size = lzma_index_size(i);
}
lzma_index_end(i, allocator);
if (ret != LZMA_OK)
return ret;
}
if (lzma_stream_footer_encode(&stream_flags, out + out_pos)
!= LZMA_OK)
return LZMA_PROG_ERROR;
out_pos += LZMA_STREAM_HEADER_SIZE;
*out_pos_ptr = out_pos;
return LZMA_OK;
}
