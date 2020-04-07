


















#include "lz_decoder.h"


struct lzma_coder_s {

lzma_dict dict;


lzma_lz_decoder lz;




lzma_next_coder next;


bool next_finished;



bool this_finished;





struct {
size_t pos;
size_t size;
uint8_t buffer[LZMA_BUFFER_SIZE];
} temp;
};


static void
lz_decoder_reset(lzma_coder *coder)
{
coder->dict.pos = 0;
coder->dict.full = 0;
coder->dict.buf[coder->dict.size - 1] = '\0';
coder->dict.need_reset = false;
return;
}


static lzma_ret
decode_buffer(lzma_coder *coder,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size)
{
while (true) {

if (coder->dict.pos == coder->dict.size)
coder->dict.pos = 0;



const size_t dict_start = coder->dict.pos;





coder->dict.limit = coder->dict.pos
+ my_min(out_size - *out_pos,
coder->dict.size - coder->dict.pos);


const lzma_ret ret = coder->lz.code(
coder->lz.coder, &coder->dict,
in, in_pos, in_size);



const size_t copy_size = coder->dict.pos - dict_start;
assert(copy_size <= out_size - *out_pos);
memcpy(out + *out_pos, coder->dict.buf + dict_start,
copy_size);
*out_pos += copy_size;


if (coder->dict.need_reset) {
lz_decoder_reset(coder);



if (ret != LZMA_OK || *out_pos == out_size)
return ret;
} else {









if (ret != LZMA_OK || *out_pos == out_size
|| coder->dict.pos < coder->dict.size)
return ret;
}
}
}


static lzma_ret
lz_decode(lzma_coder *coder,
lzma_allocator *allocator lzma_attribute((__unused__)),
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size,
lzma_action action)
{
if (coder->next.code == NULL)
return decode_buffer(coder, in, in_pos, in_size,
out, out_pos, out_size);



while (*out_pos < out_size) {

if (!coder->next_finished
&& coder->temp.pos == coder->temp.size) {
coder->temp.pos = 0;
coder->temp.size = 0;

const lzma_ret ret = coder->next.code(
coder->next.coder,
allocator, in, in_pos, in_size,
coder->temp.buffer, &coder->temp.size,
LZMA_BUFFER_SIZE, action);

if (ret == LZMA_STREAM_END)
coder->next_finished = true;
else if (ret != LZMA_OK || coder->temp.size == 0)
return ret;
}

if (coder->this_finished) {
if (coder->temp.size != 0)
return LZMA_DATA_ERROR;

if (coder->next_finished)
return LZMA_STREAM_END;

return LZMA_OK;
}

const lzma_ret ret = decode_buffer(coder, coder->temp.buffer,
&coder->temp.pos, coder->temp.size,
out, out_pos, out_size);

if (ret == LZMA_STREAM_END)
coder->this_finished = true;
else if (ret != LZMA_OK)
return ret;
else if (coder->next_finished && *out_pos < out_size)
return LZMA_DATA_ERROR;
}

return LZMA_OK;
}


static void
lz_decoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_next_end(&coder->next, allocator);
lzma_free(coder->dict.buf, allocator);

if (coder->lz.end != NULL)
coder->lz.end(coder->lz.coder, allocator);
else
lzma_free(coder->lz.coder, allocator);

lzma_free(coder, allocator);
return;
}


extern lzma_ret
lzma_lz_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters,
lzma_ret (*lz_init)(lzma_lz_decoder *lz,
lzma_allocator *allocator, const void *options,
lzma_lz_options *lz_options))
{

if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;

next->code = &lz_decode;
next->end = &lz_decoder_end;

next->coder->dict.buf = NULL;
next->coder->dict.size = 0;
next->coder->lz = LZMA_LZ_DECODER_INIT;
next->coder->next = LZMA_NEXT_CODER_INIT;
}



lzma_lz_options lz_options;
return_if_error(lz_init(&next->coder->lz, allocator,
filters[0].options, &lz_options));






if (lz_options.dict_size < 4096)
lz_options.dict_size = 4096;








if (lz_options.dict_size > SIZE_MAX - 15)
return LZMA_MEM_ERROR;

lz_options.dict_size = (lz_options.dict_size + 15) & ~((size_t)(15));


if (next->coder->dict.size != lz_options.dict_size) {
lzma_free(next->coder->dict.buf, allocator);
next->coder->dict.buf
= lzma_alloc(lz_options.dict_size, allocator);
if (next->coder->dict.buf == NULL)
return LZMA_MEM_ERROR;

next->coder->dict.size = lz_options.dict_size;
}

lz_decoder_reset(next->coder);


if (lz_options.preset_dict != NULL
&& lz_options.preset_dict_size > 0) {


const size_t copy_size = my_min(lz_options.preset_dict_size,
lz_options.dict_size);
const size_t offset = lz_options.preset_dict_size - copy_size;
memcpy(next->coder->dict.buf, lz_options.preset_dict + offset,
copy_size);
next->coder->dict.pos = copy_size;
next->coder->dict.full = copy_size;
}


next->coder->next_finished = false;
next->coder->this_finished = false;
next->coder->temp.pos = 0;
next->coder->temp.size = 0;


return lzma_next_filter_init(&next->coder->next, allocator,
filters + 1);
}


extern uint64_t
lzma_lz_decoder_memusage(size_t dictionary_size)
{
return sizeof(lzma_coder) + (uint64_t)(dictionary_size);
}


extern void
lzma_lz_decoder_uncompressed(lzma_coder *coder, lzma_vli uncompressed_size)
{
coder->lz.set_uncompressed(coder->lz.coder, uncompressed_size);
}
