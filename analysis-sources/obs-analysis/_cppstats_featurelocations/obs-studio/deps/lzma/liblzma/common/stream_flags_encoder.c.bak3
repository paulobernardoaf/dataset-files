











#include "stream_flags_common.h"


static bool
stream_flags_encode(const lzma_stream_flags *options, uint8_t *out)
{
if ((unsigned int)(options->check) > LZMA_CHECK_ID_MAX)
return true;

out[0] = 0x00;
out[1] = options->check;

return false;
}


extern LZMA_API(lzma_ret)
lzma_stream_header_encode(const lzma_stream_flags *options, uint8_t *out)
{
assert(sizeof(lzma_header_magic) + LZMA_STREAM_FLAGS_SIZE
+ 4 == LZMA_STREAM_HEADER_SIZE);

if (options->version != 0)
return LZMA_OPTIONS_ERROR;


memcpy(out, lzma_header_magic, sizeof(lzma_header_magic));


if (stream_flags_encode(options, out + sizeof(lzma_header_magic)))
return LZMA_PROG_ERROR;


const uint32_t crc = lzma_crc32(out + sizeof(lzma_header_magic),
LZMA_STREAM_FLAGS_SIZE, 0);

unaligned_write32le(out + sizeof(lzma_header_magic)
+ LZMA_STREAM_FLAGS_SIZE, crc);

return LZMA_OK;
}


extern LZMA_API(lzma_ret)
lzma_stream_footer_encode(const lzma_stream_flags *options, uint8_t *out)
{
assert(2 * 4 + LZMA_STREAM_FLAGS_SIZE + sizeof(lzma_footer_magic)
== LZMA_STREAM_HEADER_SIZE);

if (options->version != 0)
return LZMA_OPTIONS_ERROR;


if (!is_backward_size_valid(options))
return LZMA_PROG_ERROR;

unaligned_write32le(out + 4, options->backward_size / 4 - 1);


if (stream_flags_encode(options, out + 2 * 4))
return LZMA_PROG_ERROR;


const uint32_t crc = lzma_crc32(
out + 4, 4 + LZMA_STREAM_FLAGS_SIZE, 0);

unaligned_write32le(out, crc);


memcpy(out + 2 * 4 + LZMA_STREAM_FLAGS_SIZE,
lzma_footer_magic, sizeof(lzma_footer_magic));

return LZMA_OK;
}
