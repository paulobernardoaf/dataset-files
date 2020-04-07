

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdatomic.h>

#include <vlc_common.h>
#include <vlc_codec.h>
#include <vlc_avcodec.h>
#include <vlc_cpu.h>
#include <assert.h>

#include <libavcodec/avcodec.h>
#include <libavutil/mem.h>
#include <libavutil/pixdesc.h>
#if (LIBAVUTIL_VERSION_MICRO >= 100)
#include <libavutil/mastering_display_metadata.h>
#endif

#include "avcodec.h"
#include "va.h"

#include <libavutil/stereo3d.h>

#include "../cc.h"
#define FRAME_INFO_DEPTH 64

struct frame_info_s
{
bool b_eos;
bool b_display;
};




typedef struct
{
AVCodecContext *p_context;
const AVCodec *p_codec;


date_t pts; 


cc_data_t cc;


bool b_hurry_up;
bool b_show_corrupted;
bool b_from_preroll;
enum AVDiscard i_skip_frame;

struct frame_info_s frame_info[FRAME_INFO_DEPTH];

enum
{
FRAMEDROP_NONE,
FRAMEDROP_NONREF,
FRAMEDROP_AGGRESSIVE_RECOVER,
} framedrop;

int i_late_frames;
int64_t i_last_output_frame;
vlc_tick_t i_last_late_delay;


bool b_direct_rendering;
atomic_bool b_dr_failure;


bool b_first_frame;



bool palette_sent;


vlc_va_t *p_va; 
enum PixelFormat pix_fmt;
int profile;
int level;


unsigned decoder_width;
unsigned decoder_height;



vlc_mutex_t lock;
} decoder_sys_t;




static void ffmpeg_InitCodec ( decoder_t * );
static int lavc_GetFrame(struct AVCodecContext *, AVFrame *, int);
static enum PixelFormat ffmpeg_GetFormat( AVCodecContext *,
const enum PixelFormat * );
static int DecodeVideo( decoder_t *, block_t * );
static void Flush( decoder_t * );

static uint32_t ffmpeg_CodecTag( vlc_fourcc_t fcc )
{
uint8_t *p = (uint8_t*)&fcc;
return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}








static int lavc_GetVideoFormat(decoder_t *dec, video_format_t *restrict fmt,
AVCodecContext *ctx, enum AVPixelFormat pix_fmt,
enum AVPixelFormat sw_pix_fmt)
{
int width = ctx->coded_width;
int height = ctx->coded_height;

video_format_Init(fmt, 0);

if (pix_fmt == sw_pix_fmt)
{ 
int aligns[AV_NUM_DATA_POINTERS];

if (GetVlcChroma(fmt, pix_fmt))
return -1;





if (pix_fmt == AV_PIX_FMT_PAL8 && !dec->fmt_out.video.p_palette)
fmt->i_chroma = VLC_CODEC_RGB32;

avcodec_align_dimensions2(ctx, &width, &height, aligns);
}

if( width == 0 || height == 0 || width > 8192 || height > 8192 ||
width < ctx->width || height < ctx->height )
{
msg_Err(dec, "Invalid frame size %dx%d vsz %dx%d",
width, height, ctx->width, ctx->height );
return -1; 
}

fmt->i_width = width;
fmt->i_height = height;
if ( dec->fmt_in.video.i_visible_width != 0 &&
dec->fmt_in.video.i_visible_width <= (unsigned)ctx->width &&
dec->fmt_in.video.i_visible_height != 0 &&
dec->fmt_in.video.i_visible_height <= (unsigned)ctx->height )
{

fmt->i_visible_width = dec->fmt_in.video.i_visible_width;
fmt->i_visible_height = dec->fmt_in.video.i_visible_height;
fmt->i_x_offset = dec->fmt_in.video.i_x_offset;
fmt->i_y_offset = dec->fmt_in.video.i_y_offset;
}
else
{
fmt->i_visible_width = ctx->width;
fmt->i_visible_height = ctx->height;
fmt->i_x_offset = 0;
fmt->i_y_offset = 0;
}


if (dec->fmt_in.video.i_sar_num > 0 && dec->fmt_in.video.i_sar_den > 0)
{
fmt->i_sar_num = dec->fmt_in.video.i_sar_num;
fmt->i_sar_den = dec->fmt_in.video.i_sar_den;
}
else
{
fmt->i_sar_num = ctx->sample_aspect_ratio.num;
fmt->i_sar_den = ctx->sample_aspect_ratio.den;

if (fmt->i_sar_num == 0 || fmt->i_sar_den == 0)
fmt->i_sar_num = fmt->i_sar_den = 1;
}

if (dec->fmt_in.video.i_frame_rate > 0
&& dec->fmt_in.video.i_frame_rate_base > 0)
{
fmt->i_frame_rate = dec->fmt_in.video.i_frame_rate;
fmt->i_frame_rate_base = dec->fmt_in.video.i_frame_rate_base;
}
else if (ctx->framerate.num > 0 && ctx->framerate.den > 0)
{
fmt->i_frame_rate = ctx->framerate.num;
fmt->i_frame_rate_base = ctx->framerate.den;
#if LIBAVCODEC_VERSION_MICRO < 100

fmt->i_frame_rate_base *= __MAX(ctx->ticks_per_frame, 1);
#endif
}
else if (ctx->time_base.num > 0 && ctx->time_base.den > 0)
{
fmt->i_frame_rate = ctx->time_base.den;
fmt->i_frame_rate_base = ctx->time_base.num
* __MAX(ctx->ticks_per_frame, 1);
}

switch ( ctx->color_range )
{
case AVCOL_RANGE_JPEG:
fmt->color_range = COLOR_RANGE_FULL;
break;
case AVCOL_RANGE_MPEG:
fmt->color_range = COLOR_RANGE_LIMITED;
break;
default: 
break;
}

switch( ctx->colorspace )
{
case AVCOL_SPC_BT709:
fmt->space = COLOR_SPACE_BT709;
break;
case AVCOL_SPC_SMPTE170M:
case AVCOL_SPC_BT470BG:
fmt->space = COLOR_SPACE_BT601;
break;
case AVCOL_SPC_BT2020_NCL:
case AVCOL_SPC_BT2020_CL:
fmt->space = COLOR_SPACE_BT2020;
break;
default:
break;
}

switch( ctx->color_trc )
{
case AVCOL_TRC_LINEAR:
fmt->transfer = TRANSFER_FUNC_LINEAR;
break;
case AVCOL_TRC_GAMMA22:
fmt->transfer = TRANSFER_FUNC_SRGB;
break;
case AVCOL_TRC_BT709:
fmt->transfer = TRANSFER_FUNC_BT709;
break;
case AVCOL_TRC_SMPTE170M:
case AVCOL_TRC_BT2020_10:
case AVCOL_TRC_BT2020_12:
fmt->transfer = TRANSFER_FUNC_BT2020;
break;
#if LIBAVUTIL_VERSION_CHECK( 55, 14, 0, 31, 100)
case AVCOL_TRC_ARIB_STD_B67:
fmt->transfer = TRANSFER_FUNC_ARIB_B67;
break;
#endif
#if LIBAVUTIL_VERSION_CHECK( 55, 17, 0, 37, 100)
case AVCOL_TRC_SMPTE2084:
fmt->transfer = TRANSFER_FUNC_SMPTE_ST2084;
break;
case AVCOL_TRC_SMPTE240M:
fmt->transfer = TRANSFER_FUNC_SMPTE_240;
break;
case AVCOL_TRC_GAMMA28:
fmt->transfer = TRANSFER_FUNC_BT470_BG;
break;
#endif
default:
break;
}

switch( ctx->color_primaries )
{
case AVCOL_PRI_BT709:
fmt->primaries = COLOR_PRIMARIES_BT709;
break;
case AVCOL_PRI_BT470BG:
fmt->primaries = COLOR_PRIMARIES_BT601_625;
break;
case AVCOL_PRI_SMPTE170M:
case AVCOL_PRI_SMPTE240M:
fmt->primaries = COLOR_PRIMARIES_BT601_525;
break;
case AVCOL_PRI_BT2020:
fmt->primaries = COLOR_PRIMARIES_BT2020;
break;
default:
break;
}

switch( ctx->chroma_sample_location )
{
case AVCHROMA_LOC_LEFT:
fmt->chroma_location = CHROMA_LOCATION_LEFT;
break;
case AVCHROMA_LOC_CENTER:
fmt->chroma_location = CHROMA_LOCATION_CENTER;
break;
case AVCHROMA_LOC_TOPLEFT:
fmt->chroma_location = CHROMA_LOCATION_TOP_LEFT;
break;
default:
break;
}

return 0;
}

