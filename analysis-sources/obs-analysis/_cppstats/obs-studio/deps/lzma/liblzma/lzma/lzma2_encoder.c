#include "lz_encoder.h"
#include "lzma_encoder.h"
#include "fastpos.h"
#include "lzma2_encoder.h"
struct lzma_coder_s {
enum {
SEQ_INIT,
SEQ_LZMA_ENCODE,
SEQ_LZMA_COPY,
SEQ_UNCOMPRESSED_HEADER,
SEQ_UNCOMPRESSED_COPY,
} sequence;
lzma_coder *lzma;
lzma_options_lzma opt_cur;
bool need_properties;
bool need_state_reset;
bool need_dictionary_reset;
size_t uncompressed_size;
size_t compressed_size;
size_t buf_pos;
uint8_t buf[LZMA2_HEADER_MAX + LZMA2_CHUNK_MAX];
};
static void
lzma2_header_lzma(lzma_coder *coder)
{
assert(coder->uncompressed_size > 0);
assert(coder->uncompressed_size <= LZMA2_UNCOMPRESSED_MAX);
assert(coder->compressed_size > 0);
assert(coder->compressed_size <= LZMA2_CHUNK_MAX);
size_t pos;
if (coder->need_properties) {
pos = 0;
if (coder->need_dictionary_reset)
coder->buf[pos] = 0x80 + (3 << 5);
else
coder->buf[pos] = 0x80 + (2 << 5);
} else {
pos = 1;
if (coder->need_state_reset)
coder->buf[pos] = 0x80 + (1 << 5);
else
coder->buf[pos] = 0x80;
}
coder->buf_pos = pos;
size_t size = coder->uncompressed_size - 1;
coder->buf[pos++] += size >> 16;
coder->buf[pos++] = (size >> 8) & 0xFF;
coder->buf[pos++] = size & 0xFF;
size = coder->compressed_size - 1;
coder->buf[pos++] = size >> 8;
coder->buf[pos++] = size & 0xFF;
if (coder->need_properties)
lzma_lzma_lclppb_encode(&coder->opt_cur, coder->buf + pos);
coder->need_properties = false;
coder->need_state_reset = false;
coder->need_dictionary_reset = false;
coder->compressed_size += LZMA2_HEADER_MAX;
return;
}
static void
lzma2_header_uncompressed(lzma_coder *coder)
{
assert(coder->uncompressed_size > 0);
assert(coder->uncompressed_size <= LZMA2_CHUNK_MAX);
if (coder->need_dictionary_reset)
coder->buf[0] = 1;
else
coder->buf[0] = 2;
coder->need_dictionary_reset = false;
coder->buf[1] = (coder->uncompressed_size - 1) >> 8;
coder->buf[2] = (coder->uncompressed_size - 1) & 0xFF;
coder->buf_pos = 0;
return;
}
static lzma_ret
lzma2_encode(lzma_coder *restrict coder, lzma_mf *restrict mf,
uint8_t *restrict out, size_t *restrict out_pos,
size_t out_size)
{
while (*out_pos < out_size)
switch (coder->sequence) {
case SEQ_INIT:
if (mf_unencoded(mf) == 0) {
if (mf->action == LZMA_FINISH)
out[(*out_pos)++] = 0;
return mf->action == LZMA_RUN
? LZMA_OK : LZMA_STREAM_END;
}
if (coder->need_state_reset)
return_if_error(lzma_lzma_encoder_reset(
coder->lzma, &coder->opt_cur));
coder->uncompressed_size = 0;
coder->compressed_size = 0;
coder->sequence = SEQ_LZMA_ENCODE;
case SEQ_LZMA_ENCODE: {
const uint32_t left = LZMA2_UNCOMPRESSED_MAX
- coder->uncompressed_size;
uint32_t limit;
if (left < mf->match_len_max) {
limit = 0;
} else {
limit = mf->read_pos - mf->read_ahead
+ left - mf->match_len_max;
}
const uint32_t read_start = mf->read_pos - mf->read_ahead;
const lzma_ret ret = lzma_lzma_encode(coder->lzma, mf,
coder->buf + LZMA2_HEADER_MAX,
&coder->compressed_size,
LZMA2_CHUNK_MAX, limit);
coder->uncompressed_size += mf->read_pos - mf->read_ahead
- read_start;
assert(coder->compressed_size <= LZMA2_CHUNK_MAX);
assert(coder->uncompressed_size <= LZMA2_UNCOMPRESSED_MAX);
if (ret != LZMA_STREAM_END)
return LZMA_OK;
if (coder->compressed_size >= coder->uncompressed_size) {
coder->uncompressed_size += mf->read_ahead;
assert(coder->uncompressed_size
<= LZMA2_UNCOMPRESSED_MAX);
mf->read_ahead = 0;
lzma2_header_uncompressed(coder);
coder->need_state_reset = true;
coder->sequence = SEQ_UNCOMPRESSED_HEADER;
break;
}
lzma2_header_lzma(coder);
coder->sequence = SEQ_LZMA_COPY;
}
case SEQ_LZMA_COPY:
lzma_bufcpy(coder->buf, &coder->buf_pos,
coder->compressed_size,
out, out_pos, out_size);
if (coder->buf_pos != coder->compressed_size)
return LZMA_OK;
coder->sequence = SEQ_INIT;
break;
case SEQ_UNCOMPRESSED_HEADER:
lzma_bufcpy(coder->buf, &coder->buf_pos,
LZMA2_HEADER_UNCOMPRESSED,
out, out_pos, out_size);
if (coder->buf_pos != LZMA2_HEADER_UNCOMPRESSED)
return LZMA_OK;
coder->sequence = SEQ_UNCOMPRESSED_COPY;
case SEQ_UNCOMPRESSED_COPY:
mf_read(mf, out, out_pos, out_size, &coder->uncompressed_size);
if (coder->uncompressed_size != 0)
return LZMA_OK;
coder->sequence = SEQ_INIT;
break;
}
return LZMA_OK;
}
static void
lzma2_encoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_free(coder->lzma, allocator);
lzma_free(coder, allocator);
return;
}
static lzma_ret
lzma2_encoder_options_update(lzma_coder *coder, const lzma_filter *filter)
{
if (filter->options == NULL || coder->sequence != SEQ_INIT)
return LZMA_PROG_ERROR;
const lzma_options_lzma *opt = filter->options;
if (coder->opt_cur.lc != opt->lc || coder->opt_cur.lp != opt->lp
|| coder->opt_cur.pb != opt->pb) {
if (opt->lc > LZMA_LCLP_MAX || opt->lp > LZMA_LCLP_MAX
|| opt->lc + opt->lp > LZMA_LCLP_MAX
|| opt->pb > LZMA_PB_MAX)
return LZMA_OPTIONS_ERROR;
coder->opt_cur.lc = opt->lc;
coder->opt_cur.lp = opt->lp;
coder->opt_cur.pb = opt->pb;
coder->need_properties = true;
coder->need_state_reset = true;
}
return LZMA_OK;
}
static lzma_ret
lzma2_encoder_init(lzma_lz_encoder *lz, lzma_allocator *allocator,
const void *options, lzma_lz_options *lz_options)
{
if (options == NULL)
return LZMA_PROG_ERROR;
if (lz->coder == NULL) {
lz->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (lz->coder == NULL)
return LZMA_MEM_ERROR;
lz->code = &lzma2_encode;
lz->end = &lzma2_encoder_end;
lz->options_update = &lzma2_encoder_options_update;
lz->coder->lzma = NULL;
}
lz->coder->opt_cur = *(const lzma_options_lzma *)(options);
lz->coder->sequence = SEQ_INIT;
lz->coder->need_properties = true;
lz->coder->need_state_reset = false;
lz->coder->need_dictionary_reset
= lz->coder->opt_cur.preset_dict == NULL
|| lz->coder->opt_cur.preset_dict_size == 0;
return_if_error(lzma_lzma_encoder_create(&lz->coder->lzma, allocator,
&lz->coder->opt_cur, lz_options));
if (lz_options->before_size + lz_options->dict_size < LZMA2_CHUNK_MAX)
lz_options->before_size
= LZMA2_CHUNK_MAX - lz_options->dict_size;
return LZMA_OK;
}
extern lzma_ret
lzma_lzma2_encoder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters)
{
return lzma_lz_encoder_init(
next, allocator, filters, &lzma2_encoder_init);
}
extern uint64_t
lzma_lzma2_encoder_memusage(const void *options)
{
const uint64_t lzma_mem = lzma_lzma_encoder_memusage(options);
if (lzma_mem == UINT64_MAX)
return UINT64_MAX;
return sizeof(lzma_coder) + lzma_mem;
}
extern lzma_ret
lzma_lzma2_props_encode(const void *options, uint8_t *out)
{
const lzma_options_lzma *const opt = options;
uint32_t d = my_max(opt->dict_size, LZMA_DICT_SIZE_MIN);
--d;
d |= d >> 2;
d |= d >> 3;
d |= d >> 4;
d |= d >> 8;
d |= d >> 16;
if (d == UINT32_MAX)
out[0] = 40;
else
out[0] = get_pos_slot(d + 1) - 24;
return LZMA_OK;
}
