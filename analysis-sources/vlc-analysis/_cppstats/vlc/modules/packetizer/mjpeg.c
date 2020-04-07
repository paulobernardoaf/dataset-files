#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_block.h>
#include <vlc_codec.h>
#include <vlc_block_helper.h>
#include "packetizer_helper.h"
typedef struct
{
packetizer_t packetizer;
int i_next_block_flags;
date_t date;
} decoder_sys_t;
static const uint8_t p_mjpg_startcode[4] = { 0xFF, 0xD8, 0xFF, 0xE0 };
static inline const uint8_t * startcode_Find( const uint8_t *p, const uint8_t *end )
{
while( p + 3 < end )
{
p = memchr( p, 0xFF, end - p );
if( !p || end - p < 4 )
break;
if( p[1] == 0xD8 && p[2] == 0xFF && (p[3] & 0xE0) == 0xE0 )
return p;
p++;
}
return NULL;
}
static block_t *Packetize( decoder_t *p_dec, block_t **pp_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
return packetizer_Packetize( &p_sys->packetizer, pp_block );
}
static void PacketizeFlush( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
date_Set( &p_sys->date, VLC_TICK_INVALID );
p_sys->i_next_block_flags = BLOCK_FLAG_DISCONTINUITY;
packetizer_Flush( &p_sys->packetizer );
}
static void PacketizeReset( void *p_private, bool b_flush )
{
VLC_UNUSED(b_flush);
decoder_t *p_dec = p_private;
decoder_sys_t *p_sys = p_dec->p_sys;
date_Set( &p_sys->date, VLC_TICK_INVALID );
p_sys->i_next_block_flags = BLOCK_FLAG_DISCONTINUITY;
}
static block_t *PacketizeParse( void *p_private, bool *pb_ts_used, block_t *p_block )
{
decoder_t *p_dec = p_private;
decoder_sys_t *p_sys = p_dec->p_sys;
const uint8_t *p_buf = &p_block->p_buffer[2];
size_t i_buf = p_block->i_buffer - 2;
while( i_buf > 4 && p_buf[0] == 0xFF )
{
size_t i_size = 2 + GetWBE( &p_buf[2] );
if( i_size > i_buf )
break;
if( p_buf[1] == 0xC0 && i_buf > 9 )
{
uint16_t i_height = GetWBE( &p_buf[5] );
uint16_t i_width = GetWBE( &p_buf[7] );
if( i_height && i_width &&
(p_dec->fmt_out.video.i_height != i_height ||
p_dec->fmt_out.video.i_width != i_width) )
{
p_dec->fmt_out.video.i_width =
p_dec->fmt_out.video.i_visible_width = i_width;
p_dec->fmt_out.video.i_height =
p_dec->fmt_out.video.i_visible_height = i_height;
}
break;
}
i_buf -= i_size;
p_buf += i_size;
}
if( p_block->i_dts == VLC_TICK_INVALID )
p_block->i_dts = p_block->i_pts;
else if( p_block->i_pts == VLC_TICK_INVALID )
p_block->i_pts = p_block->i_dts;
vlc_tick_t i_prev_dts = date_Get( &p_sys->date );
if( p_block->i_dts != VLC_TICK_INVALID )
{
date_Set( &p_sys->date, p_block->i_dts );
}
else if( p_dec->fmt_in.video.i_frame_rate &&
p_dec->fmt_in.video.i_frame_rate_base )
{
date_Increment( &p_sys->date, 1 );
p_block->i_dts = p_block->i_pts = date_Get( &p_sys->date );
}
if( i_prev_dts != VLC_TICK_INVALID && p_block->i_dts != VLC_TICK_INVALID )
p_block->i_length = p_block->i_dts - i_prev_dts;
*pb_ts_used = true;
p_block->i_flags = p_sys->i_next_block_flags | BLOCK_FLAG_TYPE_I;
p_sys->i_next_block_flags = 0;
return p_block;
}
static int PacketizeValidate( void *p_private, block_t *p_au )
{
VLC_UNUSED(p_private);
VLC_UNUSED(p_au);
return VLC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
packetizer_Clean( &p_sys->packetizer );
free( p_sys );
}
static int Open( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;
if( p_dec->fmt_in.i_codec != VLC_CODEC_MJPG )
return VLC_EGENERIC;
p_dec->p_sys = p_sys = malloc( sizeof( decoder_sys_t ) );
if( !p_dec->p_sys )
return VLC_ENOMEM;
p_sys->i_next_block_flags = 0;
if( p_dec->fmt_in.video.i_frame_rate &&
p_dec->fmt_in.video.i_frame_rate_base )
{
date_Init( &p_sys->date, p_dec->fmt_in.video.i_frame_rate,
p_dec->fmt_in.video.i_frame_rate_base );
}
else
date_Init( &p_sys->date, 30000, 1001 );
es_format_Copy( &p_dec->fmt_out, &p_dec->fmt_in );
packetizer_Init( &p_sys->packetizer,
p_mjpg_startcode, sizeof(p_mjpg_startcode), startcode_Find,
NULL, 0, 295,
PacketizeReset, PacketizeParse, PacketizeValidate, NULL,
p_dec );
p_dec->pf_packetize = Packetize;
p_dec->pf_flush = PacketizeFlush;
p_dec->pf_get_cc = NULL;
return VLC_SUCCESS;
}
vlc_module_begin ()
set_category( CAT_SOUT )
set_subcategory( SUBCAT_SOUT_PACKETIZER )
set_description( N_("MJPEG video packetizer") )
set_capability( "packetizer", 50 )
set_callbacks( Open, Close )
vlc_module_end ()
