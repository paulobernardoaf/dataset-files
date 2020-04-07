#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_picture.h>
#include "common.h"
void InitDeinterlacingContext( struct deinterlace_ctx *p_context )
{
p_context->settings.b_double_rate = false;
p_context->settings.b_half_height = false;
p_context->settings.b_use_frame_history = false;
p_context->settings.b_custom_pts = false;
p_context->meta[0].pi_date = VLC_TICK_INVALID;
p_context->meta[0].pi_nb_fields = 2;
p_context->meta[0].pb_top_field_first = true;
for( int i = 1; i < METADATA_SIZE; i++ )
p_context->meta[i] = p_context->meta[i-1];
p_context->i_frame_offset = 0; 
for( int i = 0; i < HISTORY_SIZE; i++ )
p_context->pp_history[i] = NULL;
}
void FlushDeinterlacing(struct deinterlace_ctx *p_context)
{
p_context->meta[0].pi_date = VLC_TICK_INVALID;
p_context->meta[0].pi_nb_fields = 2;
p_context->meta[0].pb_top_field_first = true;
for( int i = 1; i < METADATA_SIZE; i++ )
p_context->meta[i] = p_context->meta[i-1];
p_context->i_frame_offset = 0; 
for( int i = 0; i < HISTORY_SIZE; i++ )
{
if( p_context->pp_history[i] )
picture_Release( p_context->pp_history[i] );
p_context->pp_history[i] = NULL;
}
}
vlc_tick_t GetFieldDuration(const struct deinterlace_ctx *p_context,
const video_format_t *fmt, const picture_t *p_pic )
{
vlc_tick_t i_field_dur = 0;
int i = 0;
int iend = METADATA_SIZE-1;
for( ; i < iend; i++ )
if( p_context->meta[i].pi_date != VLC_TICK_INVALID )
break;
if( i < iend )
{
int i_fields_total = 0;
for( int j = i ; j < iend; j++ )
i_fields_total += p_context->meta[j].pi_nb_fields;
i_field_dur = (p_pic->date - p_context->meta[i].pi_date) / i_fields_total;
}
else if (fmt->i_frame_rate)
i_field_dur = vlc_tick_from_samples( fmt->i_frame_rate_base, fmt->i_frame_rate);
return i_field_dur;
}
void GetDeinterlacingOutput( const struct deinterlace_ctx *p_context,
video_format_t *p_dst, const video_format_t *p_src )
{
*p_dst = *p_src;
if( p_context->settings.b_half_height )
{
p_dst->i_height /= 2;
p_dst->i_visible_height /= 2;
p_dst->i_y_offset /= 2;
p_dst->i_sar_den *= 2;
}
if( p_context->settings.b_double_rate )
{
p_dst->i_frame_rate *= 2;
}
}
#define CUSTOM_PTS -1
picture_t *DoDeinterlacing( filter_t *p_filter,
struct deinterlace_ctx *p_context, picture_t *p_pic )
{
picture_t *p_dst[DEINTERLACE_DST_SIZE];
int i_double_rate_alloc_end;
int i_frame_offset;
int i_meta_idx;
bool b_top_field_first;
p_dst[0] = AllocPicture( p_filter );
if( p_dst[0] == NULL )
{
picture_Release( p_pic );
return NULL;
}
picture_CopyProperties( p_dst[0], p_pic );
for( int i = 1; i < DEINTERLACE_DST_SIZE; ++i )
p_dst[i] = NULL;
if( p_context->settings.b_use_frame_history )
{
picture_t *p_dup = picture_Hold( p_pic );
if( p_context->pp_history[0] )
picture_Release( p_context->pp_history[0] );
for( int i = 1; i < HISTORY_SIZE; i++ )
p_context->pp_history[i-1] = p_context->pp_history[i];
p_context->pp_history[HISTORY_SIZE-1] = p_dup;
}
for( int i = 1; i < METADATA_SIZE; i++ )
p_context->meta[i-1] = p_context->meta[i];
p_context->meta[METADATA_SIZE-1].pi_date = p_pic->date;
p_context->meta[METADATA_SIZE-1].pi_nb_fields = p_pic->i_nb_fields;
p_context->meta[METADATA_SIZE-1].pb_top_field_first = p_pic->b_top_field_first;
i_frame_offset = p_context->i_frame_offset;
i_meta_idx = (METADATA_SIZE-1) - i_frame_offset;
int i_nb_fields;
if( i_frame_offset != CUSTOM_PTS )
{
b_top_field_first = p_context->meta[i_meta_idx].pb_top_field_first;
i_nb_fields = p_context->meta[i_meta_idx].pi_nb_fields;
}
else
{
assert( !p_context->settings.b_double_rate );
b_top_field_first = p_pic->b_top_field_first; 
i_nb_fields = p_pic->i_nb_fields; 
}
i_double_rate_alloc_end = 0; 
if( p_context->settings.b_double_rate )
{
i_double_rate_alloc_end = i_nb_fields;
if( i_nb_fields > DEINTERLACE_DST_SIZE )
{
msg_Err( p_filter, "Framerate doubler: output buffer too small; "\
"fields = %d, buffer size = %d. Dropping the "\
"remaining fields.",
i_nb_fields, DEINTERLACE_DST_SIZE );
i_double_rate_alloc_end = DEINTERLACE_DST_SIZE;
}
for( int i = 1; i < i_double_rate_alloc_end ; ++i )
{
p_dst[i-1]->p_next =
p_dst[i] = AllocPicture( p_filter );
if( p_dst[i] )
{
picture_CopyProperties( p_dst[i], p_pic );
}
else
{
msg_Err( p_filter, "Framerate doubler: could not allocate "\
"output frame %d", i+1 );
i_double_rate_alloc_end = i; 
break; 
}
}
}
assert( p_context->settings.b_double_rate || p_dst[1] == NULL );
assert( i_nb_fields > 2 || p_dst[2] == NULL );
if ( !p_context->settings.b_double_rate )
{
if ( p_context->pf_render_single_pic( p_filter, p_dst[0], p_pic ) )
goto drop;
}
else
{
if ( p_context->pf_render_ordered( p_filter, p_dst[0], p_pic,
0, !b_top_field_first ) )
goto drop;
if ( p_dst[1] )
p_context->pf_render_ordered( p_filter, p_dst[1], p_pic,
1, b_top_field_first );
if ( p_dst[2] )
p_context->pf_render_ordered( p_filter, p_dst[2], p_pic,
2, !b_top_field_first );
}
if ( p_context->settings.b_custom_pts )
{
assert(p_context->settings.b_use_frame_history);
if( p_context->pp_history[0] && p_context->pp_history[1] )
{
p_context->i_frame_offset = CUSTOM_PTS;
}
else if( !p_context->pp_history[0] && !p_context->pp_history[1] ) 
{
}
else 
{
p_context->i_frame_offset = CUSTOM_PTS;
}
}
assert( i_frame_offset <= METADATA_SIZE ||
i_frame_offset == CUSTOM_PTS );
if( i_frame_offset != CUSTOM_PTS )
{
vlc_tick_t i_base_pts = p_context->meta[i_meta_idx].pi_date;
p_dst[0]->date = i_base_pts;
if( p_context->settings.b_double_rate )
{
vlc_tick_t i_field_dur = GetFieldDuration( p_context, &p_filter->fmt_out.video, p_pic );
for( int i = 1; i < i_double_rate_alloc_end; ++i )
{
if( i_base_pts != VLC_TICK_INVALID )
p_dst[i]->date = i_base_pts + i * i_field_dur;
else
p_dst[i]->date = VLC_TICK_INVALID;
}
}
}
for( int i = 0; i < DEINTERLACE_DST_SIZE; ++i )
{
if( p_dst[i] )
{
p_dst[i]->b_progressive = true;
p_dst[i]->i_nb_fields = 2;
}
}
picture_Release( p_pic );
return p_dst[0];
drop:
picture_Release( p_dst[0] );
for( int i = 1; i < DEINTERLACE_DST_SIZE; ++i )
{
if( p_dst[i] )
picture_Release( p_dst[i] );
}
#if !defined(NDEBUG)
picture_Release( p_pic );
return NULL;
#else
return p_pic;
#endif
}
