#include <assert.h>
#include <vlc_demux.h>
#include "timestamps.h"
#define PS_STREAM_ID_END_STREAM 0xB9
#define PS_STREAM_ID_PACK_HEADER 0xBA
#define PS_STREAM_ID_SYSTEM_HEADER 0xBB
#define PS_STREAM_ID_MAP 0xBC
#define PS_STREAM_ID_PRIVATE_STREAM1 0xBD
#define PS_STREAM_ID_PADDING 0xBE
#define PS_STREAM_ID_EXTENDED 0xFD
#define PS_STREAM_ID_DIRECTORY 0xFF
#define PS_TK_COUNT (256+256+256+8 - 0xc0)
static inline unsigned ps_id_to_tk( unsigned i_id )
{
assert(i_id >= 0xc0);
if(unlikely(i_id < 0xc0))
return 0;
else if( i_id <= 0xff )
return i_id - 0xc0;
else if( (i_id & 0xff00) == 0xbd00 )
return 256-0xC0 + (i_id & 0xff);
else if( (i_id & 0xff00) == 0xfd00 )
return 512-0xc0 + (i_id & 0xff);
else
return 768-0xc0 + (i_id & 0x07);
}
typedef struct ps_psm_t ps_psm_t;
static inline uint8_t ps_id_to_type( const ps_psm_t *, uint16_t );
static inline const uint8_t *ps_id_to_lang( const ps_psm_t *, uint16_t );
typedef struct
{
bool b_configured;
bool b_seen;
int i_skip;
int i_id;
int i_next_block_flags;
es_out_id_t *es;
es_format_t fmt;
vlc_tick_t i_first_pts;
vlc_tick_t i_last_pts;
} ps_track_t;
static inline void ps_track_init( ps_track_t tk[PS_TK_COUNT] )
{
int i;
for( i = 0; i < PS_TK_COUNT; i++ )
{
tk[i].b_configured = false;
tk[i].b_seen = false;
tk[i].i_skip = 0;
tk[i].i_id = 0;
tk[i].i_next_block_flags = 0;
tk[i].es = NULL;
tk[i].i_first_pts = VLC_TICK_INVALID;
tk[i].i_last_pts = VLC_TICK_INVALID;
es_format_Init( &tk[i].fmt, UNKNOWN_ES, 0 );
}
}
static inline bool ps_is_H264( const uint8_t *p_data, size_t i_data )
{
const uint8_t startcode[3] = { 0, 0, 1 };
int i_flags = 0;
if( i_data < 9 ||
(!memcmp( p_data, startcode, 3 ) &&
!memcmp( &p_data[1], startcode, 3 )) )
return false;
while( i_data > 5 )
{
if( !memcmp( p_data, startcode, 3 ) )
{
if(p_data[3] == 0x67)
i_flags ^= 0x01;
else if(p_data[3] == 0x68)
i_flags ^= 0x02;
else if( p_data[3] & 0x80 )
return false;
else if( (p_data[3] & 0x1F) > 23 || (p_data[3] & 0x1F) < 1 )
return false;
else if( (p_data[3] & 0x1F) < 6 )
return (i_flags == 0x03);
}
p_data++;
i_data--;
}
return false;
}
static inline int ps_track_fill( ps_track_t *tk, ps_psm_t *p_psm,
int i_id,
const uint8_t *p_pkt, size_t i_pkt,
bool b_mpeg2only )
{
tk->i_skip = 0;
tk->i_id = i_id;
if( ( i_id&0xff00 ) == 0xbd00 ) 
{
if( ( i_id&0xf8 ) == 0x88 || 
( i_id&0xf8 ) == 0x98 ) 
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_DTS );
tk->i_skip = 4;
}
else if( ( i_id&0xf8 ) == 0x80 || 
( i_id&0xf0 ) == 0xc0 ) 
{
bool b_eac3 = false;
if( ( i_id&0xf0 ) == 0xc0 )
{
if( p_pkt == NULL || i_pkt < 9 )
return VLC_EGENERIC;
unsigned i_start = 9 + p_pkt[8];
if( i_start + 9 < i_pkt )
{
if( p_pkt[i_start + 4] == 0x0b ||
p_pkt[i_start + 5] == 0x77 )
{
int bsid = p_pkt[i_start + 9] >> 3;
if( bsid > 10 )
b_eac3 = true;
}
}
}
es_format_Change( &tk->fmt, AUDIO_ES, b_eac3 ? VLC_CODEC_EAC3 : VLC_CODEC_A52 );
tk->i_skip = 4;
}
else if( ( i_id&0xfc ) == 0x00 ) 
{
es_format_Change( &tk->fmt, SPU_ES, VLC_CODEC_CVD );
}
else if( ( i_id&0xff ) == 0x10 ) 
{
es_format_Change( &tk->fmt, SPU_ES, VLC_CODEC_TELETEXT );
}
else if( ( i_id&0xe0 ) == 0x20 ) 
{
es_format_Change( &tk->fmt, SPU_ES, VLC_CODEC_SPU );
tk->i_skip = 1;
}
else if( ( i_id&0xff ) == 0x70 ) 
{
es_format_Change( &tk->fmt, SPU_ES, VLC_CODEC_OGT );
}
else if( ( i_id&0xf0 ) == 0xa0 ) 
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_DVD_LPCM );
tk->i_skip = 1;
}
else if( ( i_id&0xf0 ) == 0xb0 ) 
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_TRUEHD );
tk->i_skip = 5;
}
else
{
es_format_Change( &tk->fmt, UNKNOWN_ES, 0 );
return VLC_EGENERIC;
}
}
else if( (i_id&0xff00) == 0xfd00 ) 
{
uint8_t i_sub_id = i_id & 0xff;
if( ( i_sub_id >= 0x55 && i_sub_id <= 0x5f ) || 
( i_sub_id >= 0x75 && i_sub_id <= 0x7f ) ) 
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_VC1 );
}
else
{
es_format_Change( &tk->fmt, UNKNOWN_ES, 0 );
return VLC_EGENERIC;
}
}
else if( (i_id&0xff00) == 0xa000 ) 
{
uint8_t i_sub_id = i_id & 0x07;
if( i_sub_id == 0 )
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_DVDA_LPCM );
tk->i_skip = 1;
}
else if( i_sub_id == 1 )
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_MLP );
tk->i_skip = -1; 
}
else
{
es_format_Change( &tk->fmt, UNKNOWN_ES, 0 );
return VLC_EGENERIC;
}
}
else
{
int i_type = ps_id_to_type( p_psm , i_id );
es_format_Change( &tk->fmt, UNKNOWN_ES, 0 );
if( (i_id&0xf0) == 0xe0 ) 
{
if( i_type == 0x01 )
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_MPGV );
tk->fmt.i_original_fourcc = VLC_CODEC_MP1V;
}
else if( i_type == 0x02 )
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_MPGV );
}
else if( i_type == 0x10 )
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_MP4V );
}
else if( i_type == 0x1b )
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_H264 );
}
else if( i_type == 0x24 )
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_HEVC );
}
else if( i_id == 0xe2 || 
i_id == 0xe3 ) 
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_H264 );
}
else if( p_pkt && i_type == 0x00 && 
i_pkt > 9 + 5 &&
i_pkt > 9U + 5 + p_pkt[8] &&
ps_is_H264( &p_pkt[ 9 + p_pkt[8] ],
i_pkt - 9 - p_pkt[8] ) )
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_H264 );
}
else if( tk->fmt.i_cat == UNKNOWN_ES &&
( p_pkt != NULL || b_mpeg2only ) )
{
es_format_Change( &tk->fmt, VIDEO_ES, VLC_CODEC_MPGV );
}
}
else if( ( i_id&0xe0 ) == 0xc0 ) 
{
if( i_type == 0x03 ||
i_type == 0x04 )
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_MPGA );
}
else if( i_type == 0x0f )
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_MP4A );
tk->fmt.i_original_fourcc = VLC_FOURCC('A','D','T','S');
}
else if( i_type == 0x11 )
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_MP4A );
tk->fmt.i_original_fourcc = VLC_FOURCC('L','A','T','M');
}
else if( tk->fmt.i_cat == UNKNOWN_ES )
{
es_format_Change( &tk->fmt, AUDIO_ES, VLC_CODEC_MPGA );
}
}
else if( tk->fmt.i_cat == UNKNOWN_ES ) return VLC_EGENERIC;
}
tk->fmt.b_packetized = false;
tk->fmt.i_priority = ~i_id & 0x0F;
if( ps_id_to_lang( p_psm, i_id ) )
{
tk->fmt.psz_language = malloc( 4 );
if( tk->fmt.psz_language )
{
memcpy( tk->fmt.psz_language, ps_id_to_lang( p_psm , i_id ), 3 );
tk->fmt.psz_language[3] = 0;
}
}
return (tk->fmt.i_cat != UNKNOWN_ES || p_pkt) ? VLC_SUCCESS : VLC_EGENERIC;
}
static inline int ps_pkt_id( const uint8_t *p_pkt, size_t i_pkt )
{
if(unlikely(i_pkt < 4))
return 0;
if( p_pkt[3] == 0xbd )
{
uint8_t i_sub_id = 0;
if( i_pkt >= 9 &&
i_pkt > 9 + (size_t)p_pkt[8] )
{
const unsigned i_start = 9 + p_pkt[8];
i_sub_id = p_pkt[i_start];
if( (i_sub_id & 0xfe) == 0xa0 &&
i_pkt >= i_start + 7 &&
( p_pkt[i_start + 5] >= 0xc0 ||
p_pkt[i_start + 6] != 0x80 ) )
{
return 0xa000 | (i_sub_id & 0x01);
}
}
return 0xbd00 | i_sub_id;
}
else if( i_pkt >= 9 &&
p_pkt[3] == 0xfd &&
(p_pkt[6]&0xC0) == 0x80 && 
(p_pkt[7]&0x01) == 0x01 ) 
{
const uint8_t i_flags = p_pkt[7];
unsigned int i_skip = 9;
if( (i_flags & 0x80 ) )
{
i_skip += 5; 
if( (i_flags & 0x40) )
i_skip += 5; 
}
if( (i_flags & 0x20 ) )
i_skip += 6;
if( (i_flags & 0x10 ) )
i_skip += 3;
if( (i_flags & 0x08 ) )
i_skip += 1;
if( (i_flags & 0x04 ) )
i_skip += 1;
if( (i_flags & 0x02 ) )
i_skip += 2;
if( i_skip < i_pkt && (p_pkt[i_skip]&0x01) )
{
const uint8_t i_flags2 = p_pkt[i_skip];
i_skip += 1;
if( i_flags2 & 0x80 )
i_skip += 16;
if( (i_flags2 & 0x40) && i_skip < i_pkt )
i_skip += 1 + p_pkt[i_skip];
if( i_flags2 & 0x20 )
i_skip += 2;
if( i_flags2 & 0x10 )
i_skip += 2;
if( i_skip + 1 < i_pkt )
{
const int i_extension_field_length = p_pkt[i_skip]&0x7f;
if( i_extension_field_length >=1 )
{
int i_stream_id_extension_flag = (p_pkt[i_skip+1] >> 7)&0x1;
if( i_stream_id_extension_flag == 0 )
return 0xfd00 | (p_pkt[i_skip+1]&0x7f);
}
}
}
}
return p_pkt[3];
}
static inline int ps_pkt_size( const uint8_t *p, int i_peek )
{
if( unlikely(i_peek < 4) )
return -1;
switch( p[3] )
{
case PS_STREAM_ID_END_STREAM:
return 4;
case PS_STREAM_ID_PACK_HEADER:
if( i_peek > 4 )
{
if( i_peek >= 14 && (p[4] >> 6) == 0x01 )
return 14 + (p[13]&0x07);
else if( i_peek >= 12 && (p[4] >> 4) == 0x02 )
return 12;
}
break;
case PS_STREAM_ID_SYSTEM_HEADER:
case PS_STREAM_ID_MAP:
case PS_STREAM_ID_DIRECTORY:
default:
if( i_peek >= 6 )
return 6 + ((p[4]<<8) | p[5] );
}
return -1;
}
static inline int ps_pkt_parse_pack( const uint8_t *p_pkt, size_t i_pkt,
vlc_tick_t *pi_scr, int *pi_mux_rate )
{
const uint8_t *p = p_pkt;
if( i_pkt >= 14 && (p[4] >> 6) == 0x01 )
{
*pi_scr = FROM_SCALE( ExtractPackHeaderTimestamp( &p[4] ) );
*pi_mux_rate = ( p[10] << 14 )|( p[11] << 6 )|( p[12] >> 2);
}
else if( i_pkt >= 12 && (p[4] >> 4) == 0x02 ) 
{
stime_t i_scr;
if(!ExtractPESTimestamp( &p[4], 0x02, &i_scr ))
return VLC_EGENERIC;
*pi_scr = FROM_SCALE( i_scr );
*pi_mux_rate = ( ( p[9]&0x7f )<< 15 )|( p[10] << 7 )|( p[11] >> 1);
}
else
{
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
static inline int ps_pkt_parse_system( const uint8_t *p_pkt, size_t i_pkt,
ps_psm_t *p_psm,
ps_track_t tk[PS_TK_COUNT] )
{
const uint8_t *p = &p_pkt[6 + 3 + 1 + 1 + 1];
const uint8_t *p_pktend = &p_pkt[i_pkt];
while( p < p_pktend && (p[0] & 0x80) )
{
int i_id = p[0];
switch( i_id )
{
case 0xB7:
if( p_pktend - p < 6 )
return VLC_EGENERIC;
i_id = ((int)PS_STREAM_ID_EXTENDED << 8) | (p[2] & 0x7F);
p += 6;
break;
default:
if( p_pktend - p < 3 )
return VLC_EGENERIC;
p += 3;
break;
}
if( i_id < 0xc0 )
continue;
unsigned i_tk = ps_id_to_tk( i_id );
if( !tk[i_tk].b_configured )
ps_track_fill( &tk[i_tk], p_psm, i_id, NULL, 0, false );
}
return VLC_SUCCESS;
}
static inline int ps_pkt_parse_pes( vlc_object_t *p_object, block_t *p_pes, int i_skip_extra )
{
unsigned int i_skip = 0;
stime_t i_pts = -1;
stime_t i_dts = -1;
uint8_t i_stream_id = 0;
bool b_pes_scrambling = false;
if( ParsePESHeader( p_object, p_pes->p_buffer, p_pes->i_buffer,
&i_skip, &i_dts, &i_pts, &i_stream_id, &b_pes_scrambling ) != VLC_SUCCESS )
return VLC_EGENERIC;
if( b_pes_scrambling )
p_pes->i_flags |= BLOCK_FLAG_SCRAMBLED;
if( i_skip_extra >= 0 )
i_skip += i_skip_extra;
else if( p_pes->i_buffer > i_skip + 3 &&
( ps_pkt_id( p_pes->p_buffer, p_pes->i_buffer ) == 0xa001 ||
ps_pkt_id( p_pes->p_buffer, p_pes->i_buffer ) == 0xbda1 ) )
i_skip += 4 + p_pes->p_buffer[i_skip+3];
if( p_pes->i_buffer <= i_skip )
{
return VLC_EGENERIC;
}
p_pes->p_buffer += i_skip;
p_pes->i_buffer -= i_skip;
if( i_pts >= 0 && i_dts < 0 )
i_dts = i_pts;
if( i_dts >= 0 )
p_pes->i_dts = FROM_SCALE( i_dts );
if( i_pts >= 0 )
p_pes->i_pts = FROM_SCALE( i_pts );
return VLC_SUCCESS;
}
typedef struct
{
uint8_t lang[3];
} ps_descriptors_t;
typedef struct ps_es_t
{
uint8_t i_type;
uint16_t i_id;
ps_descriptors_t desc;
} ps_es_t;
struct ps_psm_t
{
uint8_t i_version;
size_t i_es;
ps_es_t *es;
ps_descriptors_t uniqueextdesc;
};
static inline uint8_t ps_id_to_type( const ps_psm_t *p_psm, uint16_t i_id )
{
size_t i;
for( i = 0; p_psm && i < p_psm->i_es; i++ )
{
if( p_psm->es[i].i_id == i_id ) return p_psm->es[i].i_type;
}
return 0;
}
static inline const uint8_t *ps_id_to_lang( const ps_psm_t *p_psm, uint16_t i_id )
{
size_t i;
for( i = 0; p_psm && i < p_psm->i_es; i++ )
{
if( p_psm->es[i].i_id == i_id )
return p_psm->es[i].desc.lang;
}
return 0;
}
static inline void ps_psm_init( ps_psm_t *p_psm )
{
p_psm->i_version = 0xFF;
p_psm->i_es = 0;
p_psm->es = 0;
memset( &p_psm->uniqueextdesc, 0, 3 );
}
static inline void ps_psm_destroy( ps_psm_t *p_psm )
{
free( p_psm->es );
p_psm->es = NULL;
p_psm->i_es = 0;
}
static inline void ps_parse_descriptors( const uint8_t *p_data, size_t i_data,
ps_descriptors_t *p_desc )
{
while( i_data > 3 && i_data > 2u + p_data[1] )
{
switch( p_data[0] )
{
case 0x0A: 
if( i_data >= 6 )
memcpy( p_desc->lang, &p_data[2], 3 );
break;
default:
break;
}
uint8_t i_desc_size = p_data[1];
p_data += 2 + i_desc_size;
i_data -= 2 + i_desc_size;
}
}
static inline int ps_psm_fill( ps_psm_t *p_psm,
const uint8_t *p_buffer, size_t i_pkt,
ps_track_t tk[PS_TK_COUNT], es_out_t *out )
{
size_t i_length, i_info_length, i_es_base;
uint8_t i_version;
bool b_single_extension;
assert(i_pkt >= 4);
if( !p_psm || i_pkt < 10 || p_buffer[3] != PS_STREAM_ID_MAP)
return VLC_EGENERIC;
i_length = GetWBE(&p_buffer[4]) + 6;
if( i_length > i_pkt ) return VLC_EGENERIC;
if((p_buffer[6] & 0x80) == 0) 
return VLC_EGENERIC;
b_single_extension = p_buffer[6] & 0x40;
i_version = (p_buffer[6] & 0xf8);
if( p_psm->i_version == i_version ) return VLC_EGENERIC;
ps_psm_destroy( p_psm );
i_info_length = GetWBE(&p_buffer[8]);
if( i_info_length + 10 > i_length )
return VLC_EGENERIC;
i_es_base = 12 + i_info_length;
while( i_es_base + 4 < i_length )
{
ps_es_t *tmp_es = realloc( p_psm->es, sizeof(ps_es_t) * (p_psm->i_es+1) );
if( tmp_es == NULL )
break;
p_psm->es = tmp_es;
ps_es_t *p_es = &p_psm->es[ p_psm->i_es++ ];
p_es->i_type = p_buffer[ i_es_base ];
p_es->i_id = p_buffer[ i_es_base + 1 ];
i_info_length = GetWBE(&p_buffer[ i_es_base + 2 ]);
if( i_es_base + 4 + i_info_length > i_length )
break;
if( p_es->i_id == PS_STREAM_ID_EXTENDED && b_single_extension == 0 )
{
if( i_info_length < 3 )
break;
p_es->i_id = (p_es->i_id << 8) | (p_buffer[i_es_base + 6] & 0x7F);
ps_parse_descriptors( &p_buffer[i_es_base + 4 + 3],
i_info_length - 3,
&p_psm->uniqueextdesc );
}
else
{
ps_parse_descriptors( &p_buffer[i_es_base + 4],
i_info_length, &p_es->desc );
}
i_es_base += 4 + i_info_length;
}
p_psm->i_version = i_version;
for( int i = 0; i < PS_TK_COUNT; i++ )
{
if( !tk[i].b_configured || !tk[i].es ) continue;
ps_track_t tk_tmp;
es_format_Init( &tk_tmp.fmt, UNKNOWN_ES, 0 );
if( ps_track_fill( &tk_tmp, p_psm, tk[i].i_id,
p_buffer, i_pkt, false ) != VLC_SUCCESS )
continue;
if( tk_tmp.fmt.i_codec == tk[i].fmt.i_codec )
{
es_format_Clean( &tk_tmp.fmt );
continue;
}
es_out_Del( out, tk[i].es );
es_format_Clean( &tk[i].fmt );
tk_tmp.b_configured = true;
tk[i] = tk_tmp;
tk[i].es = es_out_Add( out, &tk[i].fmt );
}
return VLC_SUCCESS;
}
