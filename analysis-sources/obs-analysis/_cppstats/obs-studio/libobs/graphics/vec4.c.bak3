
















#include "vec4.h"
#include "vec3.h"
#include "matrix4.h"

void vec4_from_vec3(struct vec4 *dst, const struct vec3 *v)
{
dst->m = v->m;
dst->w = 1.0f;
}

void vec4_transform(struct vec4 *dst, const struct vec4 *v,
const struct matrix4 *m)
{
struct vec4 temp;
struct matrix4 transpose;

matrix4_transpose(&transpose, m);

temp.x = vec4_dot(&transpose.x, v);
temp.y = vec4_dot(&transpose.y, v);
temp.z = vec4_dot(&transpose.z, v);
temp.w = vec4_dot(&transpose.t, v);

vec4_copy(dst, &temp);
}
