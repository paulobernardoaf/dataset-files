#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_bits.h>
#include "../demux/mpeg/timestamps.h"
static int DecoderOpen ( vlc_object_t * );
static int PacketizerOpen( vlc_object_t * );
static void DecoderClose ( vlc_object_t * );
vlc_module_begin ()
set_description( N_("Philips OGT (SVCD subtitle) decoder") )
set_shortname( N_("SVCD subtitles") )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_SCODEC )
set_capability( "spu decoder", 50 )
set_callbacks( DecoderOpen, DecoderClose )
add_obsolete_integer ( "svcdsub-debug" )
add_submodule ()
set_description( N_("Philips OGT (SVCD subtitle) packetizer") )
set_capability( "packetizer", 50 )
set_callbacks( PacketizerOpen, DecoderClose )
vlc_module_end ()
static int Decode( decoder_t *, block_t * );
static block_t *Packetize ( decoder_t *, block_t ** );
static block_t *Reassemble ( decoder_t *, block_t * );
static void ParseHeader( decoder_t *, block_t * );
static subpicture_t *DecodePacket( decoder_t *, block_t * );
static void SVCDSubRenderImage( decoder_t *, block_t *, subpicture_region_t * );
#define GETINT16(p) GetWBE(p) ; p +=2;
typedef enum {
SUBTITLE_BLOCK_EMPTY = 0,
SUBTITLE_BLOCK_PARTIAL = 1,
SUBTITLE_BLOCK_COMPLETE = 2
} packet_state_t;
typedef struct
{
packet_state_t i_state; 
block_t *p_spu; 
uint16_t i_image; 
uint8_t i_packet; 
size_t i_spu_size; 
uint16_t i_image_offset; 
size_t i_image_length; 
size_t second_field_offset; 
size_t metadata_offset; 
size_t metadata_length; 
vlc_tick_t i_duration; 
uint16_t i_x_start, i_y_start; 
uint16_t i_width, i_height; 
uint8_t p_palette[4][4]; 
} decoder_sys_t;
static int OpenCommon( vlc_object_t *p_this, bool b_packetizer )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;
if( p_dec->fmt_in.i_codec != VLC_CODEC_OGT )
return VLC_EGENERIC;
p_dec->p_sys = p_sys = calloc( 1, sizeof( decoder_sys_t ) );
if( p_sys == NULL )
return VLC_ENOMEM;
p_sys->i_image = -1;
p_sys->i_state = SUBTITLE_BLOCK_EMPTY;
p_sys->p_spu = NULL;
p_dec->fmt_out.i_codec = VLC_CODEC_OGT;
if( b_packetizer )
p_dec->pf_packetize = Packetize;
else
p_dec->pf_decode = Decode;
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
#if !defined(NDEBUG)
msg_Dbg( p_dec, "Decode" );
#endif
if( p_block == NULL ) 
return VLCDEC_SUCCESS;
if( !(p_block = Reassemble( p_dec, p_block )) )
return VLCDEC_SUCCESS;
subpicture_t *p_spu = DecodePacket( p_dec, p_block );
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
#define SPU_HEADER_LEN 5
static block_t *Reassemble( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
uint8_t *p_buffer;
uint16_t i_expected_image;
uint8_t i_packet, i_expected_packet;
if( p_block->i_flags & (BLOCK_FLAG_CORRUPTED) )
{
block_Release( p_block );
return NULL;
}
if( p_block->i_buffer < SPU_HEADER_LEN )
{
msg_Dbg( p_dec, "invalid packet header (size %zu < %u)" ,
p_block->i_buffer, SPU_HEADER_LEN );
block_Release( p_block );
return NULL;
}
p_buffer = p_block->p_buffer;
if( p_sys->i_state == SUBTITLE_BLOCK_EMPTY )
{
i_expected_image = p_sys->i_image + 1;
i_expected_packet = 0;
}
else
{
i_expected_image = p_sys->i_image;
i_expected_packet = p_sys->i_packet + 1;
}
p_buffer += 2;
if( *p_buffer & 0x80 )
{
p_sys->i_state = SUBTITLE_BLOCK_COMPLETE;
i_packet = *p_buffer++ & 0x7F;
}
else
{
p_sys->i_state = SUBTITLE_BLOCK_PARTIAL;
i_packet = *p_buffer++;
}
p_sys->i_image = GETINT16(p_buffer);
if( p_sys->i_image != i_expected_image )
{
msg_Warn( p_dec, "expected subtitle image %u but found %u",
i_expected_image, p_sys->i_image );
}
if( i_packet != i_expected_packet )
{
msg_Warn( p_dec, "expected subtitle image packet %u but found %u",
i_expected_packet, i_packet );
}
p_block->p_buffer += SPU_HEADER_LEN;
p_block->i_buffer -= SPU_HEADER_LEN;
p_sys->i_packet = i_packet;
if( !p_sys->i_packet ) ParseHeader( p_dec, p_block );
block_ChainAppend( &p_sys->p_spu, p_block );
if( p_sys->i_state == SUBTITLE_BLOCK_COMPLETE )
{
block_t *p_spu = block_ChainGather( p_sys->p_spu );
if( unlikely( !p_spu ) )
{
block_ChainRelease( p_sys->p_spu );
p_sys->i_state = SUBTITLE_BLOCK_EMPTY;
p_sys->p_spu = NULL;
msg_Warn( p_dec, "unable to assemble blocks, discarding" );
return NULL;
}
if( p_spu->i_buffer != p_sys->i_spu_size )
{
msg_Warn( p_dec, "subtitle packets size=%zu should be %zu",
p_spu->i_buffer, p_sys->i_spu_size );
}
msg_Dbg( p_dec, "subtitle packet complete, size=%zu", p_spu->i_buffer );
p_sys->i_state = SUBTITLE_BLOCK_EMPTY;
p_sys->p_spu = NULL;
return p_spu;
}
return NULL;
}
static void ParseHeader( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
uint8_t *p = p_block->p_buffer;
size_t i_buffer = p_block->i_buffer;
uint8_t i_options, i_cmd;
int i;
if (i_buffer < 4) return;
p_sys->i_spu_size = GETINT16(p);
i_options = *p++;
p++;
i_buffer -= 4;
if( i_options & 0x08 ) {
if (i_buffer < 4) return;
p_sys->i_duration = FROM_SCALE_NZ(GetDWBE(p));
p += 4;
i_buffer -= 4;
}
else p_sys->i_duration = 0; 
if (i_buffer < 25) return;
p_sys->i_x_start = GETINT16(p);
p_sys->i_y_start = GETINT16(p);
p_sys->i_width = GETINT16(p);
p_sys->i_height = GETINT16(p);
for( i = 0; i < 4; i++ )
{
p_sys->p_palette[i][0] = *p++; 
p_sys->p_palette[i][2] = *p++; 
p_sys->p_palette[i][1] = *p++; 
p_sys->p_palette[i][3] = *p++; 
}
i_cmd = *p++;
i_buffer -= 25;
if( i_cmd ) {
if (i_buffer < 4) return;
p += 4;
i_buffer -= 4;
}
if (i_buffer < 2) return;
p_sys->second_field_offset = GETINT16(p);
i_buffer -= 2;
p_sys->i_image_offset = p - p_block->p_buffer;
p_sys->i_image_length = p_sys->i_spu_size - p_sys->i_image_offset;
p_sys->metadata_length = p_sys->i_image_offset;
#if !defined(NDEBUG)
msg_Dbg( p_dec, "x-start: %d, y-start: %d, width: %d, height %d, "
"spu size: %zu, duration: %"PRIu64" (d:%zu p:%"PRIu16")",
p_sys->i_x_start, p_sys->i_y_start,
p_sys->i_width, p_sys->i_height,
p_sys->i_spu_size, p_sys->i_duration,
p_sys->i_image_length, p_sys->i_image_offset);
for( i = 0; i < 4; i++ )
{
msg_Dbg( p_dec, "palette[%d]= T: %2x, Y: %2x, u: %2x, v: %2x", i,
p_sys->p_palette[i][3], p_sys->p_palette[i][0],
p_sys->p_palette[i][1], p_sys->p_palette[i][2] );
}
#endif
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
fmt.i_sar_num = p_sys->i_height;
fmt.i_sar_den = p_sys->i_width;
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
fmt.p_palette = NULL;
video_format_Clean( &fmt );
if( !p_region )
{
msg_Err( p_dec, "cannot allocate SVCD subtitle region" );
subpicture_Delete( p_spu );
return NULL;
}
p_spu->p_region = p_region;
p_region->i_x = p_sys->i_x_start;
p_region->i_y = p_sys->i_y_start;
SVCDSubRenderImage( p_dec, p_data, p_region );
return p_spu;
}
static void SVCDSubRenderImage( decoder_t *p_dec, block_t *p_data,
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
i_color = bs_read( &bs, 2 );
if( i_color == 0 && (i_count = bs_read( &bs, 2 )) )
{
i_count = __MIN( i_count, p_sys->i_width - i_column );
memset( &p_dest[i_row * p_region->p_picture->Y_PITCH +
i_column], 0, i_count + 1 );
i_column += i_count;
continue;
}
p_dest[i_row * p_region->p_picture->Y_PITCH + i_column] = i_color;
}
bs_align( &bs );
}
bs_init( &bs, p_data->p_buffer + p_sys->i_image_offset +
p_sys->second_field_offset,
p_data->i_buffer - p_sys->i_image_offset -
p_sys->second_field_offset );
}
}
