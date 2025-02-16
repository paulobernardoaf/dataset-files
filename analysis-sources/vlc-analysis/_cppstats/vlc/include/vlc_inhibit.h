typedef struct vlc_inhibit vlc_inhibit_t;
typedef struct vlc_inhibit_sys vlc_inhibit_sys_t;
struct vout_window_t;
enum vlc_inhibit_flags
{
VLC_INHIBIT_NONE=0 ,
VLC_INHIBIT_SUSPEND=0x1 ,
VLC_INHIBIT_DISPLAY=0x2 ,
#define VLC_INHIBIT_AUDIO (VLC_INHIBIT_SUSPEND)
#define VLC_INHIBIT_VIDEO (VLC_INHIBIT_SUSPEND|VLC_INHIBIT_DISPLAY)
};
struct vlc_inhibit
{
struct vlc_object_t obj;
vlc_inhibit_sys_t *p_sys;
void (*inhibit) (vlc_inhibit_t *, unsigned flags);
};
static inline struct vout_window_t *vlc_inhibit_GetWindow(vlc_inhibit_t *ih)
{
return (struct vout_window_t *)vlc_object_parent(ih);
}
static inline void vlc_inhibit_Set (vlc_inhibit_t *ih, unsigned flags)
{
ih->inhibit (ih, flags);
}