static int lavc_UpdateVideoFormat(decoder_t *dec, AVCodecContext *ctx,
enum AVPixelFormat fmt,
enum AVPixelFormat swfmt,
vlc_decoder_device **pp_dec_device)
{
video_format_t fmt_out;
int val;

val = lavc_GetVideoFormat(dec, &fmt_out, ctx, fmt, swfmt);
if (val)
return val;

decoder_sys_t *p_sys = dec->p_sys;


if(p_sys->pts.i_divider_num)
date_Change(&p_sys->pts, fmt_out.i_frame_rate *
__MAX(ctx->ticks_per_frame, 1),
fmt_out.i_frame_rate_base);
else
date_Init(&p_sys->pts, fmt_out.i_frame_rate *
__MAX(ctx->ticks_per_frame, 1),
fmt_out.i_frame_rate_base);

fmt_out.p_palette = dec->fmt_out.video.p_palette;
dec->fmt_out.video.p_palette = NULL;

vlc_fourcc_t i_chroma;
if (fmt == swfmt)
i_chroma = fmt_out.i_chroma;
else
i_chroma = 0;
es_format_Change(&dec->fmt_out, VIDEO_ES, i_chroma);
dec->fmt_out.video = fmt_out;
dec->fmt_out.video.i_chroma = i_chroma;
dec->fmt_out.video.orientation = dec->fmt_in.video.orientation;
dec->fmt_out.video.projection_mode = dec->fmt_in.video.projection_mode;
dec->fmt_out.video.multiview_mode = dec->fmt_in.video.multiview_mode;
dec->fmt_out.video.pose = dec->fmt_in.video.pose;
if ( dec->fmt_in.video.mastering.max_luminance )
dec->fmt_out.video.mastering = dec->fmt_in.video.mastering;
dec->fmt_out.video.lighting = dec->fmt_in.video.lighting;
p_sys->decoder_width = dec->fmt_out.video.i_width;
p_sys->decoder_height = dec->fmt_out.video.i_height;

if (pp_dec_device)
{
*pp_dec_device = decoder_GetDecoderDevice(dec);
return *pp_dec_device == NULL;
}
return 0;
}

static bool chroma_compatible(vlc_fourcc_t a, vlc_fourcc_t b)
{
static const vlc_fourcc_t compat_lists[][2] = {
{VLC_CODEC_J420, VLC_CODEC_I420},
{VLC_CODEC_J422, VLC_CODEC_I422},
{VLC_CODEC_J440, VLC_CODEC_I440},
{VLC_CODEC_J444, VLC_CODEC_I444},
};

if (a == b)
return true;

for (size_t i = 0; i < ARRAY_SIZE(compat_lists); i++) {
if ((a == compat_lists[i][0] || a == compat_lists[i][1]) &&
(b == compat_lists[i][0] || b == compat_lists[i][1]))
return true;
}
return false;
}





static int lavc_CopyPicture(decoder_t *dec, picture_t *pic, AVFrame *frame)
{
decoder_sys_t *sys = dec->p_sys;

vlc_fourcc_t fourcc = FindVlcChroma(frame->format);
if (!fourcc)
{
const char *name = av_get_pix_fmt_name(frame->format);

msg_Err(dec, "Unsupported decoded output format %d (%s)",
sys->p_context->pix_fmt, (name != NULL) ? name : "unknown");
return VLC_EGENERIC;
} else if (!chroma_compatible(fourcc, pic->format.i_chroma)

|| frame->width != (int) pic->format.i_visible_width
|| frame->height < (int) pic->format.i_visible_height)
{
msg_Warn(dec, "dropping frame because the vout changed");
return VLC_EGENERIC;
}

for (int plane = 0; plane < pic->i_planes; plane++)
{
const uint8_t *src = frame->data[plane];
uint8_t *dst = pic->p[plane].p_pixels;
size_t src_stride = frame->linesize[plane];
size_t dst_stride = pic->p[plane].i_pitch;
size_t size = __MIN(src_stride, dst_stride);

for (int line = 0; line < pic->p[plane].i_visible_lines; line++)
{
memcpy(dst, src, size);
src += src_stride;
dst += dst_stride;
}
}
return VLC_SUCCESS;
}

static int OpenVideoCodec( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *ctx = p_sys->p_context;
const AVCodec *codec = p_sys->p_codec;
int ret;

if( ctx->extradata_size <= 0 )
{
if( codec->id == AV_CODEC_ID_VC1 ||
codec->id == AV_CODEC_ID_THEORA )
{
msg_Warn( p_dec, "waiting for extra data for codec %s",
codec->name );
return 1;
}
}

ctx->width = p_dec->fmt_in.video.i_visible_width;
ctx->height = p_dec->fmt_in.video.i_visible_height;

ctx->coded_width = p_dec->fmt_in.video.i_width;
ctx->coded_height = p_dec->fmt_in.video.i_height;

ctx->bits_per_coded_sample = p_dec->fmt_in.video.i_bits_per_pixel;
p_sys->pix_fmt = AV_PIX_FMT_NONE;
p_sys->profile = -1;
p_sys->level = -1;
cc_Init( &p_sys->cc );

set_video_color_settings( &p_dec->fmt_in.video, ctx );
if( p_dec->fmt_in.video.i_frame_rate_base &&
p_dec->fmt_in.video.i_frame_rate &&
(double) p_dec->fmt_in.video.i_frame_rate /
p_dec->fmt_in.video.i_frame_rate_base < 6 )
{
ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
}

if( var_InheritBool(p_dec, "low-delay") )
{
ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
ctx->active_thread_type = FF_THREAD_SLICE;
}

ret = ffmpeg_OpenCodec( p_dec, ctx, codec );
if( ret < 0 )
return ret;

switch( ctx->active_thread_type )
{
case FF_THREAD_FRAME:
msg_Dbg( p_dec, "using frame thread mode with %d threads",
ctx->thread_count );
break;
case FF_THREAD_SLICE:
msg_Dbg( p_dec, "using slice thread mode with %d threads",
ctx->thread_count );
break;
case 0:
if( ctx->thread_count > 1 )
msg_Warn( p_dec, "failed to enable threaded decoding" );
break;
default:
msg_Warn( p_dec, "using unknown thread mode with %d threads",
ctx->thread_count );
break;
}
return 0;
}







