
















#pragma once

#include "../util/c99defs.h"








#if defined(__cplusplus)
extern "C" {
#endif

struct vec2;
struct vec3;

EXPORT void polar_to_cart(struct vec3 *dst, const struct vec3 *v);
EXPORT void cart_to_polar(struct vec3 *dst, const struct vec3 *v);

EXPORT void norm_to_polar(struct vec2 *dst, const struct vec3 *norm);
EXPORT void polar_to_norm(struct vec3 *dst, const struct vec2 *polar);

EXPORT float calc_torquef(float val1, float val2, float torque,
float min_adjust, float t);

EXPORT void calc_torque(struct vec3 *dst, const struct vec3 *v1,
const struct vec3 *v2, float torque, float min_adjust,
float t);

static inline float get_percentage(float start, float end, float mid)
{
return (mid - start) / (end - start);
}

static inline float get_percentagei(int start, int end, int mid)
{
return (float)(mid - start) / (float)(end - start);
}

EXPORT float rand_float(int positive_only);

#if defined(__cplusplus)
}
#endif
