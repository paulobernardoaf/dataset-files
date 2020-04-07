#include <vlc_common.h>
#include <vlc_filter.h>
#include <assert.h>
#define FFMAX(a,b) __MAX(a,b)
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) __MIN(a,b)
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)
typedef struct {
vlc_tick_t pi_date;
int pi_nb_fields;
bool pb_top_field_first;
} metadata_history_t;
#define METADATA_SIZE (3)
#define HISTORY_SIZE (3)
typedef struct {
bool b_double_rate; 
bool b_use_frame_history; 
bool b_custom_pts; 
bool b_half_height; 
} deinterlace_algo;
struct deinterlace_ctx
{
deinterlace_algo settings;
metadata_history_t meta[METADATA_SIZE];
int i_frame_offset;
picture_t *pp_history[HISTORY_SIZE];
union {
int (*pf_render_ordered)(filter_t *, picture_t *p_dst, picture_t *p_pic,
int order, int i_field);
int (*pf_render_single_pic)(filter_t *, picture_t *p_dst, picture_t *p_pic);
};
};
#define DEINTERLACE_DST_SIZE 3
void InitDeinterlacingContext( struct deinterlace_ctx * );
vlc_tick_t GetFieldDuration( const struct deinterlace_ctx *,
const video_format_t *fmt, const picture_t *p_pic );
void GetDeinterlacingOutput( const struct deinterlace_ctx *,
video_format_t *p_dst, const video_format_t *p_src );
picture_t *DoDeinterlacing( filter_t *, struct deinterlace_ctx *, picture_t * );
void FlushDeinterlacing( struct deinterlace_ctx * );
picture_t *AllocPicture( filter_t * );
