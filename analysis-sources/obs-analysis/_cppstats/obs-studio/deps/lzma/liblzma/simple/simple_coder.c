#include "simple_private.h"
static lzma_ret
copy_or_code(lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size, lzma_action action)
{
assert(!coder->end_was_reached);
if (coder->next.code == NULL) {
lzma_bufcpy(in, in_pos, in_size, out, out_pos, out_size);
if (coder->is_encoder && action == LZMA_FINISH
&& *in_pos == in_size)
coder->end_was_reached = true;
} else {
const lzma_ret ret = coder->next.code(
coder->next.coder, allocator,
in, in_pos, in_size,
out, out_pos, out_size, action);
if (ret == LZMA_STREAM_END) {
assert(!coder->is_encoder
|| action == LZMA_FINISH);
coder->end_was_reached = true;
} else if (ret != LZMA_OK) {
return ret;
}
}
return LZMA_OK;
}
static size_t
call_filter(lzma_coder *coder, uint8_t *buffer, size_t size)
{
const size_t filtered = coder->filter(coder->simple,
coder->now_pos, coder->is_encoder,
buffer, size);
coder->now_pos += filtered;
return filtered;
}
static lzma_ret
simple_code(lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size, lzma_action action)
{
if (action == LZMA_SYNC_FLUSH)
return LZMA_OPTIONS_ERROR;
if (coder->pos < coder->filtered) {
lzma_bufcpy(coder->buffer, &coder->pos, coder->filtered,
out, out_pos, out_size);
if (coder->pos < coder->filtered)
return LZMA_OK;
if (coder->end_was_reached) {
assert(coder->filtered == coder->size);
return LZMA_STREAM_END;
}
}
coder->filtered = 0;
assert(!coder->end_was_reached);
const size_t out_avail = out_size - *out_pos;
const size_t buf_avail = coder->size - coder->pos;
if (out_avail > buf_avail || buf_avail == 0) {
const size_t out_start = *out_pos;
memcpy(out + *out_pos, coder->buffer + coder->pos, buf_avail);
*out_pos += buf_avail;
{
const lzma_ret ret = copy_or_code(coder, allocator,
in, in_pos, in_size,
out, out_pos, out_size, action);
assert(ret != LZMA_STREAM_END);
if (ret != LZMA_OK)
return ret;
}
const size_t size = *out_pos - out_start;
const size_t filtered = call_filter(
coder, out + out_start, size);
const size_t unfiltered = size - filtered;
assert(unfiltered <= coder->allocated / 2);
coder->pos = 0;
coder->size = unfiltered;
if (coder->end_was_reached) {
coder->size = 0;
} else if (unfiltered > 0) {
*out_pos -= unfiltered;
memcpy(coder->buffer, out + *out_pos, unfiltered);
}
} else if (coder->pos > 0) {
memmove(coder->buffer, coder->buffer + coder->pos, buf_avail);
coder->size -= coder->pos;
coder->pos = 0;
}
assert(coder->pos == 0);
if (coder->size > 0) {
{
const lzma_ret ret = copy_or_code(coder, allocator,
in, in_pos, in_size,
coder->buffer, &coder->size,
coder->allocated, action);
assert(ret != LZMA_STREAM_END);
if (ret != LZMA_OK)
return ret;
}
coder->filtered = call_filter(
coder, coder->buffer, coder->size);
if (coder->end_was_reached)
coder->filtered = coder->size;
lzma_bufcpy(coder->buffer, &coder->pos, coder->filtered,
out, out_pos, out_size);
}
if (coder->end_was_reached && coder->pos == coder->size)
return LZMA_STREAM_END;
return LZMA_OK;
}
static void
simple_coder_end(lzma_coder *coder, lzma_allocator *allocator)
{
lzma_next_end(&coder->next, allocator);
lzma_free(coder->simple, allocator);
lzma_free(coder, allocator);
return;
}
static lzma_ret
simple_coder_update(lzma_coder *coder, lzma_allocator *allocator,
const lzma_filter *filters_null lzma_attribute((__unused__)),
const lzma_filter *reversed_filters)
{
return lzma_next_filter_update(
&coder->next, allocator, reversed_filters + 1);
}
extern lzma_ret
lzma_simple_coder_init(lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters,
size_t (*filter)(lzma_simple *simple, uint32_t now_pos,
bool is_encoder, uint8_t *buffer, size_t size),
size_t simple_size, size_t unfiltered_max,
uint32_t alignment, bool is_encoder)
{
if (next->coder == NULL) {
next->coder = lzma_alloc(sizeof(lzma_coder)
+ 2 * unfiltered_max, allocator);
if (next->coder == NULL)
return LZMA_MEM_ERROR;
next->code = &simple_code;
next->end = &simple_coder_end;
next->update = &simple_coder_update;
next->coder->next = LZMA_NEXT_CODER_INIT;
next->coder->filter = filter;
next->coder->allocated = 2 * unfiltered_max;
if (simple_size > 0) {
next->coder->simple = lzma_alloc(
simple_size, allocator);
if (next->coder->simple == NULL)
return LZMA_MEM_ERROR;
} else {
next->coder->simple = NULL;
}
}
if (filters[0].options != NULL) {
const lzma_options_bcj *simple = filters[0].options;
next->coder->now_pos = simple->start_offset;
if (next->coder->now_pos & (alignment - 1))
return LZMA_OPTIONS_ERROR;
} else {
next->coder->now_pos = 0;
}
next->coder->is_encoder = is_encoder;
next->coder->end_was_reached = false;
next->coder->pos = 0;
next->coder->filtered = 0;
next->coder->size = 0;
return lzma_next_filter_init(
&next->coder->next, allocator, filters + 1);
}
