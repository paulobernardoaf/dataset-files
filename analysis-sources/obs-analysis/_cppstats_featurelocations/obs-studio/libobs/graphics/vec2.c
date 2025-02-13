
















#include <math.h>
#include "math-extra.h"
#include "math-defs.h"
#include "vec2.h"

void vec2_abs(struct vec2 *dst, const struct vec2 *v)
{
vec2_set(dst, fabsf(v->x), fabsf(v->y));
}

void vec2_floor(struct vec2 *dst, const struct vec2 *v)
{
vec2_set(dst, floorf(v->x), floorf(v->y));
}

void vec2_ceil(struct vec2 *dst, const struct vec2 *v)
{
vec2_set(dst, ceilf(v->x), ceilf(v->y));
}

int vec2_close(const struct vec2 *v1, const struct vec2 *v2, float epsilon)
{
return close_float(v1->x, v2->x, epsilon) &&
close_float(v1->y, v2->y, epsilon);
}

void vec2_norm(struct vec2 *dst, const struct vec2 *v)
{
float len = vec2_len(v);

if (len > 0.0f) {
len = 1.0f / len;
vec2_mulf(dst, v, len);
}
}
