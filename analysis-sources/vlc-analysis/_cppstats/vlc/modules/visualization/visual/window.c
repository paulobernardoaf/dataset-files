#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <math.h>
#include "window.h"
#include "window_presets.h"
#include <assert.h>
#define FT_A0 1.000f
#define FT_A1 1.930f
#define FT_A2 1.290f
#define FT_A3 0.388f
#define FT_A4 0.028f
#define BH_A0 0.35875f
#define BH_A1 0.48829f
#define BH_A2 0.14128f
#define BH_A3 0.01168f
static float bessi0( float x )
{
float ax, ans;
double y; 
if( ( ax = fabsf( x ) ) < 3.75f ) 
{
y = x / 3.75;
y *= y;
ans = 1.0 + y * ( 3.5156229 + y * ( 3.0899424 + y * ( 1.2067492
+ y * ( 0.2659732 + y * ( 0.360768e-1
+ y * 0.45813e-2 ) ) ) ) );
}
else
{
y = 3.75 / ax;
ans = ( exp( ax ) / sqrt( ax ) ) * ( 0.39894228 + y * ( 0.1328592e-1
+ y * ( 0.225319e-2 + y * ( -0.157565e-2 + y * ( 0.916281e-2
+ y * ( -0.2057706e-1 + y * ( 0.2635537e-1 + y * ( -0.1647633e-1
+ y * 0.392377e-2 ) ) ) ) ) ) ) );
}
return ans;
}
void window_get_param( vlc_object_t * p_aout, window_param * p_param )
{
p_param->f_kaiser_alpha = var_InheritFloat( p_aout, "effect-kaiser-param" );
char * psz_preset = var_InheritString( p_aout, "effect-fft-window" );
if( !psz_preset )
{
goto no_preset;
}
for( int i = 0; i < NB_WINDOWS; i++ )
{
if( !strcasecmp( psz_preset, window_list[i] ) )
{
free( psz_preset );
p_param->wind_type = i;
return;
}
}
free( psz_preset );
no_preset:
msg_Warn( p_aout, "No matching window preset found; using rectangular "
"window (i.e. no window)" );
p_param->wind_type = NONE;
}
bool window_init( int i_buffer_size, window_param * p_param,
window_context * p_ctx )
{
float * pf_table = NULL;
window_type wind_type = p_param->wind_type;
if( wind_type != HANN && wind_type != FLATTOP
&& wind_type != BLACKMANHARRIS
&& wind_type != KAISER )
{
i_buffer_size = 0;
goto exit;
}
pf_table = vlc_alloc( i_buffer_size, sizeof( *pf_table ) );
if( !pf_table )
{
return false;
}
int i_buffer_size_minus_1 = i_buffer_size - 1;
switch( wind_type )
{
case HANN:
for( int i = 0; i < i_buffer_size; i++ )
{
float f_val = (float) i / (float) i_buffer_size_minus_1;
pf_table[i] = 0.5f - 0.5f * cosf( 2.0f * (float) M_PI * f_val );
}
break;
case FLATTOP:
for( int i = 0; i < i_buffer_size; i++ )
{
float f_val = (float) i / (float) i_buffer_size_minus_1;
pf_table[i] = FT_A0
- FT_A1 * cosf( 2.0f * (float) M_PI * f_val )
+ FT_A2 * cosf( 4.0f * (float) M_PI * f_val )
- FT_A3 * cosf( 6.0f * (float) M_PI * f_val )
+ FT_A4 * cosf( 8.0f * (float) M_PI * f_val );
}
break;
case BLACKMANHARRIS:
for( int i = 0; i < i_buffer_size; i++ )
{
float f_val = (float) i / (float) i_buffer_size_minus_1;
pf_table[i] = BH_A0
- BH_A1 * cosf( 2.0f * (float) M_PI * f_val )
+ BH_A2 * cosf( 4.0f * (float) M_PI * f_val )
- BH_A3 * cosf( 6.0f * (float) M_PI * f_val );
}
break;
case KAISER:
{
float f_pialph = (float) M_PI * p_param->f_kaiser_alpha;
float f_bessi0_pialph = bessi0( f_pialph );
for( int i = 0; i < i_buffer_size; i++ )
{
float f_val = (float) i / (float) i_buffer_size_minus_1;
float f_term_to_square = 2.0f * f_val - 1.0f;
float f_sqd_term = f_term_to_square * f_term_to_square;
float f_sqr_term = sqrtf( 1.0f - f_sqd_term );
pf_table[i] = bessi0( f_pialph * f_sqr_term ) / f_bessi0_pialph;
}
break;
}
default:
vlc_assert_unreachable();
break;
}
exit:
p_ctx->pf_window_table = pf_table;
p_ctx->i_buffer_size = i_buffer_size;
return true;
}
void window_scale_in_place( int16_t * p_buffer, window_context * p_ctx )
{
for( int i = 0; i < p_ctx->i_buffer_size; i++ )
{
p_buffer[i] *= p_ctx->pf_window_table[i];
}
}
void window_close( window_context * p_ctx )
{
if( p_ctx->pf_window_table )
{
free( p_ctx->pf_window_table );
p_ctx->pf_window_table = NULL;
p_ctx->i_buffer_size = 0;
}
}
