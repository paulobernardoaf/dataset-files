#include "lz_encoder.h"
#include "lz_encoder_hash.h"
#if defined(WORDS_BIGENDIAN) && !defined(HAVE_SMALL)
#include "lz_encoder_hash_table.h"
#endif
struct lzma_coder_s {
lzma_lz_encoder lz;
lzma_mf mf;
lzma_next_coder next;
};
static void
move_window(lzma_mf *mf)
{
assert(mf->read_pos > mf->keep_size_before);
const uint32_t move_offset
= (mf->read_pos - mf->keep_size_before) & ~UINT32_C(15);
assert(mf->write_pos > move_offset);
const size_t move_size = mf->write_pos - move_offset;
assert(move_offset + move_size <= mf->size);
memmove(mf->buffer, mf->buffer + move_offset, move_size);
mf->offset += move_offset;
mf->read_pos -= move_offset;
mf->read_limit -= move_offset;
mf->write_pos -= move_offset;
return;
}
static lzma_ret
fill_window(lzma_coder *coder, lzma_allocator *allocator, const uint8_t *in,
size_t *in_pos, size_t in_size, lzma_action action)
{
assert(coder->mf.read_pos <= coder->mf.write_pos);
if (coder->mf.read_pos >= coder->mf.size - coder->mf.keep_size_after)
move_window(&coder->mf);
size_t write_pos = coder->mf.write_pos;
lzma_ret ret;
if (coder->next.code == NULL) {
lzma_bufcpy(in, in_pos, in_size, coder->mf.buffer,
&write_pos, coder->mf.size);
ret = action != LZMA_RUN && *in_pos == in_size
? LZMA_STREAM_END : LZMA_OK;
} else {
ret = coder->next.code(coder->next.coder, allocator,
in, in_pos, in_size,
coder->mf.buffer, &write_pos,
coder->mf.size, action);
}
coder->mf.write_pos = write_pos;
if (ret == LZMA_STREAM_END) {
assert(*in_pos == in_size);
ret = LZMA_OK;
coder->mf.action = action;
coder->mf.read_limit = coder->mf.write_pos;
} else if (coder->mf.write_pos > coder->mf.keep_size_after) {
coder->mf.read_limit = coder->mf.write_pos
- coder->mf.keep_size_after;
}
if (coder->mf.pending > 0
&& coder->mf.read_pos < coder->mf.read_limit) {
const size_t pending = coder->mf.pending;
coder->mf.pending = 0;
assert(coder->mf.read_pos >= pending);
coder->mf.read_pos -= pending;
coder->mf.skip(&coder->mf, pending);
}
return ret;
}
static lzma_ret
lz_encode(lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size,
uint8_t *restrict out, size_t *restrict out_pos,
size_t out_size, lzma_action action)
{
while (*out_pos < out_size
&& (*in_pos < in_size || action != LZMA_RUN)) {
if (coder->mf.action == LZMA_RUN && coder->mf.read_pos
>= coder->mf.read_limit)
return_if_error(fill_window(coder, allocator,
in, in_pos, in_size, action));
const lzma_ret ret = coder->lz.code(coder->lz.coder,
&coder->mf, out, out_pos, out_size);
if (ret != LZMA_OK) {
coder->mf.action = LZMA_RUN;
return ret;
}
}
return LZMA_OK;
}
static bool
lz_encoder_prepare(lzma_mf *mf, lzma_allocator *allocator,
const lzma_lz_options *lz_options)
{
if (lz_options->dict_size < LZMA_DICT_SIZE_MIN
|| lz_options->dict_size
> (UINT32_C(1) << 30) + (UINT32_C(1) << 29)
|| lz_options->nice_len > lz_options->match_len_max)
return true;
mf->keep_size_before = lz_options->before_size + lz_options->dict_size;
mf->keep_size_after = lz_options->after_size
+ lz_options->match_len_max;
uint32_t reserve = lz_options->dict_size / 2;
if (reserve > (UINT32_C(1) << 30))
reserve /= 2;
reserve += (lz_options->before_size + lz_options->match_len_max
+ lz_options->after_size) / 2 + (UINT32_C(1) << 19);
const uint32_t old_size = mf->size;
mf->size = mf->keep_size_before + reserve + mf->keep_size_after;
if (mf->buffer != NULL && old_size != mf->size) {
lzma_free(mf->buffer, allocator);
mf->buffer = NULL;
}
mf->match_len_max = lz_options->match_len_max;
mf->nice_len = lz_options->nice_len;
mf->cyclic_size = lz_options->dict_size + 1;
switch (lz_options->match_finder) {
#if defined(HAVE_MF_HC3)
case LZMA_MF_HC3:
mf->find = &lzma_mf_hc3_find;
mf->skip = &lzma_mf_hc3_skip;
break;
#endif
#if defined(HAVE_MF_HC4)
case LZMA_MF_HC4:
mf->find = &lzma_mf_hc4_find;
mf->skip = &lzma_mf_hc4_skip;
break;
#endif
#if defined(HAVE_MF_BT2)
case LZMA_MF_BT2:
mf->find = &lzma_mf_bt2_find;
mf->skip = &lzma_mf_bt2_skip;
break;
#endif
#if defined(HAVE_MF_BT3)
case LZMA_MF_BT3:
mf->find = &lzma_mf_bt3_find;
mf->skip = &lzma_mf_bt3_skip;
break;
#endif
#if defined(HAVE_MF_BT4)
case LZMA_MF_BT4:
mf->find = &lzma_mf_bt4_find;
mf->skip = &lzma_mf_bt4_skip;
break;
#endif
default:
return true;
}
const uint32_t hash_bytes = lz_options->match_finder & 0x0F;
if (hash_bytes > mf->nice_len)
return true;
const bool is_bt = (lz_options->match_finder & 0x10) != 0;
uint32_t hs;
if (hash_bytes == 2) {
hs = 0xFFFF;
} else {
hs = lz_options->dict_size - 1;
hs |= hs >> 1;
hs |= hs >> 2;
hs |= hs >> 4;
hs |= hs >> 8;
hs >>= 1;
hs |= 0xFFFF;
if (hs > (UINT32_C(1) << 24)) {
if (hash_bytes == 3)
hs = (UINT32_C(1) << 24) - 1;
else
hs >>= 1;
}
}
mf->hash_mask = hs;
++hs;
if (hash_bytes > 2)
hs += HASH_2_SIZE;
if (hash_bytes > 3)
hs += HASH_3_SIZE;
assert(hs < UINT32_MAX / 5);
const uint32_t old_count = mf->hash_size_sum + mf->sons_count;
mf->hash_size_sum = hs;
mf->sons_count = mf->cyclic_size;
if (is_bt)
mf->sons_count *= 2;
const uint32_t new_count = mf->hash_size_sum + mf->sons_count;
if (old_count != new_count) {
lzma_free(mf->hash, allocator);
mf->hash = NULL;
}
mf->depth = lz_options->depth;
if (mf->depth == 0) {
if (is_bt)
mf->depth = 16 + mf->nice_len / 2;
else
mf->depth = 4 + mf->nice_len / 4;
}
return false;
}
static bool
lz_encoder_init(lzma_mf *mf, lzma_allocator *allocator,
const lzma_lz_options *lz_options)
{
if (mf->buffer == NULL) {
mf->buffer = lzma_alloc(mf->size, allocator);
if (mf->buffer == NULL)
return true;
}
mf->offset = mf->cyclic_size;
mf->read_pos = 0;
mf->read_ahead = 0;
mf->read_limit = 0;
mf->write_pos = 0;
mf->pending = 0;
const size_t alloc_count = mf->hash_size_sum + mf->sons_count;
#if UINT32_MAX >= SIZE_MAX / 4
if (alloc_count > SIZE_MAX / sizeof(uint32_t))
return true;
#endif
if (mf->hash == NULL) {
mf->hash = lzma_alloc(alloc_count * sizeof(uint32_t),
allocator);
if (mf->hash == NULL)
return true;
}
mf->son = mf->hash + mf->hash_size_sum;
mf->cyclic_pos = 0;
memzero(mf->hash, (size_t)(mf->hash_size_sum) * sizeof(uint32_t));
if (lz_options->preset_dict != NULL
&& lz_options->preset_dict_size > 0) {
mf->write_pos = my_min(lz_options->preset_dict_size, mf->size);
memcpy(mf->buffer, lz_options->preset_dict
+ lz_options->preset_dict_size - mf->write_pos,
mf->write_pos);
mf->action = LZMA_SYNC_FLUSH;
mf->skip(mf, mf->write_pos);
}
mf->action = LZMA_RUN;
return false;
}
extern uint64_t
lzma_lz_encoder_memusage(const lzma_lz_options *lz_options)
{
lzma_mf mf = {
.buffer = NULL,
.hash = NULL,
.hash_size_sum = 0,
.sons_count = 0,
};
if (lz_encoder_prepare(&mf, NULL, lz_options))
return UINT64_MAX;
return (uint64_t)(mf.hash_size_sum + mf.sons_count)
* sizeof(uint32_t)
+ (uint64_t)(mf.size) + sizeof(lzma_coder);
}
static void
lz_encoder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_next_end(&coder->next, allocator);
lzma_free(coder->mf.hash, allocator);
lzma_free(coder->mf.buffer, allocator);
if (coder->lz.end != NULL)
coder->lz.end(coder->lz.coder, allocator);
else
lzma_free(coder->lz.coder, allocator);
lzma_free(coder, allocator);
return;
}
static lzma_ret
lz_encoder_update(lzma_coder *coder, lzma_allocator *allocator,
const lzma_filter *filters_null lzma_attribute((__unused__)),
const lzma_filter *reversed_filters)
{
if (coder->lz.options_update == NULL)
return LZMA_PROG_ERROR;
return_if_error(coder->lz.options_update(
coder->lz.coder, reversed_filters));
return lzma_next_filter_update(
&coder->next, allocator, reversed_filters + 1);
}
extern lzma_ret
lzma_lz_encoder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters,
lzma_ret (*lz_init)(lzma_lz_encoder *lz,
lzma_allocator *allocator, const void *options,
lzma_lz_options *lz_options))
{
#if defined(HAVE_SMALL)
lzma_crc32_init();
#endif
if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder), allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;
next->code = &lz_encode;
next->end = &lz_encoder_end;
next->update = &lz_encoder_update;
next->coder->lz.coder = NULL;
next->coder->lz.code = NULL;
next->coder->lz.end = NULL;
next->coder->mf.buffer = NULL;
next->coder->mf.hash = NULL;
next->coder->mf.hash_size_sum = 0;
next->coder->mf.sons_count = 0;
next->coder->next = LZMA_NEXT_CODER_INIT;
}
lzma_lz_options lz_options;
return_if_error(lz_init(&next->coder->lz, allocator,
filters[0].options, &lz_options));
if (lz_encoder_prepare(&next->coder->mf, allocator, &lz_options))
return LZMA_OPTIONS_ERROR;
if (lz_encoder_init(&next->coder->mf, allocator, &lz_options))
return LZMA_MEM_ERROR;
return lzma_next_filter_init(&next->coder->next, allocator,
filters + 1);
}
extern LZMA_API(lzma_bool)
lzma_mf_is_supported(lzma_match_finder mf)
{
bool ret = false;
#if defined(HAVE_MF_HC3)
if (mf == LZMA_MF_HC3)
ret = true;
#endif
#if defined(HAVE_MF_HC4)
if (mf == LZMA_MF_HC4)
ret = true;
#endif
#if defined(HAVE_MF_BT2)
if (mf == LZMA_MF_BT2)
ret = true;
#endif
#if defined(HAVE_MF_BT3)
if (mf == LZMA_MF_BT3)
ret = true;
#endif
#if defined(HAVE_MF_BT4)
if (mf == LZMA_MF_BT4)
ret = true;
#endif
return ret;
}
