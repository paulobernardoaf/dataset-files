




















#if !defined(AVFILTER_VMAF_MOTION_H)
#define AVFILTER_VMAF_MOTION_H

#include <stddef.h>
#include <stdint.h>
#include "video.h"

typedef struct VMAFMotionDSPContext {
uint64_t (*sad)(const uint16_t *img1, const uint16_t *img2, int w, int h,
ptrdiff_t img1_stride, ptrdiff_t img2_stride);
void (*convolution_x)(const uint16_t *filter, int filt_w, const uint16_t *src,
uint16_t *dst, int w, int h, ptrdiff_t src_stride,
ptrdiff_t dst_stride);
void (*convolution_y)(const uint16_t *filter, int filt_w, const uint8_t *src,
uint16_t *dst, int w, int h, ptrdiff_t src_stride,
ptrdiff_t dst_stride);
} VMAFMotionDSPContext;

void ff_vmafmotion_init_x86(VMAFMotionDSPContext *dsp);

typedef struct VMAFMotionData {
uint16_t filter[5];
int width;
int height;
ptrdiff_t stride;
uint16_t *blur_data[2 ];
uint16_t *temp_data;
double motion_sum;
uint64_t nb_frames;
VMAFMotionDSPContext vmafdsp;
} VMAFMotionData;

int ff_vmafmotion_init(VMAFMotionData *data, int w, int h, enum AVPixelFormat fmt);
double ff_vmafmotion_process(VMAFMotionData *data, AVFrame *frame);
double ff_vmafmotion_uninit(VMAFMotionData *data);

#endif 
