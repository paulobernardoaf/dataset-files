


























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h> 
#include <vlc_vout.h> 
#include <vlc_filter.h>

#include <goom/goom.h>




static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );

#define WIDTH_TEXT N_("Goom display width")
#define HEIGHT_TEXT N_("Goom display height")
#define RES_LONGTEXT N_("This allows you to set the resolution of the " "Goom display (bigger resolution will be prettier but more CPU intensive).")


#define SPEED_TEXT N_("Goom animation speed")
#define SPEED_LONGTEXT N_("This allows you to set the animation speed " "(between 1 and 10, defaults to 6).")


#define MAX_SPEED 10

vlc_module_begin ()
set_shortname( N_("Goom"))
set_description( N_("Goom effect") )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_VISUAL )
set_capability( "visualization", 0 )
add_integer( "goom-width", 800,
WIDTH_TEXT, RES_LONGTEXT, false )
add_integer( "goom-height", 500,
HEIGHT_TEXT, RES_LONGTEXT, false )
add_integer_with_range( "goom-speed", 6, 1, 10,
SPEED_TEXT, SPEED_LONGTEXT, false )
set_callbacks( Open, Close )
add_shortcut( "goom" )
vlc_module_end ()




#define MAX_BLOCKS 100
#define GOOM_DELAY VLC_TICK_FROM_MS(400)

typedef struct
{
vlc_thread_t thread;
video_format_t fmt;

vout_thread_t *p_vout;
int i_speed;

vlc_mutex_t lock;
vlc_cond_t wait;
bool b_exit;


unsigned i_channels;


block_t *pp_blocks[MAX_BLOCKS];
int i_blocks;

date_t date;

} goom_thread_t;

static block_t *DoWork ( filter_t *, block_t * );
static void Flush( filter_t * );

static void *Thread( void * );




static int Open( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
goom_thread_t *p_thread;


p_filter->p_sys = p_thread = calloc( 1, sizeof(*p_thread) );

const int width = var_InheritInteger( p_filter, "goom-width" );
const int height = var_InheritInteger( p_filter, "goom-height" );

p_thread->fmt.i_width = p_thread->fmt.i_visible_width = width;
p_thread->fmt.i_height = p_thread->fmt.i_visible_height = height;
p_thread->fmt.i_chroma = VLC_CODEC_RGB32;
p_thread->fmt.i_sar_num = p_thread->fmt.i_sar_den = 1;

p_thread->p_vout = aout_filter_GetVout( p_filter, &p_thread->fmt );
if( p_thread->p_vout == NULL )
{
msg_Err( p_filter, "no suitable vout module" );
free( p_thread );
return VLC_EGENERIC;
}

p_thread->i_speed = MAX_SPEED - var_InheritInteger( p_filter, "goom-speed" );
if( p_thread->i_speed < 0 )
p_thread->i_speed = 0;

vlc_mutex_init( &p_thread->lock );
vlc_cond_init( &p_thread->wait );

p_thread->i_blocks = 0;
date_Init( &p_thread->date, p_filter->fmt_in.audio.i_rate, 1 );
date_Set( &p_thread->date, VLC_TICK_0 );
p_thread->i_channels = aout_FormatNbChannels( &p_filter->fmt_in.audio );

if( vlc_clone( &p_thread->thread,
Thread, p_thread, VLC_THREAD_PRIORITY_LOW ) )
{
msg_Err( p_filter, "cannot launch goom thread" );
vout_Close( p_thread->p_vout );
free( p_thread );
return VLC_EGENERIC;
}

p_filter->fmt_in.audio.i_format = VLC_CODEC_FL32;
p_filter->fmt_out.audio = p_filter->fmt_in.audio;
p_filter->pf_audio_filter = DoWork;
p_filter->pf_flush = Flush;
return VLC_SUCCESS;
}






static block_t *DoWork( filter_t *p_filter, block_t *p_in_buf )
{
goom_thread_t *p_thread = p_filter->p_sys;
block_t *p_block;


vlc_mutex_lock( &p_thread->lock );
if( p_thread->i_blocks == MAX_BLOCKS )
{
vlc_mutex_unlock( &p_thread->lock );
return p_in_buf;
}

p_block = block_Alloc( p_in_buf->i_buffer );
if( !p_block )
{
vlc_mutex_unlock( &p_thread->lock );
return p_in_buf;
}
memcpy( p_block->p_buffer, p_in_buf->p_buffer, p_in_buf->i_buffer );
p_block->i_pts = p_in_buf->i_pts;

p_thread->pp_blocks[p_thread->i_blocks++] = p_block;

vlc_cond_signal( &p_thread->wait );
vlc_mutex_unlock( &p_thread->lock );

return p_in_buf;
}

