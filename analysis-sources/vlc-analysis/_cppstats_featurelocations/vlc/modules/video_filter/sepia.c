

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_cpu.h>

#include <stdatomic.h>
#include <assert.h>
#include "filter_picture.h"




static int Create ( vlc_object_t * );
static void Destroy ( vlc_object_t * );

static void RVSepia( picture_t *, picture_t *, int );
static void PlanarI420Sepia( picture_t *, picture_t *, int);
static void PackedYUVSepia( picture_t *, picture_t *, int);
static picture_t *Filter( filter_t *, picture_t * );
static const char *const ppsz_filter_options[] = {
"intensity", NULL
};




#define SEPIA_INTENSITY_TEXT N_("Sepia intensity")
#define SEPIA_INTENSITY_LONGTEXT N_("Intensity of sepia effect" )

#define CFG_PREFIX "sepia-"

vlc_module_begin ()
set_description( N_("Sepia video filter") )
set_shortname( N_("Sepia" ) )
set_help( N_("Gives video a warmer tone by applying sepia effect") )
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
set_capability( "video filter", 0 )
add_integer_with_range( CFG_PREFIX "intensity", 120, 0, 255,
SEPIA_INTENSITY_TEXT, SEPIA_INTENSITY_LONGTEXT,
false )
set_callbacks( Create, Destroy )
vlc_module_end ()




static int FilterCallback( vlc_object_t *, char const *,
vlc_value_t, vlc_value_t, void * );

typedef void (*SepiaFunction)( picture_t *, picture_t *, int );

static const struct
{
vlc_fourcc_t i_chroma;
SepiaFunction pf_sepia;
} p_sepia_cfg[] = {
{ VLC_CODEC_I420, PlanarI420Sepia },
{ VLC_CODEC_RGB24, RVSepia },
{ VLC_CODEC_RGB32, RVSepia },
{ VLC_CODEC_UYVY, PackedYUVSepia },
{ VLC_CODEC_VYUY, PackedYUVSepia },
{ VLC_CODEC_YUYV, PackedYUVSepia },
{ VLC_CODEC_YVYU, PackedYUVSepia },
{ 0, NULL }
};




typedef struct
{
SepiaFunction pf_sepia;
atomic_int i_intensity;
} filter_sys_t;






static int Create( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys;


p_sys = p_filter->p_sys = malloc( sizeof( filter_sys_t ) );
if( p_filter->p_sys == NULL )
return VLC_ENOMEM;

p_sys->pf_sepia = NULL;

for( int i = 0; p_sepia_cfg[i].i_chroma != 0; i++ )
{
if( p_sepia_cfg[i].i_chroma != p_filter->fmt_in.video.i_chroma )
continue;
p_sys->pf_sepia = p_sepia_cfg[i].pf_sepia;
}

if( p_sys->pf_sepia == NULL )
{
msg_Err( p_filter, "Unsupported input chroma (%4.4s)",
(char*)&(p_filter->fmt_in.video.i_chroma) );
free( p_sys );
return VLC_EGENERIC;
}

config_ChainParse( p_filter, CFG_PREFIX, ppsz_filter_options,
p_filter->p_cfg );
atomic_init( &p_sys->i_intensity,
var_CreateGetIntegerCommand( p_filter, CFG_PREFIX "intensity" ) );
var_AddCallback( p_filter, CFG_PREFIX "intensity", FilterCallback, NULL );

p_filter->pf_video_filter = Filter;

return VLC_SUCCESS;
}






static void Destroy( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;

var_DelCallback( p_filter, CFG_PREFIX "intensity", FilterCallback, NULL );

free( p_filter->p_sys );
}








