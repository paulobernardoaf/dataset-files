



















#if !defined(AVCODEC_HEVC_PS_H)
#define AVCODEC_HEVC_PS_H

#include <stdint.h>

#include "libavutil/buffer.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"

#include "avcodec.h"
#include "get_bits.h"
#include "hevc.h"

typedef struct ShortTermRPS {
unsigned int num_negative_pics;
int num_delta_pocs;
int rps_idx_num_delta_pocs;
int32_t delta_poc[32];
uint8_t used[32];
} ShortTermRPS;

typedef struct LongTermRPS {
int poc[32];
uint8_t used[32];
uint8_t nb_refs;
} LongTermRPS;

typedef struct SliceHeader {
unsigned int pps_id;


unsigned int slice_segment_addr;

unsigned int slice_addr;

enum HEVCSliceType slice_type;

int pic_order_cnt_lsb;

uint8_t first_slice_in_pic_flag;
uint8_t dependent_slice_segment_flag;
uint8_t pic_output_flag;
uint8_t colour_plane_id;


int short_term_ref_pic_set_sps_flag;
int short_term_ref_pic_set_size;
ShortTermRPS slice_rps;
const ShortTermRPS *short_term_rps;
int long_term_ref_pic_set_size;
LongTermRPS long_term_rps;
unsigned int list_entry_lx[2][32];

uint8_t rpl_modification_flag[2];
uint8_t no_output_of_prior_pics_flag;
uint8_t slice_temporal_mvp_enabled_flag;

unsigned int nb_refs[2];

uint8_t slice_sample_adaptive_offset_flag[3];
uint8_t mvd_l1_zero_flag;

uint8_t cabac_init_flag;
uint8_t disable_deblocking_filter_flag; 
uint8_t slice_loop_filter_across_slices_enabled_flag;
uint8_t collocated_list;

unsigned int collocated_ref_idx;

int slice_qp_delta;
int slice_cb_qp_offset;
int slice_cr_qp_offset;

uint8_t cu_chroma_qp_offset_enabled_flag;

int beta_offset; 
int tc_offset; 

unsigned int max_num_merge_cand; 

unsigned *entry_point_offset;
int * offset;
int * size;
int num_entry_point_offsets;

int8_t slice_qp;

uint8_t luma_log2_weight_denom;
int16_t chroma_log2_weight_denom;

int16_t luma_weight_l0[16];
int16_t chroma_weight_l0[16][2];
int16_t chroma_weight_l1[16][2];
int16_t luma_weight_l1[16];

int16_t luma_offset_l0[16];
int16_t chroma_offset_l0[16][2];

int16_t luma_offset_l1[16];
int16_t chroma_offset_l1[16][2];

int slice_ctb_addr_rs;
} SliceHeader;

typedef struct HEVCWindow {
unsigned int left_offset;
unsigned int right_offset;
unsigned int top_offset;
unsigned int bottom_offset;
} HEVCWindow;

typedef struct VUI {
AVRational sar;

int overscan_info_present_flag;
int overscan_appropriate_flag;

int video_signal_type_present_flag;
int video_format;
int video_full_range_flag;
int colour_description_present_flag;
uint8_t colour_primaries;
uint8_t transfer_characteristic;
uint8_t matrix_coeffs;

int chroma_loc_info_present_flag;
int chroma_sample_loc_type_top_field;
int chroma_sample_loc_type_bottom_field;
int neutra_chroma_indication_flag;

int field_seq_flag;
int frame_field_info_present_flag;

int default_display_window_flag;
HEVCWindow def_disp_win;

int vui_timing_info_present_flag;
uint32_t vui_num_units_in_tick;
uint32_t vui_time_scale;
int vui_poc_proportional_to_timing_flag;
int vui_num_ticks_poc_diff_one_minus1;
int vui_hrd_parameters_present_flag;

int bitstream_restriction_flag;
int tiles_fixed_structure_flag;
int motion_vectors_over_pic_boundaries_flag;
int restricted_ref_pic_lists_flag;
int min_spatial_segmentation_idc;
int max_bytes_per_pic_denom;
int max_bits_per_min_cu_denom;
int log2_max_mv_length_horizontal;
int log2_max_mv_length_vertical;
} VUI;

