#include <stdint.h>
#include "libavutil/rational.h"
#include "rl.h"
extern const AVRational ff_h263_pixel_aspect[16];
extern const uint8_t ff_h263_intra_MCBPC_code[9];
extern const uint8_t ff_h263_intra_MCBPC_bits[9];
extern const uint8_t ff_h263_inter_MCBPC_code[28];
extern const uint8_t ff_h263_inter_MCBPC_bits[28];
extern const uint8_t ff_h263_mbtype_b_tab[15][2];
extern const uint8_t ff_cbpc_b_tab[4][2];
extern const uint8_t ff_h263_cbpy_tab[16][2];
extern const uint8_t ff_mvtab[33][2];
extern const uint16_t ff_inter_vlc[103][2];
extern const int8_t ff_inter_level[102];
extern const int8_t ff_inter_run[102];
extern RLTable ff_h263_rl_inter;
extern RLTable ff_rl_intra_aic;
extern uint8_t ff_h263_static_rl_table_store[2][2][2 * MAX_RUN + MAX_LEVEL + 3];
extern const uint16_t ff_h263_format[8][2];
extern const uint8_t ff_aic_dc_scale_table[32];
extern const uint8_t ff_modified_quant_tab[2][32];
extern const uint8_t ff_h263_chroma_qscale_table[32];
extern const uint16_t ff_mba_max[6];
extern const uint8_t ff_mba_length[7];
