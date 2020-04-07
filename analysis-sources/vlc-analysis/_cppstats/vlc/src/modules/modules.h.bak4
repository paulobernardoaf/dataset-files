





















#if !defined(LIBVLC_MODULES_H)
#define LIBVLC_MODULES_H 1

#include <stdatomic.h>


typedef struct vlc_plugin_t
{
struct vlc_plugin_t *next;
module_t *module;
unsigned modules_count;

const char *textdomain; 




struct
{
module_config_t *items; 
size_t size; 
size_t count; 
size_t booleans; 
} conf;

#if defined(HAVE_DYNAMIC_PLUGINS)
bool unloadable; 
atomic_uintptr_t handle; 
char *abspath; 

char *path; 
int64_t mtime; 
uint64_t size; 
#endif
} vlc_plugin_t;




extern struct vlc_plugin_t *vlc_plugins;

#define MODULE_SHORTCUT_MAX 20


typedef int (*vlc_plugin_cb) (int (*)(void *, void *, int, ...), void *);


int vlc_entry__core (int (*)(void *, void *, int, ...), void *);




struct module_t
{
vlc_plugin_t *plugin; 
module_t *next;


unsigned i_shortcuts;
const char **pp_shortcuts;




const char *psz_shortname; 
const char *psz_longname; 
const char *psz_help; 

const char *psz_capability; 
int i_score; 


const char *activate_name;
const char *deactivate_name;
void *pf_activate;
void (*deactivate)(vlc_object_t *);
};

vlc_plugin_t *vlc_plugin_create(void);
void vlc_plugin_destroy(vlc_plugin_t *);
module_t *vlc_module_create(vlc_plugin_t *);
void vlc_module_destroy (module_t *);

vlc_plugin_t *vlc_plugin_describe(vlc_plugin_cb);
int vlc_plugin_resolve(vlc_plugin_t *, vlc_plugin_cb);

void module_InitBank (void);
void module_LoadPlugins(vlc_object_t *);
#define module_LoadPlugins(a) module_LoadPlugins(VLC_OBJECT(a))
void module_EndBank (bool);
int module_Map(struct vlc_logger *, vlc_plugin_t *);
void *module_Symbol(struct vlc_logger *, vlc_plugin_t *, const char *name);

ssize_t module_list_cap (module_t ***, const char *);

int vlc_bindtextdomain (const char *);










void *vlc_dlopen(const char *path, bool) VLC_USED;










int vlc_dlclose(void *);














void *vlc_dlsym(void *handle, const char *) VLC_USED;






char *vlc_dlerror(void) VLC_USED;


vlc_plugin_t *vlc_cache_load(vlc_object_t *, const char *, block_t **);
vlc_plugin_t *vlc_cache_lookup(vlc_plugin_t **, const char *relpath);

void CacheSave(vlc_object_t *, const char *, vlc_plugin_t *const *, size_t);

#endif 