int InitVideoDec( vlc_object_t *obj )
{
decoder_t *p_dec = (decoder_t *)obj;
const AVCodec *p_codec;
AVCodecContext *p_context = ffmpeg_AllocContext( p_dec, &p_codec );
if( p_context == NULL )
return VLC_EGENERIC;

int i_val;


decoder_sys_t *p_sys = calloc( 1, sizeof(*p_sys) );
if( unlikely(p_sys == NULL) )
{
avcodec_free_context( &p_context );
return VLC_ENOMEM;
}

p_dec->p_sys = p_sys;
p_sys->p_context = p_context;
p_sys->p_codec = p_codec;
p_sys->p_va = NULL;
vlc_mutex_init( &p_sys->lock );


p_context->codec_tag = ffmpeg_CodecTag( p_dec->fmt_in.i_original_fourcc ?
p_dec->fmt_in.i_original_fourcc : p_dec->fmt_in.i_codec );


p_context->workaround_bugs =
var_InheritInteger( p_dec, "avcodec-workaround-bugs" );
p_context->err_recognition =
var_InheritInteger( p_dec, "avcodec-error-resilience" );

if( var_CreateGetBool( p_dec, "grayscale" ) )
p_context->flags |= AV_CODEC_FLAG_GRAY;


p_context->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT;

i_val = var_CreateGetInteger( p_dec, "avcodec-skiploopfilter" );
if( i_val >= 4 ) p_context->skip_loop_filter = AVDISCARD_ALL;
else if( i_val == 3 ) p_context->skip_loop_filter = AVDISCARD_NONKEY;
else if( i_val == 2 ) p_context->skip_loop_filter = AVDISCARD_BIDIR;
else if( i_val == 1 ) p_context->skip_loop_filter = AVDISCARD_NONREF;
else p_context->skip_loop_filter = AVDISCARD_DEFAULT;

if( var_CreateGetBool( p_dec, "avcodec-fast" ) )
p_context->flags2 |= AV_CODEC_FLAG2_FAST;


p_sys->b_hurry_up = var_CreateGetBool( p_dec, "avcodec-hurry-up" );
p_sys->b_show_corrupted = var_CreateGetBool( p_dec, "avcodec-corrupted" );

i_val = var_CreateGetInteger( p_dec, "avcodec-skip-frame" );
if( i_val >= 4 ) p_sys->i_skip_frame = AVDISCARD_ALL;
else if( i_val == 3 ) p_sys->i_skip_frame = AVDISCARD_NONKEY;
else if( i_val == 2 ) p_sys->i_skip_frame = AVDISCARD_BIDIR;
else if( i_val == 1 ) p_sys->i_skip_frame = AVDISCARD_NONREF;
else if( i_val == -1 ) p_sys->i_skip_frame = AVDISCARD_NONE;
else p_sys->i_skip_frame = AVDISCARD_DEFAULT;
p_context->skip_frame = p_sys->i_skip_frame;

i_val = var_CreateGetInteger( p_dec, "avcodec-skip-idct" );
if( i_val >= 4 ) p_context->skip_idct = AVDISCARD_ALL;
else if( i_val == 3 ) p_context->skip_idct = AVDISCARD_NONKEY;
else if( i_val == 2 ) p_context->skip_idct = AVDISCARD_BIDIR;
else if( i_val == 1 ) p_context->skip_idct = AVDISCARD_NONREF;
else if( i_val == -1 ) p_context->skip_idct = AVDISCARD_NONE;
else p_context->skip_idct = AVDISCARD_DEFAULT;


p_sys->b_direct_rendering = false;
atomic_init(&p_sys->b_dr_failure, false);
if( var_CreateGetBool( p_dec, "avcodec-dr" ) &&
(p_codec->capabilities & AV_CODEC_CAP_DR1) &&

p_sys->p_codec->id != AV_CODEC_ID_TSCC &&
p_sys->p_codec->id != AV_CODEC_ID_CSCD &&
p_sys->p_codec->id != AV_CODEC_ID_CINEPAK )
{


p_sys->b_direct_rendering = true;
}

p_context->get_format = ffmpeg_GetFormat;


p_context->get_buffer2 = lavc_GetFrame;
p_context->opaque = p_dec;
p_context->reordered_opaque = 0;

int i_thread_count = var_InheritInteger( p_dec, "avcodec-threads" );
if( i_thread_count <= 0 )
{
i_thread_count = vlc_GetCPUCount();
if( i_thread_count > 1 )
i_thread_count++;


#if VLC_WINSTORE_APP
i_thread_count = __MIN( i_thread_count, 6 );
#else
i_thread_count = __MIN( i_thread_count, p_codec->id == AV_CODEC_ID_HEVC ? 10 : 6 );
#endif
}
i_thread_count = __MIN( i_thread_count, p_codec->id == AV_CODEC_ID_HEVC ? 32 : 16 );
msg_Dbg( p_dec, "allowing %d thread(s) for decoding", i_thread_count );
p_context->thread_count = i_thread_count;
p_context->thread_safe_callbacks = true;

switch( p_codec->id )
{
case AV_CODEC_ID_MPEG4:
case AV_CODEC_ID_H263:
p_context->thread_type = 0;
break;
case AV_CODEC_ID_MPEG1VIDEO:
case AV_CODEC_ID_MPEG2VIDEO:
p_context->thread_type &= ~FF_THREAD_SLICE;

#if (LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 1, 0))
case AV_CODEC_ID_H264:
case AV_CODEC_ID_VC1:
case AV_CODEC_ID_WMV3:
p_context->thread_type &= ~FF_THREAD_FRAME;
#endif
default:
break;
}

if( p_context->thread_type & FF_THREAD_FRAME )
p_dec->i_extra_picture_buffers = 2 * p_context->thread_count;


date_Init(&p_sys->pts, 1, 30001);
p_sys->b_first_frame = true;
p_sys->i_late_frames = 0;
p_sys->b_from_preroll = false;
p_sys->i_last_output_frame = -1;
p_sys->framedrop = FRAMEDROP_NONE;


if( GetVlcChroma( &p_dec->fmt_out.video, p_context->pix_fmt ) != VLC_SUCCESS )
{

p_dec->fmt_out.i_codec = VLC_CODEC_I420;
}
p_dec->fmt_out.i_codec = p_dec->fmt_out.video.i_chroma;

p_dec->fmt_out.video.orientation = p_dec->fmt_in.video.orientation;

if( p_dec->fmt_in.video.p_palette ) {
p_sys->palette_sent = false;
p_dec->fmt_out.video.p_palette = malloc( sizeof(video_palette_t) );
if( p_dec->fmt_out.video.p_palette )
*p_dec->fmt_out.video.p_palette = *p_dec->fmt_in.video.p_palette;
} else
p_sys->palette_sent = true;


ffmpeg_InitCodec( p_dec );


if( OpenVideoCodec( p_dec ) < 0 )
{
free( p_sys );
avcodec_free_context( &p_context );
return VLC_EGENERIC;
}

p_dec->pf_decode = DecodeVideo;
p_dec->pf_flush = Flush;


if( p_context->profile != FF_PROFILE_UNKNOWN )
p_dec->fmt_in.i_profile = p_context->profile;
if( p_context->level != FF_LEVEL_UNKNOWN )
p_dec->fmt_in.i_level = p_context->level;
return VLC_SUCCESS;
}




