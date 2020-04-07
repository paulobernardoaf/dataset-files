























#if !defined(VLC_SUBPICTURE_H)
#define VLC_SUBPICTURE_H 1




#include <vlc_picture.h>
#include <vlc_text_style.h>














typedef struct subpicture_region_private_t subpicture_region_private_t;
typedef struct vlc_spu_highlight_t vlc_spu_highlight_t;
typedef struct filter_t vlc_blender_t;








struct subpicture_region_t
{
video_format_t fmt; 
picture_t *p_picture; 

int i_x; 
int i_y; 
int i_align; 
int i_alpha; 


text_segment_t *p_text; 
int i_text_align; 
bool b_noregionbg; 
bool b_gridmode; 
bool b_balanced_text; 
int i_max_width; 
int i_max_height; 

vlc_rational_t zoom_h;
vlc_rational_t zoom_v;

subpicture_region_t *p_next; 
subpicture_region_private_t *p_private; 
};

struct vlc_spu_highlight_t
{
int x_start;
int x_end;
int y_start;
int y_end;
video_palette_t palette;
};


#define SUBPICTURE_ALIGN_LEFT 0x1
#define SUBPICTURE_ALIGN_RIGHT 0x2
#define SUBPICTURE_ALIGN_TOP 0x4
#define SUBPICTURE_ALIGN_BOTTOM 0x8
#define SUBPICTURE_ALIGN_MASK ( SUBPICTURE_ALIGN_LEFT|SUBPICTURE_ALIGN_RIGHT| SUBPICTURE_ALIGN_TOP |SUBPICTURE_ALIGN_BOTTOM )






VLC_API subpicture_region_t * subpicture_region_New( const video_format_t *p_fmt );







VLC_API void subpicture_region_Delete( subpicture_region_t *p_region );







VLC_API void subpicture_region_ChainDelete( subpicture_region_t *p_head );







VLC_API subpicture_region_t *subpicture_region_Copy( subpicture_region_t *p_region );




typedef struct
{



int (*pf_validate)( subpicture_t *,
bool has_src_changed, const video_format_t *p_fmt_src,
bool has_dst_changed, const video_format_t *p_fmt_dst,
vlc_tick_t);



void (*pf_update) ( subpicture_t *,
const video_format_t *p_fmt_src,
const video_format_t *p_fmt_dst,
vlc_tick_t );

void (*pf_destroy) ( subpicture_t * );
void *p_sys;
} subpicture_updater_t;

typedef struct subpicture_private_t subpicture_private_t;









struct subpicture_t
{


ssize_t i_channel; 





int64_t i_order; 
subpicture_t * p_next; 


subpicture_region_t *p_region; 



vlc_tick_t i_start; 
vlc_tick_t i_stop; 
bool b_ephemer; 

bool b_fade; 







bool b_subtitle; 
bool b_absolute; 
int i_original_picture_width; 
int i_original_picture_height;
int i_alpha; 


subpicture_updater_t updater;

subpicture_private_t *p_private; 
};






VLC_API subpicture_t * subpicture_New( const subpicture_updater_t * );





VLC_API void subpicture_Delete( subpicture_t *p_subpic );








VLC_API subpicture_t * subpicture_NewFromPicture( vlc_object_t *, picture_t *, vlc_fourcc_t i_chroma );





VLC_API void subpicture_Update( subpicture_t *, const video_format_t *src, const video_format_t *, vlc_tick_t );











VLC_API unsigned picture_BlendSubpicture( picture_t *, vlc_blender_t *, subpicture_t * );



#endif 
