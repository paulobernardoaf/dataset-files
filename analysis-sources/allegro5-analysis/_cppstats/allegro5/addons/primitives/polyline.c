#include "allegro5/allegro.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/internal/aintern_list.h"
#include "allegro5/internal/aintern_prim.h"
#include <float.h>
#include <math.h>
static float compute_direction_and_normal(const float* begin, const float* end, float* dir, float* normal)
{
float length;
dir[0] = end[0] - begin[0];
dir[1] = end[1] - begin[1];
length = _al_prim_normalize(dir);
normal[0] = -dir[1];
normal[1] = dir[0];
return length;
}
static void compute_end_cross_points(const float* v0, const float* v1, float radius, float* p0, float* p1)
{
float dir[2];
float normal[2];
(void)radius;
compute_direction_and_normal(v0, v1, dir, normal);
p0[0] = v1[0] + normal[0] * radius;
p0[1] = v1[1] + normal[1] * radius;
p1[0] = v1[0] - normal[0] * radius;
p1[1] = v1[1] - normal[1] * radius;
}
static void compute_cross_points(const float* v0, const float* v1, const float* v2, float radius,
float* l0, float* l1, float* r0, float* r1, float* out_middle, float* out_angle, float* out_miter_distance)
{
float normal_0[2], normal_1[2];
float dir_0[2], dir_1[2];
float middle[2];
float diff[2];
float miter_distance;
float angle;
float len_0, len_1;
bool sharp = false;
len_0 = compute_direction_and_normal(v0, v1, dir_0, normal_0);
len_1 = compute_direction_and_normal(v1, v2, dir_1, normal_1);
diff[0] = dir_0[0] * dir_1[0] + dir_0[1] * dir_1[1];
diff[1] = -(dir_0[0] * dir_1[1] - dir_0[1] * dir_1[0]);
angle = (diff[0] || diff[1]) ? atan2f(diff[1], diff[0]) : 0.0f;
miter_distance = angle != 0.0f ? radius / cosf(fabsf(angle) * 0.5f) : radius;
if (miter_distance < 0) {
sharp = true;
miter_distance = 0;
}
if (miter_distance > len_0) {
sharp = true;
miter_distance = len_0;
}
if(miter_distance > len_1) {
sharp = true;
miter_distance = len_1;
}
middle[0] = normal_0[0] + normal_1[0];
middle[1] = normal_0[1] + normal_1[1];
if (middle[0] == middle[1] && middle[0] == 0.0) {
middle[0] = dir_1[0];
middle[1] = dir_1[1];
}
_al_prim_normalize(middle);
if (angle > 0.0f)
{
l0[0] = v1[0] + normal_0[0] * radius;
l0[1] = v1[1] + normal_0[1] * radius;
r0[0] = v1[0] + normal_1[0] * radius;
r0[1] = v1[1] + normal_1[1] * radius;
if (sharp) {
l1[0] = v1[0] - normal_0[0] * radius;
l1[1] = v1[1] - normal_0[1] * radius;
r1[0] = v1[0] - normal_1[0] * radius;
r1[1] = v1[1] - normal_1[1] * radius;
}
else {
l1[0] = r1[0] = v1[0] - middle[0] * miter_distance;
l1[1] = r1[1] = v1[1] - middle[1] * miter_distance;
}
}
else
{
middle[0] = -middle[0];
middle[1] = -middle[1];
l1[0] = v1[0] - normal_0[0] * radius;
l1[1] = v1[1] - normal_0[1] * radius;
r1[0] = v1[0] - normal_1[0] * radius;
r1[1] = v1[1] - normal_1[1] * radius;
if (sharp) {
l0[0] = v1[0] + normal_0[0] * radius;
l0[1] = v1[1] + normal_0[1] * radius;
r0[0] = v1[0] + normal_1[0] * radius;
r0[1] = v1[1] + normal_1[1] * radius;
}
else {
l0[0] = r0[0] = v1[0] - middle[0] * miter_distance;
l0[1] = r0[1] = v1[1] - middle[1] * miter_distance;
}
}
if (out_angle)
*out_angle = angle;
if (out_miter_distance)
*out_miter_distance = miter_distance;
if (out_middle)
memcpy(out_middle, middle, sizeof(float) * 2);
}
static void emit_arc(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* pivot, float start, float end, float radius, int segments)
{
float arc;
float c, s, t;
float v0[2];
float v1[2];
float cp[2];
int i;
if (fabsf(end - start) < 0.001f)
return;
start = fmodf(start, ALLEGRO_PI * 2.0f);
end = fmodf(end, ALLEGRO_PI * 2.0f);
if (end <= start)
end += ALLEGRO_PI * 2.0f;
arc = end - start;
segments = (int)(segments * arc / ALLEGRO_PI * 2.0f);
if (segments < 1)
segments = 1;
c = cosf(arc / segments);
s = sinf(arc / segments);
cp[0] = cosf(start) * radius;
cp[1] = sinf(start) * radius;
v0[0] = cp[0] + pivot[0];
v0[1] = cp[1] + pivot[1];
for (i = 0; i < segments - 1; ++i)
{
t = cp[0];
cp[0] = c * cp[0] - s * cp[1];
cp[1] = s * t + c * cp[1];
v1[0] = cp[0] + pivot[0];
v1[1] = cp[1] + pivot[1];
_al_prim_cache_push_triangle(cache, v0, pivot, v1);
v0[0] = v1[0];
v0[1] = v1[1];
}
v1[0] = cosf(end) * radius + pivot[0];
v1[1] = sinf(end) * radius + pivot[1];
_al_prim_cache_push_triangle(cache, v0, pivot, v1);
}
static void emit_square_end_cap(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* pivot, const float* dir, const float* normal, float radius)
{
float v0[2] = { pivot[0] + normal[0] * radius, pivot[1] + normal[1] * radius };
float v1[2] = { pivot[0] - normal[0] * radius, pivot[1] - normal[1] * radius };
float v2[2] = { v0[0] + dir[0] * radius, v0[1] + dir[1] * radius };
float v3[2] = { v1[0] + dir[0] * radius, v1[1] + dir[1] * radius };
_al_prim_cache_push_triangle(cache, v0, v2, v3);
_al_prim_cache_push_triangle(cache, v0, v3, v1);
}
static void emit_triange_end_cap(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* pivot, const float* dir, const float* normal, float radius)
{
float v0[2] = { pivot[0] + normal[0] * radius, pivot[1] + normal[1] * radius };
float v1[2] = { pivot[0] - normal[0] * radius, pivot[1] - normal[1] * radius };
float v2[2] = { pivot[0] + dir[0] * radius, pivot[1] + dir[1] * radius };
_al_prim_cache_push_triangle(cache, v0, v2, v1);
}
static void emit_round_end_cap(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* pivot, const float* dir, const float* normal, float radius)
{
float angle = atan2f(-normal[1], -normal[0]);
(void)dir;
(void)radius;
emit_arc(cache, pivot, angle, angle + ALLEGRO_PI, radius, 16);
}
static void emit_end_cap(ALLEGRO_PRIM_VERTEX_CACHE* cache, int cap_style, const float* v0, const float* v1, float radius)
{
float dir[2];
float normal[2];
ASSERT(cap_style != ALLEGRO_LINE_CAP_CLOSED);
if (cap_style == ALLEGRO_LINE_CAP_NONE)
return;
compute_direction_and_normal(v0, v1, dir, normal);
if (cap_style == ALLEGRO_LINE_CAP_SQUARE)
emit_square_end_cap(cache, v1, dir, normal, radius);
else if (cap_style == ALLEGRO_LINE_CAP_TRIANGLE)
emit_triange_end_cap(cache, v1, dir, normal, radius);
else if (cap_style == ALLEGRO_LINE_CAP_ROUND)
emit_round_end_cap(cache, v1, dir, normal, radius);
else {
ASSERT("Unknown or unsupported style of ending cap." && false);
}
}
static void emit_bevel_join(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* pivot, const float* p0, const float* p1)
{
_al_prim_cache_push_triangle(cache, pivot, p0, p1);
}
static void emit_round_join(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* pivot, const float* p0, const float* p1, float radius)
{
float start = atan2f(p1[1] - pivot[1], p1[0] - pivot[0]);
float end = atan2f(p0[1] - pivot[1], p0[0] - pivot[0]);
if (end < start)
end += ALLEGRO_PI * 2.0f;
emit_arc(cache, pivot, start, end, radius, 16);
}
static void emit_miter_join(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* pivot, const float* p0, const float* p1,
float radius, const float* middle, float angle, float miter_distance, float max_miter_distance)
{
(void)radius;
if (miter_distance > max_miter_distance) {
float normal[2] = { -middle[1], middle[0] };
float offset = (miter_distance - max_miter_distance) * tanf((ALLEGRO_PI - fabsf(angle)) * 0.5f);
float v0[2] = {
pivot[0] + middle[0] * max_miter_distance + normal[0] * offset,
pivot[1] + middle[1] * max_miter_distance + normal[1] * offset
};
float v1[2] = {
pivot[0] + middle[0] * max_miter_distance - normal[0] * offset,
pivot[1] + middle[1] * max_miter_distance - normal[1] * offset
};
_al_prim_cache_push_triangle(cache, pivot, v0, v1);
_al_prim_cache_push_triangle(cache, pivot, p0, v0);
_al_prim_cache_push_triangle(cache, pivot, p1, v1);
}
else {
float miter[2] = {
pivot[0] + middle[0] * miter_distance,
pivot[1] + middle[1] * miter_distance,
};
_al_prim_cache_push_triangle(cache, pivot, p0, miter);
_al_prim_cache_push_triangle(cache, pivot, miter, p1);
}
}
static void emit_join(ALLEGRO_PRIM_VERTEX_CACHE* cache, int join_style, const float* pivot,
const float* p0, const float* p1, float radius, const float* middle,
float angle, float miter_distance, float miter_limit)
{
if (join_style == ALLEGRO_LINE_JOIN_NONE)
return;
if (join_style == ALLEGRO_LINE_JOIN_BEVEL)
emit_bevel_join(cache, pivot, p0, p1);
else if (join_style == ALLEGRO_LINE_JOIN_ROUND)
emit_round_join(cache, pivot, p0, p1, radius);
else if (join_style == ALLEGRO_LINE_JOIN_MITER)
emit_miter_join(cache, pivot, p0, p1, radius, middle, angle, miter_distance, miter_limit * radius);
else {
ASSERT("Unknown or unsupported style of join." && false);
}
}
static void emit_polyline(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* vertices, int vertex_stride, int vertex_count, int join_style, int cap_style, float thickness, float miter_limit)
{
#define VERTEX(index) ((const float*)(((uint8_t*)vertices) + vertex_stride * ((vertex_count + (index)) % vertex_count)))
float l0[2], l1[2];
float r0[2], r1[2];
float p0[2], p1[2];
float radius;
int steps;
int i;
ASSERT(thickness > 0.0f);
if (vertex_count < 2)
return;
radius = 0.5f * thickness;
if (vertex_count == 2 && cap_style == ALLEGRO_LINE_CAP_CLOSED)
cap_style = ALLEGRO_LINE_CAP_NONE;
if (cap_style != ALLEGRO_LINE_CAP_CLOSED)
{
emit_end_cap(cache, cap_style, VERTEX(1), VERTEX(0), radius);
emit_end_cap(cache, cap_style, VERTEX(-2), VERTEX(-1), radius);
compute_end_cross_points(VERTEX(1), VERTEX(0), radius, p1, p0);
steps = vertex_count;
}
else
{
compute_cross_points(VERTEX(-1), VERTEX(0), VERTEX(1), radius, l0, l1, p0, p1, NULL, NULL, NULL);
steps = vertex_count + 1;
}
for (i = 1; i < steps; ++i)
{
const float* v0 = VERTEX(i - 1);
const float* v1 = VERTEX(i);
const float* v2 = VERTEX(i + 1);
if ((cap_style == ALLEGRO_LINE_CAP_CLOSED) || (i < steps - 1)) {
float middle[2];
float miter_distance;
float angle;
compute_cross_points(v0, v1, v2, radius, l0, l1, r0, r1, middle, &angle, &miter_distance);
if (angle >= 0.0f)
emit_join(cache, join_style, v1, l0, r0, radius, middle, angle, miter_distance, miter_limit);
else
emit_join(cache, join_style, v1, r1, l1, radius, middle, angle, miter_distance, miter_limit);
}
else
compute_end_cross_points(v0, v1, radius, l0, l1);
_al_prim_cache_push_triangle(cache, v0, v1, l1);
_al_prim_cache_push_triangle(cache, v0, l1, p1);
_al_prim_cache_push_triangle(cache, v0, p0, l0);
_al_prim_cache_push_triangle(cache, v0, l0, v1);
memcpy(p0, r0, sizeof(float) * 2);
memcpy(p1, r1, sizeof(float) * 2);
}
#undef VERTEX
}
static void do_draw_polyline(ALLEGRO_PRIM_VERTEX_CACHE* cache, const float* vertices, int vertex_stride, int vertex_count, int join_style, int cap_style, ALLEGRO_COLOR color, float thickness, float miter_limit)
{
if (thickness > 0.0f)
{
_al_prim_cache_init(cache, ALLEGRO_PRIM_VERTEX_CACHE_TRIANGLE, color);
emit_polyline(cache, vertices, vertex_stride, vertex_count, join_style, cap_style, thickness, miter_limit);
_al_prim_cache_term(cache);
}
else
{
#define VERTEX(index) ((const float*)(((uint8_t*)vertices) + vertex_stride * ((vertex_count + (index)) % vertex_count)))
int i;
_al_prim_cache_init(cache, ALLEGRO_PRIM_VERTEX_CACHE_LINE_STRIP, color);
for (i = 0; i < vertex_count; ++i) {
if (cache->size >= (ALLEGRO_VERTEX_CACHE_SIZE - 2))
_al_prim_cache_flush(cache);
_al_prim_cache_push_point(cache, VERTEX(i));
}
if (cap_style == ALLEGRO_LINE_CAP_CLOSED && vertex_count > 2) {
if (cache->size >= (ALLEGRO_VERTEX_CACHE_SIZE - 2))
_al_prim_cache_flush(cache);
_al_prim_cache_push_point(cache, VERTEX(0));
}
_al_prim_cache_term(cache);
#undef VERTEX
}
}
void al_draw_polyline(const float* vertices, int vertex_stride,
int vertex_count, int join_style, int cap_style,
ALLEGRO_COLOR color, float thickness, float miter_limit)
{
ALLEGRO_PRIM_VERTEX_CACHE cache;
do_draw_polyline(&cache, vertices, vertex_stride, vertex_count, join_style, cap_style, color, thickness, miter_limit);
}
