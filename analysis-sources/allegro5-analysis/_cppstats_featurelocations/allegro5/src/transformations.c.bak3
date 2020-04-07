
















#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_display.h"
#include "allegro5/internal/aintern_system.h"
#include "allegro5/internal/aintern_transform.h"
#include <math.h>





void al_copy_transform(ALLEGRO_TRANSFORM *dest, const ALLEGRO_TRANSFORM *src)
{
ASSERT(src);
ASSERT(dest);

memcpy(dest, src, sizeof(ALLEGRO_TRANSFORM));
}



void al_use_transform(const ALLEGRO_TRANSFORM *trans)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();
ALLEGRO_DISPLAY *display;

if (!target)
return;







if (trans != &target->transform) {
al_copy_transform(&target->transform, trans);

target->inverse_transform_dirty = true;
}





if (!al_is_bitmap_drawing_held()) {
display = _al_get_bitmap_display(target);
if (display) {
display->vt->update_transformation(display, target);
}
}
}



void al_use_projection_transform(const ALLEGRO_TRANSFORM *trans)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();
ALLEGRO_DISPLAY *display;

if (!target)
return;


if (al_get_bitmap_flags(target) & ALLEGRO_MEMORY_BITMAP)
return;







if (trans != &target->transform) {
al_copy_transform(&target->proj_transform, trans);
}

display = _al_get_bitmap_display(target);
if (display) {
display->vt->update_transformation(display, target);
}
}



const ALLEGRO_TRANSFORM *al_get_current_transform(void)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();

if (!target)
return NULL;

return &target->transform;
}



const ALLEGRO_TRANSFORM *al_get_current_projection_transform(void)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();

if (!target)
return NULL;

return &target->proj_transform;
}



const ALLEGRO_TRANSFORM *al_get_current_inverse_transform(void)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();

if (!target)
return NULL;

if (target->inverse_transform_dirty) {
al_copy_transform(&target->inverse_transform, &target->transform);
al_invert_transform(&target->inverse_transform);
}

return &target->inverse_transform;
}



void al_identity_transform(ALLEGRO_TRANSFORM *trans)
{
ASSERT(trans);

trans->m[0][0] = 1;
trans->m[0][1] = 0;
trans->m[0][2] = 0;
trans->m[0][3] = 0;

trans->m[1][0] = 0;
trans->m[1][1] = 1;
trans->m[1][2] = 0;
trans->m[1][3] = 0;

trans->m[2][0] = 0;
trans->m[2][1] = 0;
trans->m[2][2] = 1;
trans->m[2][3] = 0;

trans->m[3][0] = 0;
trans->m[3][1] = 0;
trans->m[3][2] = 0;
trans->m[3][3] = 1;
}



void al_build_transform(ALLEGRO_TRANSFORM *trans, float x, float y,
float sx, float sy, float theta)
{
float c, s;
ASSERT(trans);

c = cosf(theta);
s = sinf(theta);

trans->m[0][0] = sx * c;
trans->m[0][1] = sy * s;
trans->m[0][2] = 0;
trans->m[0][3] = 0;

trans->m[1][0] = -sx * s;
trans->m[1][1] = sy * c;
trans->m[1][2] = 0;
trans->m[1][3] = 0;

trans->m[2][0] = 0;
trans->m[2][1] = 0;
trans->m[2][2] = 1;
trans->m[2][3] = 0;

trans->m[3][0] = x;
trans->m[3][1] = y;
trans->m[3][2] = 0;
trans->m[3][3] = 1;
}



