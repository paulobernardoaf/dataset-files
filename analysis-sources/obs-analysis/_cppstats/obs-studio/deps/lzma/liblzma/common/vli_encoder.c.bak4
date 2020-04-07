











#include "common.h"


extern LZMA_API(lzma_ret)
lzma_vli_encode(lzma_vli vli, size_t *vli_pos,
uint8_t *restrict out, size_t *restrict out_pos,
size_t out_size)
{

size_t vli_pos_internal = 0;
if (vli_pos == NULL) {
vli_pos = &vli_pos_internal;



if (*out_pos >= out_size)
return LZMA_PROG_ERROR;
} else {


if (*out_pos >= out_size)
return LZMA_BUF_ERROR;
}


if (*vli_pos >= LZMA_VLI_BYTES_MAX || vli > LZMA_VLI_MAX)
return LZMA_PROG_ERROR;



vli >>= *vli_pos * 7;


while (vli >= 0x80) {



++*vli_pos;
assert(*vli_pos < LZMA_VLI_BYTES_MAX);


out[*out_pos] = (uint8_t)(vli) | 0x80;
vli >>= 7;

if (++*out_pos == out_size)
return vli_pos == &vli_pos_internal
? LZMA_PROG_ERROR : LZMA_OK;
}


out[*out_pos] = (uint8_t)(vli);
++*out_pos;
++*vli_pos;

return vli_pos == &vli_pos_internal ? LZMA_OK : LZMA_STREAM_END;

}
