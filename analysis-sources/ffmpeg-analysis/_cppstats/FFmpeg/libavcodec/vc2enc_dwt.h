#include <stddef.h>
#include <stdint.h>
typedef int32_t dwtcoef;
enum VC2TransformType {
VC2_TRANSFORM_9_7 = 0, 
VC2_TRANSFORM_5_3 = 1, 
VC2_TRANSFORM_13_7 = 2, 
VC2_TRANSFORM_HAAR = 3, 
VC2_TRANSFORM_HAAR_S = 4, 
VC2_TRANSFORM_FIDEL = 5, 
VC2_TRANSFORM_9_7_I = 6, 
VC2_TRANSFORMS_NB
};
typedef struct VC2TransformContext {
dwtcoef *buffer;
int padding;
void (*vc2_subband_dwt[VC2_TRANSFORMS_NB])(struct VC2TransformContext *t,
dwtcoef *data, ptrdiff_t stride,
int width, int height);
} VC2TransformContext;
int ff_vc2enc_init_transforms(VC2TransformContext *t, int p_stride, int p_height,
int slice_w, int slice_h);
void ff_vc2enc_free_transforms(VC2TransformContext *t);
