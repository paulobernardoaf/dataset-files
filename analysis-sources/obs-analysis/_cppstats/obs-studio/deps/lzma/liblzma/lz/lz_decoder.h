#include "common.h"
typedef struct {
uint8_t *buf;
size_t pos;
size_t full;
size_t limit;
size_t size;
bool need_reset;
} lzma_dict;
typedef struct {
size_t dict_size;
const uint8_t *preset_dict;
size_t preset_dict_size;
} lzma_lz_options;
typedef struct {
lzma_coder *coder;
lzma_ret (*code)(lzma_coder *restrict coder,
lzma_dict *restrict dict, const uint8_t *restrict in,
size_t *restrict in_pos, size_t in_size);
void (*reset)(lzma_coder *coder, const void *options);
void (*set_uncompressed)(lzma_coder *coder,
lzma_vli uncompressed_size);
void (*end)(lzma_coder *coder, lzma_allocator *allocator);
} lzma_lz_decoder;
#define LZMA_LZ_DECODER_INIT (lzma_lz_decoder){ .coder = NULL, .code = NULL, .reset = NULL, .set_uncompressed = NULL, .end = NULL, }
extern lzma_ret lzma_lz_decoder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters,
lzma_ret (*lz_init)(lzma_lz_decoder *lz,
lzma_allocator *allocator, const void *options,
lzma_lz_options *lz_options));
extern uint64_t lzma_lz_decoder_memusage(size_t dictionary_size);
extern void lzma_lz_decoder_uncompressed(
lzma_coder *coder, lzma_vli uncompressed_size);
static inline uint8_t
dict_get(const lzma_dict *const dict, const uint32_t distance)
{
return dict->buf[dict->pos - distance - 1
+ (distance < dict->pos ? 0 : dict->size)];
}
static inline bool
dict_is_empty(const lzma_dict *const dict)
{
return dict->full == 0;
}
static inline bool
dict_is_distance_valid(const lzma_dict *const dict, const size_t distance)
{
return dict->full > distance;
}
static inline bool
dict_repeat(lzma_dict *dict, uint32_t distance, uint32_t *len)
{
const size_t dict_avail = dict->limit - dict->pos;
uint32_t left = my_min(dict_avail, *len);
*len -= left;
if (distance < left) {
do {
dict->buf[dict->pos] = dict_get(dict, distance);
++dict->pos;
} while (--left > 0);
} else if (distance < dict->pos) {
memcpy(dict->buf + dict->pos,
dict->buf + dict->pos - distance - 1,
left);
dict->pos += left;
} else {
assert(dict->full == dict->size);
const uint32_t copy_pos
= dict->pos - distance - 1 + dict->size;
uint32_t copy_size = dict->size - copy_pos;
if (copy_size < left) {
memmove(dict->buf + dict->pos, dict->buf + copy_pos,
copy_size);
dict->pos += copy_size;
copy_size = left - copy_size;
memcpy(dict->buf + dict->pos, dict->buf, copy_size);
dict->pos += copy_size;
} else {
memmove(dict->buf + dict->pos, dict->buf + copy_pos,
left);
dict->pos += left;
}
}
if (dict->full < dict->pos)
dict->full = dict->pos;
return unlikely(*len != 0);
}
static inline bool
dict_put(lzma_dict *dict, uint8_t byte)
{
if (unlikely(dict->pos == dict->limit))
return true;
dict->buf[dict->pos++] = byte;
if (dict->pos > dict->full)
dict->full = dict->pos;
return false;
}
static inline void
dict_write(lzma_dict *restrict dict, const uint8_t *restrict in,
size_t *restrict in_pos, size_t in_size,
size_t *restrict left)
{
if (in_size - *in_pos > *left)
in_size = *in_pos + *left;
*left -= lzma_bufcpy(in, in_pos, in_size,
dict->buf, &dict->pos, dict->limit);
if (dict->pos > dict->full)
dict->full = dict->pos;
return;
}
static inline void
dict_reset(lzma_dict *dict)
{
dict->need_reset = true;
return;
}
