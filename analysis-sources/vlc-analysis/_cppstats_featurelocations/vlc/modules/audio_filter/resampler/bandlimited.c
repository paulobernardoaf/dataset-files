

































#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_filter.h>
#include <vlc_block.h>

#include <assert.h>

#include "bandlimited.h"






static int OpenFilter ( vlc_object_t * );
static void CloseFilter( vlc_object_t * );
static block_t *Resample( filter_t *, block_t * );

static void ResampleFloat( filter_t *p_filter,
block_t **pp_out_buf, size_t *pi_out,
float **pp_in,
int i_in, int i_in_end,
double d_factor, bool b_factor_old,
int i_nb_channels, int i_bytes_per_frame );




typedef struct
{
int32_t *p_buf; 
size_t i_buf_size;

double d_old_factor;
size_t i_old_wing;

unsigned int i_remainder; 
bool b_first;

date_t end_date;
} filter_sys_t;




vlc_module_begin ()
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_RESAMPLER )
set_description( N_("Audio filter for band-limited interpolation resampling") )
set_capability( "audio converter", 20 )
set_callbacks( OpenFilter, CloseFilter )

add_submodule()
set_capability( "audio resampler", 20 )
set_callbacks( OpenFilter, CloseFilter )
vlc_module_end ()




static block_t *Resample( filter_t * p_filter, block_t * p_in_buf )
{
if( !p_in_buf || !p_in_buf->i_nb_samples )
{
if( p_in_buf )
block_Release( p_in_buf );
return NULL;
}

filter_sys_t *p_sys = p_filter->p_sys;
unsigned int i_out_rate = p_filter->fmt_out.audio.i_rate;
int i_nb_channels = p_filter->fmt_in.audio.i_channels;


if( i_out_rate == p_filter->fmt_in.audio.i_rate )
{
if( !(p_in_buf->i_flags & BLOCK_FLAG_DISCONTINUITY) &&
p_sys->i_old_wing )
{

p_in_buf = block_Realloc( p_in_buf,
p_sys->i_old_wing * p_filter->fmt_in.audio.i_bytes_per_frame,
p_in_buf->i_buffer );
if( !p_in_buf )
return NULL;
memcpy( p_in_buf->p_buffer, p_sys->p_buf +
i_nb_channels * p_sys->i_old_wing,
p_sys->i_old_wing *
p_filter->fmt_in.audio.i_bytes_per_frame );

p_in_buf->i_nb_samples += p_sys->i_old_wing;

p_in_buf->i_pts = date_Get( &p_sys->end_date );
p_in_buf->i_length =
date_Increment( &p_sys->end_date,
p_in_buf->i_nb_samples ) - p_in_buf->i_pts;
}
p_sys->i_old_wing = 0;
p_sys->b_first = true;
return p_in_buf;
}

unsigned i_bytes_per_frame = p_filter->fmt_out.audio.i_channels *
p_filter->fmt_out.audio.i_bitspersample / 8;
size_t i_out_size = i_bytes_per_frame * ( 1 + ( p_in_buf->i_nb_samples *
p_filter->fmt_out.audio.i_rate / p_filter->fmt_in.audio.i_rate) )
+ p_filter->p_sys->i_buf_size;
block_t *p_out_buf = block_Alloc( i_out_size );
if( !p_out_buf )
{
block_Release( p_in_buf );
return NULL;
}

if( (p_in_buf->i_flags & BLOCK_FLAG_DISCONTINUITY) || p_sys->b_first )
{


p_out_buf->i_flags |= BLOCK_FLAG_DISCONTINUITY;
p_sys->i_remainder = 0;
date_Init( &p_sys->end_date, i_out_rate, 1 );
date_Set( &p_sys->end_date, p_in_buf->i_pts );
p_sys->d_old_factor = 1;
p_sys->i_old_wing = 0;
p_sys->b_first = false;
}

size_t i_in_nb = p_in_buf->i_nb_samples;
size_t i_in, i_out = 0;
double d_factor, d_scale_factor, d_old_scale_factor;
size_t i_filter_wing;

#if 0
msg_Err( p_filter, "old rate: %i, old factor: %f, old wing: %i, i_in: %i",
p_sys->i_old_rate, p_sys->d_old_factor,
p_sys->i_old_wing, i_in_nb );
#endif


assert( p_filter->fmt_in.audio.i_bytes_per_frame == i_bytes_per_frame );


if( p_sys->i_old_wing )
{ 


p_in_buf = block_Realloc( p_in_buf,
p_sys->i_old_wing * 2 * i_bytes_per_frame,
p_in_buf->i_buffer );
if( unlikely(p_in_buf == NULL) )
return NULL;
memcpy( p_in_buf->p_buffer, p_sys->p_buf,
p_sys->i_old_wing * 2 * i_bytes_per_frame );
}
i_in_nb += (p_sys->i_old_wing * 2);
float *p_in = (float *)p_in_buf->p_buffer;
const float *p_in_orig = p_in;


memset( p_out_buf->p_buffer, 0, p_out_buf->i_buffer );


d_factor = (double)i_out_rate / p_filter->fmt_in.audio.i_rate;
i_filter_wing = ((SMALL_FILTER_NMULT+1)/2.0) * __MAX(1.0,1.0/d_factor) + 1;


d_old_scale_factor = SMALL_FILTER_SCALE *
p_sys->d_old_factor + 0.5;
d_scale_factor = SMALL_FILTER_SCALE * d_factor + 0.5;


i_in = p_sys->i_old_wing;
p_in += p_sys->i_old_wing * i_nb_channels;

size_t i_old_in_end = 0;
if( p_sys->i_old_wing <= i_in_nb )
i_old_in_end = __MIN( i_filter_wing, i_in_nb - p_sys->i_old_wing );

ResampleFloat( p_filter,
&p_out_buf, &i_out, &p_in,
i_in, i_old_in_end,
p_sys->d_old_factor, true,
i_nb_channels, i_bytes_per_frame );
i_in = __MAX( i_in, i_old_in_end );


if( i_in < i_in_nb - i_filter_wing )
{
p_sys->d_old_factor = d_factor;
p_sys->i_old_wing = i_filter_wing;
}
if( p_out_buf )
{
ResampleFloat( p_filter,
&p_out_buf, &i_out, &p_in,
i_in, i_in_nb - i_filter_wing,
d_factor, false,
i_nb_channels, i_bytes_per_frame );


p_out_buf->i_nb_samples = i_out;
p_out_buf->i_dts =
p_out_buf->i_pts = date_Get( &p_sys->end_date );
p_out_buf->i_length = date_Increment( &p_sys->end_date,
p_out_buf->i_nb_samples ) - p_out_buf->i_pts;

p_out_buf->i_buffer = p_out_buf->i_nb_samples *
i_nb_channels * sizeof(int32_t);
}


if( p_sys->i_old_wing )
{
size_t newsize = p_sys->i_old_wing * 2 * i_bytes_per_frame;
if( newsize > p_sys->i_buf_size )
{
free( p_sys->p_buf );
p_sys->p_buf = malloc( newsize );
if( p_sys->p_buf != NULL )
p_sys->i_buf_size = newsize;
else
{
p_sys->i_buf_size = p_sys->i_old_wing = 0; 
block_Release( p_in_buf );
return p_out_buf;
}
}
memcpy( p_sys->p_buf,
p_in_orig + (i_in_nb - 2 * p_sys->i_old_wing) *
i_nb_channels, (2 * p_sys->i_old_wing) *
p_filter->fmt_in.audio.i_bytes_per_frame );
}

#if 0
msg_Err( p_filter, "p_out size: %i, nb bytes out: %i", p_out_buf->i_buffer,
i_out * p_filter->fmt_in.audio.i_bytes_per_frame );
#endif

block_Release( p_in_buf );
return p_out_buf;
}