static void Flush( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *p_context = p_sys->p_context;

p_sys->i_late_frames = 0;
p_sys->framedrop = FRAMEDROP_NONE;
cc_Flush( &p_sys->cc );




decoder_AbortPictures( p_dec, true );


if( avcodec_is_open( p_context ) )
avcodec_flush_buffers( p_context );

date_Set(&p_sys->pts, VLC_TICK_INVALID); 


decoder_AbortPictures( p_dec, false );
}

static block_t * filter_earlydropped_blocks( decoder_t *p_dec, block_t *block )
{
decoder_sys_t *p_sys = p_dec->p_sys;

if( !block )
return NULL;

if( block->i_flags & BLOCK_FLAG_PREROLL )
{



p_sys->i_late_frames = 0;
p_sys->framedrop = FRAMEDROP_NONE;
p_sys->b_from_preroll = true;
p_sys->i_last_late_delay = INT64_MAX;
}

if( p_sys->i_late_frames == 0 )
p_sys->framedrop = FRAMEDROP_NONE;

if( p_sys->framedrop == FRAMEDROP_NONE && p_sys->i_late_frames < 11 )
return block;

if( p_sys->i_last_output_frame >= 0 &&
p_sys->p_context->reordered_opaque - p_sys->i_last_output_frame > 24 )
{
p_sys->framedrop = FRAMEDROP_AGGRESSIVE_RECOVER;
}


if( p_sys->framedrop == FRAMEDROP_AGGRESSIVE_RECOVER )
{
if( !(block->i_flags & BLOCK_FLAG_TYPE_I) )
{
msg_Err( p_dec, "more than %"PRId64" frames of late video -> "
"dropping frame (computer too slow ?)",
p_sys->p_context->reordered_opaque - p_sys->i_last_output_frame );

vlc_mutex_lock(&p_sys->lock);
date_Set( &p_sys->pts, VLC_TICK_INVALID ); 
vlc_mutex_unlock(&p_sys->lock);
block_Release( block );
p_sys->i_late_frames--;
return NULL;
}
}

return block;
}

static vlc_tick_t interpolate_next_pts( decoder_t *p_dec, AVFrame *frame )
{
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *p_context = p_sys->p_context;

if( p_sys->pts.i_divider_num == 0 ||
date_Get( &p_sys->pts ) == VLC_TICK_INVALID )
return VLC_TICK_INVALID;

int i_tick = p_context->ticks_per_frame;
if( i_tick <= 0 )
i_tick = 1;


return date_Increment( &p_sys->pts, i_tick + frame->repeat_pict );
}

static void update_late_frame_count( decoder_t *p_dec, block_t *p_block,
vlc_tick_t current_time, vlc_tick_t i_pts,
vlc_tick_t i_next_pts, int64_t i_fnum )
{
decoder_sys_t *p_sys = p_dec->p_sys;

vlc_tick_t i_display_date = VLC_TICK_INVALID;
if( !p_block || !(p_block->i_flags & BLOCK_FLAG_PREROLL) )
i_display_date = decoder_GetDisplayDate( p_dec, current_time, i_pts );

vlc_tick_t i_threshold = i_next_pts != VLC_TICK_INVALID
? (i_next_pts - i_pts) / 2 : VLC_TICK_FROM_MS(20);

if( i_display_date != VLC_TICK_INVALID && i_display_date + i_threshold <= current_time )
{

if( p_sys->b_from_preroll )
{
if( p_sys->i_last_late_delay > current_time - i_display_date )
{
p_sys->i_last_late_delay = current_time - i_display_date;
return;
}
p_sys->b_from_preroll = false;
}

p_sys->i_late_frames++;
}
else
{
p_sys->i_last_output_frame = i_fnum;
p_sys->i_late_frames = 0;
}
}


