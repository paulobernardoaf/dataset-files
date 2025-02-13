#include "filter_decoder.h"
extern LZMA_API(lzma_ret)
lzma_raw_buffer_decode(const lzma_filter *filters, lzma_allocator *allocator,
const uint8_t *in, size_t *in_pos, size_t in_size,
uint8_t *out, size_t *out_pos, size_t out_size)
{
if (in == NULL || in_pos == NULL || *in_pos > in_size || out == NULL
|| out_pos == NULL || *out_pos > out_size)
return LZMA_PROG_ERROR;
lzma_next_coder next = LZMA_NEXT_CODER_INIT;
return_if_error(lzma_raw_decoder_init(&next, allocator, filters));
const size_t in_start = *in_pos;
const size_t out_start = *out_pos;
lzma_ret ret = next.code(next.coder, allocator, in, in_pos, in_size,
out, out_pos, out_size, LZMA_FINISH);
if (ret == LZMA_STREAM_END) {
ret = LZMA_OK;
} else {
if (ret == LZMA_OK) {
assert(*in_pos == in_size || *out_pos == out_size);
if (*in_pos != in_size) {
ret = LZMA_BUF_ERROR;
} else if (*out_pos != out_size) {
ret = LZMA_DATA_ERROR;
} else {
uint8_t tmp[1];
size_t tmp_pos = 0;
(void)next.code(next.coder, allocator,
in, in_pos, in_size,
tmp, &tmp_pos, 1, LZMA_FINISH);
if (tmp_pos == 1)
ret = LZMA_BUF_ERROR;
else
ret = LZMA_DATA_ERROR;
}
}
*in_pos = in_start;
*out_pos = out_start;
}
lzma_next_end(&next, allocator);
return ret;
}