static void Flush( filter_t *p_filter )
{
goom_thread_t *p_thread = p_filter->p_sys;
vout_FlushAll( p_thread->p_vout );
}




static inline int16_t FloatToInt16( float f )
{
if( f >= 1.0 )
return 32767;
else if( f < -1.0 )
return -32768;
else
return (int16_t)( f * 32768.0 );
}




static int FillBuffer( int16_t *p_data, int *pi_data,
date_t *pi_date, date_t *pi_date_end,
goom_thread_t *p_this )
{
int i_samples = 0;
block_t *p_block;

while( *pi_data < 512 )
{
if( !p_this->i_blocks ) return VLC_EGENERIC;

p_block = p_this->pp_blocks[0];
i_samples = __MIN( (unsigned)(512 - *pi_data),
p_block->i_buffer / sizeof(float) / p_this->i_channels );


if( p_block->i_pts != VLC_TICK_INVALID &&
p_block->i_pts != date_Get( pi_date_end ) )
{
date_Set( pi_date_end, p_block->i_pts );
}
p_block->i_pts = VLC_TICK_INVALID;

date_Increment( pi_date_end, i_samples );

while( i_samples > 0 )
{
float *p_float = (float *)p_block->p_buffer;

p_data[*pi_data] = FloatToInt16( p_float[0] );
if( p_this->i_channels > 1 )
p_data[512 + *pi_data] = FloatToInt16( p_float[1] );

(*pi_data)++;
p_block->p_buffer += (sizeof(float) * p_this->i_channels);
p_block->i_buffer -= (sizeof(float) * p_this->i_channels);
i_samples--;
}

if( !p_block->i_buffer )
{
block_Release( p_block );
p_this->i_blocks--;
if( p_this->i_blocks )
memmove( p_this->pp_blocks, p_this->pp_blocks + 1,
p_this->i_blocks * sizeof(block_t *) );
}
}

*pi_date = *pi_date_end;
*pi_data = 0;
return VLC_SUCCESS;
}




static void *Thread( void *p_thread_data )
{
goom_thread_t *p_thread = (goom_thread_t*)p_thread_data;
date_t i_pts;
int16_t p_data[2][512];
int i_data = 0, i_count = 0;
PluginInfo *p_plugin_info;
int canc = vlc_savecancel ();

p_plugin_info = goom_init( p_thread->fmt.i_width, p_thread->fmt.i_height );

for( ;; )
{
uint32_t *plane;
picture_t *p_pic;









vlc_mutex_lock( &p_thread->lock );
if( !p_thread->b_exit &&
FillBuffer( (int16_t *)p_data, &i_data,
&i_pts, &p_thread->date, p_thread ) != VLC_SUCCESS )
vlc_cond_wait( &p_thread->wait, &p_thread->lock );
bool b_exit = p_thread->b_exit;
vlc_mutex_unlock( &p_thread->lock );

if( b_exit )
break;


if( p_thread->i_speed && (++i_count % (p_thread->i_speed+1)) ) continue;


if( date_Get( &i_pts ) + GOOM_DELAY <= vlc_tick_now() ) continue;

plane = goom_update( p_plugin_info, p_data, 0, 0.0,
NULL, NULL );

p_pic = vout_GetPicture( p_thread->p_vout );
if( unlikely(p_pic == NULL) )
continue;

memcpy( p_pic->p[0].p_pixels, plane,
p_thread->fmt.i_width * p_thread->fmt.i_height * 4 );

p_pic->date = date_Get( &i_pts ) + GOOM_DELAY;
vout_PutPicture( p_thread->p_vout, p_pic );
}

goom_close( p_plugin_info );
vlc_restorecancel (canc);
return NULL;
}




static void Close( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
goom_thread_t *p_thread = p_filter->p_sys;


vlc_mutex_lock( &p_thread->lock );
p_thread->b_exit = true;
vlc_cond_signal( &p_thread->wait );
vlc_mutex_unlock( &p_thread->lock );

vlc_join( p_thread->thread, NULL );


vout_Close( p_thread->p_vout );

while( p_thread->i_blocks-- )
{
block_Release( p_thread->pp_blocks[p_thread->i_blocks] );
}

free( p_thread );
}