static int OpenFilter( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys;
unsigned int i_out_rate = p_filter->fmt_out.audio.i_rate;

if ( p_filter->fmt_in.audio.i_rate == p_filter->fmt_out.audio.i_rate
|| p_filter->fmt_in.audio.i_format != p_filter->fmt_out.audio.i_format
|| p_filter->fmt_in.audio.i_channels != p_filter->fmt_out.audio.i_channels
|| p_filter->fmt_in.audio.i_format != VLC_CODEC_FL32 )
{
return VLC_EGENERIC;
}


p_filter->p_sys = p_sys = malloc( sizeof(struct filter_sys_t) );
if( p_sys == NULL )
return VLC_ENOMEM;

p_sys->p_buf = NULL;
p_sys->i_buf_size = 0;

p_sys->i_old_wing = 0;
p_sys->b_first = true;
p_filter->pf_audio_filter = Resample;

msg_Dbg( p_this, "%4.4s/%iKHz/%i->%4.4s/%iKHz/%i",
(char *)&p_filter->fmt_in.i_codec,
p_filter->fmt_in.audio.i_rate,
p_filter->fmt_in.audio.i_channels,
(char *)&p_filter->fmt_out.i_codec,
p_filter->fmt_out.audio.i_rate,
p_filter->fmt_out.audio.i_channels);

p_filter->fmt_out = p_filter->fmt_in;
p_filter->fmt_out.audio.i_rate = i_out_rate;

return 0;
}