static int DecodeSidedata( decoder_t *p_dec, const AVFrame *frame, picture_t *p_pic )
{
decoder_sys_t *p_sys = p_dec->p_sys;
bool format_changed = false;

#if (LIBAVUTIL_VERSION_MICRO >= 100)
#define FROM_AVRAT(default_factor, avrat) (uint64_t)(default_factor) * (avrat).num / (avrat).den

const AVFrameSideData *metadata =
av_frame_get_side_data( frame,
AV_FRAME_DATA_MASTERING_DISPLAY_METADATA );
if ( metadata )
{
const AVMasteringDisplayMetadata *hdr_meta =
(const AVMasteringDisplayMetadata *) metadata->data;
if ( hdr_meta->has_luminance )
{
#define ST2086_LUMA_FACTOR 10000
p_pic->format.mastering.max_luminance =
FROM_AVRAT(ST2086_LUMA_FACTOR, hdr_meta->max_luminance);
p_pic->format.mastering.min_luminance =
FROM_AVRAT(ST2086_LUMA_FACTOR, hdr_meta->min_luminance);
}
if ( hdr_meta->has_primaries )
{
#define ST2086_RED 2
#define ST2086_GREEN 0
#define ST2086_BLUE 1
#define LAV_RED 0
#define LAV_GREEN 1
#define LAV_BLUE 2
#define ST2086_PRIM_FACTOR 50000
p_pic->format.mastering.primaries[ST2086_RED*2 + 0] =
FROM_AVRAT(ST2086_PRIM_FACTOR, hdr_meta->display_primaries[LAV_RED][0]);
p_pic->format.mastering.primaries[ST2086_RED*2 + 1] =
FROM_AVRAT(ST2086_PRIM_FACTOR, hdr_meta->display_primaries[LAV_RED][1]);
p_pic->format.mastering.primaries[ST2086_GREEN*2 + 0] =
FROM_AVRAT(ST2086_PRIM_FACTOR, hdr_meta->display_primaries[LAV_GREEN][0]);
p_pic->format.mastering.primaries[ST2086_GREEN*2 + 1] =
FROM_AVRAT(ST2086_PRIM_FACTOR, hdr_meta->display_primaries[LAV_GREEN][1]);
p_pic->format.mastering.primaries[ST2086_BLUE*2 + 0] =
FROM_AVRAT(ST2086_PRIM_FACTOR, hdr_meta->display_primaries[LAV_BLUE][0]);
p_pic->format.mastering.primaries[ST2086_BLUE*2 + 1] =
FROM_AVRAT(ST2086_PRIM_FACTOR, hdr_meta->display_primaries[LAV_BLUE][1]);
p_pic->format.mastering.white_point[0] =
FROM_AVRAT(ST2086_PRIM_FACTOR, hdr_meta->white_point[0]);
p_pic->format.mastering.white_point[1] =
FROM_AVRAT(ST2086_PRIM_FACTOR, hdr_meta->white_point[1]);
}

if ( memcmp( &p_dec->fmt_out.video.mastering,
&p_pic->format.mastering,
sizeof(p_pic->format.mastering) ) )
{
p_dec->fmt_out.video.mastering = p_pic->format.mastering;
format_changed = true;
}
#undef FROM_AVRAT
}
#endif
#if (LIBAVUTIL_VERSION_MICRO >= 100 && LIBAVUTIL_VERSION_INT >= AV_VERSION_INT( 55, 60, 100 ) )
const AVFrameSideData *metadata_lt =
av_frame_get_side_data( frame,
AV_FRAME_DATA_CONTENT_LIGHT_LEVEL );
if ( metadata_lt )
{
const AVContentLightMetadata *light_meta =
(const AVContentLightMetadata *) metadata_lt->data;
p_pic->format.lighting.MaxCLL = light_meta->MaxCLL;
p_pic->format.lighting.MaxFALL = light_meta->MaxFALL;
if ( memcmp( &p_dec->fmt_out.video.lighting,
&p_pic->format.lighting,
sizeof(p_pic->format.lighting) ) )
{
p_dec->fmt_out.video.lighting = p_pic->format.lighting;
format_changed = true;
}
}
#endif

const AVFrameSideData *p_stereo3d_data =
av_frame_get_side_data( frame,
AV_FRAME_DATA_STEREO3D );
if( p_stereo3d_data )
{
const struct AVStereo3D *stereo_data =
(const AVStereo3D *) p_stereo3d_data->data;
switch (stereo_data->type)
{
case AV_STEREO3D_SIDEBYSIDE:
p_pic->format.multiview_mode = MULTIVIEW_STEREO_SBS;
break;
case AV_STEREO3D_TOPBOTTOM:
p_pic->format.multiview_mode = MULTIVIEW_STEREO_TB;
break;
case AV_STEREO3D_FRAMESEQUENCE:
p_pic->format.multiview_mode = MULTIVIEW_STEREO_FRAME;
break;
case AV_STEREO3D_COLUMNS:
p_pic->format.multiview_mode = MULTIVIEW_STEREO_ROW;
break;
case AV_STEREO3D_LINES:
p_pic->format.multiview_mode = MULTIVIEW_STEREO_COL;
break;
case AV_STEREO3D_CHECKERBOARD:
p_pic->format.multiview_mode = MULTIVIEW_STEREO_CHECKERBOARD;
break;
default:
case AV_STEREO3D_2D:
p_pic->format.multiview_mode = MULTIVIEW_2D;
break;
}
#if LIBAVUTIL_VERSION_CHECK( 56, 7, 0, 4, 100 )
p_pic->format.b_multiview_right_eye_first = stereo_data->flags & AV_STEREO3D_FLAG_INVERT;
p_pic->format.b_multiview_left_eye = (stereo_data->view == AV_STEREO3D_VIEW_LEFT);

p_dec->fmt_out.video.b_multiview_right_eye_first = p_pic->format.b_multiview_right_eye_first;
#endif

if (p_dec->fmt_out.video.multiview_mode != p_pic->format.multiview_mode)
{
p_dec->fmt_out.video.multiview_mode = p_pic->format.multiview_mode;
format_changed = true;
}
}
else
p_pic->format.multiview_mode = p_dec->fmt_out.video.multiview_mode;

if (format_changed && decoder_UpdateVideoFormat( p_dec ))
return -1;

const AVFrameSideData *p_avcc = av_frame_get_side_data( frame, AV_FRAME_DATA_A53_CC );
if( p_avcc )
{
cc_Extract( &p_sys->cc, CC_PAYLOAD_RAW, true, p_avcc->data, p_avcc->size );
if( p_sys->cc.b_reorder || p_sys->cc.i_data )
{
block_t *p_cc = block_Alloc( p_sys->cc.i_data );
if( p_cc )
{
memcpy( p_cc->p_buffer, p_sys->cc.p_data, p_sys->cc.i_data );
if( p_sys->cc.b_reorder )
p_cc->i_dts = p_cc->i_pts = p_pic->date;
else
p_cc->i_pts = p_cc->i_dts;
decoder_cc_desc_t desc;
desc.i_608_channels = p_sys->cc.i_608channels;
desc.i_708_channels = p_sys->cc.i_708channels;
desc.i_reorder_depth = 4;
decoder_QueueCc( p_dec, p_cc, &desc );
}
cc_Flush( &p_sys->cc );
}
}
return 0;
}






