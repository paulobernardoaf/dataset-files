#include "avfilter.h"
#define SCENE_SAD_PARAMS const uint8_t *src1, ptrdiff_t stride1, const uint8_t *src2, ptrdiff_t stride2, ptrdiff_t width, ptrdiff_t height, uint64_t *sum
typedef void (*ff_scene_sad_fn)(SCENE_SAD_PARAMS);
void ff_scene_sad_c(SCENE_SAD_PARAMS);
void ff_scene_sad16_c(SCENE_SAD_PARAMS);
ff_scene_sad_fn ff_scene_sad_get_fn_x86(int depth);
ff_scene_sad_fn ff_scene_sad_get_fn(int depth);
