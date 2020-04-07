






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_cpu.h>

#include "i420_rgb.h"
#if defined(SSE2)
#include "i420_rgb_sse2.h"
#define VLC_TARGET VLC_SSE
#else
#include "i420_rgb_mmx.h"
#define VLC_TARGET VLC_MMX
#endif







static void SetOffset( int i_width, int i_height, int i_pic_width,
int i_pic_height, bool *pb_hscale,
unsigned int *pi_vscale, int *p_offset )
{



if( i_pic_width - i_width == 0 )
{ 
*pb_hscale = 0;
}
else if( i_pic_width - i_width > 0 )
{ 
int i_scale_count = i_pic_width;

*pb_hscale = 1;
for( int i_x = i_width; i_x--; )
{
while( (i_scale_count -= i_width) > 0 )
{
*p_offset++ = 0;
}
*p_offset++ = 1;
i_scale_count += i_pic_width;
}
}
else 
{ 
int i_scale_count = i_pic_width;

*pb_hscale = 1;
for( int i_x = i_pic_width; i_x--; )
{
*p_offset = 1;
while( (i_scale_count -= i_pic_width) > 0 )
{
*p_offset += 1;
}
p_offset++;
i_scale_count += i_width;
}
}




if( i_pic_height - i_height == 0 )
*pi_vscale = 0;
else if( i_pic_height - i_height > 0 )
*pi_vscale = 1;
else 
*pi_vscale = -1;
}

VLC_TARGET
void I420_R5G5B5( filter_t *p_filter, picture_t *p_src, picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;


uint16_t *p_pic = (uint16_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;

bool b_hscale; 
unsigned int i_vscale; 
unsigned int i_x, i_y; 

int i_right_margin;
int i_rewind;
int i_scale_count; 
int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2; 
uint16_t * p_pic_start; 


uint16_t * p_buffer_start;
uint16_t * p_buffer;


int * p_offset_start = p_sys->p_offset;
int * p_offset;

const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );

i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;




SetOffset( (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width),
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height),
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );

if(b_hscale &&
AllocateOrGrow(&p_sys->p_buffer, &p_sys->i_buffer_size,
p_filter->fmt_in.video.i_x_offset +
p_filter->fmt_in.video.i_visible_width,
p_sys->i_bytespp))
return;
else p_buffer_start = (uint16_t*)p_sys->p_buffer;




i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);

#if defined(SSE2)

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 15;






p_buffer = b_hscale ? p_buffer_start : p_pic;

if( 0 == (15 & (p_src->p[Y_PLANE].i_pitch|
p_dest->p->i_pitch|
((intptr_t)p_y)|
((intptr_t)p_buffer))) )
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)/16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_16_ALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_15_ALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}


if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;

SSE2_CALL (
SSE2_INIT_16_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_15_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 2 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}
else
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)/16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_16_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_15_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}


if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;

SSE2_CALL (
SSE2_INIT_16_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_15_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 2 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}


SSE2_END;

#else 

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 7;

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;
p_buffer = b_hscale ? p_buffer_start : p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 8; i_x--; )
{
MMX_CALL (
MMX_INIT_16
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_15
);
p_y += 8;
p_u += 4;
p_v += 4;
p_buffer += 8;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;

MMX_CALL (
MMX_INIT_16
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_15
);
p_y += 8;
p_u += 4;
p_v += 4;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 2 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}

MMX_END;

#endif 
}

VLC_TARGET
void I420_R5G6B5( filter_t *p_filter, picture_t *p_src, picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;


uint16_t *p_pic = (uint16_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;

bool b_hscale; 
unsigned int i_vscale; 
unsigned int i_x, i_y; 

int i_right_margin;
int i_rewind;
int i_scale_count; 
int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2; 
uint16_t * p_pic_start; 


uint16_t * p_buffer_start;
uint16_t * p_buffer;


int * p_offset_start = p_sys->p_offset;
int * p_offset;

const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );

i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;




SetOffset( (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width),
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height),
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );

if(b_hscale &&
AllocateOrGrow(&p_sys->p_buffer, &p_sys->i_buffer_size,
p_filter->fmt_in.video.i_x_offset +
p_filter->fmt_in.video.i_visible_width,
p_sys->i_bytespp))
return;
else p_buffer_start = (uint16_t*)p_sys->p_buffer;




i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);

#if defined(SSE2)

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 15;