static int DecodeBlock( decoder_t *p_dec, block_t **pp_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *p_context = p_sys->p_context;

bool b_need_output_picture = true;
bool b_error = false;

block_t *p_block;

if( !p_context->extradata_size && p_dec->fmt_in.i_extra )
{
ffmpeg_InitCodec( p_dec );
if( !avcodec_is_open( p_context ) )
OpenVideoCodec( p_dec );
}

p_block = pp_block ? *pp_block : NULL;
if(!p_block && !(p_sys->p_codec->capabilities & AV_CODEC_CAP_DELAY) )
return VLCDEC_SUCCESS;

if( !avcodec_is_open( p_context ) )
{
if( p_block )
block_Release( p_block );
return VLCDEC_SUCCESS;
}



if( !p_block || !(p_block->i_flags & BLOCK_FLAG_PREROLL) )
b_need_output_picture = true;
else
b_need_output_picture = false;


if( p_sys->b_hurry_up )
{
p_context->skip_frame = p_sys->i_skip_frame;



if( p_dec->b_frame_drop_allowed )
p_block = filter_earlydropped_blocks( p_dec, p_block );
}

if( !b_need_output_picture || p_sys->framedrop == FRAMEDROP_NONREF )
{
p_context->skip_frame = __MAX( p_context->skip_frame, AVDISCARD_NONREF );
}






if( p_block && p_block->i_buffer > 0 )
{
p_block = block_Realloc( p_block, 0,
p_block->i_buffer + FF_INPUT_BUFFER_PADDING_SIZE );
if( !p_block )
return VLCDEC_SUCCESS;
p_block->i_buffer -= FF_INPUT_BUFFER_PADDING_SIZE;
*pp_block = p_block;
memset( p_block->p_buffer + p_block->i_buffer, 0,
FF_INPUT_BUFFER_PADDING_SIZE );
}

bool b_drain = ( pp_block == NULL );
bool b_drained = false;
bool b_first_output_sequence = true;

do
{
int i_used = 0;

if( (p_block && p_block->i_buffer > 0) || b_drain )
{
AVPacket pkt;
av_init_packet( &pkt );
if( p_block && p_block->i_buffer > 0 )
{
pkt.data = p_block->p_buffer;
pkt.size = p_block->i_buffer;
pkt.pts = p_block->i_pts != VLC_TICK_INVALID ? TO_AV_TS(p_block->i_pts) : AV_NOPTS_VALUE;
pkt.dts = p_block->i_dts != VLC_TICK_INVALID ? TO_AV_TS(p_block->i_dts) : AV_NOPTS_VALUE;
if (p_block->i_flags & BLOCK_FLAG_TYPE_I)
pkt.flags |= AV_PKT_FLAG_KEY;
}
else
{

pkt.data = NULL;
pkt.size = 0;
b_drain = false;
b_drained = true;
}

if( !p_sys->palette_sent )
{
uint8_t *pal = av_packet_new_side_data(&pkt, AV_PKT_DATA_PALETTE, AVPALETTE_SIZE);
if (pal) {
memcpy(pal, p_dec->fmt_in.video.p_palette->palette, AVPALETTE_SIZE);
p_sys->palette_sent = true;
}
}


if( p_block )
{
p_block->i_pts =
p_block->i_dts = VLC_TICK_INVALID;
}

int ret = avcodec_send_packet(p_context, &pkt);
if( ret != 0 && ret != AVERROR(EAGAIN) )
{
if (ret == AVERROR(ENOMEM) || ret == AVERROR(EINVAL))
{
msg_Err(p_dec, "avcodec_send_packet critical error");
b_error = true;
}
av_packet_unref( &pkt );
break;
}

struct frame_info_s *p_frame_info = &p_sys->frame_info[p_context->reordered_opaque % FRAME_INFO_DEPTH];
p_frame_info->b_eos = p_block && (p_block->i_flags & BLOCK_FLAG_END_OF_SEQUENCE);
p_frame_info->b_display = b_need_output_picture;

p_context->reordered_opaque++;
i_used = ret != AVERROR(EAGAIN) ? pkt.size : 0;
av_packet_unref( &pkt );

if( p_frame_info->b_eos && !b_drained )
{
avcodec_send_packet( p_context, NULL );
b_drained = true;
}
}

AVFrame *frame = av_frame_alloc();
if (unlikely(frame == NULL))
{
b_error = true;
break;
}

int ret = avcodec_receive_frame(p_context, frame);
if( ret != 0 && ret != AVERROR(EAGAIN) )
{
if (ret == AVERROR(ENOMEM) || ret == AVERROR(EINVAL))
{
msg_Err(p_dec, "avcodec_receive_frame critical error");
b_error = true;
}
av_frame_free(&frame);
if( ret == AVERROR_EOF )
break;
}
bool not_received_frame = ret;

if( p_block )
{

p_block->p_buffer += i_used;
p_block->i_buffer -= i_used;
}


if( not_received_frame )
{
av_frame_free(&frame);
if( i_used == 0 ) break;
continue;
}

struct frame_info_s *p_frame_info = &p_sys->frame_info[frame->reordered_opaque % FRAME_INFO_DEPTH];
if( p_frame_info->b_eos )
p_sys->b_first_frame = true;

vlc_mutex_lock(&p_sys->lock);


#if defined(FF_API_PKT_PTS)
int64_t av_pts = frame->pts == AV_NOPTS_VALUE ? frame->pkt_dts : frame->pts;
#else
int64_t av_pts = frame->pkt_pts;
#endif
vlc_tick_t i_pts;
if( av_pts == AV_NOPTS_VALUE )
i_pts = date_Get( &p_sys->pts );
else
i_pts = FROM_AV_TS(av_pts);


if( i_pts != VLC_TICK_INVALID )
date_Set( &p_sys->pts, i_pts );

const vlc_tick_t i_next_pts = interpolate_next_pts(p_dec, frame);

if( b_first_output_sequence )
{
update_late_frame_count( p_dec, p_block, vlc_tick_now(), i_pts,
i_next_pts, frame->reordered_opaque);
b_first_output_sequence = false;
}

if( !p_frame_info->b_display ||
( !p_sys->p_va && !frame->linesize[0] ) ||
( p_dec->b_frame_drop_allowed && (frame->flags & AV_FRAME_FLAG_CORRUPT) &&
!p_sys->b_show_corrupted ) )
{
vlc_mutex_unlock(&p_sys->lock);
av_frame_free(&frame);
continue;
}

if( p_context->pix_fmt == AV_PIX_FMT_PAL8
&& !p_dec->fmt_out.video.p_palette )
{



assert( p_dec->fmt_out.video.i_chroma != VLC_CODEC_RGBP );

video_palette_t *p_palette;
p_palette = p_dec->fmt_out.video.p_palette
= malloc( sizeof(video_palette_t) );
if( !p_palette )
{
vlc_mutex_unlock(&p_sys->lock);
b_error = true;
av_frame_free(&frame);
break;
}
static_assert( sizeof(p_palette->palette) == AVPALETTE_SIZE,
"Palette size mismatch between vlc and libavutil" );
assert( frame->data[1] != NULL );
memcpy( p_palette->palette, frame->data[1], AVPALETTE_SIZE );
p_palette->i_entries = AVPALETTE_COUNT;
p_dec->fmt_out.video.i_chroma = VLC_CODEC_RGBP;
if( decoder_UpdateVideoFormat( p_dec ) )
{
vlc_mutex_unlock(&p_sys->lock);
av_frame_free(&frame);
continue;
}
}

picture_t *p_pic = frame->opaque;
if( p_pic == NULL )
{ 


if (p_sys->p_va == NULL
&& lavc_UpdateVideoFormat(p_dec, p_context, p_context->pix_fmt,
p_context->pix_fmt, NULL) == 0
&& decoder_UpdateVideoOutput(p_dec, NULL) == 0)
p_pic = decoder_NewPicture(p_dec);

if( !p_pic )
{
vlc_mutex_unlock(&p_sys->lock);
av_frame_free(&frame);
break;
}


if( lavc_CopyPicture( p_dec, p_pic, frame ) != VLC_SUCCESS )
{
vlc_mutex_unlock(&p_sys->lock);
av_frame_free(&frame);
picture_Release( p_pic );
break;
}
}
else
{





p_pic = picture_Clone( p_pic );
if( unlikely(p_pic == NULL) )
{
vlc_mutex_unlock(&p_sys->lock);
av_frame_free(&frame);
break;
}
}

if( !p_dec->fmt_in.video.i_sar_num || !p_dec->fmt_in.video.i_sar_den )
{

p_dec->fmt_out.video.i_sar_num
= p_context->sample_aspect_ratio.num;
p_dec->fmt_out.video.i_sar_den
= p_context->sample_aspect_ratio.den;

if( !p_dec->fmt_out.video.i_sar_num || !p_dec->fmt_out.video.i_sar_den )
{
p_dec->fmt_out.video.i_sar_num = 1;
p_dec->fmt_out.video.i_sar_den = 1;
}
}

p_pic->date = i_pts;

p_pic->b_force = p_sys->b_first_frame;
p_pic->i_nb_fields = 2 + frame->repeat_pict;
p_pic->b_progressive = !frame->interlaced_frame;
p_pic->b_top_field_first = frame->top_field_first;

if (DecodeSidedata(p_dec, frame, p_pic))
i_pts = VLC_TICK_INVALID;

av_frame_free(&frame);


if (i_pts != VLC_TICK_INVALID)
{
if(p_frame_info->b_eos)
p_pic->b_still = true;
p_sys->b_first_frame = false;
vlc_mutex_unlock(&p_sys->lock);
decoder_QueueVideo( p_dec, p_pic );
}
else
{
vlc_mutex_unlock(&p_sys->lock);
picture_Release( p_pic );
}
} while( true );


if( b_drained )
avcodec_flush_buffers( p_sys->p_context );

if( p_block )
block_Release( p_block );

return b_error ? VLCDEC_ECRITICAL : VLCDEC_SUCCESS;
}

