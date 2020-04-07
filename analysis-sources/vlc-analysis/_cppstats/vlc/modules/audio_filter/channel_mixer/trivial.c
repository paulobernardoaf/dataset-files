#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_filter.h>
static int Create( vlc_object_t * );
static void Destroy( vlc_object_t * );
vlc_module_begin ()
set_description( N_("Audio filter for trivial channel mixing") )
set_capability( "audio converter", 1 )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_MISC )
set_callbacks( Create, Destroy )
vlc_module_end ()
typedef struct
{
int channel_map[AOUT_CHAN_MAX];
} filter_sys_t;
static block_t *Upmix( filter_t *p_filter, block_t *p_in_buf )
{
unsigned i_input_nb = aout_FormatNbChannels( &p_filter->fmt_in.audio );
unsigned i_output_nb = aout_FormatNbChannels( &p_filter->fmt_out.audio );
assert( i_input_nb < i_output_nb );
block_t *p_out_buf = block_Alloc(
p_in_buf->i_buffer * i_output_nb / i_input_nb );
if( unlikely(p_out_buf == NULL) )
{
block_Release( p_in_buf );
return NULL;
}
p_out_buf->i_nb_samples = p_in_buf->i_nb_samples;
p_out_buf->i_dts = p_in_buf->i_dts;
p_out_buf->i_pts = p_in_buf->i_pts;
p_out_buf->i_length = p_in_buf->i_length;
filter_sys_t *p_sys = p_filter->p_sys;
float *p_dest = (float *)p_out_buf->p_buffer;
const float *p_src = (float *)p_in_buf->p_buffer;
const int *channel_map = p_sys->channel_map;
for( size_t i = 0; i < p_in_buf->i_nb_samples; i++ )
{
for( unsigned j = 0; j < i_output_nb; j++ )
p_dest[j] = channel_map[j] == -1 ? 0.f : p_src[channel_map[j]];
p_src += i_input_nb;
p_dest += i_output_nb;
}
block_Release( p_in_buf );
return p_out_buf;
}
static block_t *Downmix( filter_t *p_filter, block_t *p_buf )
{
unsigned i_input_nb = aout_FormatNbChannels( &p_filter->fmt_in.audio );
unsigned i_output_nb = aout_FormatNbChannels( &p_filter->fmt_out.audio );
assert( i_input_nb >= i_output_nb );
filter_sys_t *p_sys = p_filter->p_sys;
float *p_dest = (float *)p_buf->p_buffer;
const float *p_src = p_dest;
const int *channel_map = p_sys->channel_map;
float buffer[i_output_nb];
for( size_t i = 0; i < p_buf->i_nb_samples; i++ )
{
for( unsigned j = 0; j < i_output_nb; j++ )
buffer[j] = channel_map[j] == -1 ? 0.f : p_src[channel_map[j]];
memcpy( p_dest, buffer, i_output_nb * sizeof(float) );
p_src += i_input_nb;
p_dest += i_output_nb;
}
p_buf->i_buffer = p_buf->i_buffer * i_output_nb / i_input_nb;
return p_buf;
}
static block_t *Equals( filter_t *p_filter, block_t *p_buf )
{
(void) p_filter;
return p_buf;
}
static block_t *Extract( filter_t *p_filter, block_t *p_in_buf )
{
size_t i_out_channels = aout_FormatNbChannels( &p_filter->fmt_out.audio );
size_t i_out_size = p_in_buf->i_nb_samples
* p_filter->fmt_out.audio.i_bitspersample
* i_out_channels / 8;
block_t *p_out_buf = block_Alloc( i_out_size );
if( unlikely(p_out_buf == NULL) )
{
block_Release( p_in_buf );
return NULL;
}
p_out_buf->i_nb_samples = p_in_buf->i_nb_samples;
p_out_buf->i_dts = p_in_buf->i_dts;
p_out_buf->i_pts = p_in_buf->i_pts;
p_out_buf->i_length = p_in_buf->i_length;
static const int pi_selections[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8,
};
static_assert(sizeof(pi_selections)/sizeof(int) == AOUT_CHAN_MAX,
"channel max size mismatch!");
aout_ChannelExtract( p_out_buf->p_buffer, i_out_channels,
p_in_buf->p_buffer, p_filter->fmt_in.audio.i_channels,
p_in_buf->i_nb_samples, pi_selections,
p_filter->fmt_out.audio.i_bitspersample );
block_Release( p_in_buf );
return p_out_buf;
}
static int Create( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
const audio_format_t *infmt = &p_filter->fmt_in.audio;
const audio_format_t *outfmt = &p_filter->fmt_out.audio;
if( infmt->i_physical_channels == 0 )
{
assert( infmt->i_channels > 0 );
if( outfmt->i_physical_channels == 0 )
return VLC_EGENERIC;
if( aout_FormatNbChannels( outfmt ) == infmt->i_channels )
{
p_filter->pf_audio_filter = Equals;
return VLC_SUCCESS;
}
else
{
if( infmt->i_channels > AOUT_CHAN_MAX )
msg_Info(p_filter, "%d channels will be dropped.",
infmt->i_channels - AOUT_CHAN_MAX);
p_filter->pf_audio_filter = Extract;
return VLC_SUCCESS;
}
}
if( infmt->i_format != outfmt->i_format
|| infmt->i_rate != outfmt->i_rate
|| infmt->i_format != VLC_CODEC_FL32 )
return VLC_EGENERIC;
if( infmt->i_physical_channels == outfmt->i_physical_channels
&& infmt->i_chan_mode == outfmt->i_chan_mode )
return VLC_EGENERIC;
p_filter->p_sys = NULL;
if ( aout_FormatNbChannels( outfmt ) == 1
&& aout_FormatNbChannels( infmt ) == 1 )
{
p_filter->pf_audio_filter = Equals;
return VLC_SUCCESS;
}
uint16_t i_in_physical_channels = infmt->i_physical_channels;
uint16_t i_out_physical_channels = outfmt->i_physical_channels;
int i_src_idx = 0;
int src_chans[AOUT_CHAN_MAX];
for( unsigned i = 0; i < AOUT_CHAN_MAX; ++i )
src_chans[i] = pi_vlc_chan_order_wg4[i] & i_in_physical_channels ?
i_src_idx++ : -1;
unsigned i_dst_idx = 0;
int channel_map[AOUT_CHAN_MAX];
for( unsigned i = 0; i < AOUT_CHAN_MAX; ++i )
{
const uint32_t i_chan = pi_vlc_chan_order_wg4[i];
if( !( i_chan & i_out_physical_channels ) )
continue; 
if( aout_FormatNbChannels( infmt ) == 1 )
{
if( i_chan & AOUT_CHANS_FRONT )
channel_map[i_dst_idx] = 0;
else
channel_map[i_dst_idx] = -1;
}
else if( src_chans[i] != -1 )
{
assert( i_chan & i_in_physical_channels );
channel_map[i_dst_idx] = src_chans[i];
}
else
{
if( ( i_chan & AOUT_CHANS_MIDDLE )
&& !( i_out_physical_channels & AOUT_CHANS_REAR ) )
{
assert( i + 2 < AOUT_CHAN_MAX );
assert( pi_vlc_chan_order_wg4[i + 2] & AOUT_CHANS_REAR );
channel_map[i_dst_idx] = src_chans[i + 2];
}
else if( ( i_chan & AOUT_CHANS_REAR )
&& !( i_out_physical_channels & AOUT_CHANS_MIDDLE ) )
{
assert( (int) i - 2 >= 0 );
assert( pi_vlc_chan_order_wg4[i - 2] & AOUT_CHANS_MIDDLE );
channel_map[i_dst_idx] = src_chans[i - 2];
}
else
channel_map[i_dst_idx] = -1;
}
i_dst_idx++;
}
#if !defined(NDEBUG)
for( unsigned i = 0; i < aout_FormatNbChannels( outfmt ); ++i )
{
assert( channel_map[i] == -1
|| (unsigned) channel_map[i] < aout_FormatNbChannels( infmt ) );
}
#endif
if( aout_FormatNbChannels( outfmt ) == aout_FormatNbChannels( infmt ) )
{
bool b_equals = true;
for( unsigned i = 0; i < aout_FormatNbChannels( outfmt ); ++i )
if( channel_map[i] == -1 || (unsigned) channel_map[i] != i )
{
b_equals = false;
break;
}
if( b_equals )
{
p_filter->pf_audio_filter = Equals;
return VLC_SUCCESS;
}
}
filter_sys_t *p_sys = malloc( sizeof(*p_sys) );
if( !p_sys )
return VLC_ENOMEM;
p_filter->p_sys = p_sys;
memcpy( p_sys->channel_map, channel_map, sizeof(channel_map) );
if( aout_FormatNbChannels( outfmt ) > aout_FormatNbChannels( infmt ) )
p_filter->pf_audio_filter = Upmix;
else
p_filter->pf_audio_filter = Downmix;
return VLC_SUCCESS;
}
static void Destroy( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
free( p_filter->p_sys );
}
