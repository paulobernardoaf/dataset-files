






















#if !defined(AVCODEC_H264_PARSE_H)
#define AVCODEC_H264_PARSE_H

#include "get_bits.h"
#include "h264_ps.h"

typedef struct H264PredWeightTable {
int use_weight;
int use_weight_chroma;
int luma_log2_weight_denom;
int chroma_log2_weight_denom;
int luma_weight_flag[2]; 
int chroma_weight_flag[2]; 

int luma_weight[48][2][2];
int chroma_weight[48][2][2][2];
int implicit_weight[48][48][2];
} H264PredWeightTable;

typedef struct H264POCContext {
int poc_lsb;
int poc_msb;
int delta_poc_bottom;
int delta_poc[2];
int frame_num;
int prev_poc_msb; 
int prev_poc_lsb; 
int frame_num_offset; 
int prev_frame_num_offset; 
int prev_frame_num; 
} H264POCContext;

int ff_h264_pred_weight_table(GetBitContext *gb, const SPS *sps,
const int *ref_count, int slice_type_nos,
H264PredWeightTable *pwt,
int picture_structure, void *logctx);





int ff_h264_check_intra4x4_pred_mode(int8_t *pred_mode_cache, void *logctx,
int top_samples_available, int left_samples_available);





int ff_h264_check_intra_pred_mode(void *logctx, int top_samples_available,
int left_samples_available,
int mode, int is_chroma);

int ff_h264_parse_ref_count(int *plist_count, int ref_count[2],
GetBitContext *gb, const PPS *pps,
int slice_type_nos, int picture_structure, void *logctx);

int ff_h264_init_poc(int pic_field_poc[2], int *pic_poc,
const SPS *sps, H264POCContext *poc,
int picture_structure, int nal_ref_idc);

int ff_h264_decode_extradata(const uint8_t *data, int size, H264ParamSets *ps,
int *is_avc, int *nal_length_size,
int err_recognition, void *logctx);




int ff_h264_get_profile(const SPS *sps);

#endif 
