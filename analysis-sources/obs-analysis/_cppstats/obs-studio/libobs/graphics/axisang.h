#pragma once
#include "../util/c99defs.h"
#if defined(__cplusplus)
extern "C" {
#endif
struct quat;
struct axisang {
union {
struct {
float x, y, z, w;
};
float ptr[4];
};
};
static inline void axisang_zero(struct axisang *dst)
{
dst->x = 0.0f;
dst->y = 0.0f;
dst->z = 0.0f;
dst->w = 0.0f;
}
static inline void axisang_copy(struct axisang *dst, struct axisang *aa)
{
dst->x = aa->x;
dst->y = aa->y;
dst->z = aa->z;
dst->w = aa->w;
}
static inline void axisang_set(struct axisang *dst, float x, float y, float z,
float w)
{
dst->x = x;
dst->y = y;
dst->z = z;
dst->w = w;
}
EXPORT void axisang_from_quat(struct axisang *dst, const struct quat *q);
#if defined(__cplusplus)
}
#endif
