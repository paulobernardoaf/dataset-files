#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <math.h> 
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_filter.h>
#include <vlc_block.h>
static int OpenFilter ( vlc_object_t * );
static void CloseFilter( vlc_object_t * );
static block_t *Convert( filter_t *, block_t * );
#define MODULE_DESCRIPTION N_ ( "This effect gives you the feeling that you are standing in a room " "with a complete 7.1 speaker set when using only a headphone, " "providing a more realistic sound experience. It should also be " "more comfortable and less tiring when listening to music for " "long periods of time.\nIt works with any source format from mono " "to 7.1.")
#define HEADPHONE_DIM_TEXT N_("Characteristic dimension")
#define HEADPHONE_DIM_LONGTEXT N_( "Distance between front left speaker and listener in meters.")
#define HEADPHONE_COMPENSATE_TEXT N_("Compensate delay")
#define HEADPHONE_COMPENSATE_LONGTEXT N_( "The delay which is introduced by the physical algorithm may ""sometimes be disturbing for the synchronization between lips-movement ""and speech. In case, turn this on to compensate.")
#define HEADPHONE_DOLBY_TEXT N_("No decoding of Dolby Surround")
#define HEADPHONE_DOLBY_LONGTEXT N_( "Dolby Surround encoded streams won't be decoded before being " "processed by this filter. Enabling this setting is not recommended.")
vlc_module_begin ()
set_description( N_("Headphone virtual spatialization effect") )
set_shortname( N_("Headphone effect") )
set_help( MODULE_DESCRIPTION )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_AFILTER )
add_integer( "headphone-dim", 10, HEADPHONE_DIM_TEXT,
HEADPHONE_DIM_LONGTEXT, false )
add_bool( "headphone-compensate", false, HEADPHONE_COMPENSATE_TEXT,
HEADPHONE_COMPENSATE_LONGTEXT, true )
add_bool( "headphone-dolby", false, HEADPHONE_DOLBY_TEXT,
HEADPHONE_DOLBY_LONGTEXT, true )
set_capability( "audio filter", 0 )
set_callbacks( OpenFilter, CloseFilter )
add_shortcut( "headphone" )
vlc_module_end ()
struct atomic_operation_t
{
int i_source_channel_offset;
int i_dest_channel_offset;
unsigned int i_delay;
double d_amplitude_factor;
};
typedef struct
{
size_t i_overflow_buffer_size;
float * p_overflow_buffer;
unsigned int i_nb_atomic_operations;
struct atomic_operation_t * p_atomic_operations;
} filter_sys_t;
static void ComputeChannelOperations( filter_sys_t * p_data
, unsigned int i_rate, unsigned int i_next_atomic_operation
, int i_source_channel_offset, double d_x, double d_z
, double d_compensation_length, double d_channel_amplitude_factor )
{
double d_c = 340; 
double d_compensation_delay = (d_compensation_length-0.1) / d_c * i_rate;
p_data->p_atomic_operations[i_next_atomic_operation]
.i_source_channel_offset = i_source_channel_offset;
p_data->p_atomic_operations[i_next_atomic_operation]
.i_dest_channel_offset = 0;
p_data->p_atomic_operations[i_next_atomic_operation]
.i_delay = (int)( sqrt( (-0.1-d_x)*(-0.1-d_x) + (0-d_z)*(0-d_z) )
/ d_c * i_rate - d_compensation_delay );
if( d_x < 0 )
{
p_data->p_atomic_operations[i_next_atomic_operation]
.d_amplitude_factor = d_channel_amplitude_factor * 1.1 / 2;
}
else if( d_x > 0 )
{
p_data->p_atomic_operations[i_next_atomic_operation]
.d_amplitude_factor = d_channel_amplitude_factor * 0.9 / 2;
}
else
{
p_data->p_atomic_operations[i_next_atomic_operation]
.d_amplitude_factor = d_channel_amplitude_factor / 2;
}
p_data->p_atomic_operations[i_next_atomic_operation + 1]
.i_source_channel_offset = i_source_channel_offset;
p_data->p_atomic_operations[i_next_atomic_operation + 1]
.i_dest_channel_offset = 1;
p_data->p_atomic_operations[i_next_atomic_operation + 1]
.i_delay = (int)( sqrt( (0.1-d_x)*(0.1-d_x) + (0-d_z)*(0-d_z) )
/ d_c * i_rate - d_compensation_delay );
if( d_x < 0 )
{
p_data->p_atomic_operations[i_next_atomic_operation + 1]
.d_amplitude_factor = d_channel_amplitude_factor * 0.9 / 2;
}
else if( d_x > 0 )
{
p_data->p_atomic_operations[i_next_atomic_operation + 1]
.d_amplitude_factor = d_channel_amplitude_factor * 1.1 / 2;
}
else
{
p_data->p_atomic_operations[i_next_atomic_operation + 1]
.d_amplitude_factor = d_channel_amplitude_factor / 2;
}
}
static int Init( vlc_object_t *p_this, filter_sys_t * p_data
, unsigned int i_nb_channels, uint32_t i_physical_channels
, unsigned int i_rate )
{
double d_x = var_InheritInteger( p_this, "headphone-dim" );
double d_z = d_x;
double d_z_rear = -d_x/3;
double d_min = 0;
unsigned int i_next_atomic_operation;
int i_source_channel_offset;
unsigned int i;
if( var_InheritBool( p_this, "headphone-compensate" ) )
{
if( i_physical_channels & AOUT_CHAN_REARCENTER )
{
d_min = d_z_rear;
}
else
{
d_min = d_z;
}
}
p_data->i_nb_atomic_operations = i_nb_channels * 2;
if( i_physical_channels & AOUT_CHAN_CENTER )
{
p_data->i_nb_atomic_operations += 2;
}
p_data->p_atomic_operations = malloc( sizeof(struct atomic_operation_t)
* p_data->i_nb_atomic_operations );
if( p_data->p_atomic_operations == NULL )
return -1;
i_next_atomic_operation = 0;
i_source_channel_offset = 0;
if( i_physical_channels & AOUT_CHAN_LEFT )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, -d_x , d_z , d_min , 2.0 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
if( i_physical_channels & AOUT_CHAN_RIGHT )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, d_x , d_z , d_min , 2.0 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
if( i_physical_channels & AOUT_CHAN_MIDDLELEFT )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, -d_x , 0 , d_min , 1.5 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
if( i_physical_channels & AOUT_CHAN_MIDDLERIGHT )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, d_x , 0 , d_min , 1.5 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
if( i_physical_channels & AOUT_CHAN_REARLEFT )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, -d_x , d_z_rear , d_min , 1.5 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
if( i_physical_channels & AOUT_CHAN_REARRIGHT )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, d_x , d_z_rear , d_min , 1.5 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
if( i_physical_channels & AOUT_CHAN_REARCENTER )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, 0 , -d_z , d_min , 1.5 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
if( i_physical_channels & AOUT_CHAN_CENTER )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, d_x / 5.0 , d_z , d_min , 0.75 / i_nb_channels );
i_next_atomic_operation += 2;
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, -d_x / 5.0 , d_z , d_min , 0.75 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
if( i_physical_channels & AOUT_CHAN_LFE )
{
ComputeChannelOperations( p_data , i_rate
, i_next_atomic_operation , i_source_channel_offset
, 0 , d_z_rear , d_min , 5.0 / i_nb_channels );
i_next_atomic_operation += 2;
i_source_channel_offset++;
}
p_data->i_overflow_buffer_size = 0;
for( i = 0 ; i < p_data->i_nb_atomic_operations ; i++ )
{
if( p_data->i_overflow_buffer_size
< p_data->p_atomic_operations[i].i_delay * 2 * sizeof (float) )
{
p_data->i_overflow_buffer_size
= p_data->p_atomic_operations[i].i_delay * 2 * sizeof (float);
}
}
p_data->p_overflow_buffer = malloc( p_data->i_overflow_buffer_size );
if( p_data->p_overflow_buffer == NULL )
{
free( p_data->p_atomic_operations );
return -1;
}
memset( p_data->p_overflow_buffer, 0 , p_data->i_overflow_buffer_size );
return 0;
}
static void DoWork( filter_t * p_filter,
block_t * p_in_buf, block_t * p_out_buf )
{
filter_sys_t *p_sys = p_filter->p_sys;
int i_input_nb = aout_FormatNbChannels( &p_filter->fmt_in.audio );
int i_output_nb = aout_FormatNbChannels( &p_filter->fmt_out.audio );
float * p_in = (float*) p_in_buf->p_buffer;
float * p_out;
uint8_t * p_overflow;
uint8_t * p_end_overflow;
uint8_t * p_slide;
size_t i_overflow_size; 
size_t i_out_size; 
unsigned int i, j;
int i_source_channel_offset;
int i_dest_channel_offset;
unsigned int i_delay;
double d_amplitude_factor;
p_out = (float *)p_out_buf->p_buffer;
i_out_size = p_out_buf->i_buffer;
p_overflow = (uint8_t *) p_sys->p_overflow_buffer;
i_overflow_size = p_sys->i_overflow_buffer_size;
p_end_overflow = p_overflow + i_overflow_size;
memset( p_out, 0, i_out_size );
memcpy( p_out, p_overflow, __MIN( i_out_size, i_overflow_size ) );
p_slide = (uint8_t *) p_sys->p_overflow_buffer;
while( p_slide < p_end_overflow )
{
size_t i_bytes_copied;
if( p_slide + i_out_size < p_end_overflow )
{
memset( p_slide, 0, i_out_size );
if( p_slide + 2 * i_out_size < p_end_overflow )
i_bytes_copied = i_out_size;
else
i_bytes_copied = p_end_overflow - ( p_slide + i_out_size );
memcpy( p_slide, p_slide + i_out_size, i_bytes_copied );
}
else
{
i_bytes_copied = p_end_overflow - p_slide;
memset( p_slide, 0, i_bytes_copied );
}
p_slide += i_bytes_copied;
}
for( i = 0; i < p_sys->i_nb_atomic_operations; i++ )
{
i_source_channel_offset
= p_sys->p_atomic_operations[i].i_source_channel_offset;
i_dest_channel_offset
= p_sys->p_atomic_operations[i].i_dest_channel_offset;
i_delay = p_sys->p_atomic_operations[i].i_delay;
d_amplitude_factor
= p_sys->p_atomic_operations[i].d_amplitude_factor;
if( p_out_buf->i_nb_samples > i_delay )
{
for( j = 0; j < p_out_buf->i_nb_samples - i_delay; j++ )
{
((float*)p_out)[ (i_delay+j)*i_output_nb + i_dest_channel_offset ]
+= p_in[ j * i_input_nb + i_source_channel_offset ]
* d_amplitude_factor;
}
for( j = 0; j < i_delay; j++ )
{
((float*)p_overflow)[ j*i_output_nb + i_dest_channel_offset ]
+= p_in[ (p_out_buf->i_nb_samples - i_delay + j)
* i_input_nb + i_source_channel_offset ]
* d_amplitude_factor;
}
}
else
{
for( j = 0; j < p_out_buf->i_nb_samples; j++ )
{
((float*)p_overflow)[ (i_delay - p_out_buf->i_nb_samples + j)
* i_output_nb + i_dest_channel_offset ]
+= p_in[ j * i_input_nb + i_source_channel_offset ]
* d_amplitude_factor;
}
}
}
}
static int OpenFilter( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys;
if( p_filter->fmt_out.audio.i_physical_channels
!= (AOUT_CHAN_LEFT|AOUT_CHAN_RIGHT) )
{
msg_Dbg( p_filter, "filter discarded (incompatible format)" );
return VLC_EGENERIC;
}
p_sys = p_filter->p_sys = malloc( sizeof(filter_sys_t) );
if( p_sys == NULL )
return VLC_ENOMEM;
p_sys->i_overflow_buffer_size = 0;
p_sys->p_overflow_buffer = NULL;
p_sys->i_nb_atomic_operations = 0;
p_sys->p_atomic_operations = NULL;
if( Init( VLC_OBJECT(p_filter), p_sys
, aout_FormatNbChannels ( &(p_filter->fmt_in.audio) )
, p_filter->fmt_in.audio.i_physical_channels
, p_filter->fmt_in.audio.i_rate ) < 0 )
{
free( p_sys );
return VLC_EGENERIC;
}
p_filter->fmt_in.audio.i_format = VLC_CODEC_FL32;
p_filter->fmt_out.audio.i_format = VLC_CODEC_FL32;
p_filter->fmt_out.audio.i_rate = p_filter->fmt_in.audio.i_rate;
p_filter->fmt_in.audio.i_chan_mode =
p_filter->fmt_out.audio.i_chan_mode;
if( p_filter->fmt_in.audio.i_physical_channels == AOUT_CHANS_STEREO
&& (p_filter->fmt_in.audio.i_chan_mode & AOUT_CHANMODE_DOLBYSTEREO)
&& !var_InheritBool( p_filter, "headphone-dolby" ) )
{
p_filter->fmt_in.audio.i_physical_channels = AOUT_CHANS_5_0;
}
p_filter->pf_audio_filter = Convert;
aout_FormatPrepare(&p_filter->fmt_in.audio);
aout_FormatPrepare(&p_filter->fmt_out.audio);
return VLC_SUCCESS;
}
static void CloseFilter( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
free( p_sys->p_overflow_buffer );
free( p_sys->p_atomic_operations );
free( p_sys );
}
static block_t *Convert( filter_t *p_filter, block_t *p_block )
{
if( !p_block || !p_block->i_nb_samples )
{
if( p_block )
block_Release( p_block );
return NULL;
}
size_t i_out_size = p_block->i_buffer *
aout_FormatNbChannels( &(p_filter->fmt_out.audio) ) /
aout_FormatNbChannels( &(p_filter->fmt_in.audio) );
block_t *p_out = block_Alloc( i_out_size );
if( !p_out )
{
msg_Warn( p_filter, "can't get output buffer" );
block_Release( p_block );
return NULL;
}
p_out->i_nb_samples = p_block->i_nb_samples;
p_out->i_dts = p_block->i_dts;
p_out->i_pts = p_block->i_pts;
p_out->i_length = p_block->i_length;
DoWork( p_filter, p_block, p_out );
block_Release( p_block );
return p_out;
}
