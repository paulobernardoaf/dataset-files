#include <float.h>
#include <libavutil/lfg.h>
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/mem.h"
#include "libavutil/fifo.h"
#include "libavutil/common.h"
#include "libavutil/avassert.h"
#include "libavutil/pixfmt.h"
#include "avfilter.h"
#include "framequeue.h"
#include "filters.h"
#include "transform.h"
#include "formats.h"
#include "internal.h"
#include "opencl.h"
#include "opencl_source.h"
#include "video.h"
#define BREIFN 512
#define BRIEF_PATCH_SIZE 31
#define BRIEF_PATCH_SIZE_HALF (BRIEF_PATCH_SIZE / 2)
#define MATCHES_CONTIG_SIZE 2000
#define ROUNDED_UP_DIV(a, b) ((a + (b - 1)) / b)
typedef struct PointPair {
cl_float2 p1;
cl_float2 p2;
} PointPair;
typedef struct MotionVector {
PointPair p;
cl_int should_consider;
} MotionVector;
enum RingbufferIndices {
RingbufX,
RingbufY,
RingbufRot,
RingbufScaleX,
RingbufScaleY,
RingbufCount
};
typedef struct DebugMatches {
MotionVector *matches;
MotionVector model_matches[3];
int num_matches;
int num_model_matches;
} DebugMatches;
typedef struct AbsoluteFrameMotion {
AVFifoBuffer *ringbuffers[RingbufCount];
int curr_frame_offset;
int data_start_offset;
int data_end_offset;
AVFifoBuffer *debug_matches;
} AbsoluteFrameMotion;
static void free_debug_matches(AbsoluteFrameMotion *afm) {
DebugMatches dm;
if (!afm->debug_matches) {
return;
}
while (av_fifo_size(afm->debug_matches) > 0) {
av_fifo_generic_read(
afm->debug_matches,
&dm,
sizeof(DebugMatches),
NULL
);
av_freep(&dm.matches);
}
av_fifo_freep(&afm->debug_matches);
}
typedef struct FrameDelta {
cl_float2 translation;
float rotation;
cl_float2 scale;
cl_float2 skew;
} FrameDelta;
typedef struct SimilarityMatrix {
double matrix[6];
} SimilarityMatrix;
typedef struct CropInfo {
cl_float2 top_left;
cl_float2 bottom_right;
} CropInfo;
typedef struct IterIndices {
int start;
int end;
} IterIndices;
typedef struct DeshakeOpenCLContext {
OpenCLFilterContext ocf;
int initialized;
int64_t duration;
int eof;
AVLFG alfg;
FFFrameQueue fq;
AbsoluteFrameMotion abs_motion;
int smooth_window;
int curr_frame;
float *gauss_kernel;
float *ransac_err;
CropInfo crop_y;
CropInfo crop_uv;
int is_yuv;
int sw_format;
MotionVector *matches_host;
MotionVector *matches_contig_host;
MotionVector *inliers;
cl_command_queue command_queue;
cl_kernel kernel_grayscale;
cl_kernel kernel_harris_response;
cl_kernel kernel_refine_features;
cl_kernel kernel_brief_descriptors;
cl_kernel kernel_match_descriptors;
cl_kernel kernel_transform;
cl_kernel kernel_crop_upscale;
cl_mem grayscale;
cl_mem harris_buf;
cl_mem refined_features;
cl_mem prev_refined_features;
cl_mem brief_pattern;
cl_mem descriptors;
cl_mem prev_descriptors;
cl_mem matches;
cl_mem matches_contig;
cl_mem transform_y;
cl_mem transform_uv;
int tripod_mode;
int debug_on;
int should_crop;
cl_int refine_features;
float smooth_percent;
float smooth_window_multiplier;
cl_kernel kernel_draw_debug_info;
cl_mem debug_matches;
cl_mem debug_model_matches;
unsigned long long grayscale_time;
unsigned long long harris_response_time;
unsigned long long refine_features_time;
unsigned long long brief_descriptors_time;
unsigned long long match_descriptors_time;
unsigned long long transform_time;
unsigned long long crop_upscale_time;
unsigned long long read_buf_time;
} DeshakeOpenCLContext;
static int rand_in(int low, int high, AVLFG *alfg) {
return (av_lfg_get(alfg) % (high - low)) + low;
}
static double averaged_event_time_ms(unsigned long long total_time, int num_frames) {
return (double)total_time / (double)num_frames / 1000000.0;
}
static void run_estimate_kernel(const MotionVector *point_pairs, double *model)
{
double x1 = point_pairs[0].p.p1.s[0];
double y1 = point_pairs[0].p.p1.s[1];
double x2 = point_pairs[1].p.p1.s[0];
double y2 = point_pairs[1].p.p1.s[1];
double x3 = point_pairs[2].p.p1.s[0];
double y3 = point_pairs[2].p.p1.s[1];
double X1 = point_pairs[0].p.p2.s[0];
double Y1 = point_pairs[0].p.p2.s[1];
double X2 = point_pairs[1].p.p2.s[0];
double Y2 = point_pairs[1].p.p2.s[1];
double X3 = point_pairs[2].p.p2.s[0];
double Y3 = point_pairs[2].p.p2.s[1];
double d = 1.0 / ( x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2) );
model[0] = d * ( X1*(y2-y3) + X2*(y3-y1) + X3*(y1-y2) );
model[1] = d * ( X1*(x3-x2) + X2*(x1-x3) + X3*(x2-x1) );
model[2] = d * ( X1*(x2*y3 - x3*y2) + X2*(x3*y1 - x1*y3) + X3*(x1*y2 - x2*y1) );
model[3] = d * ( Y1*(y2-y3) + Y2*(y3-y1) + Y3*(y1-y2) );
model[4] = d * ( Y1*(x3-x2) + Y2*(x1-x3) + Y3*(x2-x1) );
model[5] = d * ( Y1*(x2*y3 - x3*y2) + Y2*(x3*y1 - x1*y3) + Y3*(x1*y2 - x2*y1) );
}
static int points_not_collinear(const cl_float2 **points)
{
int j, k, i = 2;
for (j = 0; j < i; j++) {
double dx1 = points[j]->s[0] - points[i]->s[0];
double dy1 = points[j]->s[1] - points[i]->s[1];
for (k = 0; k < j; k++) {
double dx2 = points[k]->s[0] - points[i]->s[0];
double dy2 = points[k]->s[1] - points[i]->s[1];
if (fabs(dx2*dy1 - dy2*dx1) <= 1.0) {
return 0;
}
}
}
return 1;
}
static int check_subset(const MotionVector *pairs_subset)
{
const cl_float2 *prev_points[] = {
&pairs_subset[0].p.p1,
&pairs_subset[1].p.p1,
&pairs_subset[2].p.p1
};
const cl_float2 *curr_points[] = {
&pairs_subset[0].p.p2,
&pairs_subset[1].p.p2,
&pairs_subset[2].p.p2
};
return points_not_collinear(prev_points) && points_not_collinear(curr_points);
}
static int get_subset(
AVLFG *alfg,
const MotionVector *point_pairs,
const int num_point_pairs,
MotionVector *pairs_subset,
int max_attempts
) {
int idx[3];
int i = 0, j, iters = 0;
for (; iters < max_attempts; iters++) {
for (i = 0; i < 3 && iters < max_attempts;) {
int idx_i = 0;
for (;;) {
idx_i = idx[i] = rand_in(0, num_point_pairs, alfg);
for (j = 0; j < i; j++) {
if (idx_i == idx[j]) {
break;
}
}
if (j == i) {
break;
}
}
pairs_subset[i] = point_pairs[idx[i]];
i++;
}
if (i == 3 && !check_subset(pairs_subset)) {
continue;
}
break;
}
return i == 3 && iters < max_attempts;
}
static void compute_error(
const MotionVector *point_pairs,
const int num_point_pairs,
const double *model,
float *err
) {
double F0 = model[0], F1 = model[1], F2 = model[2];
double F3 = model[3], F4 = model[4], F5 = model[5];
for (int i = 0; i < num_point_pairs; i++) {
const cl_float2 *f = &point_pairs[i].p.p1;
const cl_float2 *t = &point_pairs[i].p.p2;
double a = F0*f->s[0] + F1*f->s[1] + F2 - t->s[0];
double b = F3*f->s[0] + F4*f->s[1] + F5 - t->s[1];
err[i] = a*a + b*b;
}
}
static int find_inliers(
MotionVector *point_pairs,
const int num_point_pairs,
const double *model,
float *err,
double thresh
) {
float t = (float)(thresh * thresh);
int i, n = num_point_pairs, num_inliers = 0;
compute_error(point_pairs, num_point_pairs, model, err);
for (i = 0; i < n; i++) {
if (err[i] <= t) {
point_pairs[i].should_consider = 1;
num_inliers += 1;
} else {
point_pairs[i].should_consider = 0;
}
}
return num_inliers;
}
static int ransac_update_num_iters(double confidence, double num_outliers, int max_iters)
{
double num, denom;
confidence = av_clipd(confidence, 0.0, 1.0);
num_outliers = av_clipd(num_outliers, 0.0, 1.0);
num = FFMAX(1.0 - confidence, DBL_MIN);
denom = 1.0 - pow(1.0 - num_outliers, 3);
if (denom < DBL_MIN) {
return 0;
}
num = log(num);
denom = log(denom);
return denom >= 0 || -num >= max_iters * (-denom) ? max_iters : (int)round(num / denom);
}
static int estimate_affine_2d(
DeshakeOpenCLContext *deshake_ctx,
MotionVector *point_pairs,
DebugMatches *debug_matches,
const int num_point_pairs,
double *model_out,
const double threshold,
const int max_iters,
const double confidence
) {
int result = 0;
double best_model[6], model[6];
MotionVector pairs_subset[3], best_pairs[3];
int iter, niters = FFMAX(max_iters, 1);
int good_count, max_good_count = 0;
if (num_point_pairs < 3) {
return 0;
} else if (num_point_pairs == 3) {
run_estimate_kernel(point_pairs, model_out);
for (int i = 0; i < 3; ++i) {
point_pairs[i].should_consider = 1;
}
return 1;
}
for (iter = 0; iter < niters; ++iter) {
int found = get_subset(&deshake_ctx->alfg, point_pairs, num_point_pairs, pairs_subset, 10000);
if (!found) {
if (iter == 0) {
return 0;
}
break;
}
run_estimate_kernel(pairs_subset, model);
good_count = find_inliers(point_pairs, num_point_pairs, model, deshake_ctx->ransac_err, threshold);
if (good_count > FFMAX(max_good_count, 2)) {
for (int mi = 0; mi < 6; ++mi) {
best_model[mi] = model[mi];
}
for (int pi = 0; pi < 3; pi++) {
best_pairs[pi] = pairs_subset[pi];
}
max_good_count = good_count;
niters = ransac_update_num_iters(
confidence,
(double)(num_point_pairs - good_count) / num_point_pairs,
niters
);
}
}
if (max_good_count > 0) {
for (int mi = 0; mi < 6; ++mi) {
model_out[mi] = best_model[mi];
}
for (int pi = 0; pi < 3; ++pi) {
debug_matches->model_matches[pi] = best_pairs[pi];
}
debug_matches->num_model_matches = 3;
find_inliers(point_pairs, num_point_pairs, best_model, deshake_ctx->ransac_err, threshold);
result = 1;
}
return result;
}
static void optimize_model(
DeshakeOpenCLContext *deshake_ctx,
MotionVector *best_pairs,
MotionVector *inliers,
const int num_inliers,
float best_err,
double *model_out
) {
float move_x_val = 0.01;
float move_y_val = 0.01;
int move_x = 1;
float old_move_x_val = 0;
double model[6];
int last_changed = 0;
for (int iters = 0; iters < 200; iters++) {
float total_err = 0;
if (move_x) {
best_pairs[0].p.p2.s[0] += move_x_val;
} else {
best_pairs[0].p.p2.s[0] += move_y_val;
}
run_estimate_kernel(best_pairs, model);
compute_error(inliers, num_inliers, model, deshake_ctx->ransac_err);
for (int j = 0; j < num_inliers; j++) {
total_err += deshake_ctx->ransac_err[j];
}
if (total_err < best_err) {
for (int mi = 0; mi < 6; ++mi) {
model_out[mi] = model[mi];
}
best_err = total_err;
last_changed = iters;
} else {
if (move_x) {
best_pairs[0].p.p2.s[0] -= move_x_val;
} else {
best_pairs[0].p.p2.s[0] -= move_y_val;
}
if (iters - last_changed > 4) {
break;
}
old_move_x_val = move_x_val;
if (move_x) {
move_x_val *= -1;
} else {
move_y_val *= -1;
}
if (old_move_x_val < 0) {
move_x = 0;
} else {
move_x = 1;
}
}
}
}
static int minimize_error(
DeshakeOpenCLContext *deshake_ctx,
MotionVector *inliers,
DebugMatches *debug_matches,
const int num_inliers,
double *model_out,
const int max_iters
) {
int result = 0;
float best_err = FLT_MAX;
double best_model[6], model[6];
MotionVector pairs_subset[3], best_pairs[3];
for (int i = 0; i < max_iters; i++) {
float total_err = 0;
int found = get_subset(&deshake_ctx->alfg, inliers, num_inliers, pairs_subset, 10000);
if (!found) {
if (i == 0) {
return 0;
}
break;
}
run_estimate_kernel(pairs_subset, model);
compute_error(inliers, num_inliers, model, deshake_ctx->ransac_err);
for (int j = 0; j < num_inliers; j++) {
total_err += deshake_ctx->ransac_err[j];
}
if (total_err < best_err) {
for (int mi = 0; mi < 6; ++mi) {
best_model[mi] = model[mi];
}
for (int pi = 0; pi < 3; pi++) {
best_pairs[pi] = pairs_subset[pi];
}
best_err = total_err;
}
}
for (int mi = 0; mi < 6; ++mi) {
model_out[mi] = best_model[mi];
}
for (int pi = 0; pi < 3; ++pi) {
debug_matches->model_matches[pi] = best_pairs[pi];
}
debug_matches->num_model_matches = 3;
result = 1;
optimize_model(deshake_ctx, best_pairs, inliers, num_inliers, best_err, model_out);
return result;
}
static FrameDelta decompose_transform(double *model)
{
FrameDelta ret;
double a = model[0];
double c = model[1];
double e = model[2];
double b = model[3];
double d = model[4];
double f = model[5];
double delta = a * d - b * c;
memset(&ret, 0, sizeof(ret));
ret.translation.s[0] = e;
ret.translation.s[1] = f;
if (a != 0 || b != 0) {
double r = hypot(a, b);
ret.rotation = FFSIGN(b) * acos(a / r);
ret.scale.s[0] = r;
ret.scale.s[1] = delta / r;
ret.skew.s[0] = atan((a * c + b * d) / (r * r));
ret.skew.s[1] = 0;
} else if (c != 0 || d != 0) {
double s = sqrt(c * c + d * d);
ret.rotation = M_PI / 2 - FFSIGN(d) * acos(-c / s);
ret.scale.s[0] = delta / s;
ret.scale.s[1] = s;
ret.skew.s[0] = 0;
ret.skew.s[1] = atan((a * c + b * d) / (s * s));
} 
return ret;
}
static int make_vectors_contig(
DeshakeOpenCLContext *deshake_ctx,
int size_y,
int size_x
) {
int num_vectors = 0;
for (int i = 0; i < size_y; ++i) {
for (int j = 0; j < size_x; ++j) {
MotionVector v = deshake_ctx->matches_host[j + i * size_x];
if (v.should_consider) {
deshake_ctx->matches_contig_host[num_vectors] = v;
++num_vectors;
}
if (num_vectors == MATCHES_CONTIG_SIZE - 1) {
return num_vectors;
}
}
}
return num_vectors;
}
static float gaussian_for(int x, float sigma) {
return 1.0f / expf(((float)x * (float)x) / (2.0f * sigma * sigma));
}
static void make_gauss_kernel(float *gauss_kernel, float length, float sigma)
{
float gauss_sum = 0;
int window_half = length / 2;
for (int i = 0; i < length; ++i) {
float val = gaussian_for(i - window_half, sigma);
gauss_sum += val;
gauss_kernel[i] = val;
}
for (int i = 0; i < length; ++i) {
gauss_kernel[i] /= gauss_sum;
}
}
static IterIndices start_end_for(DeshakeOpenCLContext *deshake_ctx, int length) {
IterIndices indices;
indices.start = deshake_ctx->abs_motion.curr_frame_offset - (length / 2);
indices.end = deshake_ctx->abs_motion.curr_frame_offset + (length / 2) + (length % 2);
return indices;
}
static void ringbuf_float_at(
DeshakeOpenCLContext *deshake_ctx,
AVFifoBuffer *values,
float *val,
int offset
) {
int clip_start, clip_end, offset_clipped;
if (deshake_ctx->abs_motion.data_end_offset != -1) {
clip_end = deshake_ctx->abs_motion.data_end_offset;
} else {
clip_end = deshake_ctx->smooth_window - (av_fifo_space(values) / sizeof(float)) - 1;
}
if (deshake_ctx->abs_motion.data_start_offset != -1) {
clip_start = deshake_ctx->abs_motion.data_start_offset;
} else {
clip_start = 0;
}
offset_clipped = av_clip(
offset,
clip_start,
clip_end
);
av_fifo_generic_peek_at(
values,
val,
offset_clipped * sizeof(float),
sizeof(float),
NULL
);
}
static float smooth(
DeshakeOpenCLContext *deshake_ctx,
float *gauss_kernel,
int length,
float max_val,
AVFifoBuffer *values
) {
float new_large_s = 0, new_small_s = 0, new_best = 0, old, diff_between,
percent_of_max, inverted_percent;
IterIndices indices = start_end_for(deshake_ctx, length);
float large_sigma = 40.0f;
float small_sigma = 2.0f;
float best_sigma;
if (deshake_ctx->smooth_percent) {
best_sigma = (large_sigma - 0.5f) * deshake_ctx->smooth_percent + 0.5f;
} else {
make_gauss_kernel(gauss_kernel, length, large_sigma);
for (int i = indices.start, j = 0; i < indices.end; ++i, ++j) {
ringbuf_float_at(deshake_ctx, values, &old, i);
new_large_s += old * gauss_kernel[j];
}
make_gauss_kernel(gauss_kernel, length, small_sigma);
for (int i = indices.start, j = 0; i < indices.end; ++i, ++j) {
ringbuf_float_at(deshake_ctx, values, &old, i);
new_small_s += old * gauss_kernel[j];
}
diff_between = fabsf(new_large_s - new_small_s);
percent_of_max = diff_between / max_val;
inverted_percent = 1 - percent_of_max;
best_sigma = large_sigma * powf(inverted_percent, 40);
}
make_gauss_kernel(gauss_kernel, length, best_sigma);
for (int i = indices.start, j = 0; i < indices.end; ++i, ++j) {
ringbuf_float_at(deshake_ctx, values, &old, i);
new_best += old * gauss_kernel[j];
}
return new_best;
}
static cl_float2 transformed_point(float x, float y, float *transform) {
cl_float2 ret;
ret.s[0] = x * transform[0] + y * transform[1] + transform[2];
ret.s[1] = x * transform[3] + y * transform[4] + transform[5];
return ret;
}
static void transform_center_scale(
float x_shift,
float y_shift,
float angle,
float scale_x,
float scale_y,
float center_w,
float center_h,
float *matrix
) {
cl_float2 center_s;
float center_s_w, center_s_h;
ff_get_matrix(
0,
0,
0,
scale_x,
scale_y,
matrix
);
center_s = transformed_point(center_w, center_h, matrix);
center_s_w = center_w - center_s.s[0];
center_s_h = center_h - center_s.s[1];
ff_get_matrix(
x_shift + center_s_w,
y_shift + center_s_h,
angle,
scale_x,
scale_y,
matrix
);
}
static void update_needed_crop(
CropInfo* crop,
float *transform,
float frame_width,
float frame_height
) {
float new_width, new_height, adjusted_width, adjusted_height, adjusted_x, adjusted_y;
cl_float2 top_left = transformed_point(0, 0, transform);
cl_float2 top_right = transformed_point(frame_width, 0, transform);
cl_float2 bottom_left = transformed_point(0, frame_height, transform);
cl_float2 bottom_right = transformed_point(frame_width, frame_height, transform);
float ar_h = frame_height / frame_width;
float ar_w = frame_width / frame_height;
if (crop->bottom_right.s[0] == 0) {
crop->bottom_right.s[0] = frame_width;
crop->bottom_right.s[1] = frame_height;
}
crop->top_left.s[0] = FFMAX3(
crop->top_left.s[0],
top_left.s[0],
bottom_left.s[0]
);
crop->top_left.s[1] = FFMAX3(
crop->top_left.s[1],
top_left.s[1],
top_right.s[1]
);
crop->bottom_right.s[0] = FFMIN3(
crop->bottom_right.s[0],
bottom_right.s[0],
top_right.s[0]
);
crop->bottom_right.s[1] = FFMIN3(
crop->bottom_right.s[1],
bottom_right.s[1],
bottom_left.s[1]
);
new_height = crop->bottom_right.s[1] - crop->top_left.s[1];
new_width = crop->bottom_right.s[0] - crop->top_left.s[0];
adjusted_width = new_height * ar_w;
adjusted_x = crop->bottom_right.s[0] - adjusted_width;
if (adjusted_x >= crop->top_left.s[0]) {
crop->top_left.s[0] = adjusted_x;
} else {
adjusted_height = new_width * ar_h;
adjusted_y = crop->bottom_right.s[1] - adjusted_height;
crop->top_left.s[1] = adjusted_y;
}
}
static av_cold void deshake_opencl_uninit(AVFilterContext *avctx)
{
DeshakeOpenCLContext *ctx = avctx->priv;
cl_int cle;
for (int i = 0; i < RingbufCount; i++)
av_fifo_freep(&ctx->abs_motion.ringbuffers[i]);
if (ctx->debug_on)
free_debug_matches(&ctx->abs_motion);
if (ctx->gauss_kernel)
av_freep(&ctx->gauss_kernel);
if (ctx->ransac_err)
av_freep(&ctx->ransac_err);
if (ctx->matches_host)
av_freep(&ctx->matches_host);
if (ctx->matches_contig_host)
av_freep(&ctx->matches_contig_host);
if (ctx->inliers)
av_freep(&ctx->inliers);
ff_framequeue_free(&ctx->fq);
CL_RELEASE_KERNEL(ctx->kernel_grayscale);
CL_RELEASE_KERNEL(ctx->kernel_harris_response);
CL_RELEASE_KERNEL(ctx->kernel_refine_features);
CL_RELEASE_KERNEL(ctx->kernel_brief_descriptors);
CL_RELEASE_KERNEL(ctx->kernel_match_descriptors);
CL_RELEASE_KERNEL(ctx->kernel_crop_upscale);
if (ctx->debug_on)
CL_RELEASE_KERNEL(ctx->kernel_draw_debug_info);
CL_RELEASE_QUEUE(ctx->command_queue);
if (!ctx->is_yuv)
CL_RELEASE_MEMORY(ctx->grayscale);
CL_RELEASE_MEMORY(ctx->harris_buf);
CL_RELEASE_MEMORY(ctx->refined_features);
CL_RELEASE_MEMORY(ctx->prev_refined_features);
CL_RELEASE_MEMORY(ctx->brief_pattern);
CL_RELEASE_MEMORY(ctx->descriptors);
CL_RELEASE_MEMORY(ctx->prev_descriptors);
CL_RELEASE_MEMORY(ctx->matches);
CL_RELEASE_MEMORY(ctx->matches_contig);
CL_RELEASE_MEMORY(ctx->transform_y);
CL_RELEASE_MEMORY(ctx->transform_uv);
if (ctx->debug_on) {
CL_RELEASE_MEMORY(ctx->debug_matches);
CL_RELEASE_MEMORY(ctx->debug_model_matches);
}
ff_opencl_filter_uninit(avctx);
}
static int deshake_opencl_init(AVFilterContext *avctx)
{
DeshakeOpenCLContext *ctx = avctx->priv;
AVFilterLink *outlink = avctx->outputs[0];
AVFilterLink *inlink = avctx->inputs[0];
PointPair *pattern_host = NULL;
cl_int cle;
int err;
cl_ulong8 zeroed_ulong8;
FFFrameQueueGlobal fqg;
cl_image_format grayscale_format;
cl_image_desc grayscale_desc;
cl_command_queue_properties queue_props;
const enum AVPixelFormat disallowed_formats[14] = {
AV_PIX_FMT_GBRP,
AV_PIX_FMT_GBRP9BE,
AV_PIX_FMT_GBRP9LE,
AV_PIX_FMT_GBRP10BE,
AV_PIX_FMT_GBRP10LE,
AV_PIX_FMT_GBRP16BE,
AV_PIX_FMT_GBRP16LE,
AV_PIX_FMT_GBRAP,
AV_PIX_FMT_GBRAP16BE,
AV_PIX_FMT_GBRAP16LE,
AV_PIX_FMT_GBRAP12BE,
AV_PIX_FMT_GBRAP12LE,
AV_PIX_FMT_GBRAP10BE,
AV_PIX_FMT_GBRAP10LE
};
const int image_grid_32 = ROUNDED_UP_DIV(outlink->h, 32) * ROUNDED_UP_DIV(outlink->w, 32);
const int descriptor_buf_size = image_grid_32 * (BREIFN / 8);
const int features_buf_size = image_grid_32 * sizeof(cl_float2);
const AVHWFramesContext *hw_frames_ctx = (AVHWFramesContext*)inlink->hw_frames_ctx->data;
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(hw_frames_ctx->sw_format);
av_assert0(hw_frames_ctx);
av_assert0(desc);
ff_framequeue_global_init(&fqg);
ff_framequeue_init(&ctx->fq, &fqg);
ctx->eof = 0;
ctx->smooth_window = (int)(av_q2d(avctx->inputs[0]->frame_rate) * ctx->smooth_window_multiplier);
ctx->curr_frame = 0;
memset(&zeroed_ulong8, 0, sizeof(cl_ulong8));
ctx->gauss_kernel = av_malloc_array(ctx->smooth_window, sizeof(float));
if (!ctx->gauss_kernel) {
err = AVERROR(ENOMEM);
goto fail;
}
ctx->ransac_err = av_malloc_array(MATCHES_CONTIG_SIZE, sizeof(float));
if (!ctx->ransac_err) {
err = AVERROR(ENOMEM);
goto fail;
}
for (int i = 0; i < RingbufCount; i++) {
ctx->abs_motion.ringbuffers[i] = av_fifo_alloc_array(
ctx->smooth_window,
sizeof(float)
);
if (!ctx->abs_motion.ringbuffers[i]) {
err = AVERROR(ENOMEM);
goto fail;
}
}
if (ctx->debug_on) {
ctx->abs_motion.debug_matches = av_fifo_alloc_array(
ctx->smooth_window / 2,
sizeof(DebugMatches)
);
if (!ctx->abs_motion.debug_matches) {
err = AVERROR(ENOMEM);
goto fail;
}
}
ctx->abs_motion.curr_frame_offset = 0;
ctx->abs_motion.data_start_offset = -1;
ctx->abs_motion.data_end_offset = -1;
pattern_host = av_malloc_array(BREIFN, sizeof(PointPair));
if (!pattern_host) {
err = AVERROR(ENOMEM);
goto fail;
}
ctx->matches_host = av_malloc_array(image_grid_32, sizeof(MotionVector));
if (!ctx->matches_host) {
err = AVERROR(ENOMEM);
goto fail;
}
ctx->matches_contig_host = av_malloc_array(MATCHES_CONTIG_SIZE, sizeof(MotionVector));
if (!ctx->matches_contig_host) {
err = AVERROR(ENOMEM);
goto fail;
}
ctx->inliers = av_malloc_array(MATCHES_CONTIG_SIZE, sizeof(MotionVector));
if (!ctx->inliers) {
err = AVERROR(ENOMEM);
goto fail;
}
av_lfg_init(&ctx->alfg, 234342424);
for (int i = 0; i < BREIFN; ++i) {
PointPair pair;
for (int j = 0; j < 2; ++j) {
pair.p1.s[j] = rand_in(-BRIEF_PATCH_SIZE_HALF, BRIEF_PATCH_SIZE_HALF + 1, &ctx->alfg);
pair.p2.s[j] = rand_in(-BRIEF_PATCH_SIZE_HALF, BRIEF_PATCH_SIZE_HALF + 1, &ctx->alfg);
}
pattern_host[i] = pair;
}
for (int i = 0; i < 14; i++) {
if (ctx->sw_format == disallowed_formats[i]) {
av_log(avctx, AV_LOG_ERROR, "unsupported format in deshake_opencl.\n");
err = AVERROR(ENOSYS);
goto fail;
}
}
if (desc->flags & AV_PIX_FMT_FLAG_RGB) {
ctx->is_yuv = 0;
} else {
ctx->is_yuv = 1;
}
ctx->sw_format = hw_frames_ctx->sw_format;
err = ff_opencl_filter_load_program(avctx, &ff_opencl_source_deshake, 1);
if (err < 0)
goto fail;
if (ctx->debug_on) {
queue_props = CL_QUEUE_PROFILING_ENABLE;
} else {
queue_props = 0;
}
ctx->command_queue = clCreateCommandQueue(
ctx->ocf.hwctx->context,
ctx->ocf.hwctx->device_id,
queue_props,
&cle
);
CL_FAIL_ON_ERROR(AVERROR(EIO), "Failed to create OpenCL command queue %d.\n", cle);
CL_CREATE_KERNEL(ctx, grayscale);
CL_CREATE_KERNEL(ctx, harris_response);
CL_CREATE_KERNEL(ctx, refine_features);
CL_CREATE_KERNEL(ctx, brief_descriptors);
CL_CREATE_KERNEL(ctx, match_descriptors);
CL_CREATE_KERNEL(ctx, transform);
CL_CREATE_KERNEL(ctx, crop_upscale);
if (ctx->debug_on)
CL_CREATE_KERNEL(ctx, draw_debug_info);
if (!ctx->is_yuv) {
grayscale_format.image_channel_order = CL_R;
grayscale_format.image_channel_data_type = CL_FLOAT;
grayscale_desc = (cl_image_desc) {
.image_type = CL_MEM_OBJECT_IMAGE2D,
.image_width = outlink->w,
.image_height = outlink->h,
.image_depth = 0,
.image_array_size = 0,
.image_row_pitch = 0,
.image_slice_pitch = 0,
.num_mip_levels = 0,
.num_samples = 0,
.buffer = NULL,
};
ctx->grayscale = clCreateImage(
ctx->ocf.hwctx->context,
0,
&grayscale_format,
&grayscale_desc,
NULL,
&cle
);
CL_FAIL_ON_ERROR(AVERROR(EIO), "Failed to create grayscale image: %d.\n", cle);
}
CL_CREATE_BUFFER(ctx, harris_buf, outlink->h * outlink->w * sizeof(float));
CL_CREATE_BUFFER(ctx, refined_features, features_buf_size);
CL_CREATE_BUFFER(ctx, prev_refined_features, features_buf_size);
CL_CREATE_BUFFER_FLAGS(
ctx,
brief_pattern,
CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
BREIFN * sizeof(PointPair),
pattern_host
);
CL_CREATE_BUFFER(ctx, descriptors, descriptor_buf_size);
CL_CREATE_BUFFER(ctx, prev_descriptors, descriptor_buf_size);
CL_CREATE_BUFFER(ctx, matches, image_grid_32 * sizeof(MotionVector));
CL_CREATE_BUFFER(ctx, matches_contig, MATCHES_CONTIG_SIZE * sizeof(MotionVector));
CL_CREATE_BUFFER(ctx, transform_y, 9 * sizeof(float));
CL_CREATE_BUFFER(ctx, transform_uv, 9 * sizeof(float));
if (ctx->debug_on) {
CL_CREATE_BUFFER(ctx, debug_matches, MATCHES_CONTIG_SIZE * sizeof(MotionVector));
CL_CREATE_BUFFER(ctx, debug_model_matches, 3 * sizeof(MotionVector));
}
ctx->initialized = 1;
av_freep(&pattern_host);
return 0;
fail:
av_freep(&pattern_host);
return err;
}
static void transform_debug(AVFilterContext *avctx, float *new_vals, float *old_vals, int curr_frame) {
av_log(avctx, AV_LOG_VERBOSE,
"Frame %d:\n"
"\tframe moved from: %f x, %f y\n"
"\t to: %f x, %f y\n"
"\t rotated from: %f degrees\n"
"\t to: %f degrees\n"
"\t scaled from: %f x, %f y\n"
"\t to: %f x, %f y\n"
"\n"
"\tframe moved by: %f x, %f y\n"
"\t rotated by: %f degrees\n"
"\t scaled by: %f x, %f y\n",
curr_frame,
old_vals[RingbufX], old_vals[RingbufY],
new_vals[RingbufX], new_vals[RingbufY],
old_vals[RingbufRot] * (180.0 / M_PI),
new_vals[RingbufRot] * (180.0 / M_PI),
old_vals[RingbufScaleX], old_vals[RingbufScaleY],
new_vals[RingbufScaleX], new_vals[RingbufScaleY],
old_vals[RingbufX] - new_vals[RingbufX], old_vals[RingbufY] - new_vals[RingbufY],
old_vals[RingbufRot] * (180.0 / M_PI) - new_vals[RingbufRot] * (180.0 / M_PI),
new_vals[RingbufScaleX] / old_vals[RingbufScaleX], new_vals[RingbufScaleY] / old_vals[RingbufScaleY]
);
}
static int filter_frame(AVFilterLink *link, AVFrame *input_frame)
{
AVFilterContext *avctx = link->dst;
AVFilterLink *outlink = avctx->outputs[0];
DeshakeOpenCLContext *deshake_ctx = avctx->priv;
AVFrame *cropped_frame = NULL, *transformed_frame = NULL;
int err;
cl_int cle;
float new_vals[RingbufCount];
float old_vals[RingbufCount];
float transform_y[9];
float transform_uv[9];
float transform_crop_y[9];
float transform_crop_uv[9];
float transform_debug_rgb[9];
size_t global_work[2];
int64_t duration;
cl_mem src, transformed, dst;
cl_mem transforms[3];
CropInfo crops[3];
cl_event transform_event, crop_upscale_event;
DebugMatches debug_matches;
cl_int num_model_matches;
const float center_w = (float)input_frame->width / 2;
const float center_h = (float)input_frame->height / 2;
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(deshake_ctx->sw_format);
const int chroma_width = AV_CEIL_RSHIFT(input_frame->width, desc->log2_chroma_w);
const int chroma_height = AV_CEIL_RSHIFT(input_frame->height, desc->log2_chroma_h);
const float center_w_chroma = (float)chroma_width / 2;
const float center_h_chroma = (float)chroma_height / 2;
const float luma_w_over_chroma_w = ((float)input_frame->width / (float)chroma_width);
const float luma_h_over_chroma_h = ((float)input_frame->height / (float)chroma_height);
if (deshake_ctx->debug_on) {
av_fifo_generic_read(
deshake_ctx->abs_motion.debug_matches,
&debug_matches,
sizeof(DebugMatches),
NULL
);
}
if (input_frame->pkt_duration) {
duration = input_frame->pkt_duration;
} else {
duration = av_rescale_q(1, av_inv_q(outlink->frame_rate), outlink->time_base);
}
deshake_ctx->duration = input_frame->pts + duration;
for (int i = 0; i < RingbufCount; i++) {
av_fifo_generic_peek_at(
deshake_ctx->abs_motion.ringbuffers[i],
&old_vals[i],
deshake_ctx->abs_motion.curr_frame_offset * sizeof(float),
sizeof(float),
NULL
);
}
if (deshake_ctx->tripod_mode) {
new_vals[RingbufX] = 0.0f;
new_vals[RingbufY] = 0.0f;
new_vals[RingbufRot] = 0.0f;
new_vals[RingbufScaleX] = 1.0f;
new_vals[RingbufScaleY] = 1.0f;
} else {
new_vals[RingbufX] = smooth(
deshake_ctx,
deshake_ctx->gauss_kernel,
deshake_ctx->smooth_window,
input_frame->width,
deshake_ctx->abs_motion.ringbuffers[RingbufX]
);
new_vals[RingbufY] = smooth(
deshake_ctx,
deshake_ctx->gauss_kernel,
deshake_ctx->smooth_window,
input_frame->height,
deshake_ctx->abs_motion.ringbuffers[RingbufY]
);
new_vals[RingbufRot] = smooth(
deshake_ctx,
deshake_ctx->gauss_kernel,
deshake_ctx->smooth_window,
M_PI / 4,
deshake_ctx->abs_motion.ringbuffers[RingbufRot]
);
new_vals[RingbufScaleX] = smooth(
deshake_ctx,
deshake_ctx->gauss_kernel,
deshake_ctx->smooth_window,
2.0f,
deshake_ctx->abs_motion.ringbuffers[RingbufScaleX]
);
new_vals[RingbufScaleY] = smooth(
deshake_ctx,
deshake_ctx->gauss_kernel,
deshake_ctx->smooth_window,
2.0f,
deshake_ctx->abs_motion.ringbuffers[RingbufScaleY]
);
}
transform_center_scale(
old_vals[RingbufX] - new_vals[RingbufX],
old_vals[RingbufY] - new_vals[RingbufY],
old_vals[RingbufRot] - new_vals[RingbufRot],
new_vals[RingbufScaleX] / old_vals[RingbufScaleX],
new_vals[RingbufScaleY] / old_vals[RingbufScaleY],
center_w,
center_h,
transform_y
);
transform_center_scale(
(old_vals[RingbufX] - new_vals[RingbufX]) / luma_w_over_chroma_w,
(old_vals[RingbufY] - new_vals[RingbufY]) / luma_h_over_chroma_h,
old_vals[RingbufRot] - new_vals[RingbufRot],
new_vals[RingbufScaleX] / old_vals[RingbufScaleX],
new_vals[RingbufScaleY] / old_vals[RingbufScaleY],
center_w_chroma,
center_h_chroma,
transform_uv
);
CL_BLOCKING_WRITE_BUFFER(deshake_ctx->command_queue, deshake_ctx->transform_y, 9 * sizeof(float), transform_y, NULL);
CL_BLOCKING_WRITE_BUFFER(deshake_ctx->command_queue, deshake_ctx->transform_uv, 9 * sizeof(float), transform_uv, NULL);
if (deshake_ctx->debug_on)
transform_debug(avctx, new_vals, old_vals, deshake_ctx->curr_frame);
cropped_frame = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!cropped_frame) {
err = AVERROR(ENOMEM);
goto fail;
}
transformed_frame = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!transformed_frame) {
err = AVERROR(ENOMEM);
goto fail;
}
transforms[0] = deshake_ctx->transform_y;
transforms[1] = transforms[2] = deshake_ctx->transform_uv;
for (int p = 0; p < FF_ARRAY_ELEMS(transformed_frame->data); p++) {
src = (cl_mem)input_frame->data[p];
transformed = (cl_mem)transformed_frame->data[p];
if (!transformed)
break;
err = ff_opencl_filter_work_size_from_image(avctx, global_work, input_frame, p, 0);
if (err < 0)
goto fail;
CL_RUN_KERNEL_WITH_ARGS(
deshake_ctx->command_queue,
deshake_ctx->kernel_transform,
global_work,
NULL,
&transform_event,
{ sizeof(cl_mem), &src },
{ sizeof(cl_mem), &transformed },
{ sizeof(cl_mem), &transforms[p] },
);
}
if (deshake_ctx->debug_on && !deshake_ctx->is_yuv && debug_matches.num_matches > 0) {
CL_BLOCKING_WRITE_BUFFER(
deshake_ctx->command_queue,
deshake_ctx->debug_matches,
debug_matches.num_matches * sizeof(MotionVector),
debug_matches.matches,
NULL
);
CL_BLOCKING_WRITE_BUFFER(
deshake_ctx->command_queue,
deshake_ctx->debug_model_matches,
debug_matches.num_model_matches * sizeof(MotionVector),
debug_matches.model_matches,
NULL
);
num_model_matches = debug_matches.num_model_matches;
transform_center_scale(
new_vals[RingbufX] - old_vals[RingbufX],
new_vals[RingbufY] - old_vals[RingbufY],
new_vals[RingbufRot] - old_vals[RingbufRot],
old_vals[RingbufScaleX] / new_vals[RingbufScaleX],
old_vals[RingbufScaleY] / new_vals[RingbufScaleY],
center_w,
center_h,
transform_debug_rgb
);
CL_BLOCKING_WRITE_BUFFER(deshake_ctx->command_queue, deshake_ctx->transform_y, 9 * sizeof(float), transform_debug_rgb, NULL);
transformed = (cl_mem)transformed_frame->data[0];
CL_RUN_KERNEL_WITH_ARGS(
deshake_ctx->command_queue,
deshake_ctx->kernel_draw_debug_info,
(size_t[]){ debug_matches.num_matches },
NULL,
NULL,
{ sizeof(cl_mem), &transformed },
{ sizeof(cl_mem), &deshake_ctx->debug_matches },
{ sizeof(cl_mem), &deshake_ctx->debug_model_matches },
{ sizeof(cl_int), &num_model_matches },
{ sizeof(cl_mem), &deshake_ctx->transform_y }
);
}
if (deshake_ctx->should_crop) {
transform_center_scale(
(old_vals[RingbufX] - new_vals[RingbufX]) / 5,
(old_vals[RingbufY] - new_vals[RingbufY]) / 5,
(old_vals[RingbufRot] - new_vals[RingbufRot]) / 5,
new_vals[RingbufScaleX] / old_vals[RingbufScaleX],
new_vals[RingbufScaleY] / old_vals[RingbufScaleY],
center_w,
center_h,
transform_crop_y
);
update_needed_crop(&deshake_ctx->crop_y, transform_crop_y, input_frame->width, input_frame->height);
transform_center_scale(
(old_vals[RingbufX] - new_vals[RingbufX]) / (5 * luma_w_over_chroma_w),
(old_vals[RingbufY] - new_vals[RingbufY]) / (5 * luma_h_over_chroma_h),
(old_vals[RingbufRot] - new_vals[RingbufRot]) / 5,
new_vals[RingbufScaleX] / old_vals[RingbufScaleX],
new_vals[RingbufScaleY] / old_vals[RingbufScaleY],
center_w_chroma,
center_h_chroma,
transform_crop_uv
);
update_needed_crop(&deshake_ctx->crop_uv, transform_crop_uv, chroma_width, chroma_height);
crops[0] = deshake_ctx->crop_y;
crops[1] = crops[2] = deshake_ctx->crop_uv;
for (int p = 0; p < FF_ARRAY_ELEMS(cropped_frame->data); p++) {
dst = (cl_mem)cropped_frame->data[p];
transformed = (cl_mem)transformed_frame->data[p];
if (!dst)
break;
err = ff_opencl_filter_work_size_from_image(avctx, global_work, input_frame, p, 0);
if (err < 0)
goto fail;
CL_RUN_KERNEL_WITH_ARGS(
deshake_ctx->command_queue,
deshake_ctx->kernel_crop_upscale,
global_work,
NULL,
&crop_upscale_event,
{ sizeof(cl_mem), &transformed },
{ sizeof(cl_mem), &dst },
{ sizeof(cl_float2), &crops[p].top_left },
{ sizeof(cl_float2), &crops[p].bottom_right },
);
}
}
if (deshake_ctx->curr_frame < deshake_ctx->smooth_window / 2) {
++deshake_ctx->abs_motion.curr_frame_offset;
}
if (deshake_ctx->abs_motion.data_end_offset != -1) {
--deshake_ctx->abs_motion.data_end_offset;
if (deshake_ctx->abs_motion.data_end_offset == deshake_ctx->abs_motion.curr_frame_offset - 1) {
deshake_ctx->abs_motion.data_end_offset = -1;
deshake_ctx->abs_motion.data_start_offset = deshake_ctx->abs_motion.curr_frame_offset;
}
} else if (deshake_ctx->abs_motion.data_start_offset != -1) {
--deshake_ctx->abs_motion.data_start_offset;
}
if (deshake_ctx->debug_on) {
deshake_ctx->transform_time += ff_opencl_get_event_time(transform_event);
if (deshake_ctx->should_crop) {
deshake_ctx->crop_upscale_time += ff_opencl_get_event_time(crop_upscale_event);
}
}
++deshake_ctx->curr_frame;
if (deshake_ctx->debug_on)
av_freep(&debug_matches.matches);
if (deshake_ctx->should_crop) {
err = av_frame_copy_props(cropped_frame, input_frame);
if (err < 0)
goto fail;
av_frame_free(&transformed_frame);
av_frame_free(&input_frame);
return ff_filter_frame(outlink, cropped_frame);
} else {
err = av_frame_copy_props(transformed_frame, input_frame);
if (err < 0)
goto fail;
av_frame_free(&cropped_frame);
av_frame_free(&input_frame);
return ff_filter_frame(outlink, transformed_frame);
}
fail:
clFinish(deshake_ctx->command_queue);
if (deshake_ctx->debug_on)
if (debug_matches.matches)
av_freep(&debug_matches.matches);
av_frame_free(&input_frame);
av_frame_free(&transformed_frame);
av_frame_free(&cropped_frame);
return err;
}
static int queue_frame(AVFilterLink *link, AVFrame *input_frame)
{
AVFilterContext *avctx = link->dst;
DeshakeOpenCLContext *deshake_ctx = avctx->priv;
int err;
int num_vectors;
int num_inliers = 0;
cl_int cle;
FrameDelta relative;
SimilarityMatrix model;
size_t global_work[2];
size_t harris_global_work[2];
size_t grid_32_global_work[2];
int grid_32_h, grid_32_w;
size_t local_work[2];
cl_mem src, temp;
float prev_vals[5];
float new_vals[5];
cl_event grayscale_event, harris_response_event, refine_features_event,
brief_event, match_descriptors_event, read_buf_event;
DebugMatches debug_matches;
num_vectors = 0;
local_work[0] = 8;
local_work[1] = 8;
err = ff_opencl_filter_work_size_from_image(avctx, global_work, input_frame, 0, 0);
if (err < 0)
goto fail;
err = ff_opencl_filter_work_size_from_image(avctx, harris_global_work, input_frame, 0, 8);
if (err < 0)
goto fail;
err = ff_opencl_filter_work_size_from_image(avctx, grid_32_global_work, input_frame, 0, 32);
if (err < 0)
goto fail;
grid_32_global_work[0] /= 32;
grid_32_global_work[1] /= 32;
grid_32_h = ROUNDED_UP_DIV(input_frame->height, 32);
grid_32_w = ROUNDED_UP_DIV(input_frame->width, 32);
if (deshake_ctx->is_yuv) {
deshake_ctx->grayscale = (cl_mem)input_frame->data[0];
} else {
src = (cl_mem)input_frame->data[0];
CL_RUN_KERNEL_WITH_ARGS(
deshake_ctx->command_queue,
deshake_ctx->kernel_grayscale,
global_work,
NULL,
&grayscale_event,
{ sizeof(cl_mem), &src },
{ sizeof(cl_mem), &deshake_ctx->grayscale }
);
}
CL_RUN_KERNEL_WITH_ARGS(
deshake_ctx->command_queue,
deshake_ctx->kernel_harris_response,
harris_global_work,
local_work,
&harris_response_event,
{ sizeof(cl_mem), &deshake_ctx->grayscale },
{ sizeof(cl_mem), &deshake_ctx->harris_buf }
);
CL_RUN_KERNEL_WITH_ARGS(
deshake_ctx->command_queue,
deshake_ctx->kernel_refine_features,
grid_32_global_work,
NULL,
&refine_features_event,
{ sizeof(cl_mem), &deshake_ctx->grayscale },
{ sizeof(cl_mem), &deshake_ctx->harris_buf },
{ sizeof(cl_mem), &deshake_ctx->refined_features },
{ sizeof(cl_int), &deshake_ctx->refine_features }
);
CL_RUN_KERNEL_WITH_ARGS(
deshake_ctx->command_queue,
deshake_ctx->kernel_brief_descriptors,
grid_32_global_work,
NULL,
&brief_event,
{ sizeof(cl_mem), &deshake_ctx->grayscale },
{ sizeof(cl_mem), &deshake_ctx->refined_features },
{ sizeof(cl_mem), &deshake_ctx->descriptors },
{ sizeof(cl_mem), &deshake_ctx->brief_pattern}
);
if (av_fifo_size(deshake_ctx->abs_motion.ringbuffers[RingbufX]) == 0) {
goto no_motion_data;
}
CL_RUN_KERNEL_WITH_ARGS(
deshake_ctx->command_queue,
deshake_ctx->kernel_match_descriptors,
grid_32_global_work,
NULL,
&match_descriptors_event,
{ sizeof(cl_mem), &deshake_ctx->prev_refined_features },
{ sizeof(cl_mem), &deshake_ctx->refined_features },
{ sizeof(cl_mem), &deshake_ctx->descriptors },
{ sizeof(cl_mem), &deshake_ctx->prev_descriptors },
{ sizeof(cl_mem), &deshake_ctx->matches }
);
cle = clEnqueueReadBuffer(
deshake_ctx->command_queue,
deshake_ctx->matches,
CL_TRUE,
0,
grid_32_h * grid_32_w * sizeof(MotionVector),
deshake_ctx->matches_host,
0,
NULL,
&read_buf_event
);
CL_FAIL_ON_ERROR(AVERROR(EIO), "Failed to read matches to host: %d.\n", cle);
num_vectors = make_vectors_contig(deshake_ctx, grid_32_h, grid_32_w);
if (num_vectors < 10) {
if (deshake_ctx->abs_motion.data_end_offset == -1) {
deshake_ctx->abs_motion.data_end_offset =
av_fifo_size(deshake_ctx->abs_motion.ringbuffers[RingbufX]) / sizeof(float) - 1;
}
goto no_motion_data;
}
if (!estimate_affine_2d(
deshake_ctx,
deshake_ctx->matches_contig_host,
&debug_matches,
num_vectors,
model.matrix,
10.0,
3000,
0.999999999999
)) {
goto no_motion_data;
}
for (int i = 0; i < num_vectors; i++) {
if (deshake_ctx->matches_contig_host[i].should_consider) {
deshake_ctx->inliers[num_inliers] = deshake_ctx->matches_contig_host[i];
num_inliers++;
}
}
if (!minimize_error(
deshake_ctx,
deshake_ctx->inliers,
&debug_matches,
num_inliers,
model.matrix,
400
)) {
goto no_motion_data;
}
relative = decompose_transform(model.matrix);
for (int i = 0; i < RingbufCount; i++) {
av_fifo_generic_peek_at(
deshake_ctx->abs_motion.ringbuffers[i],
&prev_vals[i],
av_fifo_size(deshake_ctx->abs_motion.ringbuffers[i]) - sizeof(float),
sizeof(float),
NULL
);
}
new_vals[RingbufX] = prev_vals[RingbufX] + relative.translation.s[0];
new_vals[RingbufY] = prev_vals[RingbufY] + relative.translation.s[1];
new_vals[RingbufRot] = prev_vals[RingbufRot] + relative.rotation;
new_vals[RingbufScaleX] = prev_vals[RingbufScaleX] / relative.scale.s[0];
new_vals[RingbufScaleY] = prev_vals[RingbufScaleY] / relative.scale.s[1];
if (deshake_ctx->debug_on) {
if (!deshake_ctx->is_yuv) {
deshake_ctx->grayscale_time += ff_opencl_get_event_time(grayscale_event);
}
deshake_ctx->harris_response_time += ff_opencl_get_event_time(harris_response_event);
deshake_ctx->refine_features_time += ff_opencl_get_event_time(refine_features_event);
deshake_ctx->brief_descriptors_time += ff_opencl_get_event_time(brief_event);
deshake_ctx->match_descriptors_time += ff_opencl_get_event_time(match_descriptors_event);
deshake_ctx->read_buf_time += ff_opencl_get_event_time(read_buf_event);
}
goto end;
no_motion_data:
new_vals[RingbufX] = 0.0f;
new_vals[RingbufY] = 0.0f;
new_vals[RingbufRot] = 0.0f;
new_vals[RingbufScaleX] = 1.0f;
new_vals[RingbufScaleY] = 1.0f;
for (int i = 0; i < num_vectors; i++) {
deshake_ctx->matches_contig_host[i].should_consider = 0;
}
debug_matches.num_model_matches = 0;
if (deshake_ctx->debug_on) {
av_log(avctx, AV_LOG_VERBOSE,
"\n[ALERT] No motion data found in queue_frame, motion reset to 0\n\n"
);
}
goto end;
end:
temp = deshake_ctx->prev_descriptors;
deshake_ctx->prev_descriptors = deshake_ctx->descriptors;
deshake_ctx->descriptors = temp;
temp = deshake_ctx->prev_refined_features;
deshake_ctx->prev_refined_features = deshake_ctx->refined_features;
deshake_ctx->refined_features = temp;
if (deshake_ctx->debug_on) {
if (num_vectors == 0) {
debug_matches.matches = NULL;
} else {
debug_matches.matches = av_malloc_array(num_vectors, sizeof(MotionVector));
if (!debug_matches.matches) {
err = AVERROR(ENOMEM);
goto fail;
}
}
for (int i = 0; i < num_vectors; i++) {
debug_matches.matches[i] = deshake_ctx->matches_contig_host[i];
}
debug_matches.num_matches = num_vectors;
av_fifo_generic_write(
deshake_ctx->abs_motion.debug_matches,
&debug_matches,
sizeof(DebugMatches),
NULL
);
}
for (int i = 0; i < RingbufCount; i++) {
av_fifo_generic_write(
deshake_ctx->abs_motion.ringbuffers[i],
&new_vals[i],
sizeof(float),
NULL
);
}
return ff_framequeue_add(&deshake_ctx->fq, input_frame);
fail:
clFinish(deshake_ctx->command_queue);
av_frame_free(&input_frame);
return err;
}
static int activate(AVFilterContext *ctx)
{
AVFilterLink *inlink = ctx->inputs[0];
AVFilterLink *outlink = ctx->outputs[0];
DeshakeOpenCLContext *deshake_ctx = ctx->priv;
AVFrame *frame = NULL;
int ret, status;
int64_t pts;
FF_FILTER_FORWARD_STATUS_BACK(outlink, inlink);
if (!deshake_ctx->eof) {
ret = ff_inlink_consume_frame(inlink, &frame);
if (ret < 0)
return ret;
if (ret > 0) {
if (!frame->hw_frames_ctx)
return AVERROR(EINVAL);
if (!deshake_ctx->initialized) {
ret = deshake_opencl_init(ctx);
if (ret < 0)
return ret;
}
if (av_fifo_space(deshake_ctx->abs_motion.ringbuffers[RingbufX]) == 0) {
for (int i = 0; i < RingbufCount; i++) {
av_fifo_drain(deshake_ctx->abs_motion.ringbuffers[i], sizeof(float));
}
}
ret = queue_frame(inlink, frame);
if (ret < 0)
return ret;
if (ret >= 0) {
if (ff_framequeue_queued_frames(&deshake_ctx->fq) >= deshake_ctx->smooth_window / 2) {
return filter_frame(inlink, ff_framequeue_take(&deshake_ctx->fq));
}
}
}
}
if (!deshake_ctx->eof && ff_inlink_acknowledge_status(inlink, &status, &pts)) {
if (status == AVERROR_EOF) {
deshake_ctx->eof = 1;
}
}
if (deshake_ctx->eof) {
while(ff_framequeue_queued_frames(&deshake_ctx->fq) != 0) {
for (int i = 0; i < RingbufCount; i++) {
av_fifo_drain(deshake_ctx->abs_motion.ringbuffers[i], sizeof(float));
}
ret = filter_frame(inlink, ff_framequeue_take(&deshake_ctx->fq));
if (ret < 0) {
return ret;
}
}
if (deshake_ctx->debug_on) {
av_log(ctx, AV_LOG_VERBOSE,
"Average kernel execution times:\n"
"\t grayscale: %0.3f ms\n"
"\t harris_response: %0.3f ms\n"
"\t refine_features: %0.3f ms\n"
"\tbrief_descriptors: %0.3f ms\n"
"\tmatch_descriptors: %0.3f ms\n"
"\t transform: %0.3f ms\n"
"\t crop_upscale: %0.3f ms\n"
"Average buffer read times:\n"
"\t features buf: %0.3f ms\n",
averaged_event_time_ms(deshake_ctx->grayscale_time, deshake_ctx->curr_frame),
averaged_event_time_ms(deshake_ctx->harris_response_time, deshake_ctx->curr_frame),
averaged_event_time_ms(deshake_ctx->refine_features_time, deshake_ctx->curr_frame),
averaged_event_time_ms(deshake_ctx->brief_descriptors_time, deshake_ctx->curr_frame),
averaged_event_time_ms(deshake_ctx->match_descriptors_time, deshake_ctx->curr_frame),
averaged_event_time_ms(deshake_ctx->transform_time, deshake_ctx->curr_frame),
averaged_event_time_ms(deshake_ctx->crop_upscale_time, deshake_ctx->curr_frame),
averaged_event_time_ms(deshake_ctx->read_buf_time, deshake_ctx->curr_frame)
);
}
ff_outlink_set_status(outlink, AVERROR_EOF, deshake_ctx->duration);
return 0;
}
if (!deshake_ctx->eof) {
FF_FILTER_FORWARD_WANTED(outlink, inlink);
}
return FFERROR_NOT_READY;
}
static const AVFilterPad deshake_opencl_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = &ff_opencl_filter_config_input,
},
{ NULL }
};
static const AVFilterPad deshake_opencl_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = &ff_opencl_filter_config_output,
},
{ NULL }
};
#define OFFSET(x) offsetof(DeshakeOpenCLContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM
static const AVOption deshake_opencl_options[] = {
{
"tripod", "simulates a tripod by preventing any camera movement whatsoever "
"from the original frame",
OFFSET(tripod_mode), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, FLAGS
},
{
"debug", "turn on additional debugging information",
OFFSET(debug_on), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, FLAGS
},
{
"adaptive_crop", "attempt to subtly crop borders to reduce mirrored content",
OFFSET(should_crop), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, FLAGS
},
{
"refine_features", "refine feature point locations at a sub-pixel level",
OFFSET(refine_features), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, FLAGS
},
{
"smooth_strength", "smoothing strength (0 attempts to adaptively determine optimal strength)",
OFFSET(smooth_percent), AV_OPT_TYPE_FLOAT, {.dbl = 0.0f}, 0.0f, 1.0f, FLAGS
},
{
"smooth_window_multiplier", "multiplier for number of frames to buffer for motion data",
OFFSET(smooth_window_multiplier), AV_OPT_TYPE_FLOAT, {.dbl = 2.0}, 0.1, 10.0, FLAGS
},
{ NULL }
};
AVFILTER_DEFINE_CLASS(deshake_opencl);
AVFilter ff_vf_deshake_opencl = {
.name = "deshake_opencl",
.description = NULL_IF_CONFIG_SMALL("Feature-point based video stabilization filter"),
.priv_size = sizeof(DeshakeOpenCLContext),
.priv_class = &deshake_opencl_class,
.init = &ff_opencl_filter_init,
.uninit = &deshake_opencl_uninit,
.query_formats = &ff_opencl_filter_query_formats,
.activate = activate,
.inputs = deshake_opencl_inputs,
.outputs = deshake_opencl_outputs,
.flags_internal = FF_FILTER_FLAG_HWFRAME_AWARE
};