static void CloseFilter( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
free( p_filter->p_sys->p_buf );
free( p_filter->p_sys );
}

static void FilterFloatUP( const float Imp[], const float ImpD[], uint16_t Nwing, float *p_in,
float *p_out, uint32_t ui_remainder,
uint32_t ui_output_rate, int16_t Inc, int i_nb_channels )
{
const float *Hp, *Hdp, *End;
float t, temp;
uint32_t ui_linear_remainder;
int i;

Hp = &Imp[(ui_remainder<<Nhc)/ui_output_rate];
Hdp = &ImpD[(ui_remainder<<Nhc)/ui_output_rate];

End = &Imp[Nwing];

ui_linear_remainder = (ui_remainder<<Nhc) -
(ui_remainder<<Nhc)/ui_output_rate*ui_output_rate;

if (Inc == 1) 
{ 
End--; 
if (ui_remainder == 0) 
{ 
Hp += Npc; 
Hdp += Npc; 
}
}

while (Hp < End) {
t = *Hp; 

t += *Hdp * ui_linear_remainder / ui_output_rate / Npc;
for( i = 0; i < i_nb_channels; i++ )
{
temp = t;
temp *= *(p_in+i); 
*(p_out+i) += temp; 
}
Hdp += Npc; 
Hp += Npc; 
p_in += (Inc * i_nb_channels); 
}
}

static void FilterFloatUD( const float Imp[], const float ImpD[], uint16_t Nwing, float *p_in,
float *p_out, uint32_t ui_remainder,
uint32_t ui_output_rate, uint32_t ui_input_rate,
int16_t Inc, int i_nb_channels )
{
const float *Hp, *Hdp, *End;
float t, temp;
uint32_t ui_linear_remainder;
int i, ui_counter = 0;

Hp = Imp + (ui_remainder<<Nhc) / ui_input_rate;
Hdp = ImpD + (ui_remainder<<Nhc) / ui_input_rate;

End = &Imp[Nwing];

if (Inc == 1) 
{ 
End--; 
if (ui_remainder == 0) 
{ 
Hp = Imp + 
(ui_output_rate << Nhc) / ui_input_rate;
Hdp = ImpD + 
(ui_output_rate << Nhc) / ui_input_rate;
ui_counter++;
}
}

while (Hp < End) {
t = *Hp; 

ui_linear_remainder =
((ui_output_rate * ui_counter + ui_remainder)<< Nhc) -
((ui_output_rate * ui_counter + ui_remainder)<< Nhc) /
ui_input_rate * ui_input_rate;
t += *Hdp * ui_linear_remainder / ui_input_rate / Npc;
for( i = 0; i < i_nb_channels; i++ )
{
temp = t;
temp *= *(p_in+i); 
*(p_out+i) += temp; 
}

ui_counter++;


Hp = Imp + ((ui_output_rate * ui_counter + ui_remainder)<< Nhc)
/ ui_input_rate;

Hdp = ImpD + ((ui_output_rate * ui_counter + ui_remainder)<< Nhc)
/ ui_input_rate;

p_in += (Inc * i_nb_channels); 
}
}

