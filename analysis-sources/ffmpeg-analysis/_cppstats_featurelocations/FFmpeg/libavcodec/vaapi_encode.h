

















#if !defined(AVCODEC_VAAPI_ENCODE_H)
#define AVCODEC_VAAPI_ENCODE_H

#include <stdint.h>

#include <va/va.h>

#if VA_CHECK_VERSION(1, 0, 0)
#include <va/va_str.h>
#endif

#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_vaapi.h"

#include "avcodec.h"

struct VAAPIEncodeType;
struct VAAPIEncodePicture;

enum {
MAX_CONFIG_ATTRIBUTES = 4,
MAX_GLOBAL_PARAMS = 4,
MAX_DPB_SIZE = 16,
MAX_PICTURE_REFERENCES = 2,
MAX_REORDER_DELAY = 16,
MAX_PARAM_BUFFER_SIZE = 1024,
};

enum {
PICTURE_TYPE_IDR = 0,
PICTURE_TYPE_I = 1,
PICTURE_TYPE_P = 2,
PICTURE_TYPE_B = 3,
};

typedef struct VAAPIEncodeSlice {
int index;
int row_start;
int row_size;
int block_start;
int block_size;
void *priv_data;
void *codec_slice_params;
} VAAPIEncodeSlice;

typedef struct VAAPIEncodePicture {
struct VAAPIEncodePicture *next;

int64_t display_order;
int64_t encode_order;
int64_t pts;
int force_idr;

#if VA_CHECK_VERSION(1, 0, 0)

VAEncROI *roi;
#else
void *roi;
#endif

int type;
int b_depth;
int encode_issued;
int encode_complete;

AVFrame *input_image;
VASurfaceID input_surface;

AVFrame *recon_image;
VASurfaceID recon_surface;

int nb_param_buffers;
VABufferID *param_buffers;

AVBufferRef *output_buffer_ref;
VABufferID output_buffer;

void *priv_data;
void *codec_picture_params;


int is_reference;




int nb_dpb_pics;
struct VAAPIEncodePicture *dpb[MAX_DPB_SIZE];


int nb_refs;
struct VAAPIEncodePicture *refs[MAX_PICTURE_REFERENCES];


struct VAAPIEncodePicture *prev;



int ref_count[2];
int ref_removed[2];

int nb_slices;
VAAPIEncodeSlice *slices;
} VAAPIEncodePicture;

typedef struct VAAPIEncodeProfile {

int av_profile;

int depth;

int nb_components;

int log2_chroma_w;

int log2_chroma_h;

VAProfile va_profile;
} VAAPIEncodeProfile;

enum {
RC_MODE_AUTO,
RC_MODE_CQP,
RC_MODE_CBR,
RC_MODE_VBR,
RC_MODE_ICQ,
RC_MODE_QVBR,
RC_MODE_AVBR,
RC_MODE_MAX = RC_MODE_AVBR,
};

typedef struct VAAPIEncodeRCMode {

int mode;

const char *name;

int supported;

uint32_t va_mode;

int bitrate;

int maxrate;

int quality;

int hrd;
} VAAPIEncodeRCMode;

typedef struct VAAPIEncodeContext {
const AVClass *class;


const struct VAAPIEncodeType *codec;




int low_power;


int idr_interval;


int desired_b_depth;



int explicit_rc_mode;



int explicit_qp;


unsigned int desired_packed_headers;




int surface_width;
int surface_height;


int slice_block_width;
int slice_block_height;





const VAAPIEncodeProfile *profile;


const VAAPIEncodeRCMode *rc_mode;


int rc_quality;


VAProfile va_profile;

VAEntrypoint va_entrypoint;

unsigned int va_rc_mode;

unsigned int va_bit_rate;

unsigned int va_packed_headers;


VAConfigAttrib config_attributes[MAX_CONFIG_ATTRIBUTES];
int nb_config_attributes;

VAConfigID va_config;
VAContextID va_context;

AVBufferRef *device_ref;
AVHWDeviceContext *device;
AVVAAPIDeviceContext *hwctx;


AVBufferRef *input_frames_ref;
AVHWFramesContext *input_frames;


AVBufferRef *recon_frames_ref;
AVHWFramesContext *recon_frames;


AVBufferPool *output_buffer_pool;



int global_params_type[MAX_GLOBAL_PARAMS];
const void *global_params [MAX_GLOBAL_PARAMS];
size_t global_params_size[MAX_GLOBAL_PARAMS];
int nb_global_params;


VAEncMiscParameterRateControl rc_params;
VAEncMiscParameterHRD hrd_params;
VAEncMiscParameterFrameRate fr_params;
#if VA_CHECK_VERSION(0, 36, 0)
VAEncMiscParameterBufferQualityLevel quality_params;
#endif


void *codec_sequence_params;



void *codec_picture_params;


VAAPIEncodePicture *pic_start, *pic_end;


VAAPIEncodePicture *next_prev;


int64_t input_order;

int64_t output_delay;

int64_t encode_order;

int64_t decode_delay;

int64_t output_order;


int64_t first_pts;
int64_t dts_pts_diff;
int64_t ts_ring[MAX_REORDER_DELAY * 3];


int slice_block_rows;
int slice_block_cols;
int nb_slices;
int slice_size;


int gop_size;
int closed_gop;
int gop_per_idr;
int p_per_i;
int max_b_depth;
int b_per_p;
int force_idr;
int idr_counter;
int gop_counter;
int end_of_stream;


int roi_allowed;

int roi_max_regions;


int roi_quant_range;



int crop_warned;


int roi_warned;
} VAAPIEncodeContext;

