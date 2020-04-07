

















#if !defined(AVFILTER_FRAMERATE_H)
#define AVFILTER_FRAMERATE_H

#include "scene_sad.h"
#include "avfilter.h"

#define BLEND_FUNC_PARAMS const uint8_t *src1, ptrdiff_t src1_linesize, const uint8_t *src2, ptrdiff_t src2_linesize, uint8_t *dst, ptrdiff_t dst_linesize, ptrdiff_t width, ptrdiff_t height, int factor1, int factor2, int half





#define BLEND_FACTOR_DEPTH(n) (n-1)

typedef void (*blend_func)(BLEND_FUNC_PARAMS);

typedef struct FrameRateContext {
const AVClass *class;

AVRational dest_frame_rate; 
int flags; 
double scene_score; 
int interp_start; 
int interp_end; 

int line_size[4]; 
int height[4]; 
int vsub;

AVRational srce_time_base; 
AVRational dest_time_base; 

ff_scene_sad_fn sad; 
double prev_mafd; 

int blend_factor_max;
int bitdepth;
AVFrame *work;

AVFrame *f0; 
AVFrame *f1; 
int64_t pts0; 
int64_t pts1; 
int64_t delta; 
double score; 
int flush; 
int64_t start_pts; 
int64_t n; 

blend_func blend;
} FrameRateContext;

void ff_framerate_init(FrameRateContext *s);
void ff_framerate_init_x86(FrameRateContext *s);

#endif 