p_buffer = b_hscale ? p_buffer_start : p_pic;

if( 0 == (15 & (p_src->p[Y_PLANE].i_pitch|
p_dest->p->i_pitch|
((intptr_t)p_y)|
((intptr_t)p_buffer))) )
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)/16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_16_ALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_16_ALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}


if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;

SSE2_CALL (
SSE2_INIT_16_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_16_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 2 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}
else
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)/16; i_x--; )
{
SSE2_CALL(
SSE2_INIT_16_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_16_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}


if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;

SSE2_CALL(
SSE2_INIT_16_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_16_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 2 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}


SSE2_END;

#else 

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 7;

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;
p_buffer = b_hscale ? p_buffer_start : p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 8; i_x--; )
{
MMX_CALL (
MMX_INIT_16
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_16
);
p_y += 8;
p_u += 4;
p_v += 4;
p_buffer += 8;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;

MMX_CALL (
MMX_INIT_16
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_16
);
p_y += 8;
p_u += 4;
p_v += 4;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 2 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}

MMX_END;

#endif 
}

VLC_TARGET
void I420_A8R8G8B8( filter_t *p_filter, picture_t *p_src,
picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;


uint32_t *p_pic = (uint32_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;

bool b_hscale; 
unsigned int i_vscale; 
unsigned int i_x, i_y; 

int i_right_margin;
int i_rewind;
int i_scale_count; 
int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2; 
uint32_t * p_pic_start; 

uint32_t * p_buffer_start;
uint32_t * p_buffer;


int * p_offset_start = p_sys->p_offset;
int * p_offset;

const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );

i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;




SetOffset( p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width,
p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height,
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );

if(b_hscale &&
AllocateOrGrow(&p_sys->p_buffer, &p_sys->i_buffer_size,
p_filter->fmt_in.video.i_x_offset +
p_filter->fmt_in.video.i_visible_width,
p_sys->i_bytespp))
return;
else p_buffer_start = (uint32_t*)p_sys->p_buffer;




i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);

#if defined(SSE2)

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 15;






p_buffer = b_hscale ? p_buffer_start : p_pic;

if( 0 == (15 & (p_src->p[Y_PLANE].i_pitch|
p_dest->p->i_pitch|
((intptr_t)p_y)|
((intptr_t)p_buffer))) )
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_32_ALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_ARGB_ALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_ARGB_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}
else
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_ARGB_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_ARGB_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}


SSE2_END;

#else

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 7;

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;
p_buffer = b_hscale ? p_buffer_start : p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 8; i_x--; )
{
MMX_CALL (
MMX_INIT_32
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_32_ARGB
);
p_y += 8;
p_u += 4;
p_v += 4;
p_buffer += 8;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
MMX_CALL (
MMX_INIT_32
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_32_ARGB
);
p_y += 8;
p_u += 4;
p_v += 4;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}


MMX_END;

#endif
}

VLC_TARGET
void I420_R8G8B8A8( filter_t *p_filter, picture_t *p_src, picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;


uint32_t *p_pic = (uint32_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;

bool b_hscale; 
unsigned int i_vscale; 
unsigned int i_x, i_y; 

int i_right_margin;
int i_rewind;
int i_scale_count; 
int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2; 
uint32_t * p_pic_start; 

uint32_t * p_buffer_start;
uint32_t * p_buffer;


int * p_offset_start = p_sys->p_offset;
int * p_offset;

const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );

i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;




SetOffset( (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width),
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height),
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );

if(b_hscale &&
AllocateOrGrow(&p_sys->p_buffer, &p_sys->i_buffer_size,
p_filter->fmt_in.video.i_x_offset +
p_filter->fmt_in.video.i_visible_width,
p_sys->i_bytespp))
return;
else p_buffer_start = (uint32_t*)p_sys->p_buffer;




i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);

#if defined(SSE2)

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 15;






p_buffer = b_hscale ? p_buffer_start : p_pic;

if( 0 == (15 & (p_src->p[Y_PLANE].i_pitch|
p_dest->p->i_pitch|
((intptr_t)p_y)|
((intptr_t)p_buffer))) )
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_32_ALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_RGBA_ALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_RGBA_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}
else
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_RGBA_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_RGBA_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}


SSE2_END;

