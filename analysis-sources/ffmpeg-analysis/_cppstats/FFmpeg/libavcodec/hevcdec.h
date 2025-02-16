#include <stdatomic.h>
#include "libavutil/buffer.h"
#include "libavutil/md5.h"
#include "avcodec.h"
#include "bswapdsp.h"
#include "cabac.h"
#include "get_bits.h"
#include "hevcpred.h"
#include "h2645_parse.h"
#include "hevc.h"
#include "hevc_ps.h"
#include "hevc_sei.h"
#include "hevcdsp.h"
#include "internal.h"
#include "thread.h"
#include "videodsp.h"
#define MAX_NB_THREADS 16
#define SHIFT_CTB_WPP 2
#define MAX_TRANSFORM_DEPTH 5
#define MAX_TB_SIZE 32
#define MAX_QP 51
#define DEFAULT_INTRA_TC_OFFSET 2
#define HEVC_CONTEXTS 199
#define MRG_MAX_NUM_CANDS 5
#define L0 0
#define L1 1
#define EPEL_EXTRA_BEFORE 1
#define EPEL_EXTRA_AFTER 2
#define EPEL_EXTRA 3
#define QPEL_EXTRA_BEFORE 3
#define QPEL_EXTRA_AFTER 4
#define QPEL_EXTRA 7
#define EDGE_EMU_BUFFER_STRIDE 80
#define SAMPLE(tab, x, y) ((tab)[(y) * s->sps->width + (x)])
#define SAMPLE_CTB(tab, x, y) ((tab)[(y) * min_cb_width + (x)])
#define IS_IDR(s) ((s)->nal_unit_type == HEVC_NAL_IDR_W_RADL || (s)->nal_unit_type == HEVC_NAL_IDR_N_LP)
#define IS_BLA(s) ((s)->nal_unit_type == HEVC_NAL_BLA_W_RADL || (s)->nal_unit_type == HEVC_NAL_BLA_W_LP || (s)->nal_unit_type == HEVC_NAL_BLA_N_LP)
#define IS_IRAP(s) ((s)->nal_unit_type >= 16 && (s)->nal_unit_type <= 23)
enum RPSType {
ST_CURR_BEF = 0,
ST_CURR_AFT,
ST_FOLL,
LT_CURR,
LT_FOLL,
NB_RPS_TYPE,
};
enum SyntaxElement {
SAO_MERGE_FLAG = 0,
SAO_TYPE_IDX,
SAO_EO_CLASS,
SAO_BAND_POSITION,
SAO_OFFSET_ABS,
SAO_OFFSET_SIGN,
END_OF_SLICE_FLAG,
SPLIT_CODING_UNIT_FLAG,
CU_TRANSQUANT_BYPASS_FLAG,
SKIP_FLAG,
CU_QP_DELTA,
PRED_MODE_FLAG,
PART_MODE,
PCM_FLAG,
PREV_INTRA_LUMA_PRED_FLAG,
MPM_IDX,
REM_INTRA_LUMA_PRED_MODE,
INTRA_CHROMA_PRED_MODE,
MERGE_FLAG,
MERGE_IDX,
INTER_PRED_IDC,
REF_IDX_L0,
REF_IDX_L1,
ABS_MVD_GREATER0_FLAG,
ABS_MVD_GREATER1_FLAG,
ABS_MVD_MINUS2,
MVD_SIGN_FLAG,
MVP_LX_FLAG,
NO_RESIDUAL_DATA_FLAG,
SPLIT_TRANSFORM_FLAG,
CBF_LUMA,
CBF_CB_CR,
TRANSFORM_SKIP_FLAG,
EXPLICIT_RDPCM_FLAG,
EXPLICIT_RDPCM_DIR_FLAG,
LAST_SIGNIFICANT_COEFF_X_PREFIX,
LAST_SIGNIFICANT_COEFF_Y_PREFIX,
LAST_SIGNIFICANT_COEFF_X_SUFFIX,
LAST_SIGNIFICANT_COEFF_Y_SUFFIX,
SIGNIFICANT_COEFF_GROUP_FLAG,
SIGNIFICANT_COEFF_FLAG,
COEFF_ABS_LEVEL_GREATER1_FLAG,
COEFF_ABS_LEVEL_GREATER2_FLAG,
COEFF_ABS_LEVEL_REMAINING,
COEFF_SIGN_FLAG,
LOG2_RES_SCALE_ABS,
RES_SCALE_SIGN_FLAG,
CU_CHROMA_QP_OFFSET_FLAG,
CU_CHROMA_QP_OFFSET_IDX,
};
enum PartMode {
PART_2Nx2N = 0,
PART_2NxN = 1,
PART_Nx2N = 2,
PART_NxN = 3,
PART_2NxnU = 4,
PART_2NxnD = 5,
PART_nLx2N = 6,
PART_nRx2N = 7,
};
enum PredMode {
MODE_INTER = 0,
MODE_INTRA,
MODE_SKIP,
};
enum InterPredIdc {
PRED_L0 = 0,
PRED_L1,
PRED_BI,
};
enum PredFlag {
PF_INTRA = 0,
PF_L0,
PF_L1,
PF_BI,
};
enum IntraPredMode {
INTRA_PLANAR = 0,
INTRA_DC,
INTRA_ANGULAR_2,
INTRA_ANGULAR_3,
INTRA_ANGULAR_4,
INTRA_ANGULAR_5,
INTRA_ANGULAR_6,
INTRA_ANGULAR_7,
INTRA_ANGULAR_8,
INTRA_ANGULAR_9,
INTRA_ANGULAR_10,
INTRA_ANGULAR_11,
INTRA_ANGULAR_12,
INTRA_ANGULAR_13,
INTRA_ANGULAR_14,
INTRA_ANGULAR_15,
INTRA_ANGULAR_16,
INTRA_ANGULAR_17,
INTRA_ANGULAR_18,
INTRA_ANGULAR_19,
INTRA_ANGULAR_20,
INTRA_ANGULAR_21,
INTRA_ANGULAR_22,
INTRA_ANGULAR_23,
INTRA_ANGULAR_24,
INTRA_ANGULAR_25,
INTRA_ANGULAR_26,
INTRA_ANGULAR_27,
INTRA_ANGULAR_28,
INTRA_ANGULAR_29,
INTRA_ANGULAR_30,
INTRA_ANGULAR_31,
INTRA_ANGULAR_32,
INTRA_ANGULAR_33,
INTRA_ANGULAR_34,
};
enum SAOType {
SAO_NOT_APPLIED = 0,
SAO_BAND,
SAO_EDGE,
SAO_APPLIED
};
enum SAOEOClass {
SAO_EO_HORIZ = 0,
SAO_EO_VERT,
SAO_EO_135D,
SAO_EO_45D,
};
enum ScanType {
SCAN_DIAG = 0,
SCAN_HORIZ,
SCAN_VERT,
};
typedef struct RefPicList {
struct HEVCFrame *ref[HEVC_MAX_REFS];
int list[HEVC_MAX_REFS];
int isLongTerm[HEVC_MAX_REFS];
int nb_refs;
} RefPicList;
typedef struct RefPicListTab {
RefPicList refPicList[2];
} RefPicListTab;
typedef struct CodingUnit {
int x;
int y;
enum PredMode pred_mode; 
enum PartMode part_mode; 
uint8_t intra_split_flag; 
uint8_t max_trafo_depth; 
uint8_t cu_transquant_bypass_flag;
} CodingUnit;
typedef struct Mv {
int16_t x; 
int16_t y; 
} Mv;
typedef struct MvField {
DECLARE_ALIGNED(4, Mv, mv)[2];
int8_t ref_idx[2];
int8_t pred_flag;
} MvField;
typedef struct NeighbourAvailable {
int cand_bottom_left;
int cand_left;
int cand_up;
int cand_up_left;
int cand_up_right;
int cand_up_right_sap;
} NeighbourAvailable;
typedef struct PredictionUnit {
int mpm_idx;
int rem_intra_luma_pred_mode;
uint8_t intra_pred_mode[4];
Mv mvd;
uint8_t merge_flag;
uint8_t intra_pred_mode_c[4];
uint8_t chroma_mode_c[4];
} PredictionUnit;
typedef struct TransformUnit {
int cu_qp_delta;
int res_scale_val;
int intra_pred_mode;
int intra_pred_mode_c;
int chroma_mode_c;
uint8_t is_cu_qp_delta_coded;
uint8_t is_cu_chroma_qp_offset_coded;
int8_t cu_qp_offset_cb;
int8_t cu_qp_offset_cr;
uint8_t cross_pf;
} TransformUnit;
typedef struct DBParams {
int beta_offset;
int tc_offset;
} DBParams;
#define HEVC_FRAME_FLAG_OUTPUT (1 << 0)
#define HEVC_FRAME_FLAG_SHORT_REF (1 << 1)
#define HEVC_FRAME_FLAG_LONG_REF (1 << 2)
#define HEVC_FRAME_FLAG_BUMPING (1 << 3)
typedef struct HEVCFrame {
AVFrame *frame;
ThreadFrame tf;
MvField *tab_mvf;
RefPicList *refPicList;
RefPicListTab **rpl_tab;
int ctb_count;
int poc;
struct HEVCFrame *collocated_ref;
AVBufferRef *tab_mvf_buf;
AVBufferRef *rpl_tab_buf;
AVBufferRef *rpl_buf;
AVBufferRef *hwaccel_priv_buf;
void *hwaccel_picture_private;
uint16_t sequence;
uint8_t flags;
} HEVCFrame;
typedef struct HEVCLocalContext {
uint8_t cabac_state[HEVC_CONTEXTS];
uint8_t stat_coeff[4];
uint8_t first_qp_group;
GetBitContext gb;
CABACContext cc;
int8_t qp_y;
int8_t curr_qp_y;
int qPy_pred;
TransformUnit tu;
uint8_t ctb_left_flag;
uint8_t ctb_up_flag;
uint8_t ctb_up_right_flag;
uint8_t ctb_up_left_flag;
int end_of_tiles_x;
int end_of_tiles_y;
DECLARE_ALIGNED(32, uint8_t, edge_emu_buffer)[(MAX_PB_SIZE + 7) * EDGE_EMU_BUFFER_STRIDE * 2];
DECLARE_ALIGNED(32, uint8_t, edge_emu_buffer2)[(MAX_PB_SIZE + 7) * EDGE_EMU_BUFFER_STRIDE * 2];
DECLARE_ALIGNED(32, int16_t, tmp)[MAX_PB_SIZE * MAX_PB_SIZE];
int ct_depth;
CodingUnit cu;
PredictionUnit pu;
NeighbourAvailable na;
#define BOUNDARY_LEFT_SLICE (1 << 0)
#define BOUNDARY_LEFT_TILE (1 << 1)
#define BOUNDARY_UPPER_SLICE (1 << 2)
#define BOUNDARY_UPPER_TILE (1 << 3)
int boundary_flags;
} HEVCLocalContext;
typedef struct HEVCContext {
const AVClass *c; 
AVCodecContext *avctx;
struct HEVCContext *sList[MAX_NB_THREADS];
HEVCLocalContext *HEVClcList[MAX_NB_THREADS];
HEVCLocalContext *HEVClc;
uint8_t threads_type;
uint8_t threads_number;
int width;
int height;
uint8_t *cabac_state;
uint8_t slice_initialized;
AVFrame *frame;
AVFrame *output_frame;
uint8_t *sao_pixel_buffer_h[3];
uint8_t *sao_pixel_buffer_v[3];
HEVCParamSets ps;
HEVCSEI sei;
struct AVMD5 *md5_ctx;
AVBufferPool *tab_mvf_pool;
AVBufferPool *rpl_tab_pool;
RefPicList rps[5];
SliceHeader sh;
SAOParams *sao;
DBParams *deblock;
enum HEVCNALUnitType nal_unit_type;
int temporal_id; 
HEVCFrame *ref;
HEVCFrame DPB[32];
int poc;
int pocTid0;
int slice_idx; 
int eos; 
int last_eos; 
int max_ra;
int bs_width;
int bs_height;
int overlap;
int is_decoded;
int no_rasl_output_flag;
HEVCPredContext hpc;
HEVCDSPContext hevcdsp;
VideoDSPContext vdsp;
BswapDSPContext bdsp;
int8_t *qp_y_tab;
uint8_t *horizontal_bs;
uint8_t *vertical_bs;
int32_t *tab_slice_address;
uint8_t *skip_flag;
uint8_t *tab_ct_depth;
uint8_t *tab_ipm;
uint8_t *cbf_luma; 
uint8_t *is_pcm;
uint8_t *filter_slice_edges;
uint8_t *checksum_buf;
int checksum_buf_size;
uint16_t seq_decode;
uint16_t seq_output;
int enable_parallel_tiles;
atomic_int wpp_err;
const uint8_t *data;
H2645Packet pkt;
enum HEVCNALUnitType first_nal_type;
uint8_t context_initialized;
int is_nalff; 
int apply_defdispwin;
int nal_length_size; 
int nuh_layer_id;
} HEVCContext;
void ff_hevc_clear_refs(HEVCContext *s);
void ff_hevc_flush_dpb(HEVCContext *s);
RefPicList *ff_hevc_get_ref_list(HEVCContext *s, HEVCFrame *frame,
int x0, int y0);
int ff_hevc_frame_rps(HEVCContext *s);
int ff_hevc_slice_rpl(HEVCContext *s);
void ff_hevc_save_states(HEVCContext *s, int ctb_addr_ts);
int ff_hevc_cabac_init(HEVCContext *s, int ctb_addr_ts);
int ff_hevc_sao_merge_flag_decode(HEVCContext *s);
int ff_hevc_sao_type_idx_decode(HEVCContext *s);
int ff_hevc_sao_band_position_decode(HEVCContext *s);
int ff_hevc_sao_offset_abs_decode(HEVCContext *s);
int ff_hevc_sao_offset_sign_decode(HEVCContext *s);
int ff_hevc_sao_eo_class_decode(HEVCContext *s);
int ff_hevc_end_of_slice_flag_decode(HEVCContext *s);
int ff_hevc_cu_transquant_bypass_flag_decode(HEVCContext *s);
int ff_hevc_skip_flag_decode(HEVCContext *s, int x0, int y0,
int x_cb, int y_cb);
int ff_hevc_pred_mode_decode(HEVCContext *s);
int ff_hevc_split_coding_unit_flag_decode(HEVCContext *s, int ct_depth,
int x0, int y0);
int ff_hevc_part_mode_decode(HEVCContext *s, int log2_cb_size);
int ff_hevc_pcm_flag_decode(HEVCContext *s);
int ff_hevc_prev_intra_luma_pred_flag_decode(HEVCContext *s);
int ff_hevc_mpm_idx_decode(HEVCContext *s);
int ff_hevc_rem_intra_luma_pred_mode_decode(HEVCContext *s);
int ff_hevc_intra_chroma_pred_mode_decode(HEVCContext *s);
int ff_hevc_merge_idx_decode(HEVCContext *s);
int ff_hevc_merge_flag_decode(HEVCContext *s);
int ff_hevc_inter_pred_idc_decode(HEVCContext *s, int nPbW, int nPbH);
int ff_hevc_ref_idx_lx_decode(HEVCContext *s, int num_ref_idx_lx);
int ff_hevc_mvp_lx_flag_decode(HEVCContext *s);
int ff_hevc_no_residual_syntax_flag_decode(HEVCContext *s);
int ff_hevc_split_transform_flag_decode(HEVCContext *s, int log2_trafo_size);
int ff_hevc_cbf_cb_cr_decode(HEVCContext *s, int trafo_depth);
int ff_hevc_cbf_luma_decode(HEVCContext *s, int trafo_depth);
int ff_hevc_log2_res_scale_abs(HEVCContext *s, int idx);
int ff_hevc_res_scale_sign_flag(HEVCContext *s, int idx);
int ff_hevc_frame_nb_refs(const HEVCContext *s);
int ff_hevc_set_new_ref(HEVCContext *s, AVFrame **frame, int poc);
static av_always_inline int ff_hevc_nal_is_nonref(enum HEVCNALUnitType type)
{
switch (type) {
case HEVC_NAL_TRAIL_N:
case HEVC_NAL_TSA_N:
case HEVC_NAL_STSA_N:
case HEVC_NAL_RADL_N:
case HEVC_NAL_RASL_N:
case HEVC_NAL_VCL_N10:
case HEVC_NAL_VCL_N12:
case HEVC_NAL_VCL_N14:
return 1;
default: break;
}
return 0;
}
int ff_hevc_output_frame(HEVCContext *s, AVFrame *frame, int flush);
void ff_hevc_bump_frame(HEVCContext *s);
void ff_hevc_unref_frame(HEVCContext *s, HEVCFrame *frame, int flags);
void ff_hevc_set_neighbour_available(HEVCContext *s, int x0, int y0,
int nPbW, int nPbH);
void ff_hevc_luma_mv_merge_mode(HEVCContext *s, int x0, int y0,
int nPbW, int nPbH, int log2_cb_size,
int part_idx, int merge_idx, MvField *mv);
void ff_hevc_luma_mv_mvp_mode(HEVCContext *s, int x0, int y0,
int nPbW, int nPbH, int log2_cb_size,
int part_idx, int merge_idx,
MvField *mv, int mvp_lx_flag, int LX);
void ff_hevc_set_qPy(HEVCContext *s, int xBase, int yBase,
int log2_cb_size);
void ff_hevc_deblocking_boundary_strengths(HEVCContext *s, int x0, int y0,
int log2_trafo_size);
int ff_hevc_cu_qp_delta_sign_flag(HEVCContext *s);
int ff_hevc_cu_qp_delta_abs(HEVCContext *s);
int ff_hevc_cu_chroma_qp_offset_flag(HEVCContext *s);
int ff_hevc_cu_chroma_qp_offset_idx(HEVCContext *s);
void ff_hevc_hls_filter(HEVCContext *s, int x, int y, int ctb_size);
void ff_hevc_hls_filters(HEVCContext *s, int x_ctb, int y_ctb, int ctb_size);
void ff_hevc_hls_residual_coding(HEVCContext *s, int x0, int y0,
int log2_trafo_size, enum ScanType scan_idx,
int c_idx);
void ff_hevc_hls_mvd_coding(HEVCContext *s, int x0, int y0, int log2_cb_size);
extern const uint8_t ff_hevc_qpel_extra_before[4];
extern const uint8_t ff_hevc_qpel_extra_after[4];
extern const uint8_t ff_hevc_qpel_extra[4];
