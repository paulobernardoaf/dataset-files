#include <stdint.h>
#include "avcodec.h"
#include "hpeldsp.h"
#include "qpeldsp.h"
struct MpegEncContext;
#if ARCH_IA64 
#define MAX_MV 1024
#else
#define MAX_MV 4096
#endif
#define MAX_DMV (2*MAX_MV)
#define ME_MAP_SIZE 64
#define FF_ME_ZERO 0
#define FF_ME_EPZS 1
#define FF_ME_XONE 2
typedef struct MotionEstContext {
AVCodecContext *avctx;
int skip; 
int co_located_mv[4][2]; 
int direct_basis_mv[4][2];
uint8_t *scratchpad; 
uint8_t *best_mb;
uint8_t *temp_mb[2];
uint8_t *temp;
int best_bits;
uint32_t *map; 
uint32_t *score_map; 
unsigned map_generation;
int pre_penalty_factor;
int penalty_factor; 
int sub_penalty_factor;
int mb_penalty_factor;
int flags;
int sub_flags;
int mb_flags;
int pre_pass; 
int dia_size;
int xmin;
int xmax;
int ymin;
int ymax;
int pred_x;
int pred_y;
uint8_t *src[4][4];
uint8_t *ref[4][4];
int stride;
int uvstride;
int64_t mc_mb_var_sum_temp;
int64_t mb_var_sum_temp;
int scene_change_score;
op_pixels_func(*hpel_put)[4];
op_pixels_func(*hpel_avg)[4];
qpel_mc_func(*qpel_put)[16];
qpel_mc_func(*qpel_avg)[16];
uint8_t (*mv_penalty)[MAX_DMV * 2 + 1]; 
uint8_t *current_mv_penalty;
int (*sub_motion_search)(struct MpegEncContext *s,
int *mx_ptr, int *my_ptr, int dmin,
int src_index, int ref_index,
int size, int h);
} MotionEstContext;
static inline int ff_h263_round_chroma(int x)
{
static const uint8_t h263_chroma_roundtab[16] = {
0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
};
return h263_chroma_roundtab[x & 0xf] + (x >> 3);
}
int ff_init_me(struct MpegEncContext *s);
void ff_estimate_p_frame_motion(struct MpegEncContext *s, int mb_x, int mb_y);
void ff_estimate_b_frame_motion(struct MpegEncContext *s, int mb_x, int mb_y);
int ff_pre_estimate_p_frame_motion(struct MpegEncContext *s,
int mb_x, int mb_y);
int ff_epzs_motion_search(struct MpegEncContext *s, int *mx_ptr, int *my_ptr,
int P[10][2], int src_index, int ref_index,
int16_t (*last_mv)[2], int ref_mv_scale, int size,
int h);
int ff_get_mb_score(struct MpegEncContext *s, int mx, int my, int src_index,
int ref_index, int size, int h, int add_rate);
int ff_get_best_fcode(struct MpegEncContext *s,
int16_t (*mv_table)[2], int type);
void ff_fix_long_p_mvs(struct MpegEncContext *s, int type);
void ff_fix_long_mvs(struct MpegEncContext *s, uint8_t *field_select_table,
int field_select, int16_t (*mv_table)[2], int f_code,
int type, int truncate);