static picture_t *Filter( filter_t *p_filter, picture_t *p_pic )
{
picture_t *p_outpic;

if( !p_pic ) return NULL;

filter_sys_t *p_sys = p_filter->p_sys;
int intensity = atomic_load( &p_sys->i_intensity );

p_outpic = filter_NewPicture( p_filter );
if( !p_outpic )
{
msg_Warn( p_filter, "can't get output picture" );
picture_Release( p_pic );
return NULL;
}

p_sys->pf_sepia( p_pic, p_outpic, intensity );

return CopyInfoAndRelease( p_outpic, p_pic );
}

#if defined(CAN_COMPILE_SSE2)







VLC_SSE
static inline void Sepia8ySSE2(uint8_t * dst, const uint8_t * src,
int i_intensity_shifted_pair)
{
__asm__ volatile (

"movq (%1), %%xmm1\n"
"punpcklbw %%xmm7, %%xmm1\n" 
"movdqa %%xmm1, %%xmm2\n" 
"movd %2, %%xmm3\n"
"pshufd $0, %%xmm3, %%xmm3\n"
"psrlw $2, %%xmm2\n" 
"psubusb %%xmm2, %%xmm1\n"
"paddusb %%xmm3, %%xmm1\n"
"packuswb %%xmm1, %%xmm1\n" 
"movq %%xmm1, (%0) \n"
:
:"r" (dst), "r"(src), "r"(i_intensity_shifted_pair)
:"memory", "xmm1", "xmm2", "xmm3");
}

VLC_SSE
static void PlanarI420SepiaSSE( picture_t *p_pic, picture_t *p_outpic,
int i_intensity )
{

const uint8_t filling_const_8u = 128 - i_intensity / 6;
const uint8_t filling_const_8v = 128 + i_intensity / 14;

int i_intensity_shifted_pair = 0x10001 * (((uint8_t) i_intensity) >> 2);

__asm__ volatile("pxor %%xmm7, %%xmm7\n" ::: "xmm7");


for (int y = 0; y < p_pic->p[Y_PLANE].i_visible_lines - 1; y += 2)
{
const int i_dy_line1_start = y * p_outpic->p[Y_PLANE].i_pitch;
const int i_dy_line2_start = (y + 1) * p_outpic->p[Y_PLANE].i_pitch;
const int i_du_line_start = (y / 2) * p_outpic->p[U_PLANE].i_pitch;
const int i_dv_line_start = (y / 2) * p_outpic->p[V_PLANE].i_pitch;
int x = 0;

for ( ; x < p_pic->p[Y_PLANE].i_visible_pitch - 15; x += 16 )
{

Sepia8ySSE2(&p_outpic->p[Y_PLANE].p_pixels[i_dy_line1_start + x],
&p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x],
i_intensity_shifted_pair );
Sepia8ySSE2(&p_outpic->p[Y_PLANE].p_pixels[i_dy_line2_start + x],
&p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x],
i_intensity_shifted_pair );
Sepia8ySSE2(&p_outpic->p[Y_PLANE].p_pixels[i_dy_line1_start + x + 8],
&p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x + 8],
i_intensity_shifted_pair );
Sepia8ySSE2(&p_outpic->p[Y_PLANE].p_pixels[i_dy_line2_start + x + 8],
&p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x + 8],
i_intensity_shifted_pair );

memset(&p_outpic->p[U_PLANE].p_pixels[i_du_line_start + (x / 2)],
filling_const_8u, 8 );
memset(&p_outpic->p[V_PLANE].p_pixels[i_dv_line_start + (x / 2)],
filling_const_8v, 8 );
}


for ( ; x < p_pic->p[Y_PLANE].i_visible_pitch - 1; x += 2 )
{

p_outpic->p[Y_PLANE].p_pixels[i_dy_line1_start + x] =
p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x] -
(p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x] >> 2) +
(i_intensity >> 2);
p_outpic->p[Y_PLANE].p_pixels[i_dy_line1_start + x + 1] =
p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x + 1] -
(p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x + 1] >> 2) +
(i_intensity >> 2);
p_outpic->p[Y_PLANE].p_pixels[i_dy_line2_start + x] =
p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x] -
(p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x] >> 2) +
(i_intensity >> 2);
p_outpic->p[Y_PLANE].p_pixels[i_dy_line2_start + x + 1] =
p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x + 1] -
(p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x + 1] >> 2) +
(i_intensity >> 2);