void al_build_camera_transform(ALLEGRO_TRANSFORM *trans,
float position_x, float position_y, float position_z,
float look_x, float look_y, float look_z,
float up_x, float up_y, float up_z)
{
float x = position_x;
float y = position_y;
float z = position_z;
float xx, xy, xz, xnorm;
float yx, yy, yz;
float zx, zy, zz, znorm;

al_identity_transform(trans);



zx = x - look_x;
zy = y - look_y;
zz = z - look_z;
znorm = sqrt(zx * zx + zy * zy + zz * zz);
if (znorm == 0)
return;
zx /= znorm;
zy /= znorm;
zz /= znorm;





xx = up_y * zz - zy * up_z;
xy = up_z * zx - zz * up_x;
xz = up_x * zy - zx * up_y;
xnorm = sqrt(xx * xx + xy * xy + xz * xz);
if (xnorm == 0)
return;
xx /= xnorm;
xy /= xnorm;
xz /= xnorm;





yx = zy * xz - xy * zz;
yy = zz * xx - xz * zx;
yz = zx * xy - xx * zy;





trans->m[0][0] = xx;
trans->m[1][0] = xy;
trans->m[2][0] = xz;
trans->m[3][0] = xx * -x + xy * -y + xz * -z;
trans->m[0][1] = yx;
trans->m[1][1] = yy;
trans->m[2][1] = yz;
trans->m[3][1] = yx * -x + yy * -y + yz * -z;
trans->m[0][2] = zx;
trans->m[1][2] = zy;
trans->m[2][2] = zz;
trans->m[3][2] = zx * -x + zy * -y + zz * -z;
}



void al_invert_transform(ALLEGRO_TRANSFORM *trans)
{
float det, t;
ASSERT(trans);

det = trans->m[0][0] * trans->m[1][1] - trans->m[1][0] * trans->m[0][1];

t = trans->m[3][0];
trans->m[3][0] = ( trans->m[1][0] * trans->m[3][1] - t * trans->m[1][1]) / det;
trans->m[3][1] = (t * trans->m[0][1] - trans->m[0][0] * trans->m[3][1]) / det;

t = trans->m[0][0];
trans->m[0][0] = trans->m[1][1] / det;
trans->m[1][1] = t / det;

trans->m[0][1] = - trans->m[0][1] / det;
trans->m[1][0] = - trans->m[1][0] / det;
}



void al_transpose_transform(ALLEGRO_TRANSFORM *trans)
{
int i, j;
ASSERT(trans);

ALLEGRO_TRANSFORM t = *trans;
for (i = 0; i < 4; i++) {
for (j = 0; j < 4; j++) {
trans->m[i][j] = t.m[j][i];
}
}
}



int al_check_inverse(const ALLEGRO_TRANSFORM *trans, float tol)
{
float det, norm, c0, c1, c3;
ASSERT(trans);

det = fabsf(trans->m[0][0] * trans->m[1][1] - trans->m[1][0] * trans->m[0][1]);



c0 = fabsf(trans->m[0][0]) + fabsf(trans->m[0][1]);
c1 = fabsf(trans->m[1][0]) + fabsf(trans->m[1][1]);
c3 = fabsf(trans->m[3][0]) + fabsf(trans->m[3][1]) + 1;
norm = _ALLEGRO_MAX(_ALLEGRO_MAX(1, c0), _ALLEGRO_MAX(c1, c3));

return det > tol * norm;
}



void al_translate_transform(ALLEGRO_TRANSFORM *trans, float x, float y)
{
ASSERT(trans);

trans->m[3][0] += x;
trans->m[3][1] += y;
}




void al_translate_transform_3d(ALLEGRO_TRANSFORM *trans, float x, float y,
float z)
{
ASSERT(trans);

trans->m[3][0] += x;
trans->m[3][1] += y;
trans->m[3][2] += z;
}




void al_rotate_transform(ALLEGRO_TRANSFORM *trans, float theta)
{
float c, s;
float t;
ASSERT(trans);

c = cosf(theta);
s = sinf(theta);

t = trans->m[0][0];
trans->m[0][0] = t * c - trans->m[0][1] * s;
trans->m[0][1] = t * s + trans->m[0][1] * c;

t = trans->m[1][0];
trans->m[1][0] = t * c - trans->m[1][1] * s;
trans->m[1][1] = t * s + trans->m[1][1] * c;

t = trans->m[3][0];
trans->m[3][0] = t * c - trans->m[3][1] * s;
trans->m[3][1] = t * s + trans->m[3][1] * c;
}



