#if defined(__cplusplus)
extern "C" {
#endif
typedef struct libvlc_renderer_discoverer_t libvlc_renderer_discoverer_t;
typedef struct libvlc_rd_description_t
{
char *psz_name;
char *psz_longname;
} libvlc_rd_description_t;
#define LIBVLC_RENDERER_CAN_AUDIO 0x0001
#define LIBVLC_RENDERER_CAN_VIDEO 0x0002
typedef struct libvlc_renderer_item_t libvlc_renderer_item_t;
LIBVLC_API libvlc_renderer_item_t *
libvlc_renderer_item_hold(libvlc_renderer_item_t *p_item);
LIBVLC_API void
libvlc_renderer_item_release(libvlc_renderer_item_t *p_item);
LIBVLC_API const char *
libvlc_renderer_item_name(const libvlc_renderer_item_t *p_item);
LIBVLC_API const char *
libvlc_renderer_item_type(const libvlc_renderer_item_t *p_item);
LIBVLC_API const char *
libvlc_renderer_item_icon_uri(const libvlc_renderer_item_t *p_item);
LIBVLC_API int
libvlc_renderer_item_flags(const libvlc_renderer_item_t *p_item);
LIBVLC_API libvlc_renderer_discoverer_t *
libvlc_renderer_discoverer_new( libvlc_instance_t *p_inst,
const char *psz_name );
LIBVLC_API void
libvlc_renderer_discoverer_release( libvlc_renderer_discoverer_t *p_rd );
LIBVLC_API int
libvlc_renderer_discoverer_start( libvlc_renderer_discoverer_t *p_rd );
LIBVLC_API void
libvlc_renderer_discoverer_stop( libvlc_renderer_discoverer_t *p_rd );
LIBVLC_API libvlc_event_manager_t *
libvlc_renderer_discoverer_event_manager( libvlc_renderer_discoverer_t *p_rd );
LIBVLC_API size_t
libvlc_renderer_discoverer_list_get( libvlc_instance_t *p_inst,
libvlc_rd_description_t ***ppp_services );
LIBVLC_API void
libvlc_renderer_discoverer_list_release( libvlc_rd_description_t **pp_services,
size_t i_count );
#if defined(__cplusplus)
}
#endif