p_outpic->p[U_PLANE].p_pixels[i_du_line_start + (x / 2)] =
filling_const_8u;

p_outpic->p[V_PLANE].p_pixels[i_dv_line_start + (x / 2)] =
filling_const_8v;
}
}
}
#endif









static void PlanarI420Sepia( picture_t *p_pic, picture_t *p_outpic,
int i_intensity )
{
#if defined(CAN_COMPILE_SSE2)
if (vlc_CPU_SSE2())
return PlanarI420SepiaSSE( p_pic, p_outpic, i_intensity );
#endif


const uint8_t filling_const_8u = 128 - i_intensity / 6;
const uint8_t filling_const_8v = 128 + i_intensity / 14;


for( int y = 0; y < p_pic->p[Y_PLANE].i_visible_lines - 1; y += 2 )
{
const int i_dy_line1_start = y * p_outpic->p[Y_PLANE].i_pitch;
const int i_dy_line2_start = ( y + 1 ) * p_outpic->p[Y_PLANE].i_pitch;
const int i_du_line_start = (y/2) * p_outpic->p[U_PLANE].i_pitch;
const int i_dv_line_start = (y/2) * p_outpic->p[V_PLANE].i_pitch;

int i_picture_size_limit = p_pic->p[Y_PLANE].i_visible_pitch
< p_outpic->p[Y_PLANE].i_visible_pitch
? (p_pic->p[Y_PLANE].i_visible_pitch - 1) :
(p_outpic->p[Y_PLANE].i_visible_pitch - 1);

for( int x = 0; x < i_picture_size_limit; x += 2 )
{

p_outpic->p[Y_PLANE].p_pixels[i_dy_line1_start + x] =
p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x] -
(p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x] >> 2) +
(i_intensity >> 2);
p_outpic->p[Y_PLANE].p_pixels[i_dy_line1_start + x + 1] =
p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x + 1] -
(p_pic->p[Y_PLANE].p_pixels[i_dy_line1_start + x + 1] >> 2) +
(i_intensity >> 2);
p_outpic->p[Y_PLANE].p_pixels[i_dy_line2_start + x] =
p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x] -
(p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x] >> 2) +
(i_intensity >> 2);
p_outpic->p[Y_PLANE].p_pixels[i_dy_line2_start + x + 1] =
p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x + 1] -
(p_pic->p[Y_PLANE].p_pixels[i_dy_line2_start + x + 1] >> 2) +
(i_intensity >> 2);

p_outpic->p[U_PLANE].p_pixels[i_du_line_start + (x / 2)] =
filling_const_8u;

p_outpic->p[V_PLANE].p_pixels[i_dv_line_start + (x / 2)] =
filling_const_8v;
}
}
}








static void PackedYUVSepia( picture_t *p_pic, picture_t *p_outpic,
int i_intensity )
{
uint8_t *p_in, *p_in_end, *p_line_end, *p_out;
int i_yindex = 1, i_uindex = 2, i_vindex = 0;

GetPackedYuvOffsets( p_outpic->format.i_chroma,
&i_yindex, &i_uindex, &i_vindex );


const uint8_t filling_const_8u = 128 - i_intensity / 6;
const uint8_t filling_const_8v = 128 + i_intensity / 14;

p_in = p_pic->p[0].p_pixels;
p_in_end = p_in + p_pic->p[0].i_visible_lines
* p_pic->p[0].i_pitch;
p_out = p_outpic->p[0].p_pixels;

{
while( p_in < p_in_end )
{
p_line_end = p_in + p_pic->p[0].i_visible_pitch;
while( p_in < p_line_end )
{

p_out[i_yindex] =
p_in[i_yindex] - (p_in[i_yindex] >> 2) + (i_intensity >> 2);
p_out[i_yindex + 2] =
p_in[i_yindex + 2] - (p_in[i_yindex + 2] >> 2)
+ (i_intensity >> 2);
p_out[i_uindex] = filling_const_8u;
p_out[i_vindex] = filling_const_8v;
p_in += 4;
p_out += 4;
}
p_in += p_pic->p[0].i_pitch - p_pic->p[0].i_visible_pitch;
p_out += p_outpic->p[0].i_pitch
- p_outpic->p[0].i_visible_pitch;
}
}
}








