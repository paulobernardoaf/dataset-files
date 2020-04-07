
















#include "axisang.h"
#include "quat.h"

void axisang_from_quat(struct axisang *dst, const struct quat *q)
{
float len, leni;

len = q->x * q->x + q->y * q->y + q->z * q->z;
if (!close_float(len, 0.0f, EPSILON)) {
leni = 1.0f / sqrtf(len);
dst->x = q->x * leni;
dst->y = q->y * leni;
dst->z = q->z * leni;
dst->w = acosf(q->w) * 2.0f;
} else {
dst->x = 0.0f;
dst->y = 0.0f;
dst->z = 0.0f;
dst->w = 0.0f;
}
}
