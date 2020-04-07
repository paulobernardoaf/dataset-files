

























#if !defined(AVCODEC_LIBWEBPENC_COMMON_H)
#define AVCODEC_LIBWEBPENC_COMMON_H

#include <webp/encode.h>

#include "libavutil/common.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"

typedef struct LibWebPContextCommon {
AVClass *class; 
float quality; 
int lossless; 
int preset; 
int chroma_warning; 
int conversion_warning; 
WebPConfig config; 
AVFrame *ref;
int cr_size;
int cr_threshold;
} LibWebPContextCommon;

int ff_libwebp_error_to_averror(int err);

av_cold int ff_libwebp_encode_init_common(AVCodecContext *avctx);

int ff_libwebp_get_frame(AVCodecContext *avctx, LibWebPContextCommon *s,
const AVFrame *frame, AVFrame **alt_frame_ptr,
WebPPicture **pic_ptr);

#define OFFSET(x) offsetof(LibWebPContextCommon, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "lossless", "Use lossless mode", OFFSET(lossless), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, VE },
{ "preset", "Configuration preset", OFFSET(preset), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, WEBP_PRESET_TEXT, VE, "preset" },
{ "none", "do not use a preset", 0, AV_OPT_TYPE_CONST, { .i64 = -1 }, 0, 0, VE, "preset" },
{ "default", "default preset", 0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_DEFAULT }, 0, 0, VE, "preset" },
{ "picture", "digital picture, like portrait, inner shot", 0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_PICTURE }, 0, 0, VE, "preset" },
{ "photo", "outdoor photograph, with natural lighting", 0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_PHOTO }, 0, 0, VE, "preset" },
{ "drawing", "hand or line drawing, with high-contrast details", 0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_DRAWING }, 0, 0, VE, "preset" },
{ "icon", "small-sized colorful images", 0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_ICON }, 0, 0, VE, "preset" },
{ "text", "text-like", 0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_TEXT }, 0, 0, VE, "preset" },
{ "cr_threshold","Conditional replenishment threshold", OFFSET(cr_threshold), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, INT_MAX, VE },
{ "cr_size" ,"Conditional replenishment block size", OFFSET(cr_size) , AV_OPT_TYPE_INT, { .i64 = 16 }, 0, 256, VE },
{ "quality" ,"Quality", OFFSET(quality), AV_OPT_TYPE_FLOAT, { .dbl = 75 }, 0, 100, VE },
{ NULL },
};

static const AVCodecDefault libwebp_defaults[] = {
{ "compression_level", "4" },
{ "global_quality", "-1" },
{ NULL },
};

#endif 