void al_scale_transform(ALLEGRO_TRANSFORM *trans, float sx, float sy)
{
ASSERT(trans);

trans->m[0][0] *= sx;
trans->m[0][1] *= sy;

trans->m[1][0] *= sx;
trans->m[1][1] *= sy;

trans->m[3][0] *= sx;
trans->m[3][1] *= sy;
}




void al_scale_transform_3d(ALLEGRO_TRANSFORM *trans, float sx, float sy,
float sz)
{
ASSERT(trans);

trans->m[0][0] *= sx;
trans->m[0][1] *= sy;
trans->m[0][2] *= sz;

trans->m[1][0] *= sx;
trans->m[1][1] *= sy;
trans->m[1][2] *= sz;

trans->m[2][0] *= sx;
trans->m[2][1] *= sy;
trans->m[2][2] *= sz;

trans->m[3][0] *= sx;
trans->m[3][1] *= sy;
trans->m[3][2] *= sz;
}



void al_transform_coordinates(const ALLEGRO_TRANSFORM *trans, float *x, float *y)
{
float t;
ASSERT(trans);
ASSERT(x);
ASSERT(y);

t = *x;

*x = t * trans->m[0][0] + *y * trans->m[1][0] + trans->m[3][0];
*y = t * trans->m[0][1] + *y * trans->m[1][1] + trans->m[3][1];
}



void al_transform_coordinates_3d(const ALLEGRO_TRANSFORM *trans,
float *x, float *y, float *z)
{
float rx, ry, rz;
ASSERT(trans);
ASSERT(x);
ASSERT(y);
ASSERT(z);

#define M(i, j) trans->m[i][j]

rx = M(0, 0) * *x + M(1, 0) * *y + M(2, 0) * *z + M(3, 0);
ry = M(0, 1) * *x + M(1, 1) * *y + M(2, 1) * *z + M(3, 1);
rz = M(0, 2) * *x + M(1, 2) * *y + M(2, 2) * *z + M(3, 2);

#undef M

*x = rx;
*y = ry;
*z = rz;
}



void al_transform_coordinates_4d(const ALLEGRO_TRANSFORM *trans,
float *x, float *y, float *z, float *w)
{
float rx, ry, rz, rw;
ASSERT(trans);
ASSERT(x);
ASSERT(y);
ASSERT(z);
ASSERT(w);

#define M(i, j) trans->m[i][j]

rx = M(0, 0) * *x + M(1, 0) * *y + M(2, 0) * *z + M(3, 0) * *w;
ry = M(0, 1) * *x + M(1, 1) * *y + M(2, 1) * *z + M(3, 1) * *w;
rz = M(0, 2) * *x + M(1, 2) * *y + M(2, 2) * *z + M(3, 2) * *w;
rw = M(0, 3) * *x + M(1, 3) * *y + M(2, 3) * *z + M(3, 3) * *w;

#undef M

*x = rx;
*y = ry;
*z = rz;
*w = rw;
}



void al_transform_coordinates_3d_projective(const ALLEGRO_TRANSFORM *trans,
float *x, float *y, float *z)
{
float w = 1;
al_transform_coordinates_4d(trans, x, y, z, &w);
*x /= w;
*y /= w;
*z /= w;
}



void al_compose_transform(ALLEGRO_TRANSFORM *trans, const ALLEGRO_TRANSFORM *other)
{
#define E(x, y) (other->m[0][y] * trans->m[x][0] + other->m[1][y] * trans->m[x][1] + other->m[2][y] * trans->m[x][2] + other->m[3][y] * trans->m[x][3]) 





const ALLEGRO_TRANSFORM tmp = {{
{ E(0, 0), E(0, 1), E(0, 2), E(0, 3) },
{ E(1, 0), E(1, 1), E(1, 2), E(1, 3) },
{ E(2, 0), E(2, 1), E(2, 2), E(2, 3) },
{ E(3, 0), E(3, 1), E(3, 2), E(3, 3) }
}};

*trans = tmp;

#undef E
}

