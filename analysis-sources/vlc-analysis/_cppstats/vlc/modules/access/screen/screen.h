#include <vlc_input.h>
#include <vlc_access.h>
#include <vlc_demux.h>
#if defined(__APPLE__)
#define SCREEN_DISPLAY_ID
#endif
#define SCREEN_SUBSCREEN
#define SCREEN_MOUSE
#if defined(SCREEN_MOUSE)
#include <vlc_image.h>
#endif
typedef struct screen_data_t screen_data_t;
typedef struct
{
es_format_t fmt;
es_out_id_t *es;
float f_fps;
vlc_tick_t i_next_date;
vlc_tick_t i_incr;
vlc_tick_t i_start;
#if defined(SCREEN_SUBSCREEN)
bool b_follow_mouse;
unsigned int i_screen_height;
unsigned int i_screen_width;
unsigned int i_top;
unsigned int i_left;
unsigned int i_height;
unsigned int i_width;
#endif
#if defined(SCREEN_MOUSE)
picture_t *p_mouse;
filter_t *p_blend;
picture_t dst;
#endif
#if defined(SCREEN_DISPLAY_ID)
unsigned int i_display_id;
unsigned int i_screen_index;
#endif
screen_data_t *p_data;
} demux_sys_t;
int screen_InitCapture ( demux_t * );
int screen_CloseCapture( demux_t * );
block_t *screen_Capture( demux_t * );
#if defined(SCREEN_SUBSCREEN)
void FollowMouse( demux_sys_t *, int, int );
#endif
#if defined(SCREEN_MOUSE)
void RenderCursor( demux_t *, int, int, uint8_t * );
#endif
