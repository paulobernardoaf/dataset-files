#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "nvim/memory.h"
#include "nvim/vim.h"
#include "nvim/rbuffer.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "rbuffer.c.generated.h"
#endif
RBuffer *rbuffer_new(size_t capacity)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_RET
{
if (!capacity) {
capacity = 0x10000;
}
RBuffer *rv = xcalloc(1, sizeof(RBuffer) + capacity);
rv->full_cb = rv->nonfull_cb = NULL;
rv->data = NULL;
rv->size = 0;
rv->write_ptr = rv->read_ptr = rv->start_ptr;
rv->end_ptr = rv->start_ptr + capacity;
rv->temp = NULL;
return rv;
}
void rbuffer_free(RBuffer *buf)
{
xfree(buf->temp);
xfree(buf);
}
size_t rbuffer_size(RBuffer *buf) FUNC_ATTR_NONNULL_ALL
{
return buf->size;
}
size_t rbuffer_capacity(RBuffer *buf) FUNC_ATTR_NONNULL_ALL
{
return (size_t)(buf->end_ptr - buf->start_ptr);
}
size_t rbuffer_space(RBuffer *buf) FUNC_ATTR_NONNULL_ALL
{
return rbuffer_capacity(buf) - buf->size;
}
char *rbuffer_write_ptr(RBuffer *buf, size_t *write_count) FUNC_ATTR_NONNULL_ALL
{
if (buf->size == rbuffer_capacity(buf)) {
*write_count = 0;
return NULL;
}
if (buf->write_ptr >= buf->read_ptr) {
*write_count = (size_t)(buf->end_ptr - buf->write_ptr);
} else {
*write_count = (size_t)(buf->read_ptr - buf->write_ptr);
}
return buf->write_ptr;
}
void rbuffer_reset(RBuffer *buf) FUNC_ATTR_NONNULL_ALL
{
size_t temp_size;
if ((temp_size = rbuffer_size(buf))) {
if (buf->temp == NULL) {
buf->temp = xcalloc(1, rbuffer_capacity(buf));
}
rbuffer_read(buf, buf->temp, buf->size);
}
buf->read_ptr = buf->write_ptr = buf->start_ptr;
if (temp_size) {
rbuffer_write(buf, buf->temp, temp_size);
}
}
void rbuffer_produced(RBuffer *buf, size_t count) FUNC_ATTR_NONNULL_ALL
{
assert(count && count <= rbuffer_space(buf));
buf->write_ptr += count;
if (buf->write_ptr >= buf->end_ptr) {
buf->write_ptr -= rbuffer_capacity(buf);
}
buf->size += count;
if (buf->full_cb && !rbuffer_space(buf)) {
buf->full_cb(buf, buf->data);
}
}
char *rbuffer_read_ptr(RBuffer *buf, size_t *read_count) FUNC_ATTR_NONNULL_ALL
{
if (!buf->size) {
*read_count = 0;
return buf->read_ptr;
}
if (buf->read_ptr < buf->write_ptr) {
*read_count = (size_t)(buf->write_ptr - buf->read_ptr);
} else {
*read_count = (size_t)(buf->end_ptr - buf->read_ptr);
}
return buf->read_ptr;
}
void rbuffer_consumed(RBuffer *buf, size_t count)
FUNC_ATTR_NONNULL_ALL
{
assert(count && count <= buf->size);
buf->read_ptr += count;
if (buf->read_ptr >= buf->end_ptr) {
buf->read_ptr -= rbuffer_capacity(buf);
}
bool was_full = buf->size == rbuffer_capacity(buf);
buf->size -= count;
if (buf->nonfull_cb && was_full) {
buf->nonfull_cb(buf, buf->data);
}
}
size_t rbuffer_write(RBuffer *buf, const char *src, size_t src_size)
FUNC_ATTR_NONNULL_ALL
{
size_t size = src_size;
RBUFFER_UNTIL_FULL(buf, wptr, wcnt) {
size_t copy_count = MIN(src_size, wcnt);
memcpy(wptr, src, copy_count);
rbuffer_produced(buf, copy_count);
if (!(src_size -= copy_count)) {
return size;
}
src += copy_count;
}
return size - src_size;
}
size_t rbuffer_read(RBuffer *buf, char *dst, size_t dst_size)
FUNC_ATTR_NONNULL_ALL
{
size_t size = dst_size;
RBUFFER_UNTIL_EMPTY(buf, rptr, rcnt) {
size_t copy_count = MIN(dst_size, rcnt);
memcpy(dst, rptr, copy_count);
rbuffer_consumed(buf, copy_count);
if (!(dst_size -= copy_count)) {
return size;
}
dst += copy_count;
}
return size - dst_size;
}
char *rbuffer_get(RBuffer *buf, size_t index)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
assert(index < buf->size);
char *rptr = buf->read_ptr + index;
if (rptr >= buf->end_ptr) {
rptr -= rbuffer_capacity(buf);
}
return rptr;
}
int rbuffer_cmp(RBuffer *buf, const char *str, size_t count)
FUNC_ATTR_NONNULL_ALL
{
assert(count <= buf->size);
size_t rcnt;
(void)rbuffer_read_ptr(buf, &rcnt);
size_t n = MIN(count, rcnt);
int rv = memcmp(str, buf->read_ptr, n);
count -= n;
size_t remaining = buf->size - rcnt;
if (rv || !count || !remaining) {
return rv;
}
return memcmp(str + n, buf->start_ptr, count);
}
