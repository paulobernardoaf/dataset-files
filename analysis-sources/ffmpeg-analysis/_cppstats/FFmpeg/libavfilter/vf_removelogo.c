#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "avfilter.h"
#include "formats.h"
#include "internal.h"
#include "video.h"
#include "bbox.h"
#include "lavfutils.h"
#include "lswsutils.h"
typedef struct RemovelogoContext {
const AVClass *class;
char *filename;
int ***mask;
int max_mask_size;
int mask_w, mask_h;
uint8_t *full_mask_data;
FFBoundingBox full_mask_bbox;
uint8_t *half_mask_data;
FFBoundingBox half_mask_bbox;
} RemovelogoContext;
#define OFFSET(x) offsetof(RemovelogoContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM
static const AVOption removelogo_options[] = {
{ "filename", "set bitmap filename", OFFSET(filename), AV_OPT_TYPE_STRING, {.str=NULL}, .flags = FLAGS },
{ "f", "set bitmap filename", OFFSET(filename), AV_OPT_TYPE_STRING, {.str=NULL}, .flags = FLAGS },
{ NULL }
};
AVFILTER_DEFINE_CLASS(removelogo);
#define apply_mask_fudge_factor(x) (((x) >> 2) + (x))
static void convert_mask_to_strength_mask(uint8_t *data, int linesize,
int w, int h, int min_val,
int *max_mask_size)
{
int x, y;
int current_pass = 0;
for (y = 0; y < h; y++)
for (x = 0; x < w; x++)
data[y*linesize + x] = data[y*linesize + x] > min_val;
while (1) {
int has_anything_changed = 0;
uint8_t *current_pixel0 = data + 1 + linesize, *current_pixel;
current_pass++;
for (y = 1; y < h-1; y++) {
current_pixel = current_pixel0;
for (x = 1; x < w-1; x++) {
if ( *current_pixel >= current_pass &&
*(current_pixel + 1) >= current_pass &&
*(current_pixel - 1) >= current_pass &&
*(current_pixel + linesize) >= current_pass &&
*(current_pixel - linesize) >= current_pass) {
(*current_pixel)++;
has_anything_changed = 1;
}
current_pixel++;
}
current_pixel0 += linesize;
}
if (!has_anything_changed)
break;
}
for (y = 1; y < h - 1; y++)
for (x = 1; x < w - 1; x++)
data[(y * linesize) + x] = apply_mask_fudge_factor(data[(y * linesize) + x]);
*max_mask_size = apply_mask_fudge_factor(current_pass + 1);
}
static int query_formats(AVFilterContext *ctx)
{
static const enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
if (!fmts_list)
return AVERROR(ENOMEM);
return ff_set_common_formats(ctx, fmts_list);
}
static int load_mask(uint8_t **mask, int *w, int *h,
const char *filename, void *log_ctx)
{
int ret;
enum AVPixelFormat pix_fmt;
uint8_t *src_data[4], *gray_data[4];
int src_linesize[4], gray_linesize[4];
if ((ret = ff_load_image(src_data, src_linesize, w, h, &pix_fmt, filename, log_ctx)) < 0)
return ret;
if ((ret = ff_scale_image(gray_data, gray_linesize, *w, *h, AV_PIX_FMT_GRAY8,
src_data, src_linesize, *w, *h, pix_fmt,
log_ctx)) < 0)
goto end;
*mask = av_malloc(*w * *h);
if (!*mask)
ret = AVERROR(ENOMEM);
av_image_copy_plane(*mask, *w, gray_data[0], gray_linesize[0], *w, *h);
end:
av_freep(&src_data[0]);
av_freep(&gray_data[0]);
return ret;
}
static void generate_half_size_image(const uint8_t *src_data, int src_linesize,
uint8_t *dst_data, int dst_linesize,
int src_w, int src_h,
int *max_mask_size)
{
int x, y;
for (y = 0; y < src_h/2; y++) {
for (x = 0; x < src_w/2; x++) {
dst_data[(y * dst_linesize) + x] =
src_data[((y << 1) * src_linesize) + (x << 1)] ||
src_data[((y << 1) * src_linesize) + (x << 1) + 1] ||
src_data[(((y << 1) + 1) * src_linesize) + (x << 1)] ||
src_data[(((y << 1) + 1) * src_linesize) + (x << 1) + 1];
dst_data[(y * dst_linesize) + x] = FFMIN(1, dst_data[(y * dst_linesize) + x]);
}
}
convert_mask_to_strength_mask(dst_data, dst_linesize,
src_w/2, src_h/2, 0, max_mask_size);
}
static av_cold int init(AVFilterContext *ctx)
{
RemovelogoContext *s = ctx->priv;
int ***mask;
int ret = 0;
int a, b, c, w, h;
int full_max_mask_size, half_max_mask_size;
if (!s->filename) {
av_log(ctx, AV_LOG_ERROR, "The bitmap file name is mandatory\n");
return AVERROR(EINVAL);
}
if ((ret = load_mask(&s->full_mask_data, &w, &h, s->filename, ctx)) < 0)
return ret;
s->mask_w = w;
s->mask_h = h;
convert_mask_to_strength_mask(s->full_mask_data, w, w, h,
16, &full_max_mask_size);
if (!(s->half_mask_data = av_mallocz(w/2 * h/2)))
return AVERROR(ENOMEM);
generate_half_size_image(s->full_mask_data, w,
s->half_mask_data, w/2,
w, h, &half_max_mask_size);
s->max_mask_size = FFMAX(full_max_mask_size, half_max_mask_size);
mask = (int ***)av_malloc_array(s->max_mask_size + 1, sizeof(int **));
if (!mask)
return AVERROR(ENOMEM);
for (a = 0; a <= s->max_mask_size; a++) {
mask[a] = (int **)av_malloc_array((a * 2) + 1, sizeof(int *));
if (!mask[a]) {
av_free(mask);
return AVERROR(ENOMEM);
}
for (b = -a; b <= a; b++) {
mask[a][b + a] = (int *)av_malloc_array((a * 2) + 1, sizeof(int));
if (!mask[a][b + a]) {
av_free(mask);
return AVERROR(ENOMEM);
}
for (c = -a; c <= a; c++) {
if ((b * b) + (c * c) <= (a * a)) 
mask[a][b + a][c + a] = 1;
else
mask[a][b + a][c + a] = 0;
}
}
}
s->mask = mask;
ff_calculate_bounding_box(&s->full_mask_bbox, s->full_mask_data, w, w, h, 0);
ff_calculate_bounding_box(&s->half_mask_bbox, s->half_mask_data, w/2, w/2, h/2, 0);
#define SHOW_LOGO_INFO(mask_type) av_log(ctx, AV_LOG_VERBOSE, #mask_type " x1:%d x2:%d y1:%d y2:%d max_mask_size:%d\n", s->mask_type##_mask_bbox.x1, s->mask_type##_mask_bbox.x2, s->mask_type##_mask_bbox.y1, s->mask_type##_mask_bbox.y2, mask_type##_max_mask_size);
SHOW_LOGO_INFO(full);
SHOW_LOGO_INFO(half);
return 0;
}
static int config_props_input(AVFilterLink *inlink)
{
AVFilterContext *ctx = inlink->dst;
RemovelogoContext *s = ctx->priv;
if (inlink->w != s->mask_w || inlink->h != s->mask_h) {
av_log(ctx, AV_LOG_INFO,
"Mask image size %dx%d does not match with the input video size %dx%d\n",
s->mask_w, s->mask_h, inlink->w, inlink->h);
return AVERROR(EINVAL);
}
return 0;
}
static unsigned int blur_pixel(int ***mask,
const uint8_t *mask_data, int mask_linesize,
uint8_t *image_data, int image_linesize,
int w, int h, int x, int y)
{
int mask_size;
int start_posx, start_posy, end_posx, end_posy;
int i, j;
unsigned int accumulator = 0, divisor = 0;
const uint8_t *image_read_position;
const uint8_t *mask_read_position;
mask_size = mask_data[y * mask_linesize + x];
start_posx = FFMAX(0, x - mask_size);
start_posy = FFMAX(0, y - mask_size);
end_posx = FFMIN(w - 1, x + mask_size);
end_posy = FFMIN(h - 1, y + mask_size);
image_read_position = image_data + image_linesize * start_posy + start_posx;
mask_read_position = mask_data + mask_linesize * start_posy + start_posx;
for (j = start_posy; j <= end_posy; j++) {
for (i = start_posx; i <= end_posx; i++) {
if (!(*mask_read_position) && mask[mask_size][i - start_posx][j - start_posy]) {
accumulator += *image_read_position;
divisor++;
}
image_read_position++;
mask_read_position++;
}
image_read_position += (image_linesize - ((end_posx + 1) - start_posx));
mask_read_position += (mask_linesize - ((end_posx + 1) - start_posx));
}
return divisor == 0 ? 255:
(accumulator + (divisor / 2)) / divisor; 
}
static void blur_image(int ***mask,
const uint8_t *src_data, int src_linesize,
uint8_t *dst_data, int dst_linesize,
const uint8_t *mask_data, int mask_linesize,
int w, int h, int direct,
FFBoundingBox *bbox)
{
int x, y;
uint8_t *dst_line;
const uint8_t *src_line;
if (!direct)
av_image_copy_plane(dst_data, dst_linesize, src_data, src_linesize, w, h);
for (y = bbox->y1; y <= bbox->y2; y++) {
src_line = src_data + src_linesize * y;
dst_line = dst_data + dst_linesize * y;
for (x = bbox->x1; x <= bbox->x2; x++) {
if (mask_data[y * mask_linesize + x]) {
dst_line[x] = blur_pixel(mask,
mask_data, mask_linesize,
dst_data, dst_linesize,
w, h, x, y);
} else {
if (!direct)
dst_line[x] = src_line[x];
}
}
}
}
static int filter_frame(AVFilterLink *inlink, AVFrame *inpicref)
{
RemovelogoContext *s = inlink->dst->priv;
AVFilterLink *outlink = inlink->dst->outputs[0];
AVFrame *outpicref;
int direct = 0;
if (av_frame_is_writable(inpicref)) {
direct = 1;
outpicref = inpicref;
} else {
outpicref = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!outpicref) {
av_frame_free(&inpicref);
return AVERROR(ENOMEM);
}
av_frame_copy_props(outpicref, inpicref);
}
blur_image(s->mask,
inpicref ->data[0], inpicref ->linesize[0],
outpicref->data[0], outpicref->linesize[0],
s->full_mask_data, inlink->w,
inlink->w, inlink->h, direct, &s->full_mask_bbox);
blur_image(s->mask,
inpicref ->data[1], inpicref ->linesize[1],
outpicref->data[1], outpicref->linesize[1],
s->half_mask_data, inlink->w/2,
inlink->w/2, inlink->h/2, direct, &s->half_mask_bbox);
blur_image(s->mask,
inpicref ->data[2], inpicref ->linesize[2],
outpicref->data[2], outpicref->linesize[2],
s->half_mask_data, inlink->w/2,
inlink->w/2, inlink->h/2, direct, &s->half_mask_bbox);
if (!direct)
av_frame_free(&inpicref);
return ff_filter_frame(outlink, outpicref);
}
static av_cold void uninit(AVFilterContext *ctx)
{
RemovelogoContext *s = ctx->priv;
int a, b;
av_freep(&s->full_mask_data);
av_freep(&s->half_mask_data);
if (s->mask) {
for (a = 0; a <= s->max_mask_size; a++) {
for (b = -a; b <= a; b++) {
av_freep(&s->mask[a][b + a]); 
}
av_freep(&s->mask[a]);
}
av_freep(&s->mask);
}
}
static const AVFilterPad removelogo_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = config_props_input,
.filter_frame = filter_frame,
},
{ NULL }
};
static const AVFilterPad removelogo_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};
AVFilter ff_vf_removelogo = {
.name = "removelogo",
.description = NULL_IF_CONFIG_SMALL("Remove a TV logo based on a mask image."),
.priv_size = sizeof(RemovelogoContext),
.init = init,
.uninit = uninit,
.query_formats = query_formats,
.inputs = removelogo_inputs,
.outputs = removelogo_outputs,
.priv_class = &removelogo_class,
.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC,
};
