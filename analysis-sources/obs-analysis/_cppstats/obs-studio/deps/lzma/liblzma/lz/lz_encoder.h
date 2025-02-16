#include "common.h"
typedef struct {
uint32_t len;
uint32_t dist;
} lzma_match;
typedef struct lzma_mf_s lzma_mf;
struct lzma_mf_s {
uint8_t *buffer;
uint32_t size;
uint32_t keep_size_before;
uint32_t keep_size_after;
uint32_t offset;
uint32_t read_pos;
uint32_t read_ahead;
uint32_t read_limit;
uint32_t write_pos;
uint32_t pending;
uint32_t (*find)(lzma_mf *mf, lzma_match *matches);
void (*skip)(lzma_mf *mf, uint32_t num);
uint32_t *hash;
uint32_t *son;
uint32_t cyclic_pos;
uint32_t cyclic_size; 
uint32_t hash_mask;
uint32_t depth;
uint32_t nice_len;
uint32_t match_len_max;
lzma_action action;
uint32_t hash_size_sum;
uint32_t sons_count;
};
typedef struct {
size_t before_size;
size_t dict_size;
size_t after_size;
size_t match_len_max;
size_t nice_len;
lzma_match_finder match_finder;
uint32_t depth;
const uint8_t *preset_dict;
uint32_t preset_dict_size;
} lzma_lz_options;
typedef struct {
lzma_coder *coder;
lzma_ret (*code)(lzma_coder *restrict coder,
lzma_mf *restrict mf, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size);
void (*end)(lzma_coder *coder, lzma_allocator *allocator);
lzma_ret (*options_update)(lzma_coder *coder,
const lzma_filter *filter);
} lzma_lz_encoder;
static inline const uint8_t *
mf_ptr(const lzma_mf *mf)
{
return mf->buffer + mf->read_pos;
}
static inline uint32_t
mf_avail(const lzma_mf *mf)
{
return mf->write_pos - mf->read_pos;
}
static inline uint32_t
mf_unencoded(const lzma_mf *mf)
{
return mf->write_pos - mf->read_pos + mf->read_ahead;
}
static inline uint32_t
mf_position(const lzma_mf *mf)
{
return mf->read_pos - mf->read_ahead;
}
#define mf_find lzma_mf_find
static inline void
mf_skip(lzma_mf *mf, uint32_t amount)
{
if (amount != 0) {
mf->skip(mf, amount);
mf->read_ahead += amount;
}
}
static inline void
mf_read(lzma_mf *mf, uint8_t *out, size_t *out_pos, size_t out_size,
size_t *left)
{
const size_t out_avail = out_size - *out_pos;
const size_t copy_size = my_min(out_avail, *left);
assert(mf->read_ahead == 0);
assert(mf->read_pos >= *left);
memcpy(out + *out_pos, mf->buffer + mf->read_pos - *left,
copy_size);
*out_pos += copy_size;
*left -= copy_size;
return;
}
extern lzma_ret lzma_lz_encoder_init(
lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters,
lzma_ret (*lz_init)(lzma_lz_encoder *lz,
lzma_allocator *allocator, const void *options,
lzma_lz_options *lz_options));
extern uint64_t lzma_lz_encoder_memusage(const lzma_lz_options *lz_options);
extern uint32_t lzma_mf_find(
lzma_mf *mf, uint32_t *count, lzma_match *matches);
extern uint32_t lzma_mf_hc3_find(lzma_mf *dict, lzma_match *matches);
extern void lzma_mf_hc3_skip(lzma_mf *dict, uint32_t amount);
extern uint32_t lzma_mf_hc4_find(lzma_mf *dict, lzma_match *matches);
extern void lzma_mf_hc4_skip(lzma_mf *dict, uint32_t amount);
extern uint32_t lzma_mf_bt2_find(lzma_mf *dict, lzma_match *matches);
extern void lzma_mf_bt2_skip(lzma_mf *dict, uint32_t amount);
extern uint32_t lzma_mf_bt3_find(lzma_mf *dict, lzma_match *matches);
extern void lzma_mf_bt3_skip(lzma_mf *dict, uint32_t amount);
extern uint32_t lzma_mf_bt4_find(lzma_mf *dict, lzma_match *matches);
extern void lzma_mf_bt4_skip(lzma_mf *dict, uint32_t amount);
