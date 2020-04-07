#include "stream_decoder.h"
extern LZMA_API(lzma_ret)
lzma_stream_buffer_decode(uint64_t *memlimit, uint32_t flags,
lzma_allocator *allocator,
const uint8_t *in, size_t *in_pos, size_t in_size,
uint8_t *out, size_t *out_pos, size_t out_size)
{
if (in_pos == NULL || (in == NULL && *in_pos != in_size)
|| *in_pos > in_size || out_pos == NULL
|| (out == NULL && *out_pos != out_size)
|| *out_pos > out_size)
return LZMA_PROG_ERROR;
if (flags & LZMA_TELL_ANY_CHECK)
return LZMA_PROG_ERROR;
lzma_next_coder stream_decoder = LZMA_NEXT_CODER_INIT;
lzma_ret ret = lzma_stream_decoder_init(
&stream_decoder, allocator, *memlimit, flags);
if (ret == LZMA_OK) {
const size_t in_start = *in_pos;
const size_t out_start = *out_pos;
ret = stream_decoder.code(stream_decoder.coder, allocator,
in, in_pos, in_size, out, out_pos, out_size,
LZMA_FINISH);
if (ret == LZMA_STREAM_END) {
ret = LZMA_OK;
} else {
*in_pos = in_start;
*out_pos = out_start;
if (ret == LZMA_OK) {
assert(*in_pos == in_size
|| *out_pos == out_size);
if (*in_pos == in_size)
ret = LZMA_DATA_ERROR;
else
ret = LZMA_BUF_ERROR;
} else if (ret == LZMA_MEMLIMIT_ERROR) {
uint64_t memusage;
(void)stream_decoder.memconfig(
stream_decoder.coder,
memlimit, &memusage, 0);
}
}
}
lzma_next_end(&stream_decoder, allocator);
return ret;
}
