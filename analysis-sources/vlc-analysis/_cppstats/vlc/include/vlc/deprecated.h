#if defined(__cplusplus)
extern "C" {
#endif
LIBVLC_DEPRECATED LIBVLC_API void
libvlc_media_parse( libvlc_media_t *p_md );
LIBVLC_DEPRECATED LIBVLC_API void
libvlc_media_parse_async( libvlc_media_t *p_md );
LIBVLC_DEPRECATED LIBVLC_API bool
libvlc_media_is_parsed( libvlc_media_t *p_md );
LIBVLC_DEPRECATED LIBVLC_API
void libvlc_playlist_play( libvlc_instance_t *p_instance );
#if defined(__cplusplus)
}
#endif
