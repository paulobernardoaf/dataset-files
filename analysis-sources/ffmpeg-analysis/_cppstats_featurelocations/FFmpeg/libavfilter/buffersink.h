

















#if !defined(AVFILTER_BUFFERSINK_H)
#define AVFILTER_BUFFERSINK_H







#include "avfilter.h"

















int av_buffersink_get_frame_flags(AVFilterContext *ctx, AVFrame *frame, int flags);






#define AV_BUFFERSINK_FLAG_PEEK 1






#define AV_BUFFERSINK_FLAG_NO_REQUEST 2

#if FF_API_NEXT



typedef struct AVBufferSinkParams {
const enum AVPixelFormat *pixel_fmts; 
} AVBufferSinkParams;






attribute_deprecated
AVBufferSinkParams *av_buffersink_params_alloc(void);




typedef struct AVABufferSinkParams {
const enum AVSampleFormat *sample_fmts; 
const int64_t *channel_layouts; 
const int *channel_counts; 
int all_channel_counts; 
int *sample_rates; 
} AVABufferSinkParams;






attribute_deprecated
AVABufferSinkParams *av_abuffersink_params_alloc(void);
#endif








void av_buffersink_set_frame_size(AVFilterContext *ctx, unsigned frame_size);







enum AVMediaType av_buffersink_get_type (const AVFilterContext *ctx);
AVRational av_buffersink_get_time_base (const AVFilterContext *ctx);
int av_buffersink_get_format (const AVFilterContext *ctx);

AVRational av_buffersink_get_frame_rate (const AVFilterContext *ctx);
int av_buffersink_get_w (const AVFilterContext *ctx);
int av_buffersink_get_h (const AVFilterContext *ctx);
AVRational av_buffersink_get_sample_aspect_ratio (const AVFilterContext *ctx);

int av_buffersink_get_channels (const AVFilterContext *ctx);
uint64_t av_buffersink_get_channel_layout (const AVFilterContext *ctx);
int av_buffersink_get_sample_rate (const AVFilterContext *ctx);

AVBufferRef * av_buffersink_get_hw_frames_ctx (const AVFilterContext *ctx);

















int av_buffersink_get_frame(AVFilterContext *ctx, AVFrame *frame);


















int av_buffersink_get_samples(AVFilterContext *ctx, AVFrame *frame, int nb_samples);





#endif 
