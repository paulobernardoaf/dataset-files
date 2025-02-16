











#include "index.h"
#include "check.h"


struct lzma_coder_s {
enum {
SEQ_INDICATOR,
SEQ_COUNT,
SEQ_MEMUSAGE,
SEQ_UNPADDED,
SEQ_UNCOMPRESSED,
SEQ_PADDING_INIT,
SEQ_PADDING,
SEQ_CRC32,
} sequence;


uint64_t memlimit;


lzma_index *index;



lzma_index **index_ptr;


lzma_vli count;


lzma_vli unpadded_size;


lzma_vli uncompressed_size;


size_t pos;


uint32_t crc32;
};


static lzma_ret
index_decode(lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size,
uint8_t *restrict out lzma_attribute((__unused__)),
size_t *restrict out_pos lzma_attribute((__unused__)),
size_t out_size lzma_attribute((__unused__)),
lzma_action action lzma_attribute((__unused__)))
{

const size_t in_start = *in_pos;
lzma_ret ret = LZMA_OK;

while (*in_pos < in_size)
switch (coder->sequence) {
case SEQ_INDICATOR:









if (in[(*in_pos)++] != 0x00)
return LZMA_DATA_ERROR;

coder->sequence = SEQ_COUNT;
break;

case SEQ_COUNT:
ret = lzma_vli_decode(&coder->count, &coder->pos,
in, in_pos, in_size);
if (ret != LZMA_STREAM_END)
goto out;

coder->pos = 0;
coder->sequence = SEQ_MEMUSAGE;



case SEQ_MEMUSAGE:
if (lzma_index_memusage(1, coder->count) > coder->memlimit) {
ret = LZMA_MEMLIMIT_ERROR;
goto out;
}



lzma_index_prealloc(coder->index, coder->count);

ret = LZMA_OK;
coder->sequence = coder->count == 0
? SEQ_PADDING_INIT : SEQ_UNPADDED;
break;

case SEQ_UNPADDED:
case SEQ_UNCOMPRESSED: {
lzma_vli *size = coder->sequence == SEQ_UNPADDED
? &coder->unpadded_size
: &coder->uncompressed_size;

ret = lzma_vli_decode(size, &coder->pos,
in, in_pos, in_size);
if (ret != LZMA_STREAM_END)
goto out;

ret = LZMA_OK;
coder->pos = 0;

if (coder->sequence == SEQ_UNPADDED) {


if (coder->unpadded_size < UNPADDED_SIZE_MIN
|| coder->unpadded_size
> UNPADDED_SIZE_MAX)
return LZMA_DATA_ERROR;

coder->sequence = SEQ_UNCOMPRESSED;
} else {

return_if_error(lzma_index_append(
coder->index, allocator,
coder->unpadded_size,
coder->uncompressed_size));


coder->sequence = --coder->count == 0
? SEQ_PADDING_INIT
: SEQ_UNPADDED;
}

break;
}

case SEQ_PADDING_INIT:
coder->pos = lzma_index_padding_size(coder->index);
coder->sequence = SEQ_PADDING;



case SEQ_PADDING:
if (coder->pos > 0) {
--coder->pos;
if (in[(*in_pos)++] != 0x00)
return LZMA_DATA_ERROR;

break;
}


coder->crc32 = lzma_crc32(in + in_start,
*in_pos - in_start, coder->crc32);

coder->sequence = SEQ_CRC32;



case SEQ_CRC32:
do {
if (*in_pos == in_size)
return LZMA_OK;

if (((coder->crc32 >> (coder->pos * 8)) & 0xFF)
!= in[(*in_pos)++])
return LZMA_DATA_ERROR;

} while (++coder->pos < 4);



*coder->index_ptr = coder->index;


coder->index = NULL;

return LZMA_STREAM_END;

default:
assert(0);
return LZMA_PROG_ERROR;
}

out:

coder->crc32 = lzma_crc32(in + in_start,
*in_pos - in_start, coder->crc32);

return ret;
}


static void
index_decoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_index_end(coder->index, allocator);
lzma_free(coder, allocator);
return;
}


static lzma_ret
index_decoder_memconfig(lzma_coder *coder, uint64_t *memusage,
uint64_t *old_memlimit, uint64_t new_memlimit)
{
*memusage = lzma_index_memusage(1, coder->count);
*old_memlimit = coder->memlimit;

if (new_memlimit != 0) {
if (new_memlimit < *memusage)
return LZMA_MEMLIMIT_ERROR;

coder->memlimit = new_memlimit;
}

return LZMA_OK;
}


static lzma_ret
index_decoder_reset(lzma_coder *coder, lzma_allocator *allocator,
lzma_index **i, uint64_t memlimit)
{




coder->index_ptr = i;
*i = NULL;


coder->index = lzma_index_init(allocator);
if (coder->index == NULL)
return LZMA_MEM_ERROR;


coder->sequence = SEQ_INDICATOR;
coder->memlimit = memlimit;
coder->count = 0; 
coder->pos = 0;
coder->crc32 = 0;

return LZMA_OK;
}


static lzma_ret
index_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
lzma_index **i, uint64_t memlimit)
{
lzma_next_coder_init(&index_decoder_init, next, allocator);

if (i == NULL || memlimit == 0)
return LZMA_PROG_ERROR;

if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;

next->code = &index_decode;
next->end = &index_decoder_end;
next->memconfig = &index_decoder_memconfig;
next->coder->index = NULL;
} else {
lzma_index_end(next->coder->index, allocator);
}

return index_decoder_reset(next->coder, allocator, i, memlimit);
}


extern LZMA_API(lzma_ret)
lzma_index_decoder(lzma_stream *strm, lzma_index **i, uint64_t memlimit)
{
lzma_next_strm_init(index_decoder_init, strm, i, memlimit);

strm->internal->supported_actions[LZMA_RUN] = true;
strm->internal->supported_actions[LZMA_FINISH] = true;

return LZMA_OK;
}


extern LZMA_API(lzma_ret)
lzma_index_buffer_decode(
lzma_index **i, uint64_t *memlimit, lzma_allocator *allocator,
const uint8_t *in, size_t *in_pos, size_t in_size)
{

if (i == NULL || memlimit == NULL
|| in == NULL || in_pos == NULL || *in_pos > in_size)
return LZMA_PROG_ERROR;


lzma_coder coder;
return_if_error(index_decoder_reset(&coder, allocator, i, *memlimit));



const size_t in_start = *in_pos;


lzma_ret ret = index_decode(&coder, allocator, in, in_pos, in_size,
NULL, NULL, 0, LZMA_RUN);

if (ret == LZMA_STREAM_END) {
ret = LZMA_OK;
} else {


lzma_index_end(coder.index, allocator);
*in_pos = in_start;

if (ret == LZMA_OK) {



ret = LZMA_DATA_ERROR;

} else if (ret == LZMA_MEMLIMIT_ERROR) {


*memlimit = lzma_index_memusage(1, coder.count);
}
}

return ret;
}
