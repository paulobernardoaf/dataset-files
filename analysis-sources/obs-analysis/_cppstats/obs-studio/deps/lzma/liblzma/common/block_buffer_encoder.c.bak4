











#include "block_encoder.h"
#include "filter_encoder.h"
#include "lzma2_encoder.h"
#include "check.h"










#define HEADERS_BOUND ((1 + 1 + 2 * LZMA_VLI_BYTES_MAX + 3 + 4 + LZMA_CHECK_SIZE_MAX + 3) & ~3)



static lzma_vli
lzma2_bound(lzma_vli uncompressed_size)
{

if (uncompressed_size > COMPRESSED_SIZE_MAX)
return 0;





const lzma_vli overhead = ((uncompressed_size + LZMA2_CHUNK_MAX - 1)
/ LZMA2_CHUNK_MAX)
* LZMA2_HEADER_UNCOMPRESSED + 1;


if (COMPRESSED_SIZE_MAX - overhead < uncompressed_size)
return 0;

return uncompressed_size + overhead;
}


extern LZMA_API(size_t)
lzma_block_buffer_bound(size_t uncompressed_size)
{





lzma_vli lzma2_size = lzma2_bound(uncompressed_size);
if (lzma2_size == 0)
return 0;


lzma2_size = (lzma2_size + 3) & ~LZMA_VLI_C(3);

#if SIZE_MAX < LZMA_VLI_MAX



if (SIZE_MAX - HEADERS_BOUND < lzma2_size)
return 0;
#endif

return HEADERS_BOUND + lzma2_size;
}


static lzma_ret
block_encode_uncompressed(lzma_block *block, const uint8_t *in, size_t in_size,
uint8_t *out, size_t *out_pos, size_t out_size)
{






lzma_options_lzma lzma2 = {
.dict_size = LZMA_DICT_SIZE_MIN,
};

lzma_filter filters[2];
filters[0].id = LZMA_FILTER_LZMA2;
filters[0].options = &lzma2;
filters[1].id = LZMA_VLI_UNKNOWN;



lzma_filter *filters_orig = block->filters;
block->filters = filters;

if (lzma_block_header_size(block) != LZMA_OK) {
block->filters = filters_orig;
return LZMA_PROG_ERROR;
}






assert(block->compressed_size == lzma2_bound(in_size));
if (out_size - *out_pos
< block->header_size + block->compressed_size) {
block->filters = filters_orig;
return LZMA_BUF_ERROR;
}

if (lzma_block_header_encode(block, out + *out_pos) != LZMA_OK) {
block->filters = filters_orig;
return LZMA_PROG_ERROR;
}

block->filters = filters_orig;
*out_pos += block->header_size;


size_t in_pos = 0;
uint8_t control = 0x01; 

while (in_pos < in_size) {


out[(*out_pos)++] = control;
control = 0x02; 


const size_t copy_size
= my_min(in_size - in_pos, LZMA2_CHUNK_MAX);
out[(*out_pos)++] = (copy_size - 1) >> 8;
out[(*out_pos)++] = (copy_size - 1) & 0xFF;


assert(*out_pos + copy_size <= out_size);
memcpy(out + *out_pos, in + in_pos, copy_size);

in_pos += copy_size;
*out_pos += copy_size;
}


out[(*out_pos)++] = 0x00;
assert(*out_pos <= out_size);

return LZMA_OK;
}


static lzma_ret
block_encode_normal(lzma_block *block, lzma_allocator *allocator,
const uint8_t *in, size_t in_size,
uint8_t *out, size_t *out_pos, size_t out_size)
{

block->compressed_size = lzma2_bound(in_size);
if (block->compressed_size == 0)
return LZMA_DATA_ERROR;

block->uncompressed_size = in_size;
return_if_error(lzma_block_header_size(block));


if (out_size - *out_pos <= block->header_size)
return LZMA_BUF_ERROR;

const size_t out_start = *out_pos;
*out_pos += block->header_size;



if (out_size - *out_pos > block->compressed_size)
out_size = *out_pos + block->compressed_size;



lzma_next_coder raw_encoder = LZMA_NEXT_CODER_INIT;
lzma_ret ret = lzma_raw_encoder_init(
&raw_encoder, allocator, block->filters);

if (ret == LZMA_OK) {
size_t in_pos = 0;
ret = raw_encoder.code(raw_encoder.coder, allocator,
in, &in_pos, in_size, out, out_pos, out_size,
LZMA_FINISH);
}


lzma_next_end(&raw_encoder, allocator);

if (ret == LZMA_STREAM_END) {

block->compressed_size
= *out_pos - (out_start + block->header_size);
ret = lzma_block_header_encode(block, out + out_start);
if (ret != LZMA_OK)
ret = LZMA_PROG_ERROR;

} else if (ret == LZMA_OK) {

ret = LZMA_BUF_ERROR;
}


if (ret != LZMA_OK)
*out_pos = out_start;

return ret;
}


extern LZMA_API(lzma_ret)
lzma_block_buffer_encode(lzma_block *block, lzma_allocator *allocator,
const uint8_t *in, size_t in_size,
uint8_t *out, size_t *out_pos, size_t out_size)
{

if (block == NULL || (in == NULL && in_size != 0) || out == NULL
|| out_pos == NULL || *out_pos > out_size)
return LZMA_PROG_ERROR;



if (block->version != 0)
return LZMA_OPTIONS_ERROR;

if ((unsigned int)(block->check) > LZMA_CHECK_ID_MAX
|| block->filters == NULL)
return LZMA_PROG_ERROR;

if (!lzma_check_is_supported(block->check))
return LZMA_UNSUPPORTED_CHECK;




out_size -= (out_size - *out_pos) & 3;


const size_t check_size = lzma_check_size(block->check);
assert(check_size != UINT32_MAX);


if (out_size - *out_pos <= check_size)
return LZMA_BUF_ERROR;

out_size -= check_size;


const lzma_ret ret = block_encode_normal(block, allocator,
in, in_size, out, out_pos, out_size);
if (ret != LZMA_OK) {


if (ret != LZMA_BUF_ERROR)
return ret;






return_if_error(block_encode_uncompressed(block, in, in_size,
out, out_pos, out_size));
}

assert(*out_pos <= out_size);




for (size_t i = (size_t)(block->compressed_size); i & 3; ++i) {
assert(*out_pos < out_size);
out[(*out_pos)++] = 0x00;
}


if (check_size > 0) {



lzma_check_state check;
lzma_check_init(&check, block->check);
lzma_check_update(&check, block->check, in, in_size);
lzma_check_finish(&check, block->check);

memcpy(block->raw_check, check.buffer.u8, check_size);
memcpy(out + *out_pos, check.buffer.u8, check_size);
*out_pos += check_size;
}

return LZMA_OK;
}
