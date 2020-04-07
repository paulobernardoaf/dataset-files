#include "lzma2_decoder.h"
#include "lz_decoder.h"
#include "lzma_decoder.h"
struct lzma_coder_s {
enum sequence {
SEQ_CONTROL,
SEQ_UNCOMPRESSED_1,
SEQ_UNCOMPRESSED_2,
SEQ_COMPRESSED_0,
SEQ_COMPRESSED_1,
SEQ_PROPERTIES,
SEQ_LZMA,
SEQ_COPY,
} sequence;
enum sequence next_sequence;
lzma_lz_decoder lzma;
size_t uncompressed_size;
size_t compressed_size;
bool need_properties;
bool need_dictionary_reset;
lzma_options_lzma options;
};
static lzma_ret
lzma2_decode(lzma_coder *restrict coder, lzma_dict *restrict dict,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size)
{
while (*in_pos < in_size || coder->sequence == SEQ_LZMA)
switch (coder->sequence) {
case SEQ_CONTROL: {
const uint32_t control = in[*in_pos];
++*in_pos;
if (control == 0x00)
return LZMA_STREAM_END;
if (control >= 0xE0 || control == 1) {
coder->need_properties = true;
coder->need_dictionary_reset = true;
} else if (coder->need_dictionary_reset) {
return LZMA_DATA_ERROR;
}
if (control >= 0x80) {
coder->uncompressed_size = (control & 0x1F) << 16;
coder->sequence = SEQ_UNCOMPRESSED_1;
if (control >= 0xC0) {
coder->need_properties = false;
coder->next_sequence = SEQ_PROPERTIES;
} else if (coder->need_properties) {
return LZMA_DATA_ERROR;
} else {
coder->next_sequence = SEQ_LZMA;
if (control >= 0xA0)
coder->lzma.reset(coder->lzma.coder,
&coder->options);
}
} else {
if (control > 2)
return LZMA_DATA_ERROR;
coder->sequence = SEQ_COMPRESSED_0;
coder->next_sequence = SEQ_COPY;
}
if (coder->need_dictionary_reset) {
coder->need_dictionary_reset = false;
dict_reset(dict);
return LZMA_OK;
}
break;
}
case SEQ_UNCOMPRESSED_1:
coder->uncompressed_size += (uint32_t)(in[(*in_pos)++]) << 8;
coder->sequence = SEQ_UNCOMPRESSED_2;
break;
case SEQ_UNCOMPRESSED_2:
coder->uncompressed_size += in[(*in_pos)++] + 1;
coder->sequence = SEQ_COMPRESSED_0;
coder->lzma.set_uncompressed(coder->lzma.coder,
coder->uncompressed_size);
break;
case SEQ_COMPRESSED_0:
coder->compressed_size = (uint32_t)(in[(*in_pos)++]) << 8;
coder->sequence = SEQ_COMPRESSED_1;
break;
case SEQ_COMPRESSED_1:
coder->compressed_size += in[(*in_pos)++] + 1;
coder->sequence = coder->next_sequence;
break;
case SEQ_PROPERTIES:
if (lzma_lzma_lclppb_decode(&coder->options, in[(*in_pos)++]))
return LZMA_DATA_ERROR;
coder->lzma.reset(coder->lzma.coder, &coder->options);
coder->sequence = SEQ_LZMA;
break;
case SEQ_LZMA: {
const size_t in_start = *in_pos;
const lzma_ret ret = coder->lzma.code(coder->lzma.coder,
dict, in, in_pos, in_size);
const size_t in_used = *in_pos - in_start;
if (in_used > coder->compressed_size)
return LZMA_DATA_ERROR;
coder->compressed_size -= in_used;
if (ret != LZMA_STREAM_END)
return ret;
if (coder->compressed_size != 0)
return LZMA_DATA_ERROR;
coder->sequence = SEQ_CONTROL;
break;
}
case SEQ_COPY: {
dict_write(dict, in, in_pos, in_size, &coder->compressed_size);
if (coder->compressed_size != 0)
return LZMA_OK;
coder->sequence = SEQ_CONTROL;
break;
}
default:
assert(0);
return LZMA_PROG_ERROR;
}
return LZMA_OK;
}
static void
lzma2_decoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
assert(coder->lzma.end == NULL);
lzma_free(coder->lzma.coder, allocator);
lzma_free(coder, allocator);
return;
}
static lzma_ret
lzma2_decoder_init(lzma_lz_decoder *lz, lzma_allocator *allocator,
const void *opt, lzma_lz_options *lz_options)
{
if (lz->coder == NULL) {
lz->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (lz->coder == NULL)
return LZMA_MEM_ERROR;
lz->code = &lzma2_decode;
lz->end = &lzma2_decoder_end;
lz->coder->lzma = LZMA_LZ_DECODER_INIT;
}
const lzma_options_lzma *options = opt;
lz->coder->sequence = SEQ_CONTROL;
lz->coder->need_properties = true;
lz->coder->need_dictionary_reset = options->preset_dict == NULL
|| options->preset_dict_size == 0;
return lzma_lzma_decoder_create(&lz->coder->lzma,
allocator, options, lz_options);
}
extern lzma_ret
lzma_lzma2_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters)
{
assert(filters[1].init == NULL);
return lzma_lz_decoder_init(next, allocator, filters,
&lzma2_decoder_init);
}
extern uint64_t
lzma_lzma2_decoder_memusage(const void *options)
{
return sizeof(lzma_coder)
+ lzma_lzma_decoder_memusage_nocheck(options);
}
extern lzma_ret
lzma_lzma2_props_decode(void **options, lzma_allocator *allocator,
const uint8_t *props, size_t props_size)
{
if (props_size != 1)
return LZMA_OPTIONS_ERROR;
if (props[0] & 0xC0)
return LZMA_OPTIONS_ERROR;
if (props[0] > 40)
return LZMA_OPTIONS_ERROR;
lzma_options_lzma *opt = lzma_alloc(
sizeof(lzma_options_lzma), allocator);
if (opt == NULL)
return LZMA_MEM_ERROR;
if (props[0] == 40) {
opt->dict_size = UINT32_MAX;
} else {
opt->dict_size = 2 | (props[0] & 1);
opt->dict_size <<= props[0] / 2 + 11;
}
opt->preset_dict = NULL;
opt->preset_dict_size = 0;
*options = opt;
return LZMA_OK;
}