bool _al_transform_is_translation(const ALLEGRO_TRANSFORM* trans,
float *dx, float *dy)
{
if (trans->m[0][0] == 1 &&
trans->m[1][0] == 0 &&
trans->m[2][0] == 0 &&
trans->m[0][1] == 0 &&
trans->m[1][1] == 1 &&
trans->m[2][1] == 0 &&
trans->m[0][2] == 0 &&
trans->m[1][2] == 0 &&
trans->m[2][2] == 1 &&
trans->m[3][2] == 0 &&
trans->m[0][3] == 0 &&
trans->m[1][3] == 0 &&
trans->m[2][3] == 0 &&
trans->m[3][3] == 1) {
*dx = trans->m[3][0];
*dy = trans->m[3][1];
return true;
}
return false;
}



void al_orthographic_transform(ALLEGRO_TRANSFORM *trans,
float left, float top, float n,
float right, float bottom, float f)
{
float delta_x = right - left;
float delta_y = top - bottom;
float delta_z = f - n;
ALLEGRO_TRANSFORM tmp;

al_identity_transform(&tmp);

tmp.m[0][0] = 2.0f / delta_x;
tmp.m[1][1] = 2.0f / delta_y;
tmp.m[2][2] = 2.0f / delta_z;
tmp.m[3][0] = -(right + left) / delta_x;
tmp.m[3][1] = -(top + bottom) / delta_y;
tmp.m[3][2] = -(f + n) / delta_z;
tmp.m[3][3] = 1.0f;

al_compose_transform(trans, &tmp);
}




void al_rotate_transform_3d(ALLEGRO_TRANSFORM *trans,
float x, float y, float z, float angle)
{
double s = sin(angle);
double c = cos(angle);
double cc = 1 - c;
ALLEGRO_TRANSFORM tmp;

al_identity_transform(&tmp);

tmp.m[0][0] = (cc * x * x) + c;
tmp.m[0][1] = (cc * x * y) + (z * s);
tmp.m[0][2] = (cc * x * z) - (y * s);
tmp.m[0][3] = 0;

tmp.m[1][0] = (cc * x * y) - (z * s);
tmp.m[1][1] = (cc * y * y) + c;
tmp.m[1][2] = (cc * z * y) + (x * s);
tmp.m[1][3] = 0;

tmp.m[2][0] = (cc * x * z) + (y * s);
tmp.m[2][1] = (cc * y * z) - (x * s);
tmp.m[2][2] = (cc * z * z) + c;
tmp.m[2][3] = 0;

tmp.m[3][0] = 0;
tmp.m[3][1] = 0;
tmp.m[3][2] = 0;
tmp.m[3][3] = 1;

al_compose_transform(trans, &tmp);
}




void al_perspective_transform(ALLEGRO_TRANSFORM *trans,
float left, float top, float n,
float right, float bottom, float f)
{
float delta_x = right - left;
float delta_y = top - bottom;
float delta_z = f - n;
ALLEGRO_TRANSFORM tmp;

al_identity_transform(&tmp);

tmp.m[0][0] = 2.0f * n / delta_x;
tmp.m[1][1] = 2.0f * n / delta_y;
tmp.m[2][0] = (right + left) / delta_x;
tmp.m[2][1] = (top + bottom) / delta_y;
tmp.m[2][2] = -(f + n) / delta_z;
tmp.m[2][3] = -1.0f;
tmp.m[3][2] = -2.0f * f * n / delta_z;
tmp.m[3][3] = 0;

al_compose_transform(trans, &tmp);
}



void al_horizontal_shear_transform(ALLEGRO_TRANSFORM* trans, float theta)
{
float s;
ASSERT(trans);
s = -tanf(theta);

trans->m[0][0] += trans->m[0][1] * s;
trans->m[1][0] += trans->m[1][1] * s;
trans->m[3][0] += trans->m[3][1] * s;
}




void al_vertical_shear_transform(ALLEGRO_TRANSFORM* trans, float theta)
{
float s;
ASSERT(trans);
s = tanf(theta);

trans->m[0][1] += trans->m[0][0] * s;
trans->m[1][1] += trans->m[1][0] * s;
trans->m[3][1] += trans->m[3][0] * s;
}



