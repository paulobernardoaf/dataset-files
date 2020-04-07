

















#if !defined(AVFILTER_VAAPI_VPP_H)
#define AVFILTER_VAAPI_VPP_H

#include <va/va.h>
#include <va/va_vpp.h>

#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_vaapi.h"

#include "avfilter.h"


#define VAAPI_VPP_BACKGROUND_BLACK 0xff000000

typedef struct VAAPIVPPContext {
const AVClass *class;

AVVAAPIDeviceContext *hwctx;
AVBufferRef *device_ref;

int valid_ids;
VAConfigID va_config;
VAContextID va_context;

AVBufferRef *input_frames_ref;
AVHWFramesContext *input_frames;
VARectangle input_region;

enum AVPixelFormat output_format;
int output_width; 
int output_height; 

VABufferID filter_buffers[VAProcFilterCount];
int nb_filter_buffers;

int (*build_filter_params)(AVFilterContext *avctx);

void (*pipeline_uninit)(AVFilterContext *avctx);
} VAAPIVPPContext;

void ff_vaapi_vpp_ctx_init(AVFilterContext *avctx);

void ff_vaapi_vpp_ctx_uninit(AVFilterContext *avctx);

int ff_vaapi_vpp_query_formats(AVFilterContext *avctx);

void ff_vaapi_vpp_pipeline_uninit(AVFilterContext *avctx);

int ff_vaapi_vpp_config_input(AVFilterLink *inlink);

int ff_vaapi_vpp_config_output(AVFilterLink *outlink);

int ff_vaapi_vpp_init_params(AVFilterContext *avctx,
VAProcPipelineParameterBuffer *params,
const AVFrame *input_frame,
AVFrame *output_frame);

int ff_vaapi_vpp_make_param_buffers(AVFilterContext *avctx,
int type,
const void *data,
size_t size,
int count);

int ff_vaapi_vpp_render_picture(AVFilterContext *avctx,
VAProcPipelineParameterBuffer *params,
AVFrame *output_frame);

#endif 
