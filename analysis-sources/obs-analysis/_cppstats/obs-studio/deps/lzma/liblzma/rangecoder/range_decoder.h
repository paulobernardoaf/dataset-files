#include "range_common.h"
typedef struct {
uint32_t range;
uint32_t code;
uint32_t init_bytes_left;
} lzma_range_decoder;
static inline bool
rc_read_init(lzma_range_decoder *rc, const uint8_t *restrict in,
size_t *restrict in_pos, size_t in_size)
{
while (rc->init_bytes_left > 0) {
if (*in_pos == in_size)
return false;
rc->code = (rc->code << 8) | in[*in_pos];
++*in_pos;
--rc->init_bytes_left;
}
return true;
}
#define rc_to_local(range_decoder, in_pos) lzma_range_decoder rc = range_decoder; size_t rc_in_pos = (in_pos); uint32_t rc_bound
#define rc_from_local(range_decoder, in_pos) do { range_decoder = rc; in_pos = rc_in_pos; } while (0)
#define rc_reset(range_decoder) do { (range_decoder).range = UINT32_MAX; (range_decoder).code = 0; (range_decoder).init_bytes_left = 5; } while (0)
#define rc_is_finished(range_decoder) ((range_decoder).code == 0)
#define rc_normalize(seq) do { if (rc.range < RC_TOP_VALUE) { if (unlikely(rc_in_pos == in_size)) { coder->sequence = seq; goto out; } rc.range <<= RC_SHIFT_BITS; rc.code = (rc.code << RC_SHIFT_BITS) | in[rc_in_pos++]; } } while (0)
#define rc_if_0(prob, seq) rc_normalize(seq); rc_bound = (rc.range >> RC_BIT_MODEL_TOTAL_BITS) * (prob); if (rc.code < rc_bound)
#define rc_update_0(prob) do { rc.range = rc_bound; prob += (RC_BIT_MODEL_TOTAL - (prob)) >> RC_MOVE_BITS; } while (0)
#define rc_update_1(prob) do { rc.range -= rc_bound; rc.code -= rc_bound; prob -= (prob) >> RC_MOVE_BITS; } while (0)
#define rc_bit_last(prob, action0, action1, seq) do { rc_if_0(prob, seq) { rc_update_0(prob); action0; } else { rc_update_1(prob); action1; } } while (0)
#define rc_bit(prob, action0, action1, seq) rc_bit_last(prob, symbol <<= 1; action0, symbol = (symbol << 1) + 1; action1, seq);
#define rc_bit_case(prob, action0, action1, seq) case seq: rc_bit(prob, action0, action1, seq)
#define rc_direct(dest, seq) do { rc_normalize(seq); rc.range >>= 1; rc.code -= rc.range; rc_bound = UINT32_C(0) - (rc.code >> 31); rc.code += rc.range & rc_bound; dest = (dest << 1) + (rc_bound + 1); } while (0)
