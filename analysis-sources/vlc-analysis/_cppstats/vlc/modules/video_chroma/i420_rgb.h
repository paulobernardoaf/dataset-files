#include <limits.h>
#if !defined (SSE2) && !defined (MMX)
#define PLAIN
#endif
#define CMAP_RGB2_SIZE 256
typedef struct
{
uint8_t *p_buffer;
size_t i_buffer_size;
uint8_t i_bytespp;
int *p_offset;
#if defined(PLAIN)
void *p_base; 
uint8_t *p_rgb8; 
uint16_t *p_rgb16; 
uint32_t *p_rgb32; 
uint16_t p_rgb_r[CMAP_RGB2_SIZE]; 
uint16_t p_rgb_g[CMAP_RGB2_SIZE]; 
uint16_t p_rgb_b[CMAP_RGB2_SIZE]; 
#endif
} filter_sys_t;
static inline int AllocateOrGrow( uint8_t **pp_buffer, size_t *pi_buffer,
unsigned i_width, uint8_t bytespp )
{
if(UINT_MAX / bytespp < i_width)
return -1;
const size_t i_realloc = i_width * bytespp;
if(*pi_buffer >= i_realloc)
return 0;
uint8_t *p_realloc = realloc(*pp_buffer, i_realloc);
if(!p_realloc)
return -1;
*pp_buffer = p_realloc;
*pi_buffer = i_realloc;
return 0;
}
#if defined(PLAIN)
void I420_RGB8 ( filter_t *, picture_t *, picture_t * );
void I420_RGB16 ( filter_t *, picture_t *, picture_t * );
void I420_RGB32 ( filter_t *, picture_t *, picture_t * );
#else
void I420_R5G5B5 ( filter_t *, picture_t *, picture_t * );
void I420_R5G6B5 ( filter_t *, picture_t *, picture_t * );
void I420_A8R8G8B8 ( filter_t *, picture_t *, picture_t * );
void I420_R8G8B8A8 ( filter_t *, picture_t *, picture_t * );
void I420_B8G8R8A8 ( filter_t *, picture_t *, picture_t * );
void I420_A8B8G8R8 ( filter_t *, picture_t *, picture_t * );
#endif
#define CONVERT_Y_PIXEL( BPP ) p_ybase = p_yuv + *p_y++; *p_buffer++ = p_ybase[RED_OFFSET-((V_RED_COEF*128)>>SHIFT) + i_red] | p_ybase[GREEN_OFFSET-(((U_GREEN_COEF+V_GREEN_COEF)*128)>>SHIFT) + i_green ] | p_ybase[BLUE_OFFSET-((U_BLUE_COEF*128)>>SHIFT) + i_blue];
#define CONVERT_YUV_PIXEL( BPP ) i_uval = *p_u++; i_vval = *p_v++; i_red = (V_RED_COEF * i_vval) >> SHIFT; i_green = (U_GREEN_COEF * i_uval + V_GREEN_COEF * i_vval) >> SHIFT; i_blue = (U_BLUE_COEF * i_uval) >> SHIFT; CONVERT_Y_PIXEL( BPP ) 
#define CONVERT_Y_PIXEL_DITHER( BPP ) p_ybase = p_yuv + *p_y++; *p_buffer++ = p_ybase[RED_OFFSET-((V_RED_COEF*128+p_dither[i_real_y])>>SHIFT) + i_red] | p_ybase[GREEN_OFFSET-(((U_GREEN_COEF+V_GREEN_COEF)*128+p_dither[i_real_y])>>SHIFT) + i_green ] | p_ybase[BLUE_OFFSET-((U_BLUE_COEF*128+p_dither[i_real_y])>>SHIFT) + i_blue];
#define CONVERT_YUV_PIXEL_DITHER( BPP ) i_uval = *p_u++; i_vval = *p_v++; i_red = (V_RED_COEF * i_vval) >> SHIFT; i_green = (U_GREEN_COEF * i_uval + V_GREEN_COEF * i_vval) >> SHIFT; i_blue = (U_BLUE_COEF * i_uval) >> SHIFT; CONVERT_Y_PIXEL_DITHER( BPP ) 
#define CONVERT_4YUV_PIXEL( CHROMA ) *p_pic++ = p_lookup[ (((*p_y++ + dither10[i_real_y]) >> 4) << 7) + ((*p_u + dither20[i_real_y]) >> 5) * 9 + ((*p_v + dither20[i_real_y]) >> 5) ]; *p_pic++ = p_lookup[ (((*p_y++ + dither11[i_real_y]) >> 4) << 7) + ((*p_u++ + dither21[i_real_y]) >> 5) * 9 + ((*p_v++ + dither21[i_real_y]) >> 5) ]; *p_pic++ = p_lookup[ (((*p_y++ + dither12[i_real_y]) >> 4) << 7) + ((*p_u + dither22[i_real_y]) >> 5) * 9 + ((*p_v + dither22[i_real_y]) >> 5) ]; *p_pic++ = p_lookup[ (((*p_y++ + dither13[i_real_y]) >> 4) << 7) + ((*p_u++ + dither23[i_real_y]) >> 5) * 9 + ((*p_v++ + dither23[i_real_y]) >> 5) ]; 
#define CONVERT_4YUV_PIXEL_SCALE( CHROMA ) *p_pic++ = p_lookup[ ( ((*p_y + dither10[i_real_y]) >> 4) << 7) + ((*p_u + dither20[i_real_y]) >> 5) * 9 + ((*p_v + dither20[i_real_y]) >> 5) ]; p_y += *p_offset++; p_u += *p_offset; p_v += *p_offset++; *p_pic++ = p_lookup[ ( ((*p_y + dither11[i_real_y]) >> 4) << 7) + ((*p_u + dither21[i_real_y]) >> 5) * 9 + ((*p_v + dither21[i_real_y]) >> 5) ]; p_y += *p_offset++; p_u += *p_offset; p_v += *p_offset++; *p_pic++ = p_lookup[ ( ((*p_y + dither12[i_real_y]) >> 4) << 7) + ((*p_u + dither22[i_real_y]) >> 5) * 9 + ((*p_v + dither22[i_real_y]) >> 5) ]; p_y += *p_offset++; p_u += *p_offset; p_v += *p_offset++; *p_pic++ = p_lookup[ ( ((*p_y + dither13[i_real_y]) >> 4) << 7) + ((*p_u + dither23[i_real_y]) >> 5) * 9 + ((*p_v + dither23[i_real_y]) >> 5) ]; p_y += *p_offset++; p_u += *p_offset; p_v += *p_offset++; 
#define SCALE_WIDTH if( b_hscale ) { p_buffer = p_buffer_start; p_offset = p_offset_start; for( i_x = (p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width) / 16; i_x--; ) { *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; *p_pic++ = *p_buffer; p_buffer += *p_offset++; } for( i_x = (p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width) & 15; i_x--; ) { *p_pic++ = *p_buffer; p_buffer += *p_offset++; } p_pic = (void*)((uint8_t*)p_pic + i_right_margin ); } else { p_pic = (void*)((uint8_t*)p_pic + p_dest->p->i_pitch ); } 
#define SCALE_WIDTH_DITHER( CHROMA ) if( b_hscale ) { p_offset = p_offset_start; for( i_x = (p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width) / 16; i_x--; ) { CONVERT_4YUV_PIXEL_SCALE( CHROMA ) CONVERT_4YUV_PIXEL_SCALE( CHROMA ) CONVERT_4YUV_PIXEL_SCALE( CHROMA ) CONVERT_4YUV_PIXEL_SCALE( CHROMA ) } } else { for( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; ) { CONVERT_4YUV_PIXEL( CHROMA ) CONVERT_4YUV_PIXEL( CHROMA ) CONVERT_4YUV_PIXEL( CHROMA ) CONVERT_4YUV_PIXEL( CHROMA ) } } p_pic = (void*)((uint8_t*)p_pic + i_right_margin ); i_real_y = (i_real_y + 1) & 0x3; 
#define SCALE_HEIGHT( CHROMA, BPP ) if( ((CHROMA == 420) || (CHROMA == 422)) && !(i_y & 0x1) ) { p_u -= i_chroma_width; p_v -= i_chroma_width; } switch( i_vscale ) { case -1: while( (i_scale_count -= (p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height)) > 0 ) { p_y += (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width); i_y++; if( (CHROMA == 420) || (CHROMA == 422) ) { if( i_y & 0x1 ) { p_u += i_chroma_width; p_v += i_chroma_width; } } else if( CHROMA == 444 ) { p_u += (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width); p_v += (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width); } } i_scale_count += (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); break; case 1: while( (i_scale_count -= (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height)) > 0 ) { memcpy( p_pic, p_pic_start, (p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width) * BPP ); p_pic = (void*)((uint8_t*)p_pic + p_dest->p->i_pitch ); } i_scale_count += (p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height); break; } 
#define SCALE_HEIGHT_DITHER( CHROMA ) if( ((CHROMA == 420) || (CHROMA == 422)) && !(i_y & 0x1) ) { p_u -= i_chroma_width; p_v -= i_chroma_width; } switch( i_vscale ) { case -1: while( (i_scale_count -= (p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height)) > 0 ) { p_y += (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width); i_y++; if( (CHROMA == 420) || (CHROMA == 422) ) { if( i_y & 0x1 ) { p_u += i_chroma_width; p_v += i_chroma_width; } } else if( CHROMA == 444 ) { p_u += (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width); p_v += (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width); } } i_scale_count += (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); break; case 1: while( (i_scale_count -= (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height)) > 0 ) { p_y -= (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width); p_u -= i_chroma_width; p_v -= i_chroma_width; SCALE_WIDTH_DITHER( CHROMA ); } i_scale_count += (p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height); break; } 
