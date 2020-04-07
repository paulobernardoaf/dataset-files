






















#if !defined(LIBVLC_LIBVLC_H)
#define LIBVLC_LIBVLC_H 1

#include <vlc_input_item.h>

extern const char psz_vlc_changeset[];

typedef struct variable_t variable_t;




void system_Init ( void );
void system_Configure ( libvlc_int_t *, int, const char *const [] );
#if defined(_WIN32) || defined(__OS2__)
void system_End(void);
#endif
void vlc_CPU_dump(vlc_object_t *);






int vlc_clone_detach (vlc_thread_t *, void *(*)(void *), void *, int);

int vlc_set_priority( vlc_thread_t, int );

void vlc_threads_setup (libvlc_int_t *);

void vlc_trace (const char *fn, const char *file, unsigned line);
#define vlc_backtrace() vlc_trace(__func__, __FILE__, __LINE__)




typedef struct vlc_logger vlc_logger_t;

int vlc_LogPreinit(libvlc_int_t *) VLC_USED;
void vlc_LogInit(libvlc_int_t *);




typedef struct vlc_exit
{
vlc_mutex_t lock;
void (*handler) (void *);
void *opaque;
} vlc_exit_t;

void vlc_ExitInit( vlc_exit_t * );



















int vlc_object_init(vlc_object_t *obj, vlc_object_t *parent,
const char *type_name);






void vlc_object_deinit(vlc_object_t *obj);














extern void *
vlc_custom_create (vlc_object_t *p_this, size_t i_size, const char *psz_type);
#define vlc_custom_create(o, s, n) vlc_custom_create(VLC_OBJECT(o), s, n)










void *vlc_objres_new(size_t size, void (*release)(void *));







void vlc_objres_push(vlc_object_t *obj, void *data);









void vlc_objres_clear(vlc_object_t *obj);












void vlc_objres_remove(vlc_object_t *obj, void *data,
bool (*match)(void *, void *));

#define ZOOM_SECTION N_("Zoom")
#define ZOOM_QUARTER_KEY_TEXT N_("1:4 Quarter")
#define ZOOM_HALF_KEY_TEXT N_("1:2 Half")
#define ZOOM_ORIGINAL_KEY_TEXT N_("1:1 Original")
#define ZOOM_DOUBLE_KEY_TEXT N_("2:1 Double")




typedef struct vlc_dialog_provider vlc_dialog_provider;
typedef struct vlc_keystore vlc_keystore;
typedef struct vlc_actions_t vlc_actions_t;
typedef struct vlc_playlist vlc_playlist_t;
typedef struct vlc_media_source_provider_t vlc_media_source_provider_t;
typedef struct intf_thread_t intf_thread_t;

typedef struct libvlc_priv_t
{
libvlc_int_t public_data;


vlc_mutex_t lock; 
vlm_t *p_vlm; 
vlc_dialog_provider *p_dialog_provider; 
vlc_keystore *p_memory_keystore; 
intf_thread_t *interfaces; 
vlc_playlist_t *main_playlist;
struct input_preparser_t *parser; 
vlc_media_source_provider_t *media_source_provider;
vlc_actions_t *actions; 
struct vlc_medialibrary_t *p_media_library; 
struct vlc_thumbnailer_t *p_thumbnailer; 


vlc_exit_t exit;
} libvlc_priv_t;

static inline libvlc_priv_t *libvlc_priv (libvlc_int_t *libvlc)
{
return container_of(libvlc, libvlc_priv_t, public_data);
}

int intf_InsertItem(libvlc_int_t *, const char *mrl, unsigned optc,
const char * const *optv, unsigned flags);
void intf_DestroyAll( libvlc_int_t * );

int vlc_MetadataRequest(libvlc_int_t *libvlc, input_item_t *item,
input_item_meta_request_option_t i_options,
const input_preparser_callbacks_t *cbs,
void *cbs_userdata,
int timeout, void *id);




void var_OptionParse (vlc_object_t *, const char *, bool trusted);

#endif
