#include "libavutil/avutil.h"
#define AV_ME_METHOD_ESA 1
#define AV_ME_METHOD_TSS 2
#define AV_ME_METHOD_TDLS 3
#define AV_ME_METHOD_NTSS 4
#define AV_ME_METHOD_FSS 5
#define AV_ME_METHOD_DS 6
#define AV_ME_METHOD_HEXBS 7
#define AV_ME_METHOD_EPZS 8
#define AV_ME_METHOD_UMH 9
typedef struct AVMotionEstPredictor {
int mvs[10][2];
int nb;
} AVMotionEstPredictor;
typedef struct AVMotionEstContext {
uint8_t *data_cur, *data_ref;
int linesize;
int mb_size;
int search_param;
int width;
int height;
int x_min;
int x_max;
int y_min;
int y_max;
int pred_x; 
int pred_y; 
AVMotionEstPredictor preds[2];
uint64_t (*get_cost)(struct AVMotionEstContext *me_ctx, int x_mb, int y_mb,
int mv_x, int mv_y);
} AVMotionEstContext;
void ff_me_init_context(AVMotionEstContext *me_ctx, int mb_size, int search_param,
int width, int height, int x_min, int x_max, int y_min, int y_max);
uint64_t ff_me_cmp_sad(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int x_mv, int y_mv);
uint64_t ff_me_search_esa(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
uint64_t ff_me_search_tss(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
uint64_t ff_me_search_tdls(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
uint64_t ff_me_search_ntss(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
uint64_t ff_me_search_fss(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
uint64_t ff_me_search_ds(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
uint64_t ff_me_search_hexbs(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
uint64_t ff_me_search_epzs(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
uint64_t ff_me_search_umh(AVMotionEstContext *me_ctx, int x_mb, int y_mb, int *mv);