#else

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 7;

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;
p_buffer = b_hscale ? p_buffer_start : p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 8; i_x--; )
{
MMX_CALL (
MMX_INIT_32
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_32_RGBA
);
p_y += 8;
p_u += 4;
p_v += 4;
p_buffer += 8;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
MMX_CALL (
MMX_INIT_32
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_32_RGBA
);
p_y += 8;
p_u += 4;
p_v += 4;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}


MMX_END;

#endif
}

VLC_TARGET
void I420_B8G8R8A8( filter_t *p_filter, picture_t *p_src, picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;


uint32_t *p_pic = (uint32_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;

bool b_hscale; 
unsigned int i_vscale; 
unsigned int i_x, i_y; 

int i_right_margin;
int i_rewind;
int i_scale_count; 
int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2; 
uint32_t * p_pic_start; 

uint32_t * p_buffer_start;
uint32_t * p_buffer;


int * p_offset_start = p_sys->p_offset;
int * p_offset;

const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );

i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;




SetOffset( (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width),
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height),
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );

if(b_hscale &&
AllocateOrGrow(&p_sys->p_buffer, &p_sys->i_buffer_size,
p_filter->fmt_in.video.i_x_offset +
p_filter->fmt_in.video.i_visible_width,
p_sys->i_bytespp))
return;
else p_buffer_start = (uint32_t*)p_sys->p_buffer;




i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);

#if defined(SSE2)

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 15;






p_buffer = b_hscale ? p_buffer_start : p_pic;

if( 0 == (15 & (p_src->p[Y_PLANE].i_pitch|
p_dest->p->i_pitch|
((intptr_t)p_y)|
((intptr_t)p_buffer))) )
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_32_ALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_BGRA_ALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_BGRA_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}
else
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_BGRA_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_BGRA_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}


SSE2_END;

#else

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 7;

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;
p_buffer = b_hscale ? p_buffer_start : p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 8; i_x--; )
{
MMX_CALL (
MMX_INIT_32
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_32_BGRA
);
p_y += 8;
p_u += 4;
p_v += 4;
p_buffer += 8;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
MMX_CALL (
MMX_INIT_32
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_32_BGRA
);
p_y += 8;
p_u += 4;
p_v += 4;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}


MMX_END;

#endif
}

VLC_TARGET
void I420_A8B8G8R8( filter_t *p_filter, picture_t *p_src, picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;


uint32_t *p_pic = (uint32_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;

bool b_hscale; 
unsigned int i_vscale; 
unsigned int i_x, i_y; 

int i_right_margin;
int i_rewind;
int i_scale_count; 
int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2; 
uint32_t * p_pic_start; 

uint32_t * p_buffer_start;
uint32_t * p_buffer;


int * p_offset_start = p_sys->p_offset;
int * p_offset;

const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );

i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;




SetOffset( (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width),
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height),
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );

if(b_hscale &&
AllocateOrGrow(&p_sys->p_buffer, &p_sys->i_buffer_size,
p_filter->fmt_in.video.i_x_offset +
p_filter->fmt_in.video.i_visible_width,
p_sys->i_bytespp))
return;
else p_buffer_start = (uint32_t*)p_sys->p_buffer;




i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);

#if defined(SSE2)

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 15;






p_buffer = b_hscale ? p_buffer_start : p_pic;

if( 0 == (15 & (p_src->p[Y_PLANE].i_pitch|
p_dest->p->i_pitch|
((intptr_t)p_y)|
((intptr_t)p_buffer))) )
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_32_ALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_ABGR_ALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_ABGR_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}
else
{

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 16; i_x--; )
{
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_ABGR_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
p_buffer += 16;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
SSE2_CALL (
SSE2_INIT_32_UNALIGNED
SSE2_YUV_MUL
SSE2_YUV_ADD
SSE2_UNPACK_32_ABGR_UNALIGNED
);
p_y += 16;
p_u += 8;
p_v += 8;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
p_buffer = b_hscale ? p_buffer_start : p_pic;
}
}


SSE2_END;

#else

i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 7;

for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;
p_buffer = b_hscale ? p_buffer_start : p_pic;

for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 8; i_x--; )
{
MMX_CALL (
MMX_INIT_32
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_32_ABGR
);
p_y += 8;
p_u += 4;
p_v += 4;
p_buffer += 8;
}



if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
MMX_CALL (
MMX_INIT_32
MMX_YUV_MUL
MMX_YUV_ADD
MMX_UNPACK_32_ABGR
);
p_y += 8;
p_u += 4;
p_v += 4;
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );

p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}


MMX_END;

#endif
}
