

















#if !defined(AVCODEC_H264_SEI_H)
#define AVCODEC_H264_SEI_H

#include "get_bits.h"




typedef enum {
H264_SEI_TYPE_BUFFERING_PERIOD = 0, 
H264_SEI_TYPE_PIC_TIMING = 1, 
H264_SEI_TYPE_PAN_SCAN_RECT = 2, 
H264_SEI_TYPE_FILLER_PAYLOAD = 3, 
H264_SEI_TYPE_USER_DATA_REGISTERED = 4, 
H264_SEI_TYPE_USER_DATA_UNREGISTERED = 5, 
H264_SEI_TYPE_RECOVERY_POINT = 6, 
H264_SEI_TYPE_FRAME_PACKING = 45, 
H264_SEI_TYPE_DISPLAY_ORIENTATION = 47, 
H264_SEI_TYPE_GREEN_METADATA = 56, 
H264_SEI_TYPE_MASTERING_DISPLAY_COLOUR_VOLUME = 137, 
H264_SEI_TYPE_ALTERNATIVE_TRANSFER = 147, 
} H264_SEI_Type;




typedef enum {
H264_SEI_PIC_STRUCT_FRAME = 0, 
H264_SEI_PIC_STRUCT_TOP_FIELD = 1, 
H264_SEI_PIC_STRUCT_BOTTOM_FIELD = 2, 
H264_SEI_PIC_STRUCT_TOP_BOTTOM = 3, 
H264_SEI_PIC_STRUCT_BOTTOM_TOP = 4, 
H264_SEI_PIC_STRUCT_TOP_BOTTOM_TOP = 5, 
H264_SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM = 6, 
H264_SEI_PIC_STRUCT_FRAME_DOUBLING = 7, 
H264_SEI_PIC_STRUCT_FRAME_TRIPLING = 8 
} H264_SEI_PicStructType;




typedef enum {
H264_SEI_FPA_TYPE_CHECKERBOARD = 0,
H264_SEI_FPA_TYPE_INTERLEAVE_COLUMN = 1,
H264_SEI_FPA_TYPE_INTERLEAVE_ROW = 2,
H264_SEI_FPA_TYPE_SIDE_BY_SIDE = 3,
H264_SEI_FPA_TYPE_TOP_BOTTOM = 4,
H264_SEI_FPA_TYPE_INTERLEAVE_TEMPORAL = 5,
H264_SEI_FPA_TYPE_2D = 6,
} H264_SEI_FpaType;

typedef struct H264SEITimeCode {


int full;
int frame;
int seconds;
int minutes;
int hours;
int dropframe;
} H264SEITimeCode;

typedef struct H264SEIPictureTiming {
int present;
H264_SEI_PicStructType pic_struct;






int ct_type;




int dpb_output_delay;




int cpb_removal_delay;




H264SEITimeCode timecode[3];




int timecode_cnt;
} H264SEIPictureTiming;

typedef struct H264SEIAFD {
int present;
uint8_t active_format_description;
} H264SEIAFD;

typedef struct H264SEIA53Caption {
AVBufferRef *buf_ref;
} H264SEIA53Caption;

typedef struct H264SEIUnregistered {
int x264_build;
} H264SEIUnregistered;

typedef struct H264SEIRecoveryPoint {







int recovery_frame_cnt;
} H264SEIRecoveryPoint;

typedef struct H264SEIBufferingPeriod {
int present; 
int initial_cpb_removal_delay[32]; 
} H264SEIBufferingPeriod;

typedef struct H264SEIFramePacking {
int present;
int arrangement_id;
int arrangement_cancel_flag; 
H264_SEI_FpaType arrangement_type;
int arrangement_repetition_period;
int content_interpretation_type;
int quincunx_sampling_flag;
int current_frame_is_frame0_flag;
} H264SEIFramePacking;

typedef struct H264SEIDisplayOrientation {
int present;
int anticlockwise_rotation;
int hflip, vflip;
} H264SEIDisplayOrientation;

typedef struct H264SEIGreenMetaData {
uint8_t green_metadata_type;
uint8_t period_type;
uint16_t num_seconds;
uint16_t num_pictures;
uint8_t percent_non_zero_macroblocks;
uint8_t percent_intra_coded_macroblocks;
uint8_t percent_six_tap_filtering;
uint8_t percent_alpha_point_deblocking_instance;
uint8_t xsd_metric_type;
uint16_t xsd_metric_value;
} H264SEIGreenMetaData;

typedef struct H264SEIAlternativeTransfer {
int present;
int preferred_transfer_characteristics;
} H264SEIAlternativeTransfer;

typedef struct H264SEIContext {
H264SEIPictureTiming picture_timing;
H264SEIAFD afd;
H264SEIA53Caption a53_caption;
H264SEIUnregistered unregistered;
H264SEIRecoveryPoint recovery_point;
H264SEIBufferingPeriod buffering_period;
H264SEIFramePacking frame_packing;
H264SEIDisplayOrientation display_orientation;
H264SEIGreenMetaData green_metadata;
H264SEIAlternativeTransfer alternative_transfer;
} H264SEIContext;

struct H264ParamSets;

int ff_h264_sei_decode(H264SEIContext *h, GetBitContext *gb,
const struct H264ParamSets *ps, void *logctx);




void ff_h264_sei_uninit(H264SEIContext *h);




const char *ff_h264_sei_stereo_mode(const H264SEIFramePacking *h);

#endif 
