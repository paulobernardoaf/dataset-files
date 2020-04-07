#include "common.h"
#define UNPADDED_SIZE_MIN LZMA_VLI_C(5)
#define UNPADDED_SIZE_MAX (LZMA_VLI_MAX & ~LZMA_VLI_C(3))
extern uint32_t lzma_index_padding_size(const lzma_index *i);
extern void lzma_index_prealloc(lzma_index *i, lzma_vli records);
static inline lzma_vli
vli_ceil4(lzma_vli vli)
{
assert(vli <= LZMA_VLI_MAX);
return (vli + 3) & ~LZMA_VLI_C(3);
}
static inline lzma_vli
index_size_unpadded(lzma_vli count, lzma_vli index_list_size)
{
return 1 + lzma_vli_size(count) + index_list_size + 4;
}
static inline lzma_vli
index_size(lzma_vli count, lzma_vli index_list_size)
{
return vli_ceil4(index_size_unpadded(count, index_list_size));
}
static inline lzma_vli
index_stream_size(lzma_vli blocks_size,
lzma_vli count, lzma_vli index_list_size)
{
return LZMA_STREAM_HEADER_SIZE + blocks_size
+ index_size(count, index_list_size)
+ LZMA_STREAM_HEADER_SIZE;
}