typedef struct PTLCommon {
uint8_t profile_space;
uint8_t tier_flag;
uint8_t profile_idc;
uint8_t profile_compatibility_flag[32];
uint8_t progressive_source_flag;
uint8_t interlaced_source_flag;
uint8_t non_packed_constraint_flag;
uint8_t frame_only_constraint_flag;
uint8_t max_12bit_constraint_flag;
uint8_t max_10bit_constraint_flag;
uint8_t max_8bit_constraint_flag;
uint8_t max_422chroma_constraint_flag;
uint8_t max_420chroma_constraint_flag;
uint8_t max_monochrome_constraint_flag;
uint8_t intra_constraint_flag;
uint8_t one_picture_only_constraint_flag;
uint8_t lower_bit_rate_constraint_flag;
uint8_t max_14bit_constraint_flag;
uint8_t inbld_flag;
uint8_t level_idc;
} PTLCommon;

typedef struct PTL {
PTLCommon general_ptl;
PTLCommon sub_layer_ptl[HEVC_MAX_SUB_LAYERS];

uint8_t sub_layer_profile_present_flag[HEVC_MAX_SUB_LAYERS];
uint8_t sub_layer_level_present_flag[HEVC_MAX_SUB_LAYERS];
} PTL;

typedef struct HEVCVPS {
uint8_t vps_temporal_id_nesting_flag;
int vps_max_layers;
int vps_max_sub_layers; 

PTL ptl;
int vps_sub_layer_ordering_info_present_flag;
unsigned int vps_max_dec_pic_buffering[HEVC_MAX_SUB_LAYERS];
unsigned int vps_num_reorder_pics[HEVC_MAX_SUB_LAYERS];
unsigned int vps_max_latency_increase[HEVC_MAX_SUB_LAYERS];
int vps_max_layer_id;
int vps_num_layer_sets; 
uint8_t vps_timing_info_present_flag;
uint32_t vps_num_units_in_tick;
uint32_t vps_time_scale;
uint8_t vps_poc_proportional_to_timing_flag;
int vps_num_ticks_poc_diff_one; 
int vps_num_hrd_parameters;

uint8_t data[4096];
int data_size;
} HEVCVPS;

typedef struct ScalingList {


uint8_t sl[4][6][64];
uint8_t sl_dc[2][6];
} ScalingList;

typedef struct HEVCSPS {
unsigned vps_id;
int chroma_format_idc;
uint8_t separate_colour_plane_flag;

HEVCWindow output_window;

HEVCWindow pic_conf_win;

int bit_depth;
int bit_depth_chroma;
int pixel_shift;
enum AVPixelFormat pix_fmt;

unsigned int log2_max_poc_lsb;
int pcm_enabled_flag;

int max_sub_layers;
struct {
int max_dec_pic_buffering;
int num_reorder_pics;
int max_latency_increase;
} temporal_layer[HEVC_MAX_SUB_LAYERS];
uint8_t temporal_id_nesting_flag;

VUI vui;
PTL ptl;

uint8_t scaling_list_enable_flag;
ScalingList scaling_list;

unsigned int nb_st_rps;
ShortTermRPS st_rps[HEVC_MAX_SHORT_TERM_REF_PIC_SETS];

uint8_t amp_enabled_flag;
uint8_t sao_enabled;

uint8_t long_term_ref_pics_present_flag;
uint16_t lt_ref_pic_poc_lsb_sps[HEVC_MAX_LONG_TERM_REF_PICS];
uint8_t used_by_curr_pic_lt_sps_flag[HEVC_MAX_LONG_TERM_REF_PICS];
uint8_t num_long_term_ref_pics_sps;

struct {
uint8_t bit_depth;
uint8_t bit_depth_chroma;
unsigned int log2_min_pcm_cb_size;
unsigned int log2_max_pcm_cb_size;
uint8_t loop_filter_disable_flag;
} pcm;
uint8_t sps_temporal_mvp_enabled_flag;
uint8_t sps_strong_intra_smoothing_enable_flag;

unsigned int log2_min_cb_size;
unsigned int log2_diff_max_min_coding_block_size;
unsigned int log2_min_tb_size;
unsigned int log2_max_trafo_size;
unsigned int log2_ctb_size;
unsigned int log2_min_pu_size;

int max_transform_hierarchy_depth_inter;
int max_transform_hierarchy_depth_intra;

int sps_range_extension_flag;
int transform_skip_rotation_enabled_flag;
int transform_skip_context_enabled_flag;
int implicit_rdpcm_enabled_flag;
int explicit_rdpcm_enabled_flag;
int extended_precision_processing_flag;
int intra_smoothing_disabled_flag;
int high_precision_offsets_enabled_flag;
int persistent_rice_adaptation_enabled_flag;
int cabac_bypass_alignment_enabled_flag;


int width;
int height;
int ctb_width;
int ctb_height;
int ctb_size;
int min_cb_width;
int min_cb_height;
int min_tb_width;
int min_tb_height;
int min_pu_width;
int min_pu_height;
int tb_mask;

int hshift[3];
int vshift[3];

int qp_bd_offset;

uint8_t data[4096];
int data_size;
} HEVCSPS;

