


























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_demux.h>

#include "rawdv.h"




#define HURRYUP_TEXT N_( "Hurry up" )
#define HURRYUP_LONGTEXT N_( "The demuxer will advance timestamps if the " "input can't keep up with the rate." )


static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );

vlc_module_begin ()
set_shortname( "DV" )
set_description( N_("DV (Digital Video) demuxer") )
set_capability( "demux", 3 )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_DEMUX )
add_bool( "rawdv-hurry-up", false, HURRYUP_TEXT, HURRYUP_LONGTEXT, false )
set_callbacks( Open, Close )
add_shortcut( "rawdv" )
vlc_module_end ()



















typedef struct {
int8_t sct; 
int8_t dsn; 
int fsc; 
int8_t dbn; 
} dv_id_t;

typedef struct {
int dsf; 
int8_t apt;
int tf1;
int8_t ap1;
int tf2;
int8_t ap2;
int tf3;
int8_t ap3;
} dv_header_t;

typedef struct
{
int frame_size;

es_out_id_t *p_es_video;
es_format_t fmt_video;

es_out_id_t *p_es_audio;
es_format_t fmt_audio;

int i_dsf;
double f_rate;
int i_bitrate;


vlc_tick_t i_pcr;
bool b_hurry_up;
} demux_sys_t;




static int Demux( demux_t * );
static int Control( demux_t *, int i_query, va_list args );




static int Open( vlc_object_t * p_this )
{
demux_t *p_demux = (demux_t*)p_this;
demux_sys_t *p_sys;

const uint8_t *p_peek, *p_peek_backup;

uint32_t i_dword;
dv_header_t dv_header;
dv_id_t dv_id;







if( !demux_IsPathExtension( p_demux, ".dv" ) && !p_demux->obj.force )
return VLC_EGENERIC;

if( vlc_stream_Peek( p_demux->s, &p_peek, DV_PAL_FRAME_SIZE ) <
DV_NTSC_FRAME_SIZE )
{

msg_Err( p_demux, "cannot peek()" );
return VLC_EGENERIC;
}
p_peek_backup = p_peek;


i_dword = GetDWBE( p_peek ); p_peek += 4;
dv_id.sct = i_dword >> (32 - 3);
i_dword <<= 8;
dv_id.dsn = i_dword >> (32 - 4);
i_dword <<= 4;
dv_id.fsc = i_dword >> (32 - 1);
i_dword <<= 4;
dv_id.dbn = i_dword >> (32 - 8);
i_dword <<= 8;

if( dv_id.sct != 0 )
{
msg_Warn( p_demux, "not a raw DV stream header" );
return VLC_EGENERIC;
}


dv_header.dsf = i_dword >> (32 - 1);
i_dword <<= 1;
if( i_dword >> (32 - 1) ) 
{
msg_Warn( p_demux, "incorrect bit" );
return VLC_EGENERIC;
}

i_dword = GetDWBE( p_peek ); p_peek += 4;
i_dword <<= 5;
dv_header.apt = i_dword >> (32 - 3);
i_dword <<= 3;
dv_header.tf1 = i_dword >> (32 - 1);
i_dword <<= 5;
dv_header.ap1 = i_dword >> (32 - 3);
i_dword <<= 3;
dv_header.tf2 = i_dword >> (32 - 1);
i_dword <<= 5;
dv_header.ap2 = i_dword >> (32 - 3);
i_dword <<= 3;
dv_header.tf3 = i_dword >> (32 - 1);
i_dword <<= 5;
dv_header.ap3 = i_dword >> (32 - 3);

p_peek += 72; 

p_demux->p_sys = p_sys = malloc( sizeof( demux_sys_t ) );
if( !p_sys )
return VLC_ENOMEM;

p_sys->b_hurry_up = var_CreateGetBool( p_demux, "rawdv-hurry-up" );
msg_Dbg( p_demux, "Realtime DV Source: %s", (p_sys->b_hurry_up)?"Yes":"No" );

p_sys->i_dsf = dv_header.dsf;
p_sys->frame_size = dv_header.dsf ? DV_PAL_FRAME_SIZE
: DV_NTSC_FRAME_SIZE;
p_sys->f_rate = dv_header.dsf ? 25 : 29.97;

p_sys->i_pcr = 0;
p_sys->p_es_video = NULL;
p_sys->p_es_audio = NULL;

p_sys->i_bitrate = 0;

es_format_Init( &p_sys->fmt_video, VIDEO_ES, VLC_CODEC_DV );
p_sys->fmt_video.video.i_width = 720;
p_sys->fmt_video.video.i_height= dv_header.dsf ? 576 : 480;;
p_sys->fmt_video.video.i_visible_width = p_sys->fmt_video.video.i_width;
p_sys->fmt_video.video.i_visible_height = p_sys->fmt_video.video.i_height;

p_sys->p_es_video = es_out_Add( p_demux->out, &p_sys->fmt_video );


p_peek = p_peek_backup + 80*6+80*16*3 + 3; 
if( *p_peek == 0x50 )
{
dv_get_audio_format( &p_sys->fmt_audio, &p_peek[1] );
p_sys->p_es_audio = es_out_Add( p_demux->out, &p_sys->fmt_audio );
}

p_demux->pf_demux = Demux;
p_demux->pf_control = Control;
return VLC_SUCCESS;
}




static void Close( vlc_object_t *p_this )
{
demux_t *p_demux = (demux_t*)p_this;
demux_sys_t *p_sys = p_demux->p_sys;

var_Destroy( p_demux, "rawdv-hurry-up");
free( p_sys );
}






static int Demux( demux_t *p_demux )
{
demux_sys_t *p_sys = p_demux->p_sys;
block_t *p_block;
bool b_audio = false;

if( p_sys->b_hurry_up )
{

p_sys->i_pcr = vlc_tick_now() + (p_sys->i_dsf ? VLC_TICK_FROM_MS(120) : VLC_TICK_FROM_MS(90));
}


es_out_SetPCR( p_demux->out, VLC_TICK_0 + p_sys->i_pcr );
p_block = vlc_stream_Block( p_demux->s, p_sys->frame_size );
if( p_block == NULL )
return VLC_DEMUXER_EOF;

if( p_sys->p_es_audio )
{
es_out_Control( p_demux->out, ES_OUT_GET_ES_STATE,
p_sys->p_es_audio, &b_audio );
}

p_block->i_dts =
p_block->i_pts = VLC_TICK_0 + p_sys->i_pcr;

if( b_audio )
{
block_t *p_audio_block = dv_extract_audio( p_block );
if( p_audio_block )
es_out_Send( p_demux->out, p_sys->p_es_audio, p_audio_block );
}

es_out_Send( p_demux->out, p_sys->p_es_video, p_block );

if( !p_sys->b_hurry_up )
{
p_sys->i_pcr += vlc_tick_rate_duration( p_sys->f_rate );
}

return VLC_DEMUXER_SUCCESS;
}




static int Control( demux_t *p_demux, int i_query, va_list args )
{
demux_sys_t *p_sys = p_demux->p_sys;


return demux_vaControlHelper( p_demux->s,
0, -1,
p_sys->frame_size * p_sys->f_rate * 8,
p_sys->frame_size, i_query, args );
}

