











#include "block_encoder.h"
#include "filter_encoder.h"
#include "check.h"


struct lzma_coder_s {

lzma_next_coder next;




lzma_block *block;

enum {
SEQ_CODE,
SEQ_PADDING,
SEQ_CHECK,
} sequence;


lzma_vli compressed_size;


lzma_vli uncompressed_size;


size_t pos;


lzma_check_state check;
};


static lzma_ret
block_encode(lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size, lzma_action action)
{

if (LZMA_VLI_MAX - coder->uncompressed_size < in_size - *in_pos)
return LZMA_DATA_ERROR;

switch (coder->sequence) {
case SEQ_CODE: {
const size_t in_start = *in_pos;
const size_t out_start = *out_pos;

const lzma_ret ret = coder->next.code(coder->next.coder,
allocator, in, in_pos, in_size,
out, out_pos, out_size, action);

const size_t in_used = *in_pos - in_start;
const size_t out_used = *out_pos - out_start;

if (COMPRESSED_SIZE_MAX - coder->compressed_size < out_used)
return LZMA_DATA_ERROR;

coder->compressed_size += out_used;



coder->uncompressed_size += in_used;

lzma_check_update(&coder->check, coder->block->check,
in + in_start, in_used);

if (ret != LZMA_STREAM_END || action == LZMA_SYNC_FLUSH)
return ret;

assert(*in_pos == in_size);
assert(action == LZMA_FINISH);



coder->block->compressed_size = coder->compressed_size;
coder->block->uncompressed_size = coder->uncompressed_size;

coder->sequence = SEQ_PADDING;
}



case SEQ_PADDING:



while (coder->compressed_size & 3) {
if (*out_pos >= out_size)
return LZMA_OK;

out[*out_pos] = 0x00;
++*out_pos;
++coder->compressed_size;
}

if (coder->block->check == LZMA_CHECK_NONE)
return LZMA_STREAM_END;

lzma_check_finish(&coder->check, coder->block->check);

coder->sequence = SEQ_CHECK;



case SEQ_CHECK: {
const size_t check_size = lzma_check_size(coder->block->check);
lzma_bufcpy(coder->check.buffer.u8, &coder->pos, check_size,
out, out_pos, out_size);
if (coder->pos < check_size)
return LZMA_OK;

memcpy(coder->block->raw_check, coder->check.buffer.u8,
check_size);
return LZMA_STREAM_END;
}
}

return LZMA_PROG_ERROR;
}


static void
block_encoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_next_end(&coder->next, allocator);
lzma_free(coder, allocator);
return;
}


static lzma_ret
block_encoder_update(lzma_coder *coder, lzma_allocator *allocator,
const lzma_filter *filters lzma_attribute((__unused__)),
const lzma_filter *reversed_filters)
{
if (coder->sequence != SEQ_CODE)
return LZMA_PROG_ERROR;

return lzma_next_filter_update(
&coder->next, allocator, reversed_filters);
}


extern lzma_ret
lzma_block_encoder_init(lzma_next_coder *next, lzma_allocator *allocator,
lzma_block *block)
{
lzma_next_coder_init(&lzma_block_encoder_init, next, allocator);

if (block == NULL)
return LZMA_PROG_ERROR;



if (block->version != 0)
return LZMA_OPTIONS_ERROR;



if ((unsigned int)(block->check) > LZMA_CHECK_ID_MAX)
return LZMA_PROG_ERROR;

if (!lzma_check_is_supported(block->check))
return LZMA_UNSUPPORTED_CHECK;


if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;

next->code = &block_encode;
next->end = &block_encoder_end;
next->update = &block_encoder_update;
next->coder->next = LZMA_NEXT_CODER_INIT;
}


next->coder->sequence = SEQ_CODE;
next->coder->block = block;
next->coder->compressed_size = 0;
next->coder->uncompressed_size = 0;
next->coder->pos = 0;


lzma_check_init(&next->coder->check, block->check);


return lzma_raw_encoder_init(&next->coder->next, allocator,
block->filters);
}


extern LZMA_API(lzma_ret)
lzma_block_encoder(lzma_stream *strm, lzma_block *block)
{
lzma_next_strm_init(lzma_block_encoder_init, strm, block);

strm->internal->supported_actions[LZMA_RUN] = true;
strm->internal->supported_actions[LZMA_FINISH] = true;

return LZMA_OK;
}
