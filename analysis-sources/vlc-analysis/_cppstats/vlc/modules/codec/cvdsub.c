#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_bits.h>
#include "../demux/mpeg/timestamps.h"
#define DEBUG_CVDSUB 1
static int DecoderOpen ( vlc_object_t * );
static int PacketizerOpen( vlc_object_t * );
static void DecoderClose ( vlc_object_t * );
vlc_module_begin ()
set_description( N_("CVD subtitle decoder") )
set_capability( "spu decoder", 50 )
set_callbacks( DecoderOpen, DecoderClose )
add_submodule ()
set_description( N_("Chaoji VCD subtitle packetizer") )
set_capability( "packetizer", 50 )
set_callbacks( PacketizerOpen, DecoderClose )
vlc_module_end ()
static int Decode( decoder_t *, block_t * );
static block_t *Packetize ( decoder_t *, block_t ** );
static block_t *Reassemble ( decoder_t *, block_t * );
static void ParseMetaInfo ( decoder_t *, block_t * );
static void ParseHeader ( decoder_t *, block_t * );
static subpicture_t *DecodePacket( decoder_t *, block_t * );
static void RenderImage( decoder_t *, block_t *, subpicture_region_t * );
#define SUBTITLE_BLOCK_EMPTY 0
#define SUBTITLE_BLOCK_PARTIAL 1
#define SUBTITLE_BLOCK_COMPLETE 2
typedef struct
{
int b_packetizer;
int i_state; 
block_t *p_spu; 
size_t i_spu_size; 
uint16_t i_image_offset; 
size_t i_image_length; 
size_t first_field_offset; 
size_t second_field_offset; 
size_t metadata_offset; 
size_t metadata_length; 
vlc_tick_t i_duration; 
uint16_t i_x_start, i_y_start; 
uint16_t i_width, i_height; 
uint8_t p_palette[4][4]; 
uint8_t p_palette_highlight[4][4];
} decoder_sys_t;
static int OpenCommon( vlc_object_t *p_this, bool b_packetizer )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;
if( p_dec->fmt_in.i_codec != VLC_CODEC_CVD )
return VLC_EGENERIC;
p_dec->p_sys = p_sys = malloc( sizeof( decoder_sys_t ) );
if( !p_sys )
return VLC_ENOMEM;
p_sys->b_packetizer = b_packetizer;
p_sys->i_state = SUBTITLE_BLOCK_EMPTY;
p_sys->p_spu = NULL;
if( b_packetizer )
{
p_dec->pf_packetize = Packetize;
p_dec->fmt_out.i_codec = VLC_CODEC_CVD;
}
else
{
p_dec->pf_decode = Decode;
p_dec->fmt_out.i_codec = VLC_CODEC_YUVP;
}
return VLC_SUCCESS;
}
static int DecoderOpen( vlc_object_t *p_this )
{
return OpenCommon( p_this, false );
}
static int PacketizerOpen( vlc_object_t *p_this )
{
return OpenCommon( p_this, true );
}
void DecoderClose( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
if( p_sys->p_spu ) block_ChainRelease( p_sys->p_spu );
free( p_sys );
}
static int Decode( decoder_t *p_dec, block_t *p_block )
{
block_t *p_data;
if( p_block == NULL ) 
return VLCDEC_SUCCESS;
if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
block_Release( p_block );
return VLCDEC_SUCCESS;
}
if( !(p_data = Reassemble( p_dec, p_block )) )
return VLCDEC_SUCCESS;
subpicture_t *p_spu = DecodePacket( p_dec, p_data );
block_Release( p_data );
if( p_spu != NULL )
decoder_QueueSub( p_dec, p_spu );
return VLCDEC_SUCCESS;
}
static block_t *Packetize( decoder_t *p_dec, block_t **pp_block )
{
block_t *p_block, *p_spu;
if( pp_block == NULL || *pp_block == NULL ) return NULL;
p_block = *pp_block;
*pp_block = NULL;
if( !(p_spu = Reassemble( p_dec, p_block )) ) return NULL;
p_spu->i_dts = p_spu->i_pts;
p_spu->i_length = VLC_TICK_INVALID;
return p_spu;
}
#define SPU_HEADER_LEN 1
static block_t *Reassemble( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
if( p_block->i_buffer < SPU_HEADER_LEN )
{
msg_Dbg( p_dec, "invalid packet header (size %zu < %u)" ,
p_block->i_buffer, SPU_HEADER_LEN );
block_Release( p_block );
return NULL;
}
if( p_sys->i_state == SUBTITLE_BLOCK_EMPTY && p_block->i_pts == VLC_TICK_INVALID )
{
msg_Warn( p_dec, "first packet expected but no PTS present");
return NULL;
}
p_block->p_buffer += SPU_HEADER_LEN;
p_block->i_buffer -= SPU_HEADER_LEN;
if( p_sys->i_state == SUBTITLE_BLOCK_EMPTY ) ParseHeader( p_dec, p_block );
block_ChainAppend( &p_sys->p_spu, p_block );
p_sys->p_spu = block_ChainGather( p_sys->p_spu );
if( p_sys->p_spu->i_buffer >= p_sys->i_spu_size )
{
block_t *p_spu = p_sys->p_spu;
if( p_spu->i_buffer != p_sys->i_spu_size )
{
msg_Warn( p_dec, "SPU packets size=%zu should be %zu",
p_spu->i_buffer, p_sys->i_spu_size );
}
msg_Dbg( p_dec, "subtitle packet complete, size=%zuu", p_spu->i_buffer);
ParseMetaInfo( p_dec, p_spu );
p_sys->i_state = SUBTITLE_BLOCK_EMPTY;
p_sys->p_spu = 0;
return p_spu;
}
else
{
p_sys->i_state = SUBTITLE_BLOCK_PARTIAL;
}
return NULL;
}
static void ParseHeader( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
uint8_t *p = p_block->p_buffer;
p_sys->i_spu_size = (p[0] << 8) + p[1] + 4; p += 2;
p_sys->metadata_offset = (p[0] << 8) + p[1]; p +=2;
p_sys->metadata_length = p_sys->i_spu_size - p_sys->metadata_offset;
p_sys->i_image_offset = 4;
p_sys->i_image_length = p_sys->metadata_offset - p_sys->i_image_offset;
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "total size: %zu image size: %zu",
p_sys->i_spu_size, p_sys->i_image_length );
#endif
}
#define ExtractXY(x, y) x = ((p[1]&0x0f)<<6) + (p[2]>>2); y = ((p[2]&0x03)<<8) + p[3];
static void ParseMetaInfo( decoder_t *p_dec, block_t *p_spu )
{
decoder_sys_t *p_sys = p_dec->p_sys;
uint8_t *p = p_spu->p_buffer + p_sys->metadata_offset;
uint8_t *p_end = p + p_sys->metadata_length;
for( ; p < p_end; p += 4 )
{
switch( p[0] )
{
case 0x04: 
p_sys->i_duration = FROM_SCALE_NZ( (p[1]<<16) + (p[2]<<8) + p[3] );
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "subtitle display duration %"PRIu64" ms",
MS_FROM_VLC_TICK(p_sys->i_duration) );
#endif
break;
case 0x0c: 
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "subtitle command unknown "
"0x%02"PRIx8" 0x%02"PRIx8" 0x%02"PRIx8" 0x%02"PRIx8,
p[0], p[1], p[2], p[3] );
#endif
break;
case 0x17: 
ExtractXY(p_sys->i_x_start, p_sys->i_y_start);
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "start position (%"PRIu16",%"PRIu16")",
p_sys->i_x_start, p_sys->i_y_start );
#endif
break;
case 0x1f: 
{
int lastx;
int lasty;
ExtractXY(lastx, lasty);
p_sys->i_width = lastx - p_sys->i_x_start + 1;
p_sys->i_height = lasty - p_sys->i_y_start + 1;
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "end position (%d,%d), w x h: %"PRIu16"x%"PRIu16,
lastx, lasty, p_sys->i_width, p_sys->i_height );
#endif
break;
}
case 0x24:
case 0x25:
case 0x26:
case 0x27:
{
uint8_t v = p[0] - 0x24;
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "primary palette %"PRIu8" (y,u,v): "
"(0x%02"PRIx8",0x%02"PRIx8",0x%02"PRIx8")",
v, p[1], p[2], p[3] );
#endif
p_sys->p_palette[v][0] = p[1]; 
p_sys->p_palette[v][1] = p[3]; 
p_sys->p_palette[v][2] = p[2]; 
break;
}
case 0x2c:
case 0x2d:
case 0x2e:
case 0x2f:
{
uint8_t v = p[0] - 0x2c;
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec,"highlight palette %"PRIu8" (y,u,v): "
"(0x%02"PRIx8",0x%02"PRIx8",0x%02"PRIx8")",
v, p[1], p[2], p[3] );
#endif
p_sys->p_palette_highlight[v][0] = p[1]; 
p_sys->p_palette_highlight[v][1] = p[3]; 
p_sys->p_palette_highlight[v][2] = p[2]; 
break;
}
case 0x37:
p_sys->p_palette[0][3] = (p[3] & 0x0f) << 4;
p_sys->p_palette[1][3] = (p[3] >> 4) << 4;
p_sys->p_palette[2][3] = (p[2] & 0x0f) << 4;
p_sys->p_palette[3][3] = (p[2] >> 4) << 4;
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "transparency for primary palette 0..3: "
"0x%02"PRIx8" 0x%02"PRIx8" 0x%02"PRIx8" 0x%02"PRIx8,
p_sys->p_palette[0][3], p_sys->p_palette[1][3],
p_sys->p_palette[2][3], p_sys->p_palette[3][3]);
#endif
break;
case 0x3f:
p_sys->p_palette_highlight[0][3] = (p[2] & 0x0f) << 4;
p_sys->p_palette_highlight[1][3] = (p[2] >> 4) << 4;
p_sys->p_palette_highlight[2][3] = (p[1] & 0x0f) << 4;
p_sys->p_palette_highlight[3][3] = (p[1] >> 4) << 4;
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "transparency for highlight palette 0..3: "
"0x%02"PRIx8" 0x%02"PRIx8" 0x%02"PRIx8" 0x%02"PRIx8,
p_sys->p_palette_highlight[0][3],
p_sys->p_palette_highlight[1][3],
p_sys->p_palette_highlight[2][3],
p_sys->p_palette_highlight[3][3] );
#endif
break;
case 0x47:
p_sys->first_field_offset =
(p[2] << 8) + p[3] - p_sys->i_image_offset;
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "1st_field_offset %zu",
p_sys->first_field_offset );
#endif
break;
case 0x4f:
p_sys->second_field_offset =
(p[2] << 8) + p[3] - p_sys->i_image_offset;
#if defined(DEBUG_CVDSUB)
msg_Dbg( p_dec, "2nd_field_offset %zu",
p_sys->second_field_offset);
#endif
break;
default:
#if defined(DEBUG_CVDSUB)
msg_Warn( p_dec, "unknown sequence in control header "
"0x%02"PRIx8" 0x%02"PRIx8" 0x%02"PRIx8" 0x%02"PRIx8,
p[0], p[1], p[2], p[3]);
#endif
}
}
}
static subpicture_t *DecodePacket( decoder_t *p_dec, block_t *p_data )
{
decoder_sys_t *p_sys = p_dec->p_sys;
subpicture_t *p_spu;
subpicture_region_t *p_region;
video_format_t fmt;
video_palette_t palette;
int i;
p_spu = decoder_NewSubpicture( p_dec, NULL );
if( !p_spu ) return NULL;
p_spu->i_start = p_data->i_pts;
p_spu->i_stop = p_data->i_pts + p_sys->i_duration;
p_spu->b_ephemer = true;
video_format_Init( &fmt, VLC_CODEC_YUVP );
fmt.i_sar_num = 1;
fmt.i_sar_den = 1;
fmt.i_width = fmt.i_visible_width = p_sys->i_width;
fmt.i_height = fmt.i_visible_height = p_sys->i_height;
fmt.i_x_offset = fmt.i_y_offset = 0;
fmt.p_palette = &palette;
fmt.p_palette->i_entries = 4;
for( i = 0; i < fmt.p_palette->i_entries; i++ )
{
fmt.p_palette->palette[i][0] = p_sys->p_palette[i][0];
fmt.p_palette->palette[i][1] = p_sys->p_palette[i][1];
fmt.p_palette->palette[i][2] = p_sys->p_palette[i][2];
fmt.p_palette->palette[i][3] = p_sys->p_palette[i][3];
}
p_region = subpicture_region_New( &fmt );
if( !p_region )
{
msg_Err( p_dec, "cannot allocate SPU region" );
subpicture_Delete( p_spu );
return NULL;
}
p_spu->p_region = p_region;
p_region->i_x = p_sys->i_x_start;
p_region->i_x = p_region->i_x * 3 / 4; 
p_region->i_y = p_sys->i_y_start;
RenderImage( p_dec, p_data, p_region );
return p_spu;
}
static void RenderImage( decoder_t *p_dec, block_t *p_data,
subpicture_region_t *p_region )
{
decoder_sys_t *p_sys = p_dec->p_sys;
uint8_t *p_dest = p_region->p_picture->Y_PIXELS;
int i_field; 
int i_row, i_column; 
uint8_t i_color, i_count;
bs_t bs;
bs_init( &bs, p_data->p_buffer + p_sys->i_image_offset,
p_data->i_buffer - p_sys->i_image_offset );
for( i_field = 0; i_field < 2; i_field++ )
{
for( i_row = i_field; i_row < p_sys->i_height; i_row += 2 )
{
for( i_column = 0; i_column < p_sys->i_width; i_column++ )
{
uint8_t i_val = bs_read( &bs, 4 );
if( i_val == 0 )
{
i_color = bs_read( &bs, 4 );
memset( &p_dest[i_row * p_region->p_picture->Y_PITCH +
i_column], i_color,
p_sys->i_width - i_column );
i_column = p_sys->i_width;
continue;
}
else
{
i_count = (i_val >> 2);
i_color = i_val & 0x3;
i_count = __MIN( i_count, p_sys->i_width - i_column );
memset( &p_dest[i_row * p_region->p_picture->Y_PITCH +
i_column], i_color, i_count );
i_column += i_count - 1;
continue;
}
}
bs_align( &bs );
}
}
}