static int ReallocBuffer( block_t **pp_out_buf,
float **pp_out, size_t i_out,
int i_nb_channels, int i_bytes_per_frame )
{
if( i_out < (*pp_out_buf)->i_buffer/i_bytes_per_frame )
return VLC_SUCCESS;


const unsigned i_extra_frame = 256;
*pp_out_buf = block_Realloc( *pp_out_buf, 0,
(*pp_out_buf)->i_buffer +
i_extra_frame * i_bytes_per_frame );
if( !*pp_out_buf )
return VLC_EGENERIC;

*pp_out = (float*)(*pp_out_buf)->p_buffer + i_out * i_nb_channels;
memset( *pp_out, 0, i_extra_frame * i_bytes_per_frame );
return VLC_SUCCESS;
}

static void ResampleFloat( filter_t *p_filter,
block_t **pp_out_buf, size_t *pi_out,
float **pp_in,
int i_in, int i_in_end,
double d_factor, bool b_factor_old,
int i_nb_channels, int i_bytes_per_frame )
{
filter_sys_t *p_sys = p_filter->p_sys;

float *p_in = *pp_in;
size_t i_out = *pi_out;
float *p_out = (float*)(*pp_out_buf)->p_buffer + i_out * i_nb_channels;

for( ; i_in < i_in_end; i_in++ )
{
if( b_factor_old && d_factor == 1 )
{
if( ReallocBuffer( pp_out_buf, &p_out,
i_out, i_nb_channels, i_bytes_per_frame ) )
return;

memcpy( p_out, p_in, i_bytes_per_frame );
p_in += i_nb_channels;
p_out += i_nb_channels;
i_out++;
continue;
}

while( p_sys->i_remainder < p_filter->fmt_out.audio.i_rate )
{
if( ReallocBuffer( pp_out_buf, &p_out,
i_out, i_nb_channels, i_bytes_per_frame ) )
return;

if( d_factor >= 1 )
{



FilterFloatUP( SMALL_FILTER_FLOAT_IMP, SMALL_FILTER_FLOAT_IMPD,
SMALL_FILTER_NWING, p_in, p_out,
p_sys->i_remainder,
p_filter->fmt_out.audio.i_rate,
-1, i_nb_channels );

FilterFloatUP( SMALL_FILTER_FLOAT_IMP, SMALL_FILTER_FLOAT_IMPD,
SMALL_FILTER_NWING, p_in + i_nb_channels, p_out,
p_filter->fmt_out.audio.i_rate -
p_sys->i_remainder,
p_filter->fmt_out.audio.i_rate,
1, i_nb_channels );

#if 0

for( i = 0; i < i_nb_channels; i++ )
{
*(p_out+i) *= d_old_scale_factor;
}
#endif
}
else
{

FilterFloatUD( SMALL_FILTER_FLOAT_IMP, SMALL_FILTER_FLOAT_IMPD,
SMALL_FILTER_NWING, p_in, p_out,
p_sys->i_remainder,
p_filter->fmt_out.audio.i_rate, p_filter->fmt_in.audio.i_rate,
-1, i_nb_channels );

FilterFloatUD( SMALL_FILTER_FLOAT_IMP, SMALL_FILTER_FLOAT_IMPD,
SMALL_FILTER_NWING, p_in + i_nb_channels, p_out,
p_filter->fmt_out.audio.i_rate -
p_sys->i_remainder,
p_filter->fmt_out.audio.i_rate, p_filter->fmt_in.audio.i_rate,
1, i_nb_channels );
}

p_out += i_nb_channels;
i_out++;

p_sys->i_remainder += p_filter->fmt_in.audio.i_rate;
}

p_in += i_nb_channels;
p_sys->i_remainder -= p_filter->fmt_out.audio.i_rate;
}

*pp_in = p_in;
*pi_out = i_out;
}


