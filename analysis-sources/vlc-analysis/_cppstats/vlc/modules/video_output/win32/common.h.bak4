























#include <vlc_vout_display.h>





typedef struct event_thread_t event_thread_t;

typedef struct display_win32_area_t
{

vout_display_place_t place;
bool place_changed;

video_format_t texture_source;

vout_display_cfg_t vdcfg;
} display_win32_area_t;

#define RECTWidth(r) (LONG)((r).right - (r).left)
#define RECTHeight(r) (LONG)((r).bottom - (r).top)







typedef struct vout_display_sys_win32_t
{

event_thread_t *event;


HWND hvideownd; 
HWND hparent; 

#if !defined(NDEBUG) && defined(HAVE_DXGIDEBUG_H)
HINSTANCE dxgidebug_dll;
#endif
} vout_display_sys_win32_t;





#if !VLC_WINSTORE_APP
int CommonWindowInit(vlc_object_t *, display_win32_area_t *, vout_display_sys_win32_t *,
bool projection_gestures);
void CommonWindowClean(vlc_object_t *, vout_display_sys_win32_t *);
#endif 
int CommonControl(vlc_object_t *, display_win32_area_t *, vout_display_sys_win32_t *, int , va_list );

void CommonPlacePicture (vlc_object_t *, display_win32_area_t *, vout_display_sys_win32_t *);

void CommonInit(vout_display_t *, display_win32_area_t *, const vout_display_cfg_t *);

#if defined(__cplusplus)
extern "C" {
#endif
void* HookWindowsSensors(vout_display_t*, HWND);
void UnhookWindowsSensors(void*);
#if defined(__cplusplus)
}
#endif
