#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
typedef struct
{
size_t size;
unsigned pitches[PICTURE_PLANE_MAX];
unsigned lines[PICTURE_PLANE_MAX];
date_t pts;
} decoder_sys_t;
static int OpenDecoder ( vlc_object_t * );
static int OpenPacketizer( vlc_object_t * );
static void CloseCommon ( vlc_object_t * );
vlc_module_begin ()
set_description( N_("Pseudo raw video decoder") )
set_capability( "video decoder", 50 )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_VCODEC )
set_callbacks( OpenDecoder, CloseCommon )
add_submodule ()
set_description( N_("Pseudo raw video packetizer") )
set_capability( "packetizer", 100 )
set_callbacks( OpenPacketizer, CloseCommon )
vlc_module_end ()
static int OpenCommon( decoder_t *p_dec )
{
const vlc_chroma_description_t *dsc =
vlc_fourcc_GetChromaDescription( p_dec->fmt_in.i_codec );
if( dsc == NULL || dsc->plane_count == 0 )
return VLC_EGENERIC;
if( p_dec->fmt_in.video.i_width <= 0 || p_dec->fmt_in.video.i_height == 0 )
{
msg_Err( p_dec, "invalid display size %dx%d",
p_dec->fmt_in.video.i_width, p_dec->fmt_in.video.i_height );
return VLC_EGENERIC;
}
decoder_sys_t *p_sys = calloc(1, sizeof(*p_sys));
if( unlikely(p_sys == NULL) )
return VLC_ENOMEM;
if( !p_dec->fmt_in.video.i_visible_width )
p_dec->fmt_in.video.i_visible_width = p_dec->fmt_in.video.i_width;
if( !p_dec->fmt_in.video.i_visible_height )
p_dec->fmt_in.video.i_visible_height = p_dec->fmt_in.video.i_height;
es_format_Copy( &p_dec->fmt_out, &p_dec->fmt_in );
if( p_dec->fmt_in.i_codec == VLC_CODEC_YUV2 )
{
p_dec->fmt_out.video.i_chroma =
p_dec->fmt_out.i_codec = VLC_CODEC_YUYV;
}
if( p_dec->fmt_out.video.i_frame_rate == 0 ||
p_dec->fmt_out.video.i_frame_rate_base == 0)
{
msg_Warn( p_dec, "invalid frame rate %d/%d, using 25 fps instead",
p_dec->fmt_out.video.i_frame_rate,
p_dec->fmt_out.video.i_frame_rate_base);
date_Init( &p_sys->pts, 25, 1 );
}
else
date_Init( &p_sys->pts, p_dec->fmt_out.video.i_frame_rate,
p_dec->fmt_out.video.i_frame_rate_base );
for( unsigned i = 0; i < dsc->plane_count; i++ )
{
unsigned pitch = ((p_dec->fmt_in.video.i_width + (dsc->p[i].w.den - 1)) / dsc->p[i].w.den)
* dsc->p[i].w.num * dsc->pixel_size;
unsigned lines = ((p_dec->fmt_in.video.i_height + (dsc->p[i].h.den - 1)) / dsc->p[i].h.den)
* dsc->p[i].h.num;
p_sys->pitches[i] = pitch;
p_sys->lines[i] = lines;
p_sys->size += pitch * lines;
}
p_dec->p_sys = p_sys;
return VLC_SUCCESS;
}
static void Flush( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
date_Set( &p_sys->pts, VLC_TICK_INVALID );
}
static block_t *DecodeBlock( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
if( p_block->i_flags & (BLOCK_FLAG_CORRUPTED|BLOCK_FLAG_DISCONTINUITY) )
{
date_Set( &p_sys->pts, p_block->i_dts );
if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
block_Release( p_block );
return NULL;
}
}
if( p_block->i_pts == VLC_TICK_INVALID && p_block->i_dts == VLC_TICK_INVALID &&
date_Get( &p_sys->pts ) == VLC_TICK_INVALID )
{
block_Release( p_block );
return NULL;
}
if( p_block->i_pts != VLC_TICK_INVALID )
{
date_Set( &p_sys->pts, p_block->i_pts );
}
else if( p_block->i_dts != VLC_TICK_INVALID )
{
date_Set( &p_sys->pts, p_block->i_dts );
}
if( p_block->i_buffer < p_sys->size )
{
msg_Warn( p_dec, "invalid frame size (%zu < %zu)",
p_block->i_buffer, p_sys->size );
block_Release( p_block );
return NULL;
}
return p_block;
}
static void FillPicture( decoder_t *p_dec, block_t *p_block, picture_t *p_pic )
{
decoder_sys_t *p_sys = p_dec->p_sys;
const uint8_t *p_src = p_block->p_buffer;
for( int i = 0; i < p_pic->i_planes; i++ )
{
uint8_t *p_dst = p_pic->p[i].p_pixels;
for( int x = 0; x < p_pic->p[i].i_visible_lines; x++ )
{
memcpy( p_dst, p_src, p_pic->p[i].i_visible_pitch );
if( p_dec->fmt_in.i_codec == VLC_CODEC_YUV2 ) {
for( int y = 0; y < p_pic->p[i].i_visible_pitch; y++ ) {
p_dst[2*y + 1] ^= 0x80;
}
}
p_src += p_sys->pitches[i];
p_dst += p_pic->p[i].i_pitch;
}
p_src += p_sys->pitches[i]
* (p_sys->lines[i] - p_pic->p[i].i_visible_lines);
}
}
static int DecodeFrame( decoder_t *p_dec, block_t *p_block )
{
if( p_block == NULL ) 
return VLCDEC_SUCCESS;
p_block = DecodeBlock( p_dec, p_block );
if( p_block == NULL )
return VLCDEC_SUCCESS;
decoder_sys_t *p_sys = p_dec->p_sys;
picture_t *p_pic = NULL;
if( !decoder_UpdateVideoFormat( p_dec ) )
p_pic = decoder_NewPicture( p_dec );
if( p_pic == NULL )
{
block_Release( p_block );
return VLCDEC_SUCCESS;
}
FillPicture( p_dec, p_block, p_pic );
p_pic->date = date_Get( &p_sys->pts );
date_Increment( &p_sys->pts, 1 );
if( p_block->i_flags & BLOCK_FLAG_INTERLACED_MASK )
{
p_pic->b_progressive = false;
p_pic->i_nb_fields = (p_block->i_flags & BLOCK_FLAG_SINGLE_FIELD) ? 1 : 2;
if( p_block->i_flags & BLOCK_FLAG_TOP_FIELD_FIRST )
p_pic->b_top_field_first = true;
else
p_pic->b_top_field_first = false;
}
else
p_pic->b_progressive = true;
block_Release( p_block );
decoder_QueueVideo( p_dec, p_pic );
return VLCDEC_SUCCESS;
}
static int OpenDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
int ret = OpenCommon( p_dec );
if( ret == VLC_SUCCESS )
{
p_dec->pf_decode = DecodeFrame;
p_dec->pf_flush = Flush;
}
return ret;
}
static block_t *SendFrame( decoder_t *p_dec, block_t **pp_block )
{
if( pp_block == NULL ) 
return NULL;
block_t *p_block = *pp_block;
if( p_block == NULL )
return NULL;
*pp_block = NULL;
p_block = DecodeBlock( p_dec, p_block );
if( p_block == NULL )
return NULL;
decoder_sys_t *p_sys = p_dec->p_sys;
p_block->i_dts = p_block->i_pts = date_Get( &p_sys->pts );
date_Increment( &p_sys->pts, 1 );
return p_block;
}
static int OpenPacketizer( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
int ret = OpenCommon( p_dec );
if( ret == VLC_SUCCESS )
{
p_dec->pf_packetize = SendFrame;
p_dec->pf_flush = Flush;
}
return ret;
}
static void CloseCommon( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
free( p_dec->p_sys );
}
