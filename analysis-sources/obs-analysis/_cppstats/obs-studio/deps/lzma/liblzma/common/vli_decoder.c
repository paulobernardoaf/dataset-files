#include "common.h"
extern LZMA_API(lzma_ret)
lzma_vli_decode(lzma_vli *restrict vli, size_t *vli_pos,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size)
{
size_t vli_pos_internal = 0;
if (vli_pos == NULL) {
vli_pos = &vli_pos_internal;
*vli = 0;
if (*in_pos >= in_size)
return LZMA_DATA_ERROR;
} else {
if (*vli_pos == 0)
*vli = 0;
if (*vli_pos >= LZMA_VLI_BYTES_MAX
|| (*vli >> (*vli_pos * 7)) != 0)
return LZMA_PROG_ERROR;;
if (*in_pos >= in_size)
return LZMA_BUF_ERROR;
}
do {
const uint8_t byte = in[*in_pos];
++*in_pos;
*vli += (lzma_vli)(byte & 0x7F) << (*vli_pos * 7);
++*vli_pos;
if ((byte & 0x80) == 0) {
if (byte == 0x00 && *vli_pos > 1)
return LZMA_DATA_ERROR;
return vli_pos == &vli_pos_internal
? LZMA_OK : LZMA_STREAM_END;
}
if (*vli_pos == LZMA_VLI_BYTES_MAX)
return LZMA_DATA_ERROR;
} while (*in_pos < in_size);
return vli_pos == &vli_pos_internal ? LZMA_DATA_ERROR : LZMA_OK;
}