enum {

FLAG_SLICE_CONTROL = 1 << 0,

FLAG_CONSTANT_QUALITY_ONLY = 1 << 1,

FLAG_INTRA_ONLY = 1 << 2,

FLAG_B_PICTURES = 1 << 3,

FLAG_B_PICTURE_REFERENCES = 1 << 4,


FLAG_NON_IDR_KEY_PICTURES = 1 << 5,
};

typedef struct VAAPIEncodeType {


const VAAPIEncodeProfile *profiles;


int flags;



int default_quality;




int (*configure)(AVCodecContext *avctx);



size_t picture_priv_data_size;



size_t sequence_params_size;
size_t picture_params_size;
size_t slice_params_size;


int (*init_sequence_params)(AVCodecContext *avctx);
int (*init_picture_params)(AVCodecContext *avctx,
VAAPIEncodePicture *pic);
int (*init_slice_params)(AVCodecContext *avctx,
VAAPIEncodePicture *pic,
VAAPIEncodeSlice *slice);



int sequence_header_type;
int picture_header_type;
int slice_header_type;




int (*write_sequence_header)(AVCodecContext *avctx,
char *data, size_t *data_len);
int (*write_picture_header)(AVCodecContext *avctx,
VAAPIEncodePicture *pic,
char *data, size_t *data_len);
int (*write_slice_header)(AVCodecContext *avctx,
VAAPIEncodePicture *pic,
VAAPIEncodeSlice *slice,
char *data, size_t *data_len);





int (*write_extra_buffer)(AVCodecContext *avctx,
VAAPIEncodePicture *pic,
int index, int *type,
char *data, size_t *data_len);




int (*write_extra_header)(AVCodecContext *avctx,
VAAPIEncodePicture *pic,
int index, int *type,
char *data, size_t *data_len);
} VAAPIEncodeType;


int ff_vaapi_encode_send_frame(AVCodecContext *avctx, const AVFrame *frame);
int ff_vaapi_encode_receive_packet(AVCodecContext *avctx, AVPacket *pkt);

int ff_vaapi_encode_init(AVCodecContext *avctx);
int ff_vaapi_encode_close(AVCodecContext *avctx);


#define VAAPI_ENCODE_COMMON_OPTIONS { "low_power", "Use low-power encoding mode (only available on some platforms; " "may not support all encoding features)", OFFSET(common.low_power), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, FLAGS }, { "idr_interval", "Distance (in I-frames) between IDR frames", OFFSET(common.idr_interval), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, INT_MAX, FLAGS }, { "b_depth", "Maximum B-frame reference depth", OFFSET(common.desired_b_depth), AV_OPT_TYPE_INT, { .i64 = 1 }, 1, INT_MAX, FLAGS }














#define VAAPI_ENCODE_RC_MODE(name, desc) { #name, desc, 0, AV_OPT_TYPE_CONST, { .i64 = RC_MODE_ ##name }, 0, 0, FLAGS, "rc_mode" }


#define VAAPI_ENCODE_RC_OPTIONS { "rc_mode","Set rate control mode", OFFSET(common.explicit_rc_mode), AV_OPT_TYPE_INT, { .i64 = RC_MODE_AUTO }, RC_MODE_AUTO, RC_MODE_MAX, FLAGS, "rc_mode" }, { "auto", "Choose mode automatically based on other parameters", 0, AV_OPT_TYPE_CONST, { .i64 = RC_MODE_AUTO }, 0, 0, FLAGS, "rc_mode" }, VAAPI_ENCODE_RC_MODE(CQP, "Constant-quality"), VAAPI_ENCODE_RC_MODE(CBR, "Constant-bitrate"), VAAPI_ENCODE_RC_MODE(VBR, "Variable-bitrate"), VAAPI_ENCODE_RC_MODE(ICQ, "Intelligent constant-quality"), VAAPI_ENCODE_RC_MODE(QVBR, "Quality-defined variable-bitrate"), VAAPI_ENCODE_RC_MODE(AVBR, "Average variable-bitrate")














#endif 
