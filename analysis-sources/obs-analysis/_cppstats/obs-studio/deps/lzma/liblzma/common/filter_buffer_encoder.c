#include "filter_encoder.h"
extern LZMA_API(lzma_ret)
lzma_raw_buffer_encode(const lzma_filter *filters, lzma_allocator *allocator,
const uint8_t *in, size_t in_size, uint8_t *out,
size_t *out_pos, size_t out_size)
{
if ((in == NULL && in_size != 0) || out == NULL
|| out_pos == NULL || *out_pos > out_size)
return LZMA_PROG_ERROR;
lzma_next_coder next = LZMA_NEXT_CODER_INIT;
return_if_error(lzma_raw_encoder_init(&next, allocator, filters));
const size_t out_start = *out_pos;
size_t in_pos = 0;
lzma_ret ret = next.code(next.coder, allocator, in, &in_pos, in_size,
out, out_pos, out_size, LZMA_FINISH);
lzma_next_end(&next, allocator);
if (ret == LZMA_STREAM_END) {
ret = LZMA_OK;
} else {
if (ret == LZMA_OK) {
assert(*out_pos == out_size);
ret = LZMA_BUF_ERROR;
}
*out_pos = out_start;
}
return ret;
}
