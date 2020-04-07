























#if !defined(VLC_LIBVLC_MEDIA_DISCOVERER_H)
#define VLC_LIBVLC_MEDIA_DISCOVERER_H 1

#if defined(__cplusplus)
extern "C" {
#endif





typedef enum libvlc_media_discoverer_category_t {

libvlc_media_discoverer_devices,

libvlc_media_discoverer_lan,

libvlc_media_discoverer_podcasts,

libvlc_media_discoverer_localdirs,
} libvlc_media_discoverer_category_t;





typedef struct libvlc_media_discoverer_description_t {
char *psz_name;
char *psz_longname;
libvlc_media_discoverer_category_t i_cat;
} libvlc_media_discoverer_description_t;













typedef struct libvlc_media_discoverer_t libvlc_media_discoverer_t;




















LIBVLC_API libvlc_media_discoverer_t *
libvlc_media_discoverer_new( libvlc_instance_t * p_inst,
const char * psz_name );













LIBVLC_API int
libvlc_media_discoverer_start( libvlc_media_discoverer_t * p_mdis );









LIBVLC_API void
libvlc_media_discoverer_stop( libvlc_media_discoverer_t * p_mdis );







LIBVLC_API void
libvlc_media_discoverer_release( libvlc_media_discoverer_t * p_mdis );







LIBVLC_API libvlc_media_list_t *
libvlc_media_discoverer_media_list( libvlc_media_discoverer_t * p_mdis );









LIBVLC_API bool
libvlc_media_discoverer_is_running(libvlc_media_discoverer_t *p_mdis);














LIBVLC_API size_t
libvlc_media_discoverer_list_get( libvlc_instance_t *p_inst,
libvlc_media_discoverer_category_t i_cat,
libvlc_media_discoverer_description_t ***ppp_services );











LIBVLC_API void
libvlc_media_discoverer_list_release( libvlc_media_discoverer_description_t **pp_services,
size_t i_count );



#if defined(__cplusplus)
}
#endif

#endif 