typedef struct HEVCPPS {
unsigned int sps_id; 

uint8_t sign_data_hiding_flag;

uint8_t cabac_init_present_flag;

int num_ref_idx_l0_default_active; 
int num_ref_idx_l1_default_active; 
int pic_init_qp_minus26;

uint8_t constrained_intra_pred_flag;
uint8_t transform_skip_enabled_flag;

uint8_t cu_qp_delta_enabled_flag;
int diff_cu_qp_delta_depth;

int cb_qp_offset;
int cr_qp_offset;
uint8_t pic_slice_level_chroma_qp_offsets_present_flag;
uint8_t weighted_pred_flag;
uint8_t weighted_bipred_flag;
uint8_t output_flag_present_flag;
uint8_t transquant_bypass_enable_flag;

uint8_t dependent_slice_segments_enabled_flag;
uint8_t tiles_enabled_flag;
uint8_t entropy_coding_sync_enabled_flag;

uint16_t num_tile_columns; 
uint16_t num_tile_rows; 
uint8_t uniform_spacing_flag;
uint8_t loop_filter_across_tiles_enabled_flag;

uint8_t seq_loop_filter_across_slices_enabled_flag;

uint8_t deblocking_filter_control_present_flag;
uint8_t deblocking_filter_override_enabled_flag;
uint8_t disable_dbf;
int beta_offset; 
int tc_offset; 

uint8_t scaling_list_data_present_flag;
ScalingList scaling_list;

uint8_t lists_modification_present_flag;
int log2_parallel_merge_level; 
int num_extra_slice_header_bits;
uint8_t slice_header_extension_present_flag;
uint8_t log2_max_transform_skip_block_size;
uint8_t pps_range_extensions_flag;
uint8_t cross_component_prediction_enabled_flag;
uint8_t chroma_qp_offset_list_enabled_flag;
uint8_t diff_cu_chroma_qp_offset_depth;
uint8_t chroma_qp_offset_list_len_minus1;
int8_t cb_qp_offset_list[6];
int8_t cr_qp_offset_list[6];
uint8_t log2_sao_offset_scale_luma;
uint8_t log2_sao_offset_scale_chroma;


unsigned int *column_width; 
unsigned int *row_height; 
unsigned int *col_bd; 
unsigned int *row_bd; 
int *col_idxX;

int *ctb_addr_rs_to_ts; 
int *ctb_addr_ts_to_rs; 
int *tile_id; 
int *tile_pos_rs; 
int *min_tb_addr_zs; 
int *min_tb_addr_zs_tab;

uint8_t data[4096];
int data_size;
} HEVCPPS;

typedef struct HEVCParamSets {
AVBufferRef *vps_list[HEVC_MAX_VPS_COUNT];
AVBufferRef *sps_list[HEVC_MAX_SPS_COUNT];
AVBufferRef *pps_list[HEVC_MAX_PPS_COUNT];


const HEVCVPS *vps;
const HEVCSPS *sps;
const HEVCPPS *pps;
} HEVCParamSets;










int ff_hevc_parse_sps(HEVCSPS *sps, GetBitContext *gb, unsigned int *sps_id,
int apply_defdispwin, AVBufferRef **vps_list, AVCodecContext *avctx);

int ff_hevc_decode_nal_vps(GetBitContext *gb, AVCodecContext *avctx,
HEVCParamSets *ps);
int ff_hevc_decode_nal_sps(GetBitContext *gb, AVCodecContext *avctx,
HEVCParamSets *ps, int apply_defdispwin);
int ff_hevc_decode_nal_pps(GetBitContext *gb, AVCodecContext *avctx,
HEVCParamSets *ps);

void ff_hevc_ps_uninit(HEVCParamSets *ps);

int ff_hevc_decode_short_term_rps(GetBitContext *gb, AVCodecContext *avctx,
ShortTermRPS *rps, const HEVCSPS *sps, int is_slice_header);

int ff_hevc_encode_nal_vps(HEVCVPS *vps, unsigned int id,
uint8_t *buf, int buf_size);




int ff_hevc_compute_poc(const HEVCSPS *sps, int pocTid0, int poc_lsb, int nal_unit_type);

#endif 
