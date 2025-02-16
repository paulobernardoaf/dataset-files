#include <float.h>
#include "avcodec.h"
#include "blockdsp.h"
#include "error_resilience.h"
#include "fdctdsp.h"
#include "get_bits.h"
#include "h264chroma.h"
#include "h263dsp.h"
#include "hpeldsp.h"
#include "idctdsp.h"
#include "internal.h"
#include "me_cmp.h"
#include "motion_est.h"
#include "mpegpicture.h"
#include "mpegvideodsp.h"
#include "mpegvideoencdsp.h"
#include "mpegvideodata.h"
#include "pixblockdsp.h"
#include "put_bits.h"
#include "ratecontrol.h"
#include "parser.h"
#include "mpegutils.h"
#include "mpeg12data.h"
#include "qpeldsp.h"
#include "thread.h"
#include "videodsp.h"
#include "libavutil/opt.h"
#include "libavutil/timecode.h"
#define MAX_THREADS 32
#define MAX_B_FRAMES 16
#define SEQ_END_CODE 0x000001b7
#define SEQ_START_CODE 0x000001b3
#define GOP_START_CODE 0x000001b8
#define PICTURE_START_CODE 0x00000100
#define SLICE_MIN_START_CODE 0x00000101
#define SLICE_MAX_START_CODE 0x000001af
#define EXT_START_CODE 0x000001b5
#define USER_START_CODE 0x000001b2
#define SLICE_START_CODE 0x000001b7
typedef struct MpegEncContext {
AVClass *class;
int y_dc_scale, c_dc_scale;
int ac_pred;
int block_last_index[12]; 
int h263_aic; 
ScanTable inter_scantable; 
ScanTable intra_scantable;
ScanTable intra_h_scantable;
ScanTable intra_v_scantable;
struct AVCodecContext *avctx;
int width, height;
int gop_size;
int intra_only; 
int64_t bit_rate; 
enum OutputFormat out_format; 
int h263_pred; 
int pb_frame; 
int h263_plus; 
int h263_flv; 
enum AVCodecID codec_id; 
int fixed_qscale; 
int encoding; 
int max_b_frames; 
int luma_elim_threshold;
int chroma_elim_threshold;
int strict_std_compliance; 
int workaround_bugs; 
int codec_tag; 
int context_initialized;
int input_picture_number; 
int coded_picture_number; 
int picture_number; 
int picture_in_gop_number; 
int mb_width, mb_height; 
int mb_stride; 
int b8_stride; 
int h_edge_pos, v_edge_pos;
int mb_num; 
ptrdiff_t linesize; 
ptrdiff_t uvlinesize; 
Picture *picture; 
Picture **input_picture; 
Picture **reordered_input_picture; 
int64_t user_specified_pts; 
int64_t dts_delta;
int64_t reordered_pts;
PutBitContext pb;
int start_mb_y; 
int end_mb_y; 
struct MpegEncContext *thread_context[MAX_THREADS];
int slice_context_count; 
Picture last_picture;
Picture next_picture;
Picture new_picture;
Picture current_picture; 
Picture *last_picture_ptr; 
Picture *next_picture_ptr; 
Picture *current_picture_ptr; 
int last_dc[3]; 
int16_t *dc_val_base;
int16_t *dc_val[3]; 
const uint8_t *y_dc_scale_table; 
const uint8_t *c_dc_scale_table; 
const uint8_t *chroma_qscale_table; 
uint8_t *coded_block_base;
uint8_t *coded_block; 
int16_t (*ac_val_base)[16];
int16_t (*ac_val[3])[16]; 
int mb_skipped; 
uint8_t *mbskip_table; 
uint8_t *mbintra_table; 
uint8_t *cbp_table; 
uint8_t *pred_dir_table; 
ScratchpadContext sc;
int qscale; 
int chroma_qscale; 
unsigned int lambda; 
unsigned int lambda2; 
int *lambda_table;
int adaptive_quant; 
int dquant; 
int closed_gop; 
int pict_type; 
int vbv_delay;
int last_pict_type; 
int last_non_b_pict_type; 
int droppable;
int frame_rate_index;
AVRational mpeg2_frame_rate_ext;
int last_lambda_for[5]; 
int skipdct; 
int unrestricted_mv; 
int h263_long_vectors; 
BlockDSPContext bdsp;
FDCTDSPContext fdsp;
H264ChromaContext h264chroma;
HpelDSPContext hdsp;
IDCTDSPContext idsp;
MECmpContext mecc;
MpegVideoDSPContext mdsp;
MpegvideoEncDSPContext mpvencdsp;
PixblockDSPContext pdsp;
QpelDSPContext qdsp;
VideoDSPContext vdsp;
H263DSPContext h263dsp;
int f_code; 
int b_code; 
int16_t (*p_mv_table_base)[2];
int16_t (*b_forw_mv_table_base)[2];
int16_t (*b_back_mv_table_base)[2];
int16_t (*b_bidir_forw_mv_table_base)[2];
int16_t (*b_bidir_back_mv_table_base)[2];
int16_t (*b_direct_mv_table_base)[2];
int16_t (*p_field_mv_table_base[2][2])[2];
int16_t (*b_field_mv_table_base[2][2][2])[2];
int16_t (*p_mv_table)[2]; 
int16_t (*b_forw_mv_table)[2]; 
int16_t (*b_back_mv_table)[2]; 
int16_t (*b_bidir_forw_mv_table)[2]; 
int16_t (*b_bidir_back_mv_table)[2]; 
int16_t (*b_direct_mv_table)[2]; 
int16_t (*p_field_mv_table[2][2])[2]; 
int16_t (*b_field_mv_table[2][2][2])[2];
uint8_t (*p_field_select_table[2]);
uint8_t (*b_field_select_table[2][2]);
int motion_est; 
int me_penalty_compensation;
int me_pre; 
int mv_dir;
#define MV_DIR_FORWARD 1
#define MV_DIR_BACKWARD 2
#define MV_DIRECT 4 
int mv_type;
#define MV_TYPE_16X16 0 
#define MV_TYPE_8X8 1 
#define MV_TYPE_16X8 2 
#define MV_TYPE_FIELD 3 
#define MV_TYPE_DMV 4 
int mv[2][4][2];
int field_select[2][2];
int last_mv[2][2][2]; 
uint8_t *fcode_tab; 
int16_t direct_scale_mv[2][64]; 
MotionEstContext me;
int no_rounding; 
int mb_x, mb_y;
int mb_skip_run;
int mb_intra;
uint16_t *mb_type; 
int block_index[6]; 
int block_wrap[6];
uint8_t *dest[3];
int *mb_index2xy; 
uint16_t intra_matrix[64];
uint16_t chroma_intra_matrix[64];
uint16_t inter_matrix[64];
uint16_t chroma_inter_matrix[64];
int force_duplicated_matrix; 
int intra_quant_bias; 
int inter_quant_bias; 
int min_qcoeff; 
int max_qcoeff; 
int ac_esc_length; 
uint8_t *intra_ac_vlc_length;
uint8_t *intra_ac_vlc_last_length;
uint8_t *intra_chroma_ac_vlc_length;
uint8_t *intra_chroma_ac_vlc_last_length;
uint8_t *inter_ac_vlc_length;
uint8_t *inter_ac_vlc_last_length;
uint8_t *luma_dc_vlc_length;
#define UNI_AC_ENC_INDEX(run,level) ((run)*128 + (level))
int coded_score[12];
int (*q_intra_matrix)[64];
int (*q_chroma_intra_matrix)[64];
int (*q_inter_matrix)[64];
uint16_t (*q_intra_matrix16)[2][64];
uint16_t (*q_chroma_intra_matrix16)[2][64];
uint16_t (*q_inter_matrix16)[2][64];
int (*dct_error_sum)[64];
int dct_count[2];
uint16_t (*dct_offset)[64];
int64_t total_bits;
int frame_bits; 
int stuffing_bits; 
int next_lambda; 
RateControlContext rc_context; 
int mv_bits;
int header_bits;
int i_tex_bits;
int p_tex_bits;
int i_count;
int f_count;
int b_count;
int skip_count;
int misc_bits; 
int last_bits; 
int resync_mb_x; 
int resync_mb_y; 
GetBitContext last_resync_gb; 
int mb_num_left; 
int next_p_frame_damaged; 
ParseContext parse_context;
int gob_index;
int obmc; 
int mb_info; 
int prev_mb_info, last_mb_info;
uint8_t *mb_info_ptr;
int mb_info_size;
int ehc_mode;
int rc_strategy; 
int umvplus; 
int h263_aic_dir; 
int h263_slice_structured;
int alt_inter_vlc; 
int modified_quant;
int loop_filter;
int custom_pcf;
int studio_profile;
int dct_precision;
int time_increment_bits;
int last_time_base;
int time_base; 
int64_t time; 
int64_t last_non_b_time;
uint16_t pp_time; 
uint16_t pb_time; 
uint16_t pp_field_time;
uint16_t pb_field_time; 
int real_sprite_warping_points;
int sprite_offset[2][2]; 
int sprite_delta[2][2]; 
int mcsel;
int quant_precision;
int quarter_sample; 
int aspect_ratio_info; 
int sprite_warping_accuracy;
int data_partitioning; 
int partitioned_frame; 
int low_delay; 
int vo_type;
PutBitContext tex_pb; 
PutBitContext pb2; 
int mpeg_quant;
int padding_bug_score; 
int divx_packed;
uint8_t *bitstream_buffer; 
int bitstream_buffer_size;
unsigned int allocated_bitstream_buffer_size;
int rv10_version; 
int rv10_first_dc_coded[3];
struct MJpegContext *mjpeg_ctx;
int esc_pos;
int pred;
int huffman;
int mv_table_index;
int rl_table_index;
int rl_chroma_table_index;
int dc_table_index;
int use_skip_mb_code;
int slice_height; 
int first_slice_line; 
int flipflop_rounding;
int msmpeg4_version; 
int per_mb_rl_table;
int esc3_level_length;
int esc3_run_length;
int (*ac_stats)[2][MAX_LEVEL+1][MAX_RUN+1][2];
int inter_intra_pred;
int mspel;
GetBitContext gb;
int gop_picture_number; 
int last_mv_dir; 
uint8_t *vbv_delay_ptr; 
int progressive_sequence;
int mpeg_f_code[2][2];
int a53_cc;
int picture_structure;
int64_t timecode_frame_start; 
int intra_dc_precision;
int frame_pred_frame_dct;
int top_field_first;
int concealment_motion_vectors;
int q_scale_type;
int brd_scale;
int intra_vlc_format;
int alternate_scan;
int seq_disp_ext;
int video_format;
#define VIDEO_FORMAT_COMPONENT 0
#define VIDEO_FORMAT_PAL 1
#define VIDEO_FORMAT_NTSC 2
#define VIDEO_FORMAT_SECAM 3
#define VIDEO_FORMAT_MAC 4
#define VIDEO_FORMAT_UNSPECIFIED 5
int repeat_first_field;
int chroma_420_type;
int chroma_format;
#define CHROMA_420 1
#define CHROMA_422 2
#define CHROMA_444 3
int chroma_x_shift;
int chroma_y_shift;
int progressive_frame;
int full_pel[2];
int interlaced_dct;
int first_field; 
int drop_frame_timecode; 
int scan_offset; 
int rtp_mode;
int rtp_payload_size;
char *tc_opt_str; 
AVTimecode tc; 
uint8_t *ptr_lastgob;
int swap_uv; 
int pack_pblocks; 
int16_t (*pblocks[12])[64];
int16_t (*block)[64]; 
int16_t (*blocks)[12][64]; 
int (*decode_mb)(struct MpegEncContext *s, int16_t block[12][64]); 
int32_t (*block32)[12][64];
int dpcm_direction; 
int16_t (*dpcm_macroblock)[3][256];
#define SLICE_OK 0
#define SLICE_ERROR -1
#define SLICE_END -2 
#define SLICE_NOEND -3 
void (*dct_unquantize_mpeg1_intra)(struct MpegEncContext *s,
int16_t *block, int n, int qscale);
void (*dct_unquantize_mpeg1_inter)(struct MpegEncContext *s,
int16_t *block, int n, int qscale);
void (*dct_unquantize_mpeg2_intra)(struct MpegEncContext *s,
int16_t *block, int n, int qscale);
void (*dct_unquantize_mpeg2_inter)(struct MpegEncContext *s,
int16_t *block, int n, int qscale);
void (*dct_unquantize_h263_intra)(struct MpegEncContext *s,
int16_t *block, int n, int qscale);
void (*dct_unquantize_h263_inter)(struct MpegEncContext *s,
int16_t *block, int n, int qscale);
void (*dct_unquantize_intra)(struct MpegEncContext *s, 
int16_t *block, int n, int qscale);
void (*dct_unquantize_inter)(struct MpegEncContext *s, 
int16_t *block, int n, int qscale);
int (*dct_quantize)(struct MpegEncContext *s, int16_t *block, int n, int qscale, int *overflow);
int (*fast_dct_quantize)(struct MpegEncContext *s, int16_t *block, int n, int qscale, int *overflow);
void (*denoise_dct)(struct MpegEncContext *s, int16_t *block);
int mpv_flags; 
int quantizer_noise_shaping;
float rc_qsquish;
float rc_qmod_amp;
int rc_qmod_freq;
float rc_initial_cplx;
float rc_buffer_aggressivity;
float border_masking;
int lmin, lmax;
int vbv_ignore_qmax;
char *rc_eq;
float *cplx_tab, *bits_tab;
int context_reinit;
ERContext er;
int error_rate;
AVFrame *tmp_frames[MAX_B_FRAMES + 2];
int b_frame_strategy;
int b_sensitivity;
int frame_skip_threshold;
int frame_skip_factor;
int frame_skip_exp;
int frame_skip_cmp;
int scenechange_threshold;
int noise_reduction;
int intra_penalty;
} MpegEncContext;
#define FF_MPV_FLAG_SKIP_RD 0x0001
#define FF_MPV_FLAG_STRICT_GOP 0x0002
#define FF_MPV_FLAG_QP_RD 0x0004
#define FF_MPV_FLAG_CBP_RD 0x0008
#define FF_MPV_FLAG_NAQ 0x0010
#define FF_MPV_FLAG_MV0 0x0020
#define FF_MPV_OPT_CMP_FUNC { "sad", "Sum of absolute differences, fast", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_SAD }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "sse", "Sum of squared errors", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_SSE }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "satd", "Sum of absolute Hadamard transformed differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_SATD }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "dct", "Sum of absolute DCT transformed differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_DCT }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "psnr", "Sum of squared quantization errors, low quality", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_PSNR }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "bit", "Number of bits needed for the block", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_BIT }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "rd", "Rate distortion optimal, slow", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_RD }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "zero", "Zero", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_ZERO }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "vsad", "Sum of absolute vertical differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_VSAD }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "vsse", "Sum of squared vertical differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_VSSE }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "nsse", "Noise preserving sum of squared differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_NSSE }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "dct264", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_DCT264 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "dctmax", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_DCTMAX }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "chroma", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_CHROMA }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, { "msad", "Sum of absolute differences, median predicted", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_MEDIAN_SAD }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }
#if !defined(FF_MPV_OFFSET)
#define FF_MPV_OFFSET(x) offsetof(MpegEncContext, x)
#endif
#define FF_MPV_OPT_FLAGS (AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM)
#define FF_MPV_COMMON_OPTS FF_MPV_OPT_CMP_FUNC, { "mpv_flags", "Flags common for all mpegvideo-based encoders.", FF_MPV_OFFSET(mpv_flags), AV_OPT_TYPE_FLAGS, { .i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "mpv_flags" },{ "skip_rd", "RD optimal MB level residual skipping", 0, AV_OPT_TYPE_CONST, { .i64 = FF_MPV_FLAG_SKIP_RD }, 0, 0, FF_MPV_OPT_FLAGS, "mpv_flags" },{ "strict_gop", "Strictly enforce gop size", 0, AV_OPT_TYPE_CONST, { .i64 = FF_MPV_FLAG_STRICT_GOP }, 0, 0, FF_MPV_OPT_FLAGS, "mpv_flags" },{ "qp_rd", "Use rate distortion optimization for qp selection", 0, AV_OPT_TYPE_CONST, { .i64 = FF_MPV_FLAG_QP_RD }, 0, 0, FF_MPV_OPT_FLAGS, "mpv_flags" },{ "cbp_rd", "use rate distortion optimization for CBP", 0, AV_OPT_TYPE_CONST, { .i64 = FF_MPV_FLAG_CBP_RD }, 0, 0, FF_MPV_OPT_FLAGS, "mpv_flags" },{ "naq", "normalize adaptive quantization", 0, AV_OPT_TYPE_CONST, { .i64 = FF_MPV_FLAG_NAQ }, 0, 0, FF_MPV_OPT_FLAGS, "mpv_flags" },{ "mv0", "always try a mb with mv=<0,0>", 0, AV_OPT_TYPE_CONST, { .i64 = FF_MPV_FLAG_MV0 }, 0, 0, FF_MPV_OPT_FLAGS, "mpv_flags" },{ "luma_elim_threshold", "single coefficient elimination threshold for luminance (negative values also consider dc coefficient)",FF_MPV_OFFSET(luma_elim_threshold), AV_OPT_TYPE_INT, { .i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS },{ "chroma_elim_threshold", "single coefficient elimination threshold for chrominance (negative values also consider dc coefficient)",FF_MPV_OFFSET(chroma_elim_threshold), AV_OPT_TYPE_INT, { .i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS },{ "quantizer_noise_shaping", NULL, FF_MPV_OFFSET(quantizer_noise_shaping), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, INT_MAX, FF_MPV_OPT_FLAGS },{ "error_rate", "Simulate errors in the bitstream to test error concealment.", FF_MPV_OFFSET(error_rate), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, INT_MAX, FF_MPV_OPT_FLAGS },{"qsquish", "how to keep quantizer between qmin and qmax (0 = clip, 1 = use differentiable function)", FF_MPV_OFFSET(rc_qsquish), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, 0, 99, FF_MPV_OPT_FLAGS}, {"rc_qmod_amp", "experimental quantizer modulation", FF_MPV_OFFSET(rc_qmod_amp), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, -FLT_MAX, FLT_MAX, FF_MPV_OPT_FLAGS}, {"rc_qmod_freq", "experimental quantizer modulation", FF_MPV_OFFSET(rc_qmod_freq), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS}, {"rc_eq", "Set rate control equation. When computing the expression, besides the standard functions " "defined in the section 'Expression Evaluation', the following functions are available: " "bits2qp(bits), qp2bits(qp). Also the following constants are available: iTex pTex tex mv " "fCode iCount mcVar var isI isP isB avgQP qComp avgIITex avgPITex avgPPTex avgBPTex avgTex.", FF_MPV_OFFSET(rc_eq), AV_OPT_TYPE_STRING, .flags = FF_MPV_OPT_FLAGS }, {"rc_init_cplx", "initial complexity for 1-pass encoding", FF_MPV_OFFSET(rc_initial_cplx), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, -FLT_MAX, FLT_MAX, FF_MPV_OPT_FLAGS}, {"rc_buf_aggressivity", "currently useless", FF_MPV_OFFSET(rc_buffer_aggressivity), AV_OPT_TYPE_FLOAT, {.dbl = 1.0 }, -FLT_MAX, FLT_MAX, FF_MPV_OPT_FLAGS}, {"border_mask", "increase the quantizer for macroblocks close to borders", FF_MPV_OFFSET(border_masking), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, -FLT_MAX, FLT_MAX, FF_MPV_OPT_FLAGS}, {"lmin", "minimum Lagrange factor (VBR)", FF_MPV_OFFSET(lmin), AV_OPT_TYPE_INT, {.i64 = 2*FF_QP2LAMBDA }, 0, INT_MAX, FF_MPV_OPT_FLAGS }, {"lmax", "maximum Lagrange factor (VBR)", FF_MPV_OFFSET(lmax), AV_OPT_TYPE_INT, {.i64 = 31*FF_QP2LAMBDA }, 0, INT_MAX, FF_MPV_OPT_FLAGS }, {"ibias", "intra quant bias", FF_MPV_OFFSET(intra_quant_bias), AV_OPT_TYPE_INT, {.i64 = FF_DEFAULT_QUANT_BIAS }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"pbias", "inter quant bias", FF_MPV_OFFSET(inter_quant_bias), AV_OPT_TYPE_INT, {.i64 = FF_DEFAULT_QUANT_BIAS }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"rc_strategy", "ratecontrol method", FF_MPV_OFFSET(rc_strategy), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, 1, FF_MPV_OPT_FLAGS | AV_OPT_FLAG_DEPRECATED, "rc_strategy" }, { "ffmpeg", "deprecated, does nothing", 0, AV_OPT_TYPE_CONST, { .i64 = 0 }, 0, 0, FF_MPV_OPT_FLAGS | AV_OPT_FLAG_DEPRECATED, "rc_strategy" }, { "xvid", "deprecated, does nothing", 0, AV_OPT_TYPE_CONST, { .i64 = 0 }, 0, 0, FF_MPV_OPT_FLAGS | AV_OPT_FLAG_DEPRECATED, "rc_strategy" }, {"motion_est", "motion estimation algorithm", FF_MPV_OFFSET(motion_est), AV_OPT_TYPE_INT, {.i64 = FF_ME_EPZS }, FF_ME_ZERO, FF_ME_XONE, FF_MPV_OPT_FLAGS, "motion_est" }, { "zero", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = FF_ME_ZERO }, 0, 0, FF_MPV_OPT_FLAGS, "motion_est" }, { "epzs", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = FF_ME_EPZS }, 0, 0, FF_MPV_OPT_FLAGS, "motion_est" }, { "xone", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = FF_ME_XONE }, 0, 0, FF_MPV_OPT_FLAGS, "motion_est" }, { "force_duplicated_matrix", "Always write luma and chroma matrix for mjpeg, useful for rtp streaming.", FF_MPV_OFFSET(force_duplicated_matrix), AV_OPT_TYPE_BOOL, {.i64 = 0 }, 0, 1, FF_MPV_OPT_FLAGS }, {"b_strategy", "Strategy to choose between I/P/B-frames", FF_MPV_OFFSET(b_frame_strategy), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, 2, FF_MPV_OPT_FLAGS }, {"b_sensitivity", "Adjust sensitivity of b_frame_strategy 1", FF_MPV_OFFSET(b_sensitivity), AV_OPT_TYPE_INT, {.i64 = 40 }, 1, INT_MAX, FF_MPV_OPT_FLAGS }, {"brd_scale", "Downscale frames for dynamic B-frame decision", FF_MPV_OFFSET(brd_scale), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, 3, FF_MPV_OPT_FLAGS }, {"skip_threshold", "Frame skip threshold", FF_MPV_OFFSET(frame_skip_threshold), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"skip_factor", "Frame skip factor", FF_MPV_OFFSET(frame_skip_factor), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"skip_exp", "Frame skip exponent", FF_MPV_OFFSET(frame_skip_exp), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"skip_cmp", "Frame skip compare function", FF_MPV_OFFSET(frame_skip_cmp), AV_OPT_TYPE_INT, {.i64 = FF_CMP_DCTMAX }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS, "cmp_func" }, {"sc_threshold", "Scene change threshold", FF_MPV_OFFSET(scenechange_threshold), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"noise_reduction", "Noise reduction", FF_MPV_OFFSET(noise_reduction), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"mpeg_quant", "Use MPEG quantizers instead of H.263", FF_MPV_OFFSET(mpeg_quant), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, 1, FF_MPV_OPT_FLAGS }, {"ps", "RTP payload size in bytes", FF_MPV_OFFSET(rtp_payload_size), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"mepc", "Motion estimation bitrate penalty compensation (1.0 = 256)", FF_MPV_OFFSET(me_penalty_compensation), AV_OPT_TYPE_INT, {.i64 = 256 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"mepre", "pre motion estimation", FF_MPV_OFFSET(me_pre), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, FF_MPV_OPT_FLAGS }, {"intra_penalty", "Penalty for intra blocks in block decision", FF_MPV_OFFSET(intra_penalty), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, INT_MAX/2, FF_MPV_OPT_FLAGS }, {"a53cc", "Use A53 Closed Captions (if available)", FF_MPV_OFFSET(a53_cc), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, FF_MPV_OPT_FLAGS }, 
extern const AVOption ff_mpv_generic_options[];
void ff_mpv_common_defaults(MpegEncContext *s);
void ff_dct_encode_init_x86(MpegEncContext *s);
int ff_mpv_common_init(MpegEncContext *s);
void ff_mpv_common_init_arm(MpegEncContext *s);
void ff_mpv_common_init_axp(MpegEncContext *s);
void ff_mpv_common_init_neon(MpegEncContext *s);
void ff_mpv_common_init_ppc(MpegEncContext *s);
void ff_mpv_common_init_x86(MpegEncContext *s);
void ff_mpv_common_init_mips(MpegEncContext *s);
int ff_mpv_common_frame_size_change(MpegEncContext *s);
void ff_mpv_common_end(MpegEncContext *s);
void ff_mpv_decode_defaults(MpegEncContext *s);
void ff_mpv_decode_init(MpegEncContext *s, AVCodecContext *avctx);
void ff_mpv_reconstruct_mb(MpegEncContext *s, int16_t block[12][64]);
void ff_mpv_report_decode_progress(MpegEncContext *s);
int ff_mpv_frame_start(MpegEncContext *s, AVCodecContext *avctx);
void ff_mpv_frame_end(MpegEncContext *s);
int ff_mpv_encode_init(AVCodecContext *avctx);
void ff_mpv_encode_init_x86(MpegEncContext *s);
int ff_mpv_encode_end(AVCodecContext *avctx);
int ff_mpv_encode_picture(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *frame, int *got_packet);
int ff_mpv_reallocate_putbitbuffer(MpegEncContext *s, size_t threshold, size_t size_increase);
void ff_clean_intra_table_entries(MpegEncContext *s);
void ff_mpeg_draw_horiz_band(MpegEncContext *s, int y, int h);
void ff_mpeg_flush(AVCodecContext *avctx);
void ff_print_debug_info(MpegEncContext *s, Picture *p, AVFrame *pict);
int ff_mpv_export_qp_table(MpegEncContext *s, AVFrame *f, Picture *p, int qp_type);
void ff_write_quant_matrix(PutBitContext *pb, uint16_t *matrix);
int ff_update_duplicate_context(MpegEncContext *dst, MpegEncContext *src);
int ff_mpeg_update_thread_context(AVCodecContext *dst, const AVCodecContext *src);
void ff_set_qscale(MpegEncContext * s, int qscale);
void ff_mpv_idct_init(MpegEncContext *s);
int ff_dct_encode_init(MpegEncContext *s);
void ff_convert_matrix(MpegEncContext *s, int (*qmat)[64], uint16_t (*qmat16)[2][64],
const uint16_t *quant_matrix, int bias, int qmin, int qmax, int intra);
int ff_dct_quantize_c(MpegEncContext *s, int16_t *block, int n, int qscale, int *overflow);
void ff_block_permute(int16_t *block, uint8_t *permutation,
const uint8_t *scantable, int last);
void ff_init_block_index(MpegEncContext *s);
void ff_mpv_motion(MpegEncContext *s,
uint8_t *dest_y, uint8_t *dest_cb,
uint8_t *dest_cr, int dir,
uint8_t **ref_picture,
op_pixels_func (*pix_op)[4],
qpel_mc_func (*qpix_op)[16]);
static inline void ff_update_block_index(MpegEncContext *s){
const int bytes_per_pixel = 1 + (s->avctx->bits_per_raw_sample > 8);
const int block_size= (8*bytes_per_pixel) >> s->avctx->lowres;
s->block_index[0]+=2;
s->block_index[1]+=2;
s->block_index[2]+=2;
s->block_index[3]+=2;
s->block_index[4]++;
s->block_index[5]++;
s->dest[0]+= 2*block_size;
s->dest[1]+= (2 >> s->chroma_x_shift) * block_size;
s->dest[2]+= (2 >> s->chroma_x_shift) * block_size;
}
static inline int get_bits_diff(MpegEncContext *s){
const int bits= put_bits_count(&s->pb);
const int last= s->last_bits;
s->last_bits = bits;
return bits - last;
}
static inline int mpeg_get_qscale(MpegEncContext *s)
{
int qscale = get_bits(&s->gb, 5);
if (s->q_scale_type)
return ff_mpeg2_non_linear_qscale[qscale];
else
return qscale << 1;
}
