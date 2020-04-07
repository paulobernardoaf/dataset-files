#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "dirac.h"
#include "golomb.h"
#include "internal.h"
#include "mpeg12data.h"
#if CONFIG_DIRAC_PARSE
typedef struct dirac_source_params {
unsigned width;
unsigned height;
uint8_t chroma_format; 
uint8_t interlaced;
uint8_t top_field_first;
uint8_t frame_rate_index; 
uint8_t aspect_ratio_index; 
uint16_t clean_width;
uint16_t clean_height;
uint16_t clean_left_offset;
uint16_t clean_right_offset;
uint8_t pixel_range_index; 
uint8_t color_spec_index; 
} dirac_source_params;
static const dirac_source_params dirac_source_parameters_defaults[] = {
{ 640, 480, 2, 0, 0, 1, 1, 640, 480, 0, 0, 1, 0 },
{ 176, 120, 2, 0, 0, 9, 2, 176, 120, 0, 0, 1, 1 },
{ 176, 144, 2, 0, 1, 10, 3, 176, 144, 0, 0, 1, 2 },
{ 352, 240, 2, 0, 0, 9, 2, 352, 240, 0, 0, 1, 1 },
{ 352, 288, 2, 0, 1, 10, 3, 352, 288, 0, 0, 1, 2 },
{ 704, 480, 2, 0, 0, 9, 2, 704, 480, 0, 0, 1, 1 },
{ 704, 576, 2, 0, 1, 10, 3, 704, 576, 0, 0, 1, 2 },
{ 720, 480, 1, 1, 0, 4, 2, 704, 480, 8, 0, 3, 1 },
{ 720, 576, 1, 1, 1, 3, 3, 704, 576, 8, 0, 3, 2 },
{ 1280, 720, 1, 0, 1, 7, 1, 1280, 720, 0, 0, 3, 3 },
{ 1280, 720, 1, 0, 1, 6, 1, 1280, 720, 0, 0, 3, 3 },
{ 1920, 1080, 1, 1, 1, 4, 1, 1920, 1080, 0, 0, 3, 3 },
{ 1920, 1080, 1, 1, 1, 3, 1, 1920, 1080, 0, 0, 3, 3 },
{ 1920, 1080, 1, 0, 1, 7, 1, 1920, 1080, 0, 0, 3, 3 },
{ 1920, 1080, 1, 0, 1, 6, 1, 1920, 1080, 0, 0, 3, 3 },
{ 2048, 1080, 0, 0, 1, 2, 1, 2048, 1080, 0, 0, 4, 4 },
{ 4096, 2160, 0, 0, 1, 2, 1, 4096, 2160, 0, 0, 4, 4 },
{ 3840, 2160, 1, 0, 1, 7, 1, 3840, 2160, 0, 0, 3, 3 },
{ 3840, 2160, 1, 0, 1, 6, 1, 3840, 2160, 0, 0, 3, 3 },
{ 7680, 4320, 1, 0, 1, 7, 1, 3840, 2160, 0, 0, 3, 3 },
{ 7680, 4320, 1, 0, 1, 6, 1, 3840, 2160, 0, 0, 3, 3 },
};
static const AVRational dirac_preset_aspect_ratios[] = {
{ 1, 1 },
{ 10, 11 },
{ 12, 11 },
{ 40, 33 },
{ 16, 11 },
{ 4, 3 },
};
static const AVRational dirac_frame_rate[] = {
{ 15000, 1001 },
{ 25, 2 },
};
static const struct {
uint8_t bitdepth;
enum AVColorRange color_range;
} pixel_range_presets[] = {
{ 8, AVCOL_RANGE_JPEG },
{ 8, AVCOL_RANGE_MPEG },
{ 10, AVCOL_RANGE_MPEG },
{ 12, AVCOL_RANGE_MPEG },
};
static const enum AVColorPrimaries dirac_primaries[] = {
AVCOL_PRI_BT709,
AVCOL_PRI_SMPTE170M,
AVCOL_PRI_BT470BG,
};
static const struct {
enum AVColorPrimaries color_primaries;
enum AVColorSpace colorspace;
enum AVColorTransferCharacteristic color_trc;
} dirac_color_presets[] = {
{ AVCOL_PRI_BT709, AVCOL_SPC_BT709, AVCOL_TRC_BT709 },
{ AVCOL_PRI_SMPTE170M, AVCOL_SPC_BT470BG, AVCOL_TRC_BT709 },
{ AVCOL_PRI_BT470BG, AVCOL_SPC_BT470BG, AVCOL_TRC_BT709 },
{ AVCOL_PRI_BT709, AVCOL_SPC_BT709, AVCOL_TRC_BT709 },
{ AVCOL_PRI_BT709, AVCOL_SPC_BT709, AVCOL_TRC_UNSPECIFIED },
};
static const enum AVPixelFormat dirac_pix_fmt[][3] = {
{AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUV444P10, AV_PIX_FMT_YUV444P12},
{AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUV422P10, AV_PIX_FMT_YUV422P12},
{AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV420P10, AV_PIX_FMT_YUV420P12},
};
static int parse_source_parameters(AVDiracSeqHeader *dsh, GetBitContext *gb,
void *log_ctx)
{
AVRational frame_rate = { 0, 0 };
unsigned luma_depth = 8, luma_offset = 16;
int idx;
int chroma_x_shift, chroma_y_shift;
int ret;
if (get_bits1(gb)) {
dsh->width = get_interleaved_ue_golomb(gb); 
dsh->height = get_interleaved_ue_golomb(gb); 
}
if (get_bits1(gb))
dsh->chroma_format = get_interleaved_ue_golomb(gb);
if (dsh->chroma_format > 2U) {
if (log_ctx)
av_log(log_ctx, AV_LOG_ERROR, "Unknown chroma format %d\n",
dsh->chroma_format);
return AVERROR_INVALIDDATA;
}
if (get_bits1(gb))
dsh->interlaced = get_interleaved_ue_golomb(gb);
if (dsh->interlaced > 1U)
return AVERROR_INVALIDDATA;
if (get_bits1(gb)) { 
dsh->frame_rate_index = get_interleaved_ue_golomb(gb);
if (dsh->frame_rate_index > 10U)
return AVERROR_INVALIDDATA;
if (!dsh->frame_rate_index) {
frame_rate.num = get_interleaved_ue_golomb(gb);
frame_rate.den = get_interleaved_ue_golomb(gb);
}
}
if (dsh->frame_rate_index > 0) {
if (dsh->frame_rate_index <= 8)
frame_rate = ff_mpeg12_frame_rate_tab[dsh->frame_rate_index];
else
frame_rate = dirac_frame_rate[dsh->frame_rate_index - 9];
}
dsh->framerate = frame_rate;
if (get_bits1(gb)) { 
dsh->aspect_ratio_index = get_interleaved_ue_golomb(gb);
if (dsh->aspect_ratio_index > 6U)
return AVERROR_INVALIDDATA;
if (!dsh->aspect_ratio_index) {
dsh->sample_aspect_ratio.num = get_interleaved_ue_golomb(gb);
dsh->sample_aspect_ratio.den = get_interleaved_ue_golomb(gb);
}
}
if (dsh->aspect_ratio_index > 0)
dsh->sample_aspect_ratio =
dirac_preset_aspect_ratios[dsh->aspect_ratio_index - 1];
if (get_bits1(gb)) { 
dsh->clean_width = get_interleaved_ue_golomb(gb);
dsh->clean_height = get_interleaved_ue_golomb(gb);
dsh->clean_left_offset = get_interleaved_ue_golomb(gb);
dsh->clean_right_offset = get_interleaved_ue_golomb(gb);
}
if (get_bits1(gb)) { 
dsh->pixel_range_index = get_interleaved_ue_golomb(gb);
if (dsh->pixel_range_index > 4U)
return AVERROR_INVALIDDATA;
if (!dsh->pixel_range_index) {
luma_offset = get_interleaved_ue_golomb(gb);
luma_depth = av_log2(get_interleaved_ue_golomb(gb)) + 1;
get_interleaved_ue_golomb(gb); 
get_interleaved_ue_golomb(gb); 
dsh->color_range = luma_offset ? AVCOL_RANGE_MPEG
: AVCOL_RANGE_JPEG;
}
}
if (dsh->pixel_range_index > 0) {
idx = dsh->pixel_range_index - 1;
luma_depth = pixel_range_presets[idx].bitdepth;
dsh->color_range = pixel_range_presets[idx].color_range;
}
dsh->bit_depth = luma_depth;
dsh->pixel_range_index += dsh->pixel_range_index == 1;
if (dsh->pixel_range_index < 2U)
return AVERROR_INVALIDDATA;
dsh->pix_fmt = dirac_pix_fmt[dsh->chroma_format][dsh->pixel_range_index-2];
ret = av_pix_fmt_get_chroma_sub_sample(dsh->pix_fmt, &chroma_x_shift, &chroma_y_shift);
if (ret)
return ret;
if ((dsh->width % (1<<chroma_x_shift)) || (dsh->height % (1<<chroma_y_shift))) {
if (log_ctx)
av_log(log_ctx, AV_LOG_ERROR, "Dimensions must be an integer multiple of the chroma subsampling\n");
return AVERROR_INVALIDDATA;
}
if (get_bits1(gb)) { 
idx = dsh->color_spec_index = get_interleaved_ue_golomb(gb);
if (dsh->color_spec_index > 4U)
return AVERROR_INVALIDDATA;
dsh->color_primaries = dirac_color_presets[idx].color_primaries;
dsh->colorspace = dirac_color_presets[idx].colorspace;
dsh->color_trc = dirac_color_presets[idx].color_trc;
if (!dsh->color_spec_index) {
if (get_bits1(gb)) {
idx = get_interleaved_ue_golomb(gb);
if (idx < 3U)
dsh->color_primaries = dirac_primaries[idx];
}
if (get_bits1(gb)) {
idx = get_interleaved_ue_golomb(gb);
if (!idx)
dsh->colorspace = AVCOL_SPC_BT709;
else if (idx == 1)
dsh->colorspace = AVCOL_SPC_BT470BG;
}
if (get_bits1(gb) && !get_interleaved_ue_golomb(gb))
dsh->color_trc = AVCOL_TRC_BT709;
}
} else {
idx = dsh->color_spec_index;
dsh->color_primaries = dirac_color_presets[idx].color_primaries;
dsh->colorspace = dirac_color_presets[idx].colorspace;
dsh->color_trc = dirac_color_presets[idx].color_trc;
}
return 0;
}
int av_dirac_parse_sequence_header(AVDiracSeqHeader **pdsh,
const uint8_t *buf, size_t buf_size,
void *log_ctx)
{
AVDiracSeqHeader *dsh;
GetBitContext gb;
unsigned video_format, picture_coding_mode;
int ret;
dsh = av_mallocz(sizeof(*dsh));
if (!dsh)
return AVERROR(ENOMEM);
ret = init_get_bits8(&gb, buf, buf_size);
if (ret < 0)
goto fail;
dsh->version.major = get_interleaved_ue_golomb(&gb);
dsh->version.minor = get_interleaved_ue_golomb(&gb);
dsh->profile = get_interleaved_ue_golomb(&gb);
dsh->level = get_interleaved_ue_golomb(&gb);
video_format = get_interleaved_ue_golomb(&gb);
if (dsh->version.major < 2 && log_ctx)
av_log(log_ctx, AV_LOG_WARNING, "Stream is old and may not work\n");
else if (dsh->version.major > 2 && log_ctx)
av_log(log_ctx, AV_LOG_WARNING, "Stream may have unhandled features\n");
if (video_format > 20U) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
dsh->width = dirac_source_parameters_defaults[video_format].width;
dsh->height = dirac_source_parameters_defaults[video_format].height;
dsh->chroma_format = dirac_source_parameters_defaults[video_format].chroma_format;
dsh->interlaced = dirac_source_parameters_defaults[video_format].interlaced;
dsh->top_field_first = dirac_source_parameters_defaults[video_format].top_field_first;
dsh->frame_rate_index = dirac_source_parameters_defaults[video_format].frame_rate_index;
dsh->aspect_ratio_index = dirac_source_parameters_defaults[video_format].aspect_ratio_index;
dsh->clean_width = dirac_source_parameters_defaults[video_format].clean_width;
dsh->clean_height = dirac_source_parameters_defaults[video_format].clean_height;
dsh->clean_left_offset = dirac_source_parameters_defaults[video_format].clean_left_offset;
dsh->clean_right_offset = dirac_source_parameters_defaults[video_format].clean_right_offset;
dsh->pixel_range_index = dirac_source_parameters_defaults[video_format].pixel_range_index;
dsh->color_spec_index = dirac_source_parameters_defaults[video_format].color_spec_index;
ret = parse_source_parameters(dsh, &gb, log_ctx);
if (ret < 0)
goto fail;
picture_coding_mode = get_interleaved_ue_golomb(&gb);
if (picture_coding_mode != 0) {
if (log_ctx) {
av_log(log_ctx, AV_LOG_ERROR, "Unsupported picture coding mode %d",
picture_coding_mode);
}
ret = AVERROR_INVALIDDATA;
goto fail;
}
*pdsh = dsh;
return 0;
fail:
av_freep(&dsh);
*pdsh = NULL;
return ret;
}
#else
int av_dirac_parse_sequence_header(AVDiracSeqHeader **pdsh,
const uint8_t *buf, size_t buf_size,
void *log_ctx)
{
return AVERROR(ENOSYS);
}
#endif