static int DecodeVideo( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
block_t **pp_block = p_block ? &p_block : NULL ;

if( p_block &&
p_block->i_flags & (BLOCK_FLAG_DISCONTINUITY|BLOCK_FLAG_CORRUPTED) )
{

if( p_block->i_flags & BLOCK_FLAG_DISCONTINUITY )
DecodeBlock( p_dec, NULL );
p_sys->i_late_frames = 0;
p_sys->i_last_output_frame = -1;
p_sys->framedrop = FRAMEDROP_NONE;

vlc_mutex_lock(&p_sys->lock);
date_Set( &p_sys->pts, VLC_TICK_INVALID ); 
vlc_mutex_unlock(&p_sys->lock);

cc_Flush( &p_sys->cc );

if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
block_Release( p_block );
p_block = NULL; 
}
}

return DecodeBlock( p_dec, pp_block );
}







void EndVideoDec( vlc_object_t *obj )
{
decoder_t *p_dec = (decoder_t *)obj;
decoder_sys_t *p_sys = p_dec->p_sys;
AVCodecContext *ctx = p_sys->p_context;


if( avcodec_is_open( ctx ) )
avcodec_flush_buffers( ctx );

cc_Flush( &p_sys->cc );

avcodec_free_context( &ctx );

if( p_sys->p_va )
vlc_va_Delete( p_sys->p_va );

free( p_sys );
}




static void ffmpeg_InitCodec( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
size_t i_size = p_dec->fmt_in.i_extra;

if( !i_size ) return;

if( p_sys->p_codec->id == AV_CODEC_ID_SVQ3 )
{
uint8_t *p;

p_sys->p_context->extradata_size = i_size + 12;
p = p_sys->p_context->extradata =
av_malloc( p_sys->p_context->extradata_size +
FF_INPUT_BUFFER_PADDING_SIZE );
if( !p )
return;

memcpy( &p[0], "SVQ3", 4 );
memset( &p[4], 0, 8 );
memcpy( &p[12], p_dec->fmt_in.p_extra, i_size );


if( p_sys->p_context->extradata_size > 0x5a &&
strncmp( (char*)&p[0x56], "SMI ", 4 ) )
{
uint8_t *psz = &p[0x52];

while( psz < &p[p_sys->p_context->extradata_size - 8] )
{
uint_fast32_t atom_size = GetDWBE( psz );
if( atom_size <= 1 )
{

break;
}
if( !strncmp( (char*)&psz[4], "SMI ", 4 ) )
{
memmove( &p[0x52], psz,
&p[p_sys->p_context->extradata_size] - psz );
break;
}

psz += atom_size;
}
}
}
else
{
p_sys->p_context->extradata_size = i_size;
p_sys->p_context->extradata =
av_malloc( i_size + FF_INPUT_BUFFER_PADDING_SIZE );
if( p_sys->p_context->extradata )
{
memcpy( p_sys->p_context->extradata,
p_dec->fmt_in.p_extra, i_size );
memset( p_sys->p_context->extradata + i_size,
0, FF_INPUT_BUFFER_PADDING_SIZE );
}
}
}

static void lavc_ReleaseFrame(void *opaque, uint8_t *data)
{
(void) data;
picture_t *picture = opaque;

picture_Release(picture);
}

static int lavc_va_GetFrame(struct AVCodecContext *ctx, AVFrame *frame)
{
decoder_t *dec = ctx->opaque;
decoder_sys_t *p_sys = dec->p_sys;
vlc_va_t *va = p_sys->p_va;

picture_t *pic;
pic = decoder_NewPicture(dec);
if (pic == NULL)
return -1;

if (vlc_va_Get(va, pic, &frame->data[0]))
{
msg_Err(dec, "hardware acceleration picture allocation failed");
picture_Release(pic);
return -1;
}
assert(frame->data[0] != NULL);


frame->data[3] = frame->data[0];

frame->buf[0] = av_buffer_create(frame->data[0], 0, lavc_ReleaseFrame, pic, 0);
if (unlikely(frame->buf[0] == NULL))
{
lavc_ReleaseFrame(pic, frame->data[0]);
return -1;
}

frame->opaque = pic;
return 0;
}

static int lavc_dr_GetFrame(struct AVCodecContext *ctx, AVFrame *frame)
{
decoder_t *dec = ctx->opaque;
decoder_sys_t *sys = dec->p_sys;

if (ctx->pix_fmt == AV_PIX_FMT_PAL8)
return -1;

picture_t *pic = decoder_NewPicture(dec);
if (pic == NULL)
return -1;

int width = frame->width;
int height = frame->height;
int aligns[AV_NUM_DATA_POINTERS];

avcodec_align_dimensions2(ctx, &width, &height, aligns);


assert(pic->p[0].i_pitch >= width * pic->p[0].i_pixel_pitch);
assert(pic->p[0].i_lines >= height);

for (int i = 0; i < pic->i_planes; i++)
{
if (pic->p[i].i_pitch % aligns[i])
{
if (!atomic_exchange(&sys->b_dr_failure, true))
msg_Warn(dec, "plane %d: pitch not aligned (%d%%%d): disabling direct rendering",
i, pic->p[i].i_pitch, aligns[i]);
goto error;
}
if (((uintptr_t)pic->p[i].p_pixels) % aligns[i])
{
if (!atomic_exchange(&sys->b_dr_failure, true))
msg_Warn(dec, "plane %d not aligned: disabling direct rendering", i);
goto error;
}
}


assert(pic->i_planes < PICTURE_PLANE_MAX);
static_assert(PICTURE_PLANE_MAX <= AV_NUM_DATA_POINTERS, "Oops!");

for (int i = 0; i < pic->i_planes; i++)
{
uint8_t *data = pic->p[i].p_pixels;
int size = pic->p[i].i_pitch * pic->p[i].i_lines;

frame->data[i] = data;
frame->linesize[i] = pic->p[i].i_pitch;
frame->buf[i] = av_buffer_create(data, size, lavc_ReleaseFrame,
pic, 0);
if (unlikely(frame->buf[i] == NULL))
{
while (i > 0)
av_buffer_unref(&frame->buf[--i]);
goto error;
}
picture_Hold(pic);
}

frame->opaque = pic;

assert(pic->i_planes > 0);
picture_Release(pic);
return 0;
error:
picture_Release(pic);
return -1;
}







