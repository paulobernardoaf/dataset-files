#define CASE_PLANAR_YUV_SQUARE case VLC_CODEC_I420: case VLC_CODEC_J420: case VLC_CODEC_YV12: case VLC_CODEC_I411: case VLC_CODEC_I410: case VLC_CODEC_I444: case VLC_CODEC_J444: case VLC_CODEC_YUVA:
#define CASE_PLANAR_YUV_NONSQUARE case VLC_CODEC_I422: case VLC_CODEC_J422:
#define CASE_PLANAR_YUV10 case VLC_CODEC_I420_10L: case VLC_CODEC_I420_10B: case VLC_CODEC_I444_10L: case VLC_CODEC_I444_10B:
#define CASE_PLANAR_YUV9 case VLC_CODEC_I420_9L: case VLC_CODEC_I420_9B: case VLC_CODEC_I444_9L: case VLC_CODEC_I444_9B:
#define CASE_PLANAR_YUV CASE_PLANAR_YUV_SQUARE CASE_PLANAR_YUV_NONSQUARE 
#define CASE_PACKED_YUV_422 case VLC_CODEC_UYVY: case VLC_CODEC_YUYV: case VLC_CODEC_YVYU:
static inline int GetPackedYuvOffsets( vlc_fourcc_t i_chroma,
int *i_y_offset, int *i_u_offset, int *i_v_offset )
{
switch( i_chroma )
{
case VLC_CODEC_UYVY:
*i_y_offset = 1;
*i_u_offset = 0;
*i_v_offset = 2;
return VLC_SUCCESS;
case VLC_CODEC_VYUY:
*i_y_offset = 1;
*i_u_offset = 2;
*i_v_offset = 0;
return VLC_SUCCESS;
case VLC_CODEC_YUYV:
*i_y_offset = 0;
*i_u_offset = 1;
*i_v_offset = 3;
return VLC_SUCCESS;
case VLC_CODEC_YVYU:
*i_y_offset = 0;
*i_u_offset = 3;
*i_v_offset = 1;
return VLC_SUCCESS;
default:
return VLC_EGENERIC;
}
}
static inline int GetPackedRgbIndexes( const video_format_t *p_fmt, int *i_r_index,
int *i_g_index, int *i_b_index )
{
if( p_fmt->i_chroma != VLC_CODEC_RGB24 && p_fmt->i_chroma != VLC_CODEC_RGB32 )
return VLC_EGENERIC;
#if defined(WORDS_BIGENDIAN)
const int i_mask_bits = p_fmt->i_chroma == VLC_CODEC_RGB24 ? 16 : 24;
*i_r_index = (i_mask_bits - vlc_ctz(p_fmt->i_rmask)) / 8;
*i_g_index = (i_mask_bits - vlc_ctz(p_fmt->i_gmask)) / 8;
*i_b_index = (i_mask_bits - vlc_ctz(p_fmt->i_bmask)) / 8;
#else
*i_r_index = vlc_ctz(p_fmt->i_rmask) / 8;
*i_g_index = vlc_ctz(p_fmt->i_gmask) / 8;
*i_b_index = vlc_ctz(p_fmt->i_bmask) / 8;
#endif
return VLC_SUCCESS;
}
static inline uint8_t vlc_uint8( int v )
{
if( v > 255 )
return 255;
else if( v < 0 )
return 0;
return v;
}
static inline void yuv_to_rgb( int *r, int *g, int *b,
uint8_t y1, uint8_t u1, uint8_t v1 )
{
#define SCALEBITS 10
#define ONE_HALF (1 << (SCALEBITS - 1))
#define FIX(x) ((int) ((x) * (1<<SCALEBITS) + 0.5))
int y, cb, cr, r_add, g_add, b_add;
cb = u1 - 128;
cr = v1 - 128;
r_add = FIX(1.40200*255.0/224.0) * cr + ONE_HALF;
g_add = - FIX(0.34414*255.0/224.0) * cb
- FIX(0.71414*255.0/224.0) * cr + ONE_HALF;
b_add = FIX(1.77200*255.0/224.0) * cb + ONE_HALF;
y = (y1 - 16) * FIX(255.0/219.0);
*r = vlc_uint8( (y + r_add) >> SCALEBITS );
*g = vlc_uint8( (y + g_add) >> SCALEBITS );
*b = vlc_uint8( (y + b_add) >> SCALEBITS );
#undef FIX
#undef ONE_HALF
#undef SCALEBITS
}
static inline void rgb_to_yuv( uint8_t *y, uint8_t *u, uint8_t *v,
int r, int g, int b )
{
*y = ( ( ( 66 * r + 129 * g + 25 * b + 128 ) >> 8 ) + 16 );
*u = ( ( -38 * r - 74 * g + 112 * b + 128 ) >> 8 ) + 128 ;
*v = ( ( 112 * r - 94 * g - 18 * b + 128 ) >> 8 ) + 128 ;
}
static inline picture_t *CopyInfoAndRelease( picture_t *p_outpic, picture_t *p_inpic )
{
picture_CopyProperties( p_outpic, p_inpic );
picture_Release( p_inpic );
return p_outpic;
}
