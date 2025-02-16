#include "block_decoder.h"
extern LZMA_API(lzma_ret)
lzma_block_buffer_decode(lzma_block *block, lzma_allocator *allocator,
const uint8_t *in, size_t *in_pos, size_t in_size,
uint8_t *out, size_t *out_pos, size_t out_size)
{
if (in_pos == NULL || (in == NULL && *in_pos != in_size)
|| *in_pos > in_size || out_pos == NULL
|| (out == NULL && *out_pos != out_size)
|| *out_pos > out_size)
return LZMA_PROG_ERROR;
lzma_next_coder block_decoder = LZMA_NEXT_CODER_INIT;
lzma_ret ret = lzma_block_decoder_init(
&block_decoder, allocator, block);
if (ret == LZMA_OK) {
const size_t in_start = *in_pos;
const size_t out_start = *out_pos;
ret = block_decoder.code(block_decoder.coder, allocator,
in, in_pos, in_size, out, out_pos, out_size,
LZMA_FINISH);
if (ret == LZMA_STREAM_END) {
ret = LZMA_OK;
} else {
if (ret == LZMA_OK) {
assert(*in_pos == in_size
|| *out_pos == out_size);
if (*in_pos == in_size)
ret = LZMA_DATA_ERROR;
else
ret = LZMA_BUF_ERROR;
}
*in_pos = in_start;
*out_pos = out_start;
}
}
lzma_next_end(&block_decoder, allocator);
return ret;
}
