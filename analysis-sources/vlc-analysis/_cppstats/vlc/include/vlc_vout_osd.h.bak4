























#if !defined(VLC_VOUT_OSD_H)
#define VLC_VOUT_OSD_H 1

#if defined(__cplusplus)
extern "C" {
#endif












enum
{

OSD_PLAY_ICON = 1,
OSD_PAUSE_ICON,
OSD_SPEAKER_ICON,
OSD_MUTE_ICON,

OSD_HOR_SLIDER,
OSD_VERT_SLIDER,
};

VLC_API int vout_OSDEpg( vout_thread_t *, input_item_t * );









VLC_API void vout_OSDText( vout_thread_t *vout, int channel, int position, vlc_tick_t duration, const char *text );











VLC_API void vout_OSDMessageVa(vout_thread_t *, int, const char *, va_list);

static inline void
vout_OSDMessage(vout_thread_t *vout, int channel, const char *format, ...)
{
va_list args;
va_start(args, format);
vout_OSDMessageVa(vout, channel, format, args);
va_end(args);
}








VLC_API void vout_OSDSlider( vout_thread_t *, int, int , short );







VLC_API void vout_OSDIcon( vout_thread_t *, int, short );


#if defined(__cplusplus)
}
#endif

#endif 

