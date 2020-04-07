#include "block_decoder.h"
#include "filter_decoder.h"
#include "check.h"
struct lzma_coder_s {
enum {
SEQ_CODE,
SEQ_PADDING,
SEQ_CHECK,
} sequence;
lzma_next_coder next;
lzma_block *block;
lzma_vli compressed_size;
lzma_vli uncompressed_size;
lzma_vli compressed_limit;
size_t check_pos;
lzma_check_state check;
};
static inline bool
update_size(lzma_vli *size, lzma_vli add, lzma_vli limit)
{
if (limit > LZMA_VLI_MAX)
limit = LZMA_VLI_MAX;
if (limit < *size || limit - *size < add)
return true;
*size += add;
return false;
}
static inline bool
is_size_valid(lzma_vli size, lzma_vli reference)
{
return reference == LZMA_VLI_UNKNOWN || reference == size;
}
static lzma_ret
block_decode(lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size, lzma_action action)
{
switch (coder->sequence) {
case SEQ_CODE: {
const size_t in_start = *in_pos;
const size_t out_start = *out_pos;
const lzma_ret ret = coder->next.code(coder->next.coder,
allocator, in, in_pos, in_size,
out, out_pos, out_size, action);
const size_t in_used = *in_pos - in_start;
const size_t out_used = *out_pos - out_start;
if (update_size(&coder->compressed_size, in_used,
coder->compressed_limit)
|| update_size(&coder->uncompressed_size,
out_used,
coder->block->uncompressed_size))
return LZMA_DATA_ERROR;
lzma_check_update(&coder->check, coder->block->check,
out + out_start, out_used);
if (ret != LZMA_STREAM_END)
return ret;
if (!is_size_valid(coder->compressed_size,
coder->block->compressed_size)
|| !is_size_valid(coder->uncompressed_size,
coder->block->uncompressed_size))
return LZMA_DATA_ERROR;
coder->block->compressed_size = coder->compressed_size;
coder->block->uncompressed_size = coder->uncompressed_size;
coder->sequence = SEQ_PADDING;
}
case SEQ_PADDING:
while (coder->compressed_size & 3) {
if (*in_pos >= in_size)
return LZMA_OK;
++coder->compressed_size;
if (in[(*in_pos)++] != 0x00)
return LZMA_DATA_ERROR;
}
if (coder->block->check == LZMA_CHECK_NONE)
return LZMA_STREAM_END;
lzma_check_finish(&coder->check, coder->block->check);
coder->sequence = SEQ_CHECK;
case SEQ_CHECK: {
const size_t check_size = lzma_check_size(coder->block->check);
lzma_bufcpy(in, in_pos, in_size, coder->block->raw_check,
&coder->check_pos, check_size);
if (coder->check_pos < check_size)
return LZMA_OK;
if (lzma_check_is_supported(coder->block->check)
&& memcmp(coder->block->raw_check,
coder->check.buffer.u8,
check_size) != 0)
return LZMA_DATA_ERROR;
return LZMA_STREAM_END;
}
}
return LZMA_PROG_ERROR;
}
static void
block_decoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_next_end(&coder->next, allocator);
lzma_free(coder, allocator);
return;
}
extern lzma_ret
lzma_block_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
lzma_block *block)
{
lzma_next_coder_init(&lzma_block_decoder_init, next, allocator);
if (lzma_block_unpadded_size(block) == 0
|| !lzma_vli_is_valid(block->uncompressed_size))
return LZMA_PROG_ERROR;
if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;
next->code = &block_decode;
next->end = &block_decoder_end;
next->coder->next = LZMA_NEXT_CODER_INIT;
}
next->coder->sequence = SEQ_CODE;
next->coder->block = block;
next->coder->compressed_size = 0;
next->coder->uncompressed_size = 0;
next->coder->compressed_limit
= block->compressed_size == LZMA_VLI_UNKNOWN
? (LZMA_VLI_MAX & ~LZMA_VLI_C(3))
- block->header_size
- lzma_check_size(block->check)
: block->compressed_size;
next->coder->check_pos = 0;
lzma_check_init(&next->coder->check, block->check);
return lzma_raw_decoder_init(&next->coder->next, allocator,
block->filters);
}
extern LZMA_API(lzma_ret)
lzma_block_decoder(lzma_stream *strm, lzma_block *block)
{
lzma_next_strm_init(lzma_block_decoder_init, strm, block);
strm->internal->supported_actions[LZMA_RUN] = true;
strm->internal->supported_actions[LZMA_FINISH] = true;
return LZMA_OK;
}
