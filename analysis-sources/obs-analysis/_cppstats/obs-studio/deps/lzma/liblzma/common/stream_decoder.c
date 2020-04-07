#include "stream_decoder.h"
#include "block_decoder.h"
struct lzma_coder_s {
enum {
SEQ_STREAM_HEADER,
SEQ_BLOCK_HEADER,
SEQ_BLOCK,
SEQ_INDEX,
SEQ_STREAM_FOOTER,
SEQ_STREAM_PADDING,
} sequence;
lzma_next_coder block_decoder;
lzma_block block_options;
lzma_stream_flags stream_flags;
lzma_index_hash *index_hash;
uint64_t memlimit;
uint64_t memusage;
bool tell_no_check;
bool tell_unsupported_check;
bool tell_any_check;
bool concatenated;
bool first_stream;
size_t pos;
uint8_t buffer[LZMA_BLOCK_HEADER_SIZE_MAX];
};
static lzma_ret
stream_decoder_reset(lzma_coder *coder, lzma_allocator *allocator)
{
coder->index_hash = lzma_index_hash_init(coder->index_hash, allocator);
if (coder->index_hash == NULL)
return LZMA_MEM_ERROR;
coder->sequence = SEQ_STREAM_HEADER;
coder->pos = 0;
return LZMA_OK;
}
static lzma_ret
stream_decode(lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size, lzma_action action)
{
while (true)
switch (coder->sequence) {
case SEQ_STREAM_HEADER: {
lzma_bufcpy(in, in_pos, in_size, coder->buffer, &coder->pos,
LZMA_STREAM_HEADER_SIZE);
if (coder->pos < LZMA_STREAM_HEADER_SIZE)
return LZMA_OK;
coder->pos = 0;
const lzma_ret ret = lzma_stream_header_decode(
&coder->stream_flags, coder->buffer);
if (ret != LZMA_OK)
return ret == LZMA_FORMAT_ERROR && !coder->first_stream
? LZMA_DATA_ERROR : ret;
coder->first_stream = false;
coder->block_options.check = coder->stream_flags.check;
coder->sequence = SEQ_BLOCK_HEADER;
if (coder->tell_no_check && coder->stream_flags.check
== LZMA_CHECK_NONE)
return LZMA_NO_CHECK;
if (coder->tell_unsupported_check
&& !lzma_check_is_supported(
coder->stream_flags.check))
return LZMA_UNSUPPORTED_CHECK;
if (coder->tell_any_check)
return LZMA_GET_CHECK;
}
case SEQ_BLOCK_HEADER: {
if (*in_pos >= in_size)
return LZMA_OK;
if (coder->pos == 0) {
if (in[*in_pos] == 0x00) {
coder->sequence = SEQ_INDEX;
break;
}
coder->block_options.header_size
= lzma_block_header_size_decode(
in[*in_pos]);
}
lzma_bufcpy(in, in_pos, in_size, coder->buffer, &coder->pos,
coder->block_options.header_size);
if (coder->pos < coder->block_options.header_size)
return LZMA_OK;
coder->pos = 0;
coder->block_options.version = 0;
lzma_filter filters[LZMA_FILTERS_MAX + 1];
coder->block_options.filters = filters;
return_if_error(lzma_block_header_decode(&coder->block_options,
allocator, coder->buffer));
const uint64_t memusage = lzma_raw_decoder_memusage(filters);
lzma_ret ret;
if (memusage == UINT64_MAX) {
ret = LZMA_OPTIONS_ERROR;
} else {
coder->memusage = memusage;
if (memusage > coder->memlimit) {
ret = LZMA_MEMLIMIT_ERROR;
} else {
ret = lzma_block_decoder_init(
&coder->block_decoder,
allocator,
&coder->block_options);
}
}
for (size_t i = 0; i < LZMA_FILTERS_MAX; ++i)
lzma_free(filters[i].options, allocator);
coder->block_options.filters = NULL;
if (ret != LZMA_OK)
return ret;
coder->sequence = SEQ_BLOCK;
}
case SEQ_BLOCK: {
const lzma_ret ret = coder->block_decoder.code(
coder->block_decoder.coder, allocator,
in, in_pos, in_size, out, out_pos, out_size,
action);
if (ret != LZMA_STREAM_END)
return ret;
return_if_error(lzma_index_hash_append(coder->index_hash,
lzma_block_unpadded_size(
&coder->block_options),
coder->block_options.uncompressed_size));
coder->sequence = SEQ_BLOCK_HEADER;
break;
}
case SEQ_INDEX: {
if (*in_pos >= in_size)
return LZMA_OK;
const lzma_ret ret = lzma_index_hash_decode(coder->index_hash,
in, in_pos, in_size);
if (ret != LZMA_STREAM_END)
return ret;
coder->sequence = SEQ_STREAM_FOOTER;
}
case SEQ_STREAM_FOOTER: {
lzma_bufcpy(in, in_pos, in_size, coder->buffer, &coder->pos,
LZMA_STREAM_HEADER_SIZE);
if (coder->pos < LZMA_STREAM_HEADER_SIZE)
return LZMA_OK;
coder->pos = 0;
lzma_stream_flags footer_flags;
const lzma_ret ret = lzma_stream_footer_decode(
&footer_flags, coder->buffer);
if (ret != LZMA_OK)
return ret == LZMA_FORMAT_ERROR
? LZMA_DATA_ERROR : ret;
if (lzma_index_hash_size(coder->index_hash)
!= footer_flags.backward_size)
return LZMA_DATA_ERROR;
return_if_error(lzma_stream_flags_compare(
&coder->stream_flags, &footer_flags));
if (!coder->concatenated)
return LZMA_STREAM_END;
coder->sequence = SEQ_STREAM_PADDING;
}
case SEQ_STREAM_PADDING:
assert(coder->concatenated);
while (true) {
if (*in_pos >= in_size) {
if (action != LZMA_FINISH)
return LZMA_OK;
return coder->pos == 0
? LZMA_STREAM_END
: LZMA_DATA_ERROR;
}
if (in[*in_pos] != 0x00)
break;
++*in_pos;
coder->pos = (coder->pos + 1) & 3;
}
if (coder->pos != 0) {
++*in_pos;
return LZMA_DATA_ERROR;
}
return_if_error(stream_decoder_reset(coder, allocator));
break;
default:
assert(0);
return LZMA_PROG_ERROR;
}
}
static void
stream_decoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_next_end(&coder->block_decoder, allocator);
lzma_index_hash_end(coder->index_hash, allocator);
lzma_free(coder, allocator);
return;
}
static lzma_check
stream_decoder_get_check(const lzma_coder *coder)
{
return coder->stream_flags.check;
}
static lzma_ret
stream_decoder_memconfig(lzma_coder *coder, uint64_t *memusage,
uint64_t *old_memlimit, uint64_t new_memlimit)
{
*memusage = coder->memusage;
*old_memlimit = coder->memlimit;
if (new_memlimit != 0) {
if (new_memlimit < coder->memusage)
return LZMA_MEMLIMIT_ERROR;
coder->memlimit = new_memlimit;
}
return LZMA_OK;
}
extern lzma_ret
lzma_stream_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
uint64_t memlimit, uint32_t flags)
{
lzma_next_coder_init(&lzma_stream_decoder_init, next, allocator);
if (memlimit == 0)
return LZMA_PROG_ERROR;
if (flags & ~LZMA_SUPPORTED_FLAGS)
return LZMA_OPTIONS_ERROR;
if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;
next->code = &stream_decode;
next->end = &stream_decoder_end;
next->get_check = &stream_decoder_get_check;
next->memconfig = &stream_decoder_memconfig;
next->coder->block_decoder = LZMA_NEXT_CODER_INIT;
next->coder->index_hash = NULL;
}
next->coder->memlimit = memlimit;
next->coder->memusage = LZMA_MEMUSAGE_BASE;
next->coder->tell_no_check = (flags & LZMA_TELL_NO_CHECK) != 0;
next->coder->tell_unsupported_check
= (flags & LZMA_TELL_UNSUPPORTED_CHECK) != 0;
next->coder->tell_any_check = (flags & LZMA_TELL_ANY_CHECK) != 0;
next->coder->concatenated = (flags & LZMA_CONCATENATED) != 0;
next->coder->first_stream = true;
return stream_decoder_reset(next->coder, allocator);
}
extern LZMA_API(lzma_ret)
lzma_stream_decoder(lzma_stream *strm, uint64_t memlimit, uint32_t flags)
{
lzma_next_strm_init(lzma_stream_decoder_init, strm, memlimit, flags);
strm->internal->supported_actions[LZMA_RUN] = true;
strm->internal->supported_actions[LZMA_FINISH] = true;
return LZMA_OK;
}
