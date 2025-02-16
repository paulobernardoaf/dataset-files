

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_rand.h>
#include <vlc_tick.h>

#include "filter_picture.h"

static inline int64_t MOD(int64_t a, int64_t b) {
return ( ( a % b ) + b ) % b; }

#define MAX_BLUE_RED_LINES 100

typedef struct {
int32_t i_offset;
uint16_t i_intensity;
bool b_blue_red;
vlc_tick_t i_stop_trigger;
} blue_red_line_t;

typedef struct
{


bool b_init;
int32_t i_planes;
int32_t *i_height; 
int32_t *i_width;
int32_t *i_visible_pitch;
vlc_tick_t i_start_time;
vlc_tick_t i_last_time;
vlc_tick_t i_cur_time;


int32_t i_phase_speed;
int32_t i_phase_ofs;
int32_t i_offset_ofs;
int32_t i_sliding_ofs;
int32_t i_sliding_speed;
vlc_tick_t i_offset_trigger;
vlc_tick_t i_sliding_trigger;
vlc_tick_t i_sliding_stop_trig;
bool i_sliding_type_duplicate;


vlc_tick_t i_BR_line_trigger;
blue_red_line_t *p_BR_lines[MAX_BLUE_RED_LINES];

} filter_sys_t;





static picture_t *Filter( filter_t *, picture_t * );

static int vhs_allocate_data( filter_t *, picture_t * );
static void vhs_free_allocated_data( filter_t * );

static int vhs_blue_red_line_effect( filter_t *, picture_t * );
static void vhs_blue_red_dots_effect( filter_t *, picture_t * );
static int vhs_sliding_effect( filter_t *, picture_t * );

static int vhs_sliding_effect_apply( filter_t *, picture_t * );





static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );

vlc_module_begin()
set_description( N_("VHS movie effect video filter") )
set_shortname( N_("VHS movie" ) )
set_capability( "video filter", 0 )
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )

set_callbacks( Open, Close )
vlc_module_end()




static int Open( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t*)p_this;
filter_sys_t *p_sys;


if( !es_format_IsSimilar( &p_filter->fmt_in, &p_filter->fmt_out ) ) {
msg_Err( p_filter, "Input and output format does not match" );
return VLC_EGENERIC;
}


const vlc_fourcc_t fourcc = p_filter->fmt_in.video.i_chroma;
const vlc_chroma_description_t *p_chroma =
vlc_fourcc_GetChromaDescription( p_filter->fmt_in.video.i_chroma );
if( !p_chroma || p_chroma->pixel_size == 0
|| p_chroma->plane_count < 3 || p_chroma->pixel_size > 1
|| !vlc_fourcc_IsYUV( fourcc ) )
{
msg_Err( p_filter, "Unsupported chroma (%4.4s)", (char*)&fourcc );
return VLC_EGENERIC;
}


p_filter->p_sys = p_sys = calloc(1, sizeof(*p_sys) );
if( unlikely( !p_sys ) )
return VLC_ENOMEM;


p_filter->pf_video_filter = Filter;
p_sys->i_start_time = p_sys->i_cur_time = p_sys->i_last_time = vlc_tick_now();

return VLC_SUCCESS;
}




static void Close( vlc_object_t *p_this ) {
filter_t *p_filter = (filter_t*)p_this;
filter_sys_t *p_sys = p_filter->p_sys;


vhs_free_allocated_data( p_filter );
free( p_sys );
}




static picture_t *Filter( filter_t *p_filter, picture_t *p_pic_in ) {
if( unlikely( !p_pic_in || !p_filter) )
return NULL;

filter_sys_t *p_sys = p_filter->p_sys;

picture_t *p_pic_out = filter_NewPicture( p_filter );
if( unlikely( !p_pic_out ) ) {
picture_Release( p_pic_in );
return NULL;
}




p_sys->i_last_time = p_sys->i_cur_time;
p_sys->i_cur_time = vlc_tick_now();




if ( unlikely( !p_sys->b_init ) )
if ( unlikely( vhs_allocate_data( p_filter, p_pic_in ) != VLC_SUCCESS ) ) {
picture_Release( p_pic_in );
return NULL;
}
p_sys->b_init = true;




picture_CopyPixels(p_pic_out, p_pic_in);




if ( unlikely( vhs_blue_red_line_effect( p_filter, p_pic_out ) != VLC_SUCCESS ) )
return CopyInfoAndRelease( p_pic_out, p_pic_in );

if ( unlikely( vhs_sliding_effect(p_filter, p_pic_out ) != VLC_SUCCESS ) )
return CopyInfoAndRelease( p_pic_out, p_pic_in );

vhs_blue_red_dots_effect( p_filter, p_pic_out );

return CopyInfoAndRelease( p_pic_out, p_pic_in );
}




