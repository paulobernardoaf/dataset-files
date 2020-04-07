#if defined(__cplusplus)
extern "C" {
#endif
VLC_API pa_context *vlc_pa_connect (vlc_object_t *obj,
pa_threaded_mainloop **);
VLC_API void vlc_pa_disconnect (vlc_object_t *obj, pa_context *ctx,
pa_threaded_mainloop *);
VLC_API void vlc_pa_error (vlc_object_t *, const char *msg, pa_context *);
#define vlc_pa_error(o, m, c) vlc_pa_error(VLC_OBJECT(o), m, c)
VLC_API vlc_tick_t vlc_pa_get_latency (vlc_object_t *, pa_context *, pa_stream *);
#define vlc_pa_get_latency(o, c, s) vlc_pa_get_latency(VLC_OBJECT(o), c, s)
VLC_API void vlc_pa_rttime_free (pa_threaded_mainloop *, pa_time_event *);
#if defined(__cplusplus)
}
#endif
