#if defined(HAVE_CONFIG_H)
#include "common.h"
#endif
#define RC_SHIFT_BITS 8
#define RC_TOP_BITS 24
#define RC_TOP_VALUE (UINT32_C(1) << RC_TOP_BITS)
#define RC_BIT_MODEL_TOTAL_BITS 11
#define RC_BIT_MODEL_TOTAL (UINT32_C(1) << RC_BIT_MODEL_TOTAL_BITS)
#define RC_MOVE_BITS 5
#define bit_reset(prob) prob = RC_BIT_MODEL_TOTAL >> 1
#define bittree_reset(probs, bit_levels) for (uint32_t bt_i = 0; bt_i < (1 << (bit_levels)); ++bt_i) bit_reset((probs)[bt_i])
typedef uint16_t probability;