static int vhs_allocate_data( filter_t *p_filter, picture_t *p_pic_in ) {
filter_sys_t *p_sys = p_filter->p_sys;

vhs_free_allocated_data( p_filter );




p_sys->i_planes = p_pic_in->i_planes;
p_sys->i_height = calloc( p_sys->i_planes, sizeof(int32_t) );
p_sys->i_width = calloc( p_sys->i_planes, sizeof(int32_t) );
p_sys->i_visible_pitch = calloc( p_sys->i_planes, sizeof(int32_t) );

if( unlikely( !p_sys->i_height || !p_sys->i_width || !p_sys->i_visible_pitch ) ) {
vhs_free_allocated_data( p_filter );
return VLC_ENOMEM;
}

for ( int32_t i_p = 0; i_p < p_sys->i_planes; i_p++) {
p_sys->i_visible_pitch [i_p] = (int) p_pic_in->p[i_p].i_visible_pitch;
p_sys->i_height[i_p] = (int) p_pic_in->p[i_p].i_visible_lines;
p_sys->i_width [i_p] = (int) p_pic_in->p[i_p].i_visible_pitch / p_pic_in->p[i_p].i_pixel_pitch;
}
return VLC_SUCCESS;
}




static void vhs_free_allocated_data( filter_t *p_filter ) {
filter_sys_t *p_sys = p_filter->p_sys;

for ( uint32_t i_b = 0; i_b < MAX_BLUE_RED_LINES; i_b++ )
FREENULL( p_sys->p_BR_lines[i_b] );

p_sys->i_planes = 0;
FREENULL( p_sys->i_height );
FREENULL( p_sys->i_width );
FREENULL( p_sys->i_visible_pitch );
}

static vlc_tick_t RandomEnd(filter_sys_t *p_sys, vlc_tick_t modulo)
{
return p_sys->i_cur_time + (uint64_t)vlc_mrand48() % modulo + modulo / 2;
}




static int vhs_blue_red_line_effect( filter_t *p_filter, picture_t *p_pic_out ) {
filter_sys_t *p_sys = p_filter->p_sys;

#define BR_LINES_GENERATOR_PERIOD VLC_TICK_FROM_SEC(50)
#define BR_LINES_DURATION VLC_TICK_FROM_MS(20)


if ( p_sys->i_BR_line_trigger <= p_sys->i_cur_time ) {
for ( uint32_t i_b = 0; i_b < MAX_BLUE_RED_LINES; i_b++ )
if (p_sys->p_BR_lines[i_b] == NULL) {

p_sys->p_BR_lines[i_b] = calloc( 1, sizeof(blue_red_line_t) );
if ( unlikely( !p_sys->p_BR_lines[i_b] ) )
return VLC_ENOMEM;


p_sys->p_BR_lines[i_b]->i_offset = (unsigned)vlc_mrand48()
% __MAX( 1, p_sys->i_height[Y_PLANE] - 10 )
+ 5;

p_sys->p_BR_lines[i_b]->b_blue_red = (unsigned)vlc_mrand48() & 0x01;

p_sys->p_BR_lines[i_b]->i_stop_trigger = RandomEnd( p_sys, BR_LINES_DURATION );

break;
}
p_sys->i_BR_line_trigger = RandomEnd( p_sys, BR_LINES_GENERATOR_PERIOD );
}



for ( uint8_t i_b = 0; i_b < MAX_BLUE_RED_LINES; i_b++ )
if ( p_sys->p_BR_lines[i_b] ) {

if ( p_sys->p_BR_lines[i_b]->i_stop_trigger <= p_sys->i_cur_time ) {
FREENULL( p_sys->p_BR_lines[i_b] );
continue;
}


for ( int32_t i_p=0; i_p < p_sys->i_planes; i_p++ ) {
uint32_t i_pix_ofs = p_sys->p_BR_lines[i_b]->i_offset
* p_pic_out->p[i_p].i_visible_lines
/ p_sys->i_height[Y_PLANE]
* p_pic_out->p[i_p].i_pitch;

switch ( i_p ) {
case Y_PLANE:
memset( &p_pic_out->p[i_p].p_pixels[i_pix_ofs], 127,
p_pic_out->p[i_p].i_visible_pitch);
break;
case U_PLANE:
memset( &p_pic_out->p[i_p].p_pixels[i_pix_ofs],
(p_sys->p_BR_lines[i_b]->b_blue_red?255:0),
p_pic_out->p[i_p].i_visible_pitch);
break;
case V_PLANE:
memset( &p_pic_out->p[i_p].p_pixels[i_pix_ofs],
(p_sys->p_BR_lines[i_b]->b_blue_red?0:255),
p_pic_out->p[i_p].i_visible_pitch);
break;
}

}
}
return VLC_SUCCESS;
}