static void RVSepia( picture_t *p_pic, picture_t *p_outpic, int i_intensity )
{
#define SCALEBITS 10
#define ONE_HALF (1 << (SCALEBITS - 1))
#define FIX(x) ((int) ((x) * (1<<SCALEBITS) + 0.5))
uint8_t *p_in, *p_in_end, *p_line_end, *p_out;
bool b_isRV32 = p_pic->format.i_chroma == VLC_CODEC_RGB32;
int i_rindex = 0, i_gindex = 1, i_bindex = 2;

GetPackedRgbIndexes( &p_outpic->format, &i_rindex, &i_gindex, &i_bindex );

p_in = p_pic->p[0].p_pixels;
p_in_end = p_in + p_pic->p[0].i_visible_lines
* p_pic->p[0].i_pitch;
p_out = p_outpic->p[0].p_pixels;



uint8_t r_intensity = (( FIX( 1.40200 * 255.0 / 224.0 ) * (i_intensity * 14)
+ ONE_HALF )) >> SCALEBITS;
uint8_t g_intensity = (( - FIX(0.34414*255.0/224.0) * ( - i_intensity / 6 )
- FIX( 0.71414 * 255.0 / 224.0) * ( i_intensity * 14 )
+ ONE_HALF )) >> SCALEBITS;
uint8_t b_intensity = (( FIX( 1.77200 * 255.0 / 224.0) * ( - i_intensity / 6 )
+ ONE_HALF )) >> SCALEBITS;

while (p_in < p_in_end)
{
p_line_end = p_in + p_pic->p[0].i_visible_pitch;
while (p_in < p_line_end)
{






uint8_t i_y = ((( 66 * p_in[i_rindex] + 129 * p_in[i_gindex] + 25
* p_in[i_bindex] + 128 ) >> 8 ) * FIX(255.0/219.0))
- (((( 66 * p_in[i_rindex] + 129 * p_in[i_gindex] + 25
* p_in[i_bindex] + 128 ) >> 8 )
* FIX( 255.0 / 219.0 )) >> 2 ) + ( i_intensity >> 2 );
p_out[i_rindex] = vlc_uint8(i_y + r_intensity);
p_out[i_gindex] = vlc_uint8(i_y + g_intensity);
p_out[i_bindex] = vlc_uint8(i_y + b_intensity);
p_in += 3;
p_out += 3;

if (b_isRV32) {

*p_out++ = *p_in++;
}
}

p_in += p_pic->p[0].i_pitch - p_pic->p[0].i_visible_pitch;
p_out += p_outpic->p[0].i_pitch
- p_outpic->p[0].i_visible_pitch;
}
#undef SCALEBITS
#undef ONE_HALF
#undef FIX
}

static int FilterCallback ( vlc_object_t *p_this, char const *psz_var,
vlc_value_t oldval, vlc_value_t newval,
void *p_data )
{
VLC_UNUSED(psz_var); VLC_UNUSED(oldval); VLC_UNUSED(p_data);
filter_t *p_filter = (filter_t*)p_this;
filter_sys_t *p_sys = p_filter->p_sys;

atomic_store( &p_sys->i_intensity, newval.i_int );
return VLC_SUCCESS;
}
