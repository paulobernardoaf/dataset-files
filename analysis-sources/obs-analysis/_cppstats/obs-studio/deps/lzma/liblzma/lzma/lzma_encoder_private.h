#include "lz_encoder.h"
#include "range_encoder.h"
#include "lzma_common.h"
#include "lzma_encoder.h"
#if defined(TUKLIB_FAST_UNALIGNED_ACCESS)
#define not_equal_16(a, b) (*(const uint16_t *)(a) != *(const uint16_t *)(b))
#else
#define not_equal_16(a, b) ((a)[0] != (b)[0] || (a)[1] != (b)[1])
#endif
#define OPTS (1 << 12)
typedef struct {
probability choice;
probability choice2;
probability low[POS_STATES_MAX][LEN_LOW_SYMBOLS];
probability mid[POS_STATES_MAX][LEN_MID_SYMBOLS];
probability high[LEN_HIGH_SYMBOLS];
uint32_t prices[POS_STATES_MAX][LEN_SYMBOLS];
uint32_t table_size;
uint32_t counters[POS_STATES_MAX];
} lzma_length_encoder;
typedef struct {
lzma_lzma_state state;
bool prev_1_is_literal;
bool prev_2;
uint32_t pos_prev_2;
uint32_t back_prev_2;
uint32_t price;
uint32_t pos_prev; 
uint32_t back_prev;
uint32_t backs[REP_DISTANCES];
} lzma_optimal;
struct lzma_coder_s {
lzma_range_encoder rc;
lzma_lzma_state state;
uint32_t reps[REP_DISTANCES];
lzma_match matches[MATCH_LEN_MAX + 1];
uint32_t matches_count;
uint32_t longest_match_length;
bool fast_mode;
bool is_initialized;
bool is_flushed;
uint32_t pos_mask; 
uint32_t literal_context_bits;
uint32_t literal_pos_mask;
probability literal[LITERAL_CODERS_MAX][LITERAL_CODER_SIZE];
probability is_match[STATES][POS_STATES_MAX];
probability is_rep[STATES];
probability is_rep0[STATES];
probability is_rep1[STATES];
probability is_rep2[STATES];
probability is_rep0_long[STATES][POS_STATES_MAX];
probability pos_slot[LEN_TO_POS_STATES][POS_SLOTS];
probability pos_special[FULL_DISTANCES - END_POS_MODEL_INDEX];
probability pos_align[ALIGN_TABLE_SIZE];
lzma_length_encoder match_len_encoder;
lzma_length_encoder rep_len_encoder;
uint32_t pos_slot_prices[LEN_TO_POS_STATES][POS_SLOTS];
uint32_t distances_prices[LEN_TO_POS_STATES][FULL_DISTANCES];
uint32_t dist_table_size;
uint32_t match_price_count;
uint32_t align_prices[ALIGN_TABLE_SIZE];
uint32_t align_price_count;
uint32_t opts_end_index;
uint32_t opts_current_index;
lzma_optimal opts[OPTS];
};
extern void lzma_lzma_optimum_fast(
lzma_coder *restrict coder, lzma_mf *restrict mf,
uint32_t *restrict back_res, uint32_t *restrict len_res);
extern void lzma_lzma_optimum_normal(lzma_coder *restrict coder,
lzma_mf *restrict mf, uint32_t *restrict back_res,
uint32_t *restrict len_res, uint32_t position);