static void vhs_blue_red_dots_effect( filter_t *p_filter, picture_t *p_pic_out ) {
#define BR_DOTS_RATIO 10000

filter_sys_t *p_sys = p_filter->p_sys;

for ( int32_t i_dots = 0;
i_dots < p_sys->i_width[Y_PLANE] * p_sys->i_height[Y_PLANE] / BR_DOTS_RATIO;
i_dots++) {

uint32_t i_length = (unsigned)vlc_mrand48()
% __MAX( 1, p_sys->i_width[Y_PLANE] / 30 ) + 1;

uint16_t i_x = (unsigned)vlc_mrand48()
% __MAX( 1, p_sys->i_width[Y_PLANE] - i_length );
uint16_t i_y = (unsigned)vlc_mrand48() % p_sys->i_height[Y_PLANE];
bool b_color = ( ( (unsigned)vlc_mrand48() % 2 ) == 0);

for ( int32_t i_p = 0; i_p < p_sys->i_planes; i_p++ ) {
uint32_t i_pix_ofs = i_y
* p_pic_out->p[i_p].i_visible_lines
/ p_sys->i_height[Y_PLANE]
* p_pic_out->p[i_p].i_pitch
+ i_x
* p_pic_out->p[i_p].i_pixel_pitch;

uint32_t i_length_in_plane = i_length
* p_pic_out->p[i_p].i_visible_pitch
/ p_pic_out->p[Y_PLANE].i_visible_pitch;

switch ( i_p ) {
case Y_PLANE:
memset( &p_pic_out->p[i_p].p_pixels[i_pix_ofs], 127,
i_length_in_plane );
break;
case U_PLANE:
memset( &p_pic_out->p[i_p].p_pixels[i_pix_ofs],
(b_color?255:0),
i_length_in_plane );
break;
case V_PLANE:
memset( &p_pic_out->p[i_p].p_pixels[i_pix_ofs],
(b_color?0:255),
i_length_in_plane );
break;
}
}
}
}




