











#include "common.h"
#include "index.h"


extern LZMA_API(lzma_ret)
lzma_block_compressed_size(lzma_block *block, lzma_vli unpadded_size)
{

if (lzma_block_unpadded_size(block) == 0)
return LZMA_PROG_ERROR;

const uint32_t container_size = block->header_size
+ lzma_check_size(block->check);


if (unpadded_size <= container_size)
return LZMA_DATA_ERROR;




const lzma_vli compressed_size = unpadded_size - container_size;
if (block->compressed_size != LZMA_VLI_UNKNOWN
&& block->compressed_size != compressed_size)
return LZMA_DATA_ERROR;

block->compressed_size = compressed_size;

return LZMA_OK;
}


extern LZMA_API(lzma_vli)
lzma_block_unpadded_size(const lzma_block *block)
{






if (block == NULL || block->version != 0
|| block->header_size < LZMA_BLOCK_HEADER_SIZE_MIN
|| block->header_size > LZMA_BLOCK_HEADER_SIZE_MAX
|| (block->header_size & 3)
|| !lzma_vli_is_valid(block->compressed_size)
|| block->compressed_size == 0
|| (unsigned int)(block->check) > LZMA_CHECK_ID_MAX)
return 0;



if (block->compressed_size == LZMA_VLI_UNKNOWN)
return LZMA_VLI_UNKNOWN;


const lzma_vli unpadded_size = block->compressed_size
+ block->header_size
+ lzma_check_size(block->check);

assert(unpadded_size >= UNPADDED_SIZE_MIN);
if (unpadded_size > UNPADDED_SIZE_MAX)
return 0;

return unpadded_size;
}


extern LZMA_API(lzma_vli)
lzma_block_total_size(const lzma_block *block)
{
lzma_vli unpadded_size = lzma_block_unpadded_size(block);

if (unpadded_size != LZMA_VLI_UNKNOWN)
unpadded_size = vli_ceil4(unpadded_size);

return unpadded_size;
}
