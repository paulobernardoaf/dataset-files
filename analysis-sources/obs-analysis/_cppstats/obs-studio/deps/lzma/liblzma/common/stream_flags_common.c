#include "stream_flags_common.h"
const uint8_t lzma_header_magic[6] = { 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 };
const uint8_t lzma_footer_magic[2] = { 0x59, 0x5A };
extern LZMA_API(lzma_ret)
lzma_stream_flags_compare(
const lzma_stream_flags *a, const lzma_stream_flags *b)
{
if (a->version != 0 || b->version != 0)
return LZMA_OPTIONS_ERROR;
if ((unsigned int)(a->check) > LZMA_CHECK_ID_MAX
|| (unsigned int)(b->check) > LZMA_CHECK_ID_MAX)
return LZMA_PROG_ERROR;
if (a->check != b->check)
return LZMA_DATA_ERROR;
if (a->backward_size != LZMA_VLI_UNKNOWN
&& b->backward_size != LZMA_VLI_UNKNOWN) {
if (!is_backward_size_valid(a) || !is_backward_size_valid(b))
return LZMA_PROG_ERROR;
if (a->backward_size != b->backward_size)
return LZMA_DATA_ERROR;
}
return LZMA_OK;
}