static int lavc_GetFrame(struct AVCodecContext *ctx, AVFrame *frame, int flags)
{
decoder_t *dec = ctx->opaque;
decoder_sys_t *sys = dec->p_sys;

for (unsigned i = 0; i < AV_NUM_DATA_POINTERS; i++)
{
frame->data[i] = NULL;
frame->linesize[i] = 0;
frame->buf[i] = NULL;
}
frame->opaque = NULL;

vlc_mutex_lock(&sys->lock);
if (sys->p_va == NULL)
{
if (!sys->b_direct_rendering)
{
vlc_mutex_unlock(&sys->lock);
return avcodec_default_get_buffer2(ctx, frame, flags);
}




if (lavc_UpdateVideoFormat(dec, ctx, ctx->pix_fmt, ctx->pix_fmt, NULL) ||
decoder_UpdateVideoOutput(dec, NULL))
{
vlc_mutex_unlock(&sys->lock);
return -1;
}
}

if (sys->p_va != NULL)
{
int ret = lavc_va_GetFrame(ctx, frame);
vlc_mutex_unlock(&sys->lock);
return ret;
}



int ret = lavc_dr_GetFrame(ctx, frame);
vlc_mutex_unlock(&sys->lock);
if (ret)
ret = avcodec_default_get_buffer2(ctx, frame, flags);
return ret;
}

static enum PixelFormat ffmpeg_GetFormat( AVCodecContext *p_context,
const enum PixelFormat *pi_fmt )
{
decoder_t *p_dec = p_context->opaque;
decoder_sys_t *p_sys = p_dec->p_sys;
video_format_t fmt;


enum PixelFormat swfmt = avcodec_default_get_format(p_context, pi_fmt);
bool can_hwaccel = false;

for (size_t i = 0; pi_fmt[i] != AV_PIX_FMT_NONE; i++)
{
const AVPixFmtDescriptor *dsc = av_pix_fmt_desc_get(pi_fmt[i]);
if (dsc == NULL)
continue;
bool hwaccel = (dsc->flags & AV_PIX_FMT_FLAG_HWACCEL) != 0;

msg_Dbg( p_dec, "available %sware decoder output format %d (%s)",
hwaccel ? "hard" : "soft", pi_fmt[i], dsc->name );
if (hwaccel)
can_hwaccel = true;
}

if (p_sys->pix_fmt == AV_PIX_FMT_NONE)
goto no_reuse;





if (lavc_GetVideoFormat(p_dec, &fmt, p_context, p_sys->pix_fmt, swfmt) != 0)
{
msg_Dbg(p_dec, "get format failed");
goto no_reuse;
}
if (fmt.i_width != p_sys->decoder_width ||
fmt.i_height != p_sys->decoder_height)
{
msg_Dbg(p_dec, "mismatched dimensions %ux%u was %ux%u", fmt.i_width, fmt.i_height,
p_sys->decoder_width, p_sys->decoder_height);
goto no_reuse;
}
if (p_context->profile != p_sys->profile || p_context->level > p_sys->level)
{
msg_Dbg(p_dec, "mismatched profile level %d/%d was %d/%d", p_context->profile,
p_context->level, p_sys->profile, p_sys->level);
goto no_reuse;
}

for (size_t i = 0; pi_fmt[i] != AV_PIX_FMT_NONE; i++)
if (pi_fmt[i] == p_sys->pix_fmt)
{
msg_Dbg(p_dec, "reusing decoder output format %d", pi_fmt[i]);
return p_sys->pix_fmt;
}

no_reuse:
if (p_sys->p_va != NULL)
{
msg_Err(p_dec, "existing hardware acceleration cannot be reused");
vlc_va_Delete(p_sys->p_va);
p_sys->p_va = NULL;
p_context->hwaccel_context = NULL;
}

p_sys->profile = p_context->profile;
p_sys->level = p_context->level;

if (!can_hwaccel)
return swfmt;

#if (LIBAVCODEC_VERSION_MICRO >= 100) && (LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 83, 101))

if (p_context->active_thread_type)
{
msg_Warn(p_dec, "thread type %d: disabling hardware acceleration",
p_context->active_thread_type);
return swfmt;
}
#endif

vlc_mutex_lock(&p_sys->lock);

static const enum PixelFormat hwfmts[] =
{
#if defined(_WIN32)
AV_PIX_FMT_D3D11VA_VLD,
AV_PIX_FMT_DXVA2_VLD,
#endif
AV_PIX_FMT_VAAPI_VLD,
AV_PIX_FMT_VDPAU,
AV_PIX_FMT_NONE,
};

const AVPixFmtDescriptor *src_desc = av_pix_fmt_desc_get(swfmt);

for( size_t i = 0; hwfmts[i] != AV_PIX_FMT_NONE; i++ )
{
enum PixelFormat hwfmt = AV_PIX_FMT_NONE;
for( size_t j = 0; hwfmt == AV_PIX_FMT_NONE && pi_fmt[j] != AV_PIX_FMT_NONE; j++ )
if( hwfmts[i] == pi_fmt[j] )
hwfmt = hwfmts[i];

if( hwfmt == AV_PIX_FMT_NONE )
continue;

if (!vlc_va_MightDecode(hwfmt, swfmt))
continue; 
if (p_context->width == 0 || p_context->height == 0)
{ 
msg_Err(p_dec, "unspecified video dimensions");
continue;
}
const AVPixFmtDescriptor *dsc = av_pix_fmt_desc_get(hwfmt);
vlc_decoder_device *init_device = NULL;
msg_Dbg(p_dec, "trying format %s", dsc ? dsc->name : "unknown");
if (lavc_UpdateVideoFormat(p_dec, p_context, hwfmt, swfmt, &init_device) ||
init_device == NULL)
continue; 
vlc_mutex_unlock(&p_sys->lock);

p_dec->fmt_out.video.i_chroma = 0; 
vlc_video_context *vctx_out;
vlc_va_t *va = vlc_va_New(VLC_OBJECT(p_dec), p_context, hwfmt, src_desc,
&p_dec->fmt_in, init_device,
&p_dec->fmt_out.video, &vctx_out);
if (init_device)
vlc_decoder_device_Release(init_device);
vlc_mutex_lock(&p_sys->lock);
if (va == NULL)
continue; 
assert(p_dec->fmt_out.video.i_chroma != 0);
assert(vctx_out != NULL);
p_dec->fmt_out.i_codec = p_dec->fmt_out.video.i_chroma;

if (decoder_UpdateVideoOutput(p_dec, vctx_out))
{
vlc_va_Delete(va);
p_context->hwaccel_context = NULL;
continue; 
}

p_sys->p_va = va;
p_sys->pix_fmt = hwfmt;
p_context->draw_horiz_band = NULL;
vlc_mutex_unlock(&p_sys->lock);
return hwfmt;
}

vlc_mutex_unlock(&p_sys->lock);

p_sys->pix_fmt = swfmt;
return swfmt;
}
