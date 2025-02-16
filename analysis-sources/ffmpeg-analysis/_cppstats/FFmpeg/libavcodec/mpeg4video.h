#include <stdint.h>
#include "get_bits.h"
#include "mpegvideo.h"
#include "rl.h"
#define RECT_SHAPE 0
#define BIN_SHAPE 1
#define BIN_ONLY_SHAPE 2
#define GRAY_SHAPE 3
#define SIMPLE_VO_TYPE 1
#define CORE_VO_TYPE 3
#define MAIN_VO_TYPE 4
#define NBIT_VO_TYPE 5
#define ARTS_VO_TYPE 10
#define ACE_VO_TYPE 12
#define SIMPLE_STUDIO_VO_TYPE 14
#define CORE_STUDIO_VO_TYPE 15
#define ADV_SIMPLE_VO_TYPE 17
#define VOT_VIDEO_ID 1
#define VOT_STILL_TEXTURE_ID 2
#define EXTENDED_PAR 15
#define STATIC_SPRITE 1
#define GMC_SPRITE 2
#define MOTION_MARKER 0x1F001
#define DC_MARKER 0x6B001
#define VOS_STARTCODE 0x1B0
#define USER_DATA_STARTCODE 0x1B2
#define GOP_STARTCODE 0x1B3
#define VISUAL_OBJ_STARTCODE 0x1B5
#define VOP_STARTCODE 0x1B6
#define SLICE_STARTCODE 0x1B7
#define EXT_STARTCODE 0x1B8
#define QUANT_MATRIX_EXT_ID 0x3
#define MAX_NVOP_SIZE 19
typedef struct Mpeg4DecContext {
MpegEncContext m;
int time_increment_bits;
int shape;
int vol_sprite_usage;
int sprite_brightness_change;
int num_sprite_warping_points;
uint16_t sprite_traj[4][2];
int sprite_shift[2];
int rvlc;
int resync_marker;
int t_frame;
int new_pred;
int enhancement_type;
int scalability;
int use_intra_dc_vlc;
int intra_dc_threshold;
int divx_version;
int divx_build;
int xvid_build;
int lavc_build;
int showed_packed_warning;
int vol_control_parameters;
int cplx_estimation_trash_i;
int cplx_estimation_trash_p;
int cplx_estimation_trash_b;
VLC studio_intra_tab[12];
VLC studio_luma_dc;
VLC studio_chroma_dc;
int rgb;
} Mpeg4DecContext;
static const uint8_t mpeg4_block_count[4] = {0, 6, 8, 12};
extern const uint8_t ff_mpeg4_DCtab_lum[13][2];
extern const uint8_t ff_mpeg4_DCtab_chrom[13][2];
extern const uint16_t ff_mpeg4_intra_vlc[103][2];
extern const int8_t ff_mpeg4_intra_level[102];
extern const int8_t ff_mpeg4_intra_run[102];
extern RLTable ff_mpeg4_rl_intra;
extern RLTable ff_rvlc_rl_inter;
extern RLTable ff_rvlc_rl_intra;
extern const uint16_t ff_sprite_trajectory_tab[15][2];
extern const uint8_t ff_mb_type_b_tab[4][2];
extern const int16_t ff_mpeg4_default_intra_matrix[64];
extern const int16_t ff_mpeg4_default_non_intra_matrix[64];
extern const uint8_t ff_mpeg4_y_dc_scale_table[32];
extern const uint8_t ff_mpeg4_c_dc_scale_table[32];
extern const uint16_t ff_mpeg4_resync_prefix[8];
extern const uint8_t ff_mpeg4_dc_threshold[8];
extern const uint16_t ff_mpeg4_studio_dc_luma[19][2];
extern const uint16_t ff_mpeg4_studio_dc_chroma[19][2];
extern const uint16_t ff_mpeg4_studio_intra[12][22][2];
void ff_mpeg4_encode_mb(MpegEncContext *s,
int16_t block[6][64],
int motion_x, int motion_y);
void ff_mpeg4_pred_ac(MpegEncContext *s, int16_t *block, int n,
int dir);
void ff_set_mpeg4_time(MpegEncContext *s);
int ff_mpeg4_encode_picture_header(MpegEncContext *s, int picture_number);
int ff_mpeg4_decode_picture_header(Mpeg4DecContext *ctx, GetBitContext *gb, int header);
void ff_mpeg4_encode_video_packet_header(MpegEncContext *s);
void ff_mpeg4_clean_buffers(MpegEncContext *s);
void ff_mpeg4_stuffing(PutBitContext *pbc);
void ff_mpeg4_init_partitions(MpegEncContext *s);
void ff_mpeg4_merge_partitions(MpegEncContext *s);
void ff_clean_mpeg4_qscales(MpegEncContext *s);
int ff_mpeg4_decode_partitions(Mpeg4DecContext *ctx);
int ff_mpeg4_get_video_packet_prefix_length(MpegEncContext *s);
int ff_mpeg4_decode_video_packet_header(Mpeg4DecContext *ctx);
int ff_mpeg4_decode_studio_slice_header(Mpeg4DecContext *ctx);
void ff_mpeg4_init_direct_mv(MpegEncContext *s);
void ff_mpeg4videodec_static_init(void);
int ff_mpeg4_workaround_bugs(AVCodecContext *avctx);
int ff_mpeg4_frame_end(AVCodecContext *avctx, const uint8_t *buf, int buf_size);
int ff_mpeg4_set_direct_mv(MpegEncContext *s, int mx, int my);
extern uint8_t ff_mpeg4_static_rl_table_store[3][2][2 * MAX_RUN + MAX_LEVEL + 3];
#if 0 
#define IS_3IV1 s->codec_tag == AV_RL32("3IV1")
#else
#define IS_3IV1 0
#endif
static inline int ff_mpeg4_pred_dc(MpegEncContext *s, int n, int level,
int *dir_ptr, int encoding)
{
int a, b, c, wrap, pred, scale, ret;
int16_t *dc_val;
if (n < 4)
scale = s->y_dc_scale;
else
scale = s->c_dc_scale;
if (IS_3IV1)
scale = 8;
wrap = s->block_wrap[n];
dc_val = s->dc_val[0] + s->block_index[n];
a = dc_val[-1];
b = dc_val[-1 - wrap];
c = dc_val[-wrap];
if (s->first_slice_line && n != 3) {
if (n != 2)
b = c = 1024;
if (n != 1 && s->mb_x == s->resync_mb_x)
b = a = 1024;
}
if (s->mb_x == s->resync_mb_x && s->mb_y == s->resync_mb_y + 1) {
if (n == 0 || n == 4 || n == 5)
b = 1024;
}
if (abs(a - b) < abs(b - c)) {
pred = c;
*dir_ptr = 1; 
} else {
pred = a;
*dir_ptr = 0; 
}
pred = FASTDIV((pred + (scale >> 1)), scale);
if (encoding) {
ret = level - pred;
} else {
level += pred;
ret = level;
}
level *= scale;
if (level & (~2047)) {
if (!s->encoding && (s->avctx->err_recognition & (AV_EF_BITSTREAM | AV_EF_AGGRESSIVE))) {
if (level < 0) {
av_log(s->avctx, AV_LOG_ERROR,
"dc<0 at %dx%d\n", s->mb_x, s->mb_y);
return AVERROR_INVALIDDATA;
}
if (level > 2048 + scale) {
av_log(s->avctx, AV_LOG_ERROR,
"dc overflow at %dx%d\n", s->mb_x, s->mb_y);
return AVERROR_INVALIDDATA;
}
}
if (level < 0)
level = 0;
else if (!(s->workaround_bugs & FF_BUG_DC_CLIP))
level = 2047;
}
dc_val[0] = level;
return ret;
}
