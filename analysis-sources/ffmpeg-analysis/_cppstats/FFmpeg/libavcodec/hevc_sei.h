#include <stdint.h>
#include "get_bits.h"
typedef enum {
HEVC_SEI_TYPE_BUFFERING_PERIOD = 0,
HEVC_SEI_TYPE_PICTURE_TIMING = 1,
HEVC_SEI_TYPE_PAN_SCAN_RECT = 2,
HEVC_SEI_TYPE_FILLER_PAYLOAD = 3,
HEVC_SEI_TYPE_USER_DATA_REGISTERED_ITU_T_T35 = 4,
HEVC_SEI_TYPE_USER_DATA_UNREGISTERED = 5,
HEVC_SEI_TYPE_RECOVERY_POINT = 6,
HEVC_SEI_TYPE_SCENE_INFO = 9,
HEVC_SEI_TYPE_FULL_FRAME_SNAPSHOT = 15,
HEVC_SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_START = 16,
HEVC_SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_END = 17,
HEVC_SEI_TYPE_FILM_GRAIN_CHARACTERISTICS = 19,
HEVC_SEI_TYPE_POST_FILTER_HINT = 22,
HEVC_SEI_TYPE_TONE_MAPPING_INFO = 23,
HEVC_SEI_TYPE_FRAME_PACKING = 45,
HEVC_SEI_TYPE_DISPLAY_ORIENTATION = 47,
HEVC_SEI_TYPE_SOP_DESCRIPTION = 128,
HEVC_SEI_TYPE_ACTIVE_PARAMETER_SETS = 129,
HEVC_SEI_TYPE_DECODING_UNIT_INFO = 130,
HEVC_SEI_TYPE_TEMPORAL_LEVEL0_INDEX = 131,
HEVC_SEI_TYPE_DECODED_PICTURE_HASH = 132,
HEVC_SEI_TYPE_SCALABLE_NESTING = 133,
HEVC_SEI_TYPE_REGION_REFRESH_INFO = 134,
HEVC_SEI_TYPE_TIME_CODE = 136,
HEVC_SEI_TYPE_MASTERING_DISPLAY_INFO = 137,
HEVC_SEI_TYPE_CONTENT_LIGHT_LEVEL_INFO = 144,
HEVC_SEI_TYPE_ALTERNATIVE_TRANSFER_CHARACTERISTICS = 147,
HEVC_SEI_TYPE_ALPHA_CHANNEL_INFO = 165,
} HEVC_SEI_Type;
typedef enum {
HEVC_SEI_PIC_STRUCT_FRAME_DOUBLING = 7,
HEVC_SEI_PIC_STRUCT_FRAME_TRIPLING = 8
} HEVC_SEI_PicStructType;
typedef struct HEVCSEIPictureHash {
uint8_t md5[3][16];
uint8_t is_md5;
} HEVCSEIPictureHash;
typedef struct HEVCSEIFramePacking {
int present;
int arrangement_type;
int content_interpretation_type;
int quincunx_subsampling;
int current_frame_is_frame0_flag;
} HEVCSEIFramePacking;
typedef struct HEVCSEIDisplayOrientation {
int present;
int anticlockwise_rotation;
int hflip, vflip;
} HEVCSEIDisplayOrientation;
typedef struct HEVCSEIPictureTiming {
int picture_struct;
} HEVCSEIPictureTiming;
typedef struct HEVCSEIA53Caption {
AVBufferRef *buf_ref;
} HEVCSEIA53Caption;
typedef struct HEVCSEIMasteringDisplay {
int present;
uint16_t display_primaries[3][2];
uint16_t white_point[2];
uint32_t max_luminance;
uint32_t min_luminance;
} HEVCSEIMasteringDisplay;
typedef struct HEVCSEIContentLight {
int present;
uint16_t max_content_light_level;
uint16_t max_pic_average_light_level;
} HEVCSEIContentLight;
typedef struct HEVCSEIAlternativeTransfer {
int present;
int preferred_transfer_characteristics;
} HEVCSEIAlternativeTransfer;
typedef struct HEVCSEI {
HEVCSEIPictureHash picture_hash;
HEVCSEIFramePacking frame_packing;
HEVCSEIDisplayOrientation display_orientation;
HEVCSEIPictureTiming picture_timing;
HEVCSEIA53Caption a53_caption;
HEVCSEIMasteringDisplay mastering_display;
HEVCSEIContentLight content_light;
int active_seq_parameter_set_id;
HEVCSEIAlternativeTransfer alternative_transfer;
} HEVCSEI;
struct HEVCParamSets;
int ff_hevc_decode_nal_sei(GetBitContext *gb, void *logctx, HEVCSEI *s,
const struct HEVCParamSets *ps, int type);
void ff_hevc_reset_sei(HEVCSEI *s);