static int vhs_sliding_effect( filter_t *p_filter, picture_t *p_pic_out ) {
filter_sys_t *p_sys = p_filter->p_sys;





#define OFFSET_AVERAGE_PERIOD VLC_TICK_FROM_SEC(10)


if ( p_sys->i_offset_trigger == 0
|| p_sys->i_sliding_speed != 0 ) { 


p_sys->i_offset_trigger = RandomEnd( p_sys, OFFSET_AVERAGE_PERIOD );
p_sys->i_offset_ofs = 0;
} else if (p_sys->i_offset_trigger <= p_sys->i_cur_time) {

p_sys->i_offset_trigger = 0;
p_sys->i_offset_ofs = (uint32_t)vlc_mrand48()
% p_sys->i_height[Y_PLANE];
}
else
p_sys->i_offset_ofs = 0;






#define MAX_PHASE_OFS (p_sys->i_height[Y_PLANE]*100/15)

p_sys->i_phase_speed += MOD( (int32_t)vlc_mrand48(), 3) - 1;
p_sys->i_phase_ofs += p_sys->i_phase_speed;
p_sys->i_phase_ofs = VLC_CLIP( p_sys->i_phase_ofs, -MAX_PHASE_OFS, +MAX_PHASE_OFS);
if ( abs( p_sys->i_phase_ofs ) >= MAX_PHASE_OFS )
p_sys->i_phase_speed = 0;






#define SLIDING_AVERAGE_PERIOD VLC_TICK_FROM_SEC(20)
#define SLIDING_AVERAGE_DURATION VLC_TICK_FROM_SEC(3)


if ( ( p_sys->i_sliding_stop_trig == 0 ) &&
( p_sys->i_sliding_trigger == 0 ) &&
( p_sys->i_sliding_speed == 0 ) ) {


p_sys->i_sliding_trigger = RandomEnd( p_sys, SLIDING_AVERAGE_PERIOD );
}


else if ( ( p_sys->i_sliding_stop_trig == 0 ) &&
( p_sys->i_sliding_trigger <= p_sys->i_cur_time ) &&
( p_sys->i_sliding_speed == 0 ) ) {


p_sys->i_sliding_trigger = 0;
p_sys->i_sliding_stop_trig = RandomEnd( p_sys, SLIDING_AVERAGE_DURATION );
p_sys->i_sliding_ofs = 0;

p_sys->i_sliding_speed = MOD( (int32_t)vlc_mrand48(), 1001 ) - 500;
p_sys->i_sliding_type_duplicate = (unsigned)vlc_mrand48() & 0x01;
}


else if ( ( p_sys->i_sliding_stop_trig <= p_sys->i_cur_time )
&& ( p_sys->i_sliding_trigger == 0 ) ) {


if ( abs( p_sys->i_sliding_speed ) < 5 )
p_sys->i_sliding_speed += 1;


if ( abs( p_sys->i_sliding_ofs ) < abs( p_sys->i_sliding_speed
* p_sys->i_height[Y_PLANE]
* SEC_FROM_VLC_TICK( p_sys->i_cur_time - p_sys->i_last_time ) )
|| abs( p_sys->i_sliding_ofs ) < p_sys->i_height[Y_PLANE] * 100 / 20 ) {


p_sys->i_sliding_ofs = p_sys->i_sliding_speed = 0;
p_sys->i_sliding_trigger = p_sys->i_sliding_stop_trig = 0;
p_sys->i_sliding_type_duplicate = false;
}
}


p_sys->i_sliding_ofs = MOD( p_sys->i_sliding_ofs
+ p_sys->i_sliding_speed * p_sys->i_height[Y_PLANE]
* SEC_FROM_VLC_TICK( p_sys->i_cur_time - p_sys->i_last_time),
p_sys->i_height[Y_PLANE] * 100 );

return vhs_sliding_effect_apply( p_filter, p_pic_out );
}




static int vhs_sliding_effect_apply( filter_t *p_filter, picture_t *p_pic_out )
{
filter_sys_t *p_sys = p_filter->p_sys;

for ( uint8_t i_p = 0; i_p < p_pic_out->i_planes; i_p++ ) {

uint8_t *p_temp_buf;
if ( !p_sys->i_sliding_type_duplicate ) {
p_temp_buf= calloc( p_pic_out->p[i_p].i_lines
* p_pic_out->p[i_p].i_pitch, sizeof(uint8_t) );
if ( unlikely( !p_temp_buf ) )
return VLC_ENOMEM;
memcpy( p_temp_buf, p_pic_out->p[i_p].p_pixels,
p_pic_out->p[i_p].i_lines * p_pic_out->p[i_p].i_pitch );
}
else
p_temp_buf = p_pic_out->p[i_p].p_pixels;


for ( int32_t i_y = 0; i_y < p_pic_out->p[i_p].i_visible_lines; i_y++ )
{
int32_t i_ofs = p_sys->i_offset_ofs + p_sys->i_sliding_ofs;

if ( ( p_sys->i_sliding_speed == 0 ) || !p_sys->i_sliding_type_duplicate )
i_ofs += p_sys->i_phase_ofs;

i_ofs = MOD( i_ofs / 100, p_sys->i_height[Y_PLANE] );
i_ofs *= p_pic_out->p[i_p].i_visible_lines;
i_ofs /= p_sys->i_height[Y_PLANE];

memcpy( &p_pic_out->p[i_p].p_pixels[ i_y * p_pic_out->p[i_p].i_pitch ],
&p_temp_buf[ ( ( i_y + i_ofs ) % p_pic_out->p[i_p].i_visible_lines ) * p_pic_out->p[i_p].i_pitch ],
p_pic_out->p[i_p].i_visible_pitch );
}
if ( !p_sys->i_sliding_type_duplicate )
free(p_temp_buf);
}

return VLC_SUCCESS;
}
