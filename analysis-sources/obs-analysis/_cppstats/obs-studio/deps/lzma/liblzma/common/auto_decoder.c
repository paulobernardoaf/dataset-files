#include "stream_decoder.h"
#include "alone_decoder.h"
struct lzma_coder_s {
lzma_next_coder next;
uint64_t memlimit;
uint32_t flags;
enum {
SEQ_INIT,
SEQ_CODE,
SEQ_FINISH,
} sequence;
};
static lzma_ret
auto_decode(lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size, lzma_action action)
{
switch (coder->sequence) {
case SEQ_INIT:
if (*in_pos >= in_size)
return LZMA_OK;
coder->sequence = SEQ_CODE;
if (in[*in_pos] == 0xFD) {
return_if_error(lzma_stream_decoder_init(
&coder->next, allocator,
coder->memlimit, coder->flags));
} else {
return_if_error(lzma_alone_decoder_init(&coder->next,
allocator, coder->memlimit, true));
if (coder->flags & LZMA_TELL_NO_CHECK)
return LZMA_NO_CHECK;
if (coder->flags & LZMA_TELL_ANY_CHECK)
return LZMA_GET_CHECK;
}
case SEQ_CODE: {
const lzma_ret ret = coder->next.code(
coder->next.coder, allocator,
in, in_pos, in_size,
out, out_pos, out_size, action);
if (ret != LZMA_STREAM_END
|| (coder->flags & LZMA_CONCATENATED) == 0)
return ret;
coder->sequence = SEQ_FINISH;
}
case SEQ_FINISH:
if (*in_pos < in_size)
return LZMA_DATA_ERROR;
return action == LZMA_FINISH ? LZMA_STREAM_END : LZMA_OK;
default:
assert(0);
return LZMA_PROG_ERROR;
}
}
static void
auto_decoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_next_end(&coder->next, allocator);
lzma_free(coder, allocator);
return;
}
static lzma_check
auto_decoder_get_check(const lzma_coder *coder)
{
return coder->next.get_check == NULL ? LZMA_CHECK_NONE
: coder->next.get_check(coder->next.coder);
}
static lzma_ret
auto_decoder_memconfig(lzma_coder *coder, uint64_t *memusage,
uint64_t *old_memlimit, uint64_t new_memlimit)
{
lzma_ret ret;
if (coder->next.memconfig != NULL) {
ret = coder->next.memconfig(coder->next.coder,
memusage, old_memlimit, new_memlimit);
assert(*old_memlimit == coder->memlimit);
} else {
*memusage = LZMA_MEMUSAGE_BASE;
*old_memlimit = coder->memlimit;
ret = LZMA_OK;
}
if (ret == LZMA_OK && new_memlimit != 0)
coder->memlimit = new_memlimit;
return ret;
}
static lzma_ret
auto_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
uint64_t memlimit, uint32_t flags)
{
lzma_next_coder_init(&auto_decoder_init, next, allocator);
if (memlimit == 0)
return LZMA_PROG_ERROR;
if (flags & ~LZMA_SUPPORTED_FLAGS)
return LZMA_OPTIONS_ERROR;
if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;
next->code = &auto_decode;
next->end = &auto_decoder_end;
next->get_check = &auto_decoder_get_check;
next->memconfig = &auto_decoder_memconfig;
next->coder->next = LZMA_NEXT_CODER_INIT;
}
next->coder->memlimit = memlimit;
next->coder->flags = flags;
next->coder->sequence = SEQ_INIT;
return LZMA_OK;
}
extern LZMA_API(lzma_ret)
lzma_auto_decoder(lzma_stream *strm, uint64_t memlimit, uint32_t flags)
{
lzma_next_strm_init(auto_decoder_init, strm, memlimit, flags);
strm->internal->supported_actions[LZMA_RUN] = true;
strm->internal->supported_actions[LZMA_FINISH] = true;
return LZMA_OK;
}
