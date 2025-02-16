#include "avcodec.h"
#include "h264chroma.h"
#include "mpegvideo.h"
#include "intrax8.h"
#include "vc1_common.h"
#include "vc1dsp.h"
#define AC_VLC_BITS 9
enum QuantMode {
QUANT_FRAME_IMPLICIT, 
QUANT_FRAME_EXPLICIT, 
QUANT_NON_UNIFORM, 
QUANT_UNIFORM 
};
enum DQProfile {
DQPROFILE_FOUR_EDGES,
DQPROFILE_DOUBLE_EDGES,
DQPROFILE_SINGLE_EDGE,
DQPROFILE_ALL_MBS
};
enum DQSingleEdge {
DQSINGLE_BEDGE_LEFT,
DQSINGLE_BEDGE_TOP,
DQSINGLE_BEDGE_RIGHT,
DQSINGLE_BEDGE_BOTTOM
};
enum DQDoubleEdge {
DQDOUBLE_BEDGE_TOPLEFT,
DQDOUBLE_BEDGE_TOPRIGHT,
DQDOUBLE_BEDGE_BOTTOMRIGHT,
DQDOUBLE_BEDGE_BOTTOMLEFT
};
enum MVModes {
MV_PMODE_1MV_HPEL_BILIN,
MV_PMODE_1MV,
MV_PMODE_1MV_HPEL,
MV_PMODE_MIXED_MV,
MV_PMODE_INTENSITY_COMP
};
enum MBModesIntfr {
MV_PMODE_INTFR_1MV,
MV_PMODE_INTFR_2MV_FIELD,
MV_PMODE_INTFR_2MV,
MV_PMODE_INTFR_4MV_FIELD,
MV_PMODE_INTFR_4MV,
MV_PMODE_INTFR_INTRA,
};
enum BMVTypes {
BMV_TYPE_BACKWARD,
BMV_TYPE_FORWARD,
BMV_TYPE_INTERPOLATED,
BMV_TYPE_DIRECT
};
enum TransformTypes {
TT_8X8,
TT_8X4_BOTTOM,
TT_8X4_TOP,
TT_8X4, 
TT_4X8_RIGHT,
TT_4X8_LEFT,
TT_4X8, 
TT_4X4
};
enum CodingSet {
CS_HIGH_MOT_INTRA = 0,
CS_HIGH_MOT_INTER,
CS_LOW_MOT_INTRA,
CS_LOW_MOT_INTER,
CS_MID_RATE_INTRA,
CS_MID_RATE_INTER,
CS_HIGH_RATE_INTRA,
CS_HIGH_RATE_INTER
};
enum COTypes {
CONDOVER_NONE = 0,
CONDOVER_ALL,
CONDOVER_SELECT
};
enum FrameCodingMode {
PROGRESSIVE = 0, 
ILACE_FRAME, 
ILACE_FIELD 
};
enum Imode {
IMODE_RAW,
IMODE_NORM2,
IMODE_DIFF2,
IMODE_NORM6,
IMODE_DIFF6,
IMODE_ROWSKIP,
IMODE_COLSKIP
};
typedef struct VC1Context{
MpegEncContext s;
IntraX8Context x8;
H264ChromaContext h264chroma;
VC1DSPContext vc1dsp;
int res_sprite; 
int res_y411; 
int res_x8; 
int multires; 
int res_fasttx; 
int res_transtab; 
int rangered; 
int res_rtm_flag; 
int reserved; 
int level; 
int chromaformat; 
int postprocflag; 
int broadcast; 
int interlace; 
int tfcntrflag; 
int panscanflag; 
int refdist_flag; 
int extended_dmv; 
int color_prim; 
int transfer_char; 
int matrix_coef; 
int hrd_param_flag; 
int psf; 
int profile; 
int frmrtq_postproc; 
int bitrtq_postproc; 
int max_coded_width, max_coded_height;
int fastuvmc; 
int extended_mv; 
int dquant; 
int vstransform; 
int overlap; 
int quantizer_mode; 
int finterpflag; 
uint8_t mv_mode; 
uint8_t mv_mode2; 
int k_x; 
int k_y; 
int range_x, range_y; 
uint8_t pq, altpq; 
uint8_t zz_8x8[4][64]; 
int left_blk_sh, top_blk_sh; 
const uint8_t* zz_8x4; 
const uint8_t* zz_4x8; 
uint8_t dquantfrm;
uint8_t dqprofile;
uint8_t dqsbedge;
uint8_t dqbilevel;
int c_ac_table_index; 
int y_ac_table_index; 
int ttfrm; 
uint8_t ttmbf; 
int *ttblk_base, *ttblk; 
int codingset; 
int codingset2; 
int pqindex; 
int a_avail, c_avail;
uint8_t *mb_type_base, *mb_type[3];
uint8_t lumscale;
uint8_t lumshift;
int16_t bfraction; 
uint8_t halfpq; 
uint8_t respic; 
int buffer_fullness; 
uint8_t mvrange; 
uint8_t pquantizer; 
VLC *cbpcy_vlc; 
int tt_index; 
uint8_t* mv_type_mb_plane; 
uint8_t* direct_mb_plane; 
uint8_t* forward_mb_plane; 
int mv_type_is_raw; 
int dmb_is_raw; 
int fmb_is_raw; 
int skip_is_raw; 
uint8_t last_luty[2][256], last_lutuv[2][256]; 
uint8_t aux_luty[2][256], aux_lutuv[2][256]; 
uint8_t next_luty[2][256], next_lutuv[2][256]; 
uint8_t (*curr_luty)[256] ,(*curr_lutuv)[256];
int last_use_ic, *curr_use_ic, next_use_ic, aux_use_ic;
int rnd; 
int cbptab;
uint8_t rangeredfrm; 
uint8_t interpfrm;
enum FrameCodingMode fcm;
uint8_t numpanscanwin;
uint8_t tfcntr;
uint8_t rptfrm, tff, rff;
uint16_t topleftx;
uint16_t toplefty;
uint16_t bottomrightx;
uint16_t bottomrighty;
uint8_t uvsamp;
uint8_t postproc;
int hrd_num_leaky_buckets;
uint8_t bit_rate_exponent;
uint8_t buffer_size_exponent;
uint8_t* acpred_plane; 
int acpred_is_raw;
uint8_t* over_flags_plane; 
int overflg_is_raw;
uint8_t condover;
uint16_t *hrd_rate, *hrd_buffer;
uint8_t *hrd_fullness;
uint8_t range_mapy_flag;
uint8_t range_mapuv_flag;
uint8_t range_mapy;
uint8_t range_mapuv;
uint8_t dmvrange; 
int fourmvswitch;
int intcomp;
uint8_t lumscale2; 
uint8_t lumshift2;
VLC* mbmode_vlc;
VLC* imv_vlc;
VLC* twomvbp_vlc;
VLC* fourmvbp_vlc;
uint8_t twomvbp;
uint8_t fourmvbp;
uint8_t* fieldtx_plane;
int fieldtx_is_raw;
uint8_t zzi_8x8[64];
uint8_t *blk_mv_type_base, *blk_mv_type; 
uint8_t *mv_f_base, *mv_f[2]; 
uint8_t *mv_f_next_base, *mv_f_next[2];
int field_mode; 
int fptype;
int second_field;
int refdist; 
int numref; 
int reffield; 
int intcompfield; 
int cur_field_type; 
int ref_field_type[2]; 
int blocks_off, mb_off;
int qs_last; 
int bmvtype;
int frfd, brfd; 
int first_pic_header_flag;
int pic_header_flag;
int mbmodetab;
int icbptab;
int imvtab;
int twomvbptab;
int fourmvbptab;
int new_sprite;
int two_sprites;
AVFrame *sprite_output_frame;
int output_width, output_height, sprite_width, sprite_height;
uint8_t* sr_rows[2][2]; 
int p_frame_skipped;
int bi_type;
int x8_type;
int16_t (*block)[6][64];
int n_allocated_blks, cur_blk_idx, left_blk_idx, topleft_blk_idx, top_blk_idx;
uint32_t *cbp_base, *cbp;
uint8_t *is_intra_base, *is_intra;
int16_t (*luma_mv_base)[2], (*luma_mv)[2];
uint8_t bfraction_lut_index; 
uint8_t broken_link; 
uint8_t closed_entry; 
int end_mb_x; 
int parse_only; 
int resync_marker; 
} VC1Context;
int ff_vc1_decode_sequence_header(AVCodecContext *avctx, VC1Context *v, GetBitContext *gb);
int ff_vc1_decode_entry_point(AVCodecContext *avctx, VC1Context *v, GetBitContext *gb);
int ff_vc1_parse_frame_header (VC1Context *v, GetBitContext *gb);
int ff_vc1_parse_frame_header_adv(VC1Context *v, GetBitContext *gb);
int ff_vc1_init_common(VC1Context *v);
int ff_vc1_decode_init_alloc_tables(VC1Context *v);
void ff_vc1_init_transposed_scantables(VC1Context *v);
int ff_vc1_decode_end(AVCodecContext *avctx);
void ff_vc1_decode_blocks(VC1Context *v);
void ff_vc1_i_overlap_filter(VC1Context *v);
void ff_vc1_p_overlap_filter(VC1Context *v);
void ff_vc1_i_loop_filter(VC1Context *v);
void ff_vc1_p_loop_filter(VC1Context *v);
void ff_vc1_p_intfr_loop_filter(VC1Context *v);
void ff_vc1_b_intfi_loop_filter(VC1Context *v);
void ff_vc1_mc_1mv(VC1Context *v, int dir);
void ff_vc1_mc_4mv_luma(VC1Context *v, int n, int dir, int avg);
void ff_vc1_mc_4mv_chroma(VC1Context *v, int dir);
void ff_vc1_mc_4mv_chroma4(VC1Context *v, int dir, int dir2, int avg);
void ff_vc1_interp_mc(VC1Context *v);
