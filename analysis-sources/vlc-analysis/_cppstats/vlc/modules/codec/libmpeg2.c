#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_block_helper.h>
#include <vlc_cpu.h>
#include "cc.h"
#include "synchro.h"
#include <mpeg2.h>
#define DPB_COUNT (3+1)
typedef struct
{
picture_t *p_picture;
bool b_linked;
bool b_displayed;
} picture_dpb_t;
typedef struct
{
mpeg2dec_t *p_mpeg2dec;
const mpeg2_info_t *p_info;
bool b_skip;
vlc_tick_t i_previous_pts;
vlc_tick_t i_current_pts;
vlc_tick_t i_previous_dts;
vlc_tick_t i_current_dts;
bool b_garbage_pic;
bool b_after_sequence_header; 
bool b_slice_i; 
bool b_second_field;
bool b_preroll;
picture_dpb_t p_dpb[DPB_COUNT];
decoder_synchro_t *p_synchro;
int i_sar_num;
int i_sar_den;
vlc_tick_t i_last_frame_pts;
uint32_t i_cc_flags;
vlc_tick_t i_cc_pts;
vlc_tick_t i_cc_dts;
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
cc_data_t cc;
#endif
uint8_t *p_gop_user_data;
uint32_t i_gop_user_data;
} decoder_sys_t;
static int OpenDecoder( vlc_object_t * );
static void CloseDecoder( vlc_object_t * );
static int DecodeVideo( decoder_t *, block_t *);
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
static void SendCc( decoder_t *p_dec );
#endif
static picture_t *GetNewPicture( decoder_t * );
static void PutPicture( decoder_t *, picture_t * );
static void GetAR( decoder_t *p_dec );
static void Reset( decoder_t *p_dec );
static void DpbInit( decoder_t * );
static void DpbClean( decoder_t * );
static picture_t *DpbNewPicture( decoder_t * );
static void DpbUnlinkPicture( decoder_t *, picture_t * );
static int DpbDisplayPicture( decoder_t *, picture_t * );
vlc_module_begin ()
set_description( N_("MPEG I/II video decoder (using libmpeg2)") )
set_capability( "video decoder", 50 )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_VCODEC )
set_callbacks( OpenDecoder, CloseDecoder )
add_shortcut( "libmpeg2" )
vlc_module_end ()
static int OpenDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;
uint32_t i_accel = 0;
if( p_dec->fmt_in.i_codec != VLC_CODEC_MPGV )
return VLC_EGENERIC;
switch( p_dec->fmt_in.i_original_fourcc )
{
case VLC_FOURCC('m','p','g','1'):
case VLC_FOURCC('m','p','g','2'):
case VLC_FOURCC('m','p','g','v'):
case VLC_FOURCC('P','I','M','1'):
case VLC_FOURCC('h','d','v','2'):
break;
default:
if( p_dec->fmt_in.i_original_fourcc )
return VLC_EGENERIC;
break;
}
if( ( p_dec->p_sys = p_sys = calloc( 1, sizeof(*p_sys)) ) == NULL )
return VLC_ENOMEM;
p_sys->p_mpeg2dec = NULL;
p_sys->p_synchro = NULL;
p_sys->p_info = NULL;
p_sys->i_current_pts = 0;
p_sys->i_previous_pts = 0;
p_sys->i_current_dts = 0;
p_sys->i_previous_dts = 0;
p_sys->i_sar_num = 0;
p_sys->i_sar_den = 0;
p_sys->b_garbage_pic = false;
p_sys->b_slice_i = false;
p_sys->b_second_field = false;
p_sys->b_skip = false;
p_sys->b_preroll = false;
DpbInit( p_dec );
p_sys->i_cc_pts = 0;
p_sys->i_cc_dts = 0;
p_sys->i_cc_flags = 0;
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
cc_Init( &p_sys->cc );
#endif
p_sys->p_gop_user_data = NULL;
p_sys->i_gop_user_data = 0;
#if defined( __i386__ ) || defined( __x86_64__ )
if( vlc_CPU_MMX() )
i_accel |= MPEG2_ACCEL_X86_MMX;
if( vlc_CPU_3dNOW() )
i_accel |= MPEG2_ACCEL_X86_3DNOW;
if( vlc_CPU_MMXEXT() )
i_accel |= MPEG2_ACCEL_X86_MMXEXT;
#elif defined( __powerpc__ ) || defined( __ppc__ ) || defined( __ppc64__ )
if( vlc_CPU_ALTIVEC() )
i_accel |= MPEG2_ACCEL_PPC_ALTIVEC;
#elif defined(__arm__)
#if defined(MPEG2_ACCEL_ARM)
i_accel |= MPEG2_ACCEL_ARM;
#endif
#if defined(MPEG2_ACCEL_ARM_NEON)
if( vlc_CPU_ARM_NEON() )
i_accel |= MPEG2_ACCEL_ARM_NEON;
#endif
#else
i_accel = MPEG2_ACCEL_DETECT;
#endif
mpeg2_accel( i_accel );
p_sys->p_mpeg2dec = mpeg2_init();
if( p_sys->p_mpeg2dec == NULL)
{
msg_Err( p_dec, "mpeg2_init() failed" );
free( p_sys );
return VLC_EGENERIC;
}
p_sys->p_info = mpeg2_info( p_sys->p_mpeg2dec );
p_dec->pf_decode = DecodeVideo;
p_dec->pf_flush = Reset;
p_dec->fmt_out.i_codec = 0;
return VLC_SUCCESS;
}
static picture_t *DecodeBlock( decoder_t *p_dec, block_t **pp_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
mpeg2_state_t state;
block_t *p_block;
if( !pp_block || !*pp_block )
return NULL;
p_block = *pp_block;
if( p_block->i_flags & (BLOCK_FLAG_CORRUPTED) )
Reset( p_dec );
while( 1 )
{
state = mpeg2_parse( p_sys->p_mpeg2dec );
switch( state )
{
case STATE_SEQUENCE:
{
DpbClean( p_dec );
mpeg2_custom_fbuf( p_sys->p_mpeg2dec, 1 );
if( p_sys->p_synchro )
decoder_SynchroRelease( p_sys->p_synchro );
if( p_sys->p_info->sequence->frame_period <= 0 )
p_sys->p_synchro = NULL;
else
p_sys->p_synchro =
decoder_SynchroInit( p_dec, (uint32_t)(UINT64_C(1001000000) *
27 / p_sys->p_info->sequence->frame_period) );
p_sys->b_after_sequence_header = true;
GetAR( p_dec );
for( int i = 0; i < 2; i++ )
{
picture_t *p_picture = DpbNewPicture( p_dec );
if( !p_picture )
{
Reset( p_dec );
block_Release( p_block );
return NULL;
}
PutPicture( p_dec, p_picture );
}
break;
}
case STATE_GOP:
if( p_sys->p_info->user_data_len > 2 )
{
free( p_sys->p_gop_user_data );
p_sys->p_gop_user_data = calloc( p_sys->p_info->user_data_len, sizeof(uint8_t) );
if( p_sys->p_gop_user_data )
{
p_sys->i_gop_user_data = p_sys->p_info->user_data_len;
memcpy( p_sys->p_gop_user_data, p_sys->p_info->user_data, p_sys->p_info->user_data_len );
}
}
break;
case STATE_PICTURE:
{
const mpeg2_info_t *p_info = p_sys->p_info;
const mpeg2_picture_t *p_current = p_info->current_picture;
vlc_tick_t i_pts, i_dts;
if( p_sys->b_after_sequence_header &&
(p_current->flags &
PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_P )
{
msg_Dbg( p_dec, "intra-slice refresh stream" );
decoder_SynchroNewPicture( p_sys->p_synchro,
I_CODING_TYPE, 2, VLC_TICK_INVALID, VLC_TICK_INVALID,
p_info->sequence->flags & SEQ_FLAG_LOW_DELAY );
decoder_SynchroDecode( p_sys->p_synchro );
decoder_SynchroEnd( p_sys->p_synchro, I_CODING_TYPE, 0 );
p_sys->b_slice_i = true;
}
p_sys->b_after_sequence_header = false;
#if defined(PIC_FLAG_PTS)
i_pts = p_current->flags & PIC_FLAG_PTS ?
( ( p_current->pts ==
(uint32_t)p_sys->i_current_pts ) ?
p_sys->i_current_pts : p_sys->i_previous_pts ) : 0;
i_dts = 0;
if( !i_pts && !p_block->i_pts && p_block->i_dts > 0 )
{
if( p_info->sequence->flags & SEQ_FLAG_LOW_DELAY ||
(p_current->flags &
PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_B )
{
i_pts = p_block->i_dts;
}
}
p_block->i_pts = p_block->i_dts = 0;
#else 
i_pts = p_current->flags & PIC_FLAG_TAGS ?
( ( p_current->tag == (uint32_t)p_sys->i_current_pts ) ?
p_sys->i_current_pts : p_sys->i_previous_pts ) : 0;
i_dts = p_current->flags & PIC_FLAG_TAGS ?
( ( p_current->tag2 == (uint32_t)p_sys->i_current_dts ) ?
p_sys->i_current_dts : p_sys->i_previous_dts ) : 0;
#endif
decoder_SynchroNewPicture( p_sys->p_synchro,
p_current->flags & PIC_MASK_CODING_TYPE,
p_current->nb_fields == 1 ? 2 :
p_current->nb_fields, i_pts, i_dts,
p_info->sequence->flags & SEQ_FLAG_LOW_DELAY );
picture_t *p_pic;
if( p_dec->b_frame_drop_allowed && !p_sys->b_preroll &&
!(p_sys->b_slice_i
&& ((p_current->flags
& PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_P))
&& !decoder_SynchroChoose( p_sys->p_synchro,
p_current->flags
& PIC_MASK_CODING_TYPE,
0 ,
p_info->sequence->flags & SEQ_FLAG_LOW_DELAY ) )
p_pic = NULL;
else
{
p_pic = DpbNewPicture( p_dec );
if( !p_pic )
{
Reset( p_dec );
p_pic = DpbNewPicture( p_dec );
if( !p_pic )
{
mpeg2_reset( p_sys->p_mpeg2dec, 1 );
block_Release( p_block );
return NULL;
}
}
}
mpeg2_skip( p_sys->p_mpeg2dec, p_pic == NULL );
p_sys->b_skip = p_pic == NULL;
if( p_pic != NULL )
decoder_SynchroDecode( p_sys->p_synchro );
else
decoder_SynchroTrash( p_sys->p_synchro );
PutPicture( p_dec, p_pic );
if( p_info->user_data_len > 2 || p_sys->i_gop_user_data > 2 )
{
p_sys->i_cc_pts = i_pts;
p_sys->i_cc_dts = i_dts;
if( (p_current->flags
& PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_P )
p_sys->i_cc_flags = BLOCK_FLAG_TYPE_P;
else if( (p_current->flags
& PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_B )
p_sys->i_cc_flags = BLOCK_FLAG_TYPE_B;
else p_sys->i_cc_flags = BLOCK_FLAG_TYPE_I;
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
bool b_top_field_first = p_sys->p_info->current_picture->flags
& PIC_FLAG_TOP_FIELD_FIRST;
#endif
if( p_sys->i_gop_user_data > 2 )
{
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
cc_ProbeAndExtract( &p_sys->cc, b_top_field_first,
&p_sys->p_gop_user_data[0], p_sys->i_gop_user_data );
#endif
p_sys->i_gop_user_data = 0;
}
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
if( p_info->user_data_len > 2 )
cc_ProbeAndExtract( &p_sys->cc, b_top_field_first,
&p_info->user_data[0], p_info->user_data_len );
if( p_sys->cc.i_data )
SendCc( p_dec );
#endif
}
}
break;
case STATE_BUFFER:
if( !p_block->i_buffer )
{
block_Release( p_block );
return NULL;
}
if( (p_block->i_flags & (BLOCK_FLAG_DISCONTINUITY
| BLOCK_FLAG_CORRUPTED)) &&
p_sys->p_synchro &&
p_sys->p_info->sequence &&
p_sys->p_info->sequence->width != (unsigned)-1 )
{
decoder_SynchroReset( p_sys->p_synchro );
if( p_sys->p_info->current_fbuf != NULL &&
p_sys->p_info->current_fbuf->id != NULL )
{
p_sys->b_garbage_pic = true;
}
if( p_sys->b_slice_i )
{
decoder_SynchroNewPicture( p_sys->p_synchro,
I_CODING_TYPE, 2, VLC_TICK_INVALID, VLC_TICK_INVALID,
p_sys->p_info->sequence->flags &
SEQ_FLAG_LOW_DELAY );
decoder_SynchroDecode( p_sys->p_synchro );
decoder_SynchroEnd( p_sys->p_synchro, I_CODING_TYPE, 0 );
}
}
if( p_block->i_flags & BLOCK_FLAG_PREROLL )
{
p_sys->b_preroll = true;
}
else if( p_sys->b_preroll )
{
p_sys->b_preroll = false;
if( p_sys->p_synchro )
decoder_SynchroReset( p_sys->p_synchro );
}
#if defined(PIC_FLAG_PTS)
if( p_block->i_pts )
{
mpeg2_pts( p_sys->p_mpeg2dec, (uint32_t)p_block->i_pts );
#else 
if( p_block->i_pts || p_block->i_dts )
{
mpeg2_tag_picture( p_sys->p_mpeg2dec,
(uint32_t)p_block->i_pts,
(uint32_t)p_block->i_dts );
#endif
p_sys->i_previous_pts = p_sys->i_current_pts;
p_sys->i_current_pts = p_block->i_pts;
p_sys->i_previous_dts = p_sys->i_current_dts;
p_sys->i_current_dts = p_block->i_dts;
}
mpeg2_buffer( p_sys->p_mpeg2dec, p_block->p_buffer,
p_block->p_buffer + p_block->i_buffer );
p_block->i_buffer = 0;
break;
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
case STATE_SEQUENCE_MODIFIED:
GetAR( p_dec );
break;
#endif
case STATE_PICTURE_2ND:
p_sys->b_second_field = true;
break;
case STATE_INVALID_END:
case STATE_END:
case STATE_SLICE:
{
picture_t *p_pic = NULL;
if( p_sys->p_info->display_fbuf &&
p_sys->p_info->display_fbuf->id )
{
p_pic = p_sys->p_info->display_fbuf->id;
if( DpbDisplayPicture( p_dec, p_pic ) )
p_pic = NULL;
decoder_SynchroEnd( p_sys->p_synchro,
p_sys->p_info->display_picture->flags & PIC_MASK_CODING_TYPE,
p_sys->b_garbage_pic );
if( p_pic )
{
p_pic->date = decoder_SynchroDate( p_sys->p_synchro );
if( p_sys->b_garbage_pic )
{
p_pic->date = VLC_TICK_INVALID; 
p_sys->b_garbage_pic = false;
}
}
}
if( p_sys->p_info->discard_fbuf &&
p_sys->p_info->discard_fbuf->id )
{
DpbUnlinkPicture( p_dec, p_sys->p_info->discard_fbuf->id );
}
if( p_pic )
{
if( state == STATE_END )
p_pic->b_force = true; 
if( p_sys->i_last_frame_pts == p_pic->date )
p_pic->date++;
p_sys->i_last_frame_pts = p_pic->date;
return p_pic;
}
break;
}
case STATE_INVALID:
{
msg_Err( p_dec, "invalid picture encountered" );
break;
}
default:
break;
}
}
return NULL;
}
static int DecodeVideo( decoder_t *p_dec, block_t *p_block)
{
if( p_block == NULL ) 
return VLCDEC_SUCCESS;
block_t **pp_block = &p_block;
picture_t *p_pic;
while( ( p_pic = DecodeBlock( p_dec, pp_block ) ) != NULL )
decoder_QueueVideo( p_dec, p_pic );
return VLCDEC_SUCCESS;
}
static void CloseDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
cc_Flush( &p_sys->cc );
#endif
DpbClean( p_dec );
free( p_sys->p_gop_user_data );
if( p_sys->p_synchro ) decoder_SynchroRelease( p_sys->p_synchro );
if( p_sys->p_mpeg2dec ) mpeg2_close( p_sys->p_mpeg2dec );
free( p_sys );
}
static void Reset( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
cc_Flush( &p_sys->cc );
#endif
mpeg2_reset( p_sys->p_mpeg2dec, 0 );
DpbClean( p_dec );
}
static picture_t *GetNewPicture( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
picture_t *p_pic;
p_dec->fmt_out.video.i_width = p_sys->p_info->sequence->width;
p_dec->fmt_out.video.i_visible_width =
p_sys->p_info->sequence->picture_width;
p_dec->fmt_out.video.i_height = p_sys->p_info->sequence->height;
p_dec->fmt_out.video.i_visible_height =
p_sys->p_info->sequence->picture_height;
p_dec->fmt_out.video.i_sar_num = p_sys->i_sar_num;
p_dec->fmt_out.video.i_sar_den = p_sys->i_sar_den;
if( p_sys->p_info->sequence->frame_period > 0 )
{
p_dec->fmt_out.video.i_frame_rate =
(uint32_t)( (uint64_t)1001000000 * 27 /
p_sys->p_info->sequence->frame_period );
p_dec->fmt_out.video.i_frame_rate_base = 1001;
}
p_dec->fmt_out.i_codec =
( p_sys->p_info->sequence->chroma_height <
p_sys->p_info->sequence->height ) ?
VLC_CODEC_I420 : VLC_CODEC_I422;
if( decoder_UpdateVideoFormat( p_dec ) )
return NULL;
p_pic = decoder_NewPicture( p_dec );
if( p_pic == NULL )
return NULL;
p_pic->b_progressive = p_sys->p_info->current_picture != NULL ?
p_sys->p_info->current_picture->flags & PIC_FLAG_PROGRESSIVE_FRAME : 1;
p_pic->b_top_field_first = p_sys->p_info->current_picture != NULL ?
p_sys->p_info->current_picture->flags & PIC_FLAG_TOP_FIELD_FIRST : 1;
p_pic->i_nb_fields = p_sys->p_info->current_picture != NULL ?
p_sys->p_info->current_picture->nb_fields : 2;
return p_pic;
}
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
static void SendCc( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
block_t *p_cc = NULL;
if( !p_sys->cc.b_reorder && p_sys->cc.i_data <= 0 )
return;
p_cc = block_Alloc( p_sys->cc.i_data);
if( p_cc )
{
memcpy( p_cc->p_buffer, p_sys->cc.p_data, p_sys->cc.i_data );
p_cc->i_dts =
p_cc->i_pts = p_sys->cc.b_reorder ? p_sys->i_cc_pts : p_sys->i_cc_dts;
p_cc->i_flags = p_sys->i_cc_flags & BLOCK_FLAG_TYPE_MASK;
decoder_cc_desc_t desc;
desc.i_608_channels = p_sys->cc.i_608channels;
desc.i_708_channels = p_sys->cc.i_708channels;
desc.i_reorder_depth = p_sys->cc.b_reorder ? 0 : -1;
decoder_QueueCc( p_dec, p_cc, &desc );
}
cc_Flush( &p_sys->cc );
return;
}
#endif
static void GetAR( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
int i_old_sar_num = p_sys->i_sar_num;
int i_old_sar_den = p_sys->i_sar_den;
if( p_dec->fmt_in.video.i_sar_num > 0 &&
p_dec->fmt_in.video.i_sar_den > 0 )
{
p_sys->i_sar_num = p_dec->fmt_in.video.i_sar_num;
p_sys->i_sar_den = p_dec->fmt_in.video.i_sar_den;
}
else if( p_sys->p_info->sequence->pixel_height > 0 )
{
p_sys->i_sar_num = p_sys->p_info->sequence->pixel_width;
p_sys->i_sar_den = p_sys->p_info->sequence->pixel_height;
}
else
{
p_sys->i_sar_num = p_sys->p_info->sequence->picture_height * 4;
p_sys->i_sar_den = p_sys->p_info->sequence->picture_width * 3;
}
if( p_sys->i_sar_num == i_old_sar_num &&
p_sys->i_sar_den == i_old_sar_den )
return;
if( p_sys->p_info->sequence->frame_period > 0 )
msg_Dbg( p_dec,
"%dx%d (display %d,%d), sar %i:%i, %u.%03u fps",
p_sys->p_info->sequence->picture_width,
p_sys->p_info->sequence->picture_height,
p_sys->p_info->sequence->display_width,
p_sys->p_info->sequence->display_height,
p_sys->i_sar_num, p_sys->i_sar_den,
(uint32_t)((uint64_t)1001000000 * 27 /
p_sys->p_info->sequence->frame_period / 1001),
(uint32_t)((uint64_t)1001000000 * 27 /
p_sys->p_info->sequence->frame_period % 1001) );
else
msg_Dbg( p_dec, "bad frame period" );
}
static void PutPicture( decoder_t *p_dec, picture_t *p_picture )
{
decoder_sys_t *p_sys = p_dec->p_sys;
uint8_t *pp_buf[3];
for( int j = 0; j < 3; j++ )
pp_buf[j] = p_picture ? p_picture->p[j].p_pixels : NULL;
mpeg2_set_buf( p_sys->p_mpeg2dec, pp_buf, p_picture );
if( p_picture )
mpeg2_stride( p_sys->p_mpeg2dec, p_picture->p[Y_PLANE].i_pitch );
}
static void DpbInit( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
for( int i = 0; i < DPB_COUNT; i++ )
p_sys->p_dpb[i].p_picture = NULL;
}
static void DpbClean( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
for( int i = 0; i < DPB_COUNT; i++ )
{
picture_dpb_t *p = &p_sys->p_dpb[i];
if( !p->p_picture )
continue;
if( p->b_linked )
picture_Release( p->p_picture );
if( !p->b_displayed )
picture_Release( p->p_picture );
p->p_picture = NULL;
}
}
static picture_t *DpbNewPicture( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
picture_dpb_t *p;
int i;
for( i = 0; i < DPB_COUNT; i++ )
{
p = &p_sys->p_dpb[i];
if( !p->p_picture )
break;
}
if( i >= DPB_COUNT )
{
msg_Err( p_dec, "Leaking picture" );
return NULL;
}
p->p_picture = GetNewPicture( p_dec );
if( p->p_picture )
{
picture_Hold( p->p_picture );
p->b_linked = true;
p->b_displayed = false;
p->p_picture->date = VLC_TICK_INVALID;
}
return p->p_picture;
}
static picture_dpb_t *DpbFindPicture( decoder_t *p_dec, picture_t *p_picture )
{
decoder_sys_t *p_sys = p_dec->p_sys;
for( int i = 0; i < DPB_COUNT; i++ )
{
picture_dpb_t *p = &p_sys->p_dpb[i];
if( p->p_picture == p_picture )
return p;
}
return NULL;
}
static void DpbUnlinkPicture( decoder_t *p_dec, picture_t *p_picture )
{
picture_dpb_t *p = DpbFindPicture( p_dec, p_picture );
if( !p || !p->b_linked )
{
msg_Err( p_dec, "DpbUnlinkPicture called on an invalid picture" );
return;
}
assert( p && p->b_linked );
picture_Release( p->p_picture );
p->b_linked = false;
if( !p->b_displayed )
picture_Release( p->p_picture );
p->p_picture = NULL;
}
static int DpbDisplayPicture( decoder_t *p_dec, picture_t *p_picture )
{
picture_dpb_t *p = DpbFindPicture( p_dec, p_picture );
if( !p || p->b_displayed || !p->b_linked )
{
msg_Err( p_dec, "DpbDisplayPicture called on an invalid picture" );
return VLC_EGENERIC;
}
assert( p && !p->b_displayed && p->b_linked );
p->b_displayed = true;
return VLC_SUCCESS;
}
