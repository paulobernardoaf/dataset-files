#include <stdint.h>
enum InterpolateMethod {
INTERPOLATE_NEAREST, 
INTERPOLATE_BILINEAR, 
INTERPOLATE_BIQUADRATIC, 
INTERPOLATE_COUNT, 
};
#define INTERPOLATE_DEFAULT INTERPOLATE_BILINEAR
#define INTERPOLATE_FAST INTERPOLATE_NEAREST
#define INTERPOLATE_BEST INTERPOLATE_BIQUADRATIC
enum FillMethod {
FILL_BLANK, 
FILL_ORIGINAL, 
FILL_CLAMP, 
FILL_MIRROR, 
FILL_COUNT, 
};
#define FILL_DEFAULT FILL_ORIGINAL
void ff_get_matrix(
float x_shift,
float y_shift,
float angle,
float scale_x,
float scale_y,
float *matrix
);
void avfilter_add_matrix(const float *m1, const float *m2, float *result);
void avfilter_sub_matrix(const float *m1, const float *m2, float *result);
void avfilter_mul_matrix(const float *m1, float scalar, float *result);
int avfilter_transform(const uint8_t *src, uint8_t *dst,
int src_stride, int dst_stride,
int width, int height, const float *matrix,
enum InterpolateMethod interpolate,
enum FillMethod fill);
