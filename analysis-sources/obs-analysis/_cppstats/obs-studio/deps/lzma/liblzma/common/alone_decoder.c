#include "alone_decoder.h"
#include "lzma_decoder.h"
#include "lz_decoder.h"
struct lzma_coder_s {
lzma_next_coder next;
enum {
SEQ_PROPERTIES,
SEQ_DICTIONARY_SIZE,
SEQ_UNCOMPRESSED_SIZE,
SEQ_CODER_INIT,
SEQ_CODE,
} sequence;
bool picky;
size_t pos;
lzma_vli uncompressed_size;
uint64_t memlimit;
uint64_t memusage;
lzma_options_lzma options;
};
static lzma_ret
alone_decode(lzma_coder *coder,
lzma_allocator *allocator lzma_attribute((__unused__)),
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size,
lzma_action action)
{
while (*out_pos < out_size
&& (coder->sequence == SEQ_CODE || *in_pos < in_size))
switch (coder->sequence) {
case SEQ_PROPERTIES:
if (lzma_lzma_lclppb_decode(&coder->options, in[*in_pos]))
return LZMA_FORMAT_ERROR;
coder->sequence = SEQ_DICTIONARY_SIZE;
++*in_pos;
break;
case SEQ_DICTIONARY_SIZE:
coder->options.dict_size
|= (size_t)(in[*in_pos]) << (coder->pos * 8);
if (++coder->pos == 4) {
if (coder->picky && coder->options.dict_size
!= UINT32_MAX) {
uint32_t d = coder->options.dict_size - 1;
d |= d >> 2;
d |= d >> 3;
d |= d >> 4;
d |= d >> 8;
d |= d >> 16;
++d;
if (d != coder->options.dict_size)
return LZMA_FORMAT_ERROR;
}
coder->pos = 0;
coder->sequence = SEQ_UNCOMPRESSED_SIZE;
}
++*in_pos;
break;
case SEQ_UNCOMPRESSED_SIZE:
coder->uncompressed_size
|= (lzma_vli)(in[*in_pos]) << (coder->pos * 8);
++*in_pos;
if (++coder->pos < 8)
break;
if (coder->picky
&& coder->uncompressed_size != LZMA_VLI_UNKNOWN
&& coder->uncompressed_size
>= (LZMA_VLI_C(1) << 38))
return LZMA_FORMAT_ERROR;
coder->memusage = lzma_lzma_decoder_memusage(&coder->options)
+ LZMA_MEMUSAGE_BASE;
coder->pos = 0;
coder->sequence = SEQ_CODER_INIT;
case SEQ_CODER_INIT: {
if (coder->memusage > coder->memlimit)
return LZMA_MEMLIMIT_ERROR;
lzma_filter_info filters[2] = {
{
.init = &lzma_lzma_decoder_init,
.options = &coder->options,
}, {
.init = NULL,
}
};
const lzma_ret ret = lzma_next_filter_init(&coder->next,
allocator, filters);
if (ret != LZMA_OK)
return ret;
lzma_lz_decoder_uncompressed(coder->next.coder,
coder->uncompressed_size);
coder->sequence = SEQ_CODE;
break;
}
case SEQ_CODE: {
return coder->next.code(coder->next.coder,
allocator, in, in_pos, in_size,
out, out_pos, out_size, action);
}
default:
return LZMA_PROG_ERROR;
}
return LZMA_OK;
}
static void
alone_decoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_next_end(&coder->next, allocator);
lzma_free(coder, allocator);
return;
}
static lzma_ret
alone_decoder_memconfig(lzma_coder *coder, uint64_t *memusage,
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
lzma_alone_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
uint64_t memlimit, bool picky)
{
lzma_next_coder_init(&lzma_alone_decoder_init, next, allocator);
if (memlimit == 0)
return LZMA_PROG_ERROR;
if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;
next->code = &alone_decode;
next->end = &alone_decoder_end;
next->memconfig = &alone_decoder_memconfig;
next->coder->next = LZMA_NEXT_CODER_INIT;
}
next->coder->sequence = SEQ_PROPERTIES;
next->coder->picky = picky;
next->coder->pos = 0;
next->coder->options.dict_size = 0;
next->coder->options.preset_dict = NULL;
next->coder->options.preset_dict_size = 0;
next->coder->uncompressed_size = 0;
next->coder->memlimit = memlimit;
next->coder->memusage = LZMA_MEMUSAGE_BASE;
return LZMA_OK;
}
extern LZMA_API(lzma_ret)
lzma_alone_decoder(lzma_stream *strm, uint64_t memlimit)
{
lzma_next_strm_init(lzma_alone_decoder_init, strm, memlimit, false);
strm->internal->supported_actions[LZMA_RUN] = true;
strm->internal->supported_actions[LZMA_FINISH] = true;
return LZMA_OK;
}
