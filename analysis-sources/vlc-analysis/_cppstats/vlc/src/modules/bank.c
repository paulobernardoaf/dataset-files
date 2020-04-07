#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#if defined(HAVE_SEARCH_H)
#include <search.h>
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_modules.h>
#include <vlc_fs.h>
#include <vlc_block.h>
#include "libvlc.h"
#include "config/configuration.h"
#include "modules/modules.h"
typedef struct vlc_modcap
{
char *name;
module_t **modv;
size_t modc;
} vlc_modcap_t;
static int vlc_modcap_cmp(const void *a, const void *b)
{
const vlc_modcap_t *capa = a, *capb = b;
return strcmp(capa->name, capb->name);
}
static void vlc_modcap_free(void *data)
{
vlc_modcap_t *cap = data;
free(cap->modv);
free(cap->name);
free(cap);
}
static int vlc_module_cmp (const void *a, const void *b)
{
const module_t *const *ma = a, *const *mb = b;
return (*mb)->i_score - (*ma)->i_score;
}
static void vlc_modcap_sort(const void *node, const VISIT which,
const int depth)
{
vlc_modcap_t *const *cp = node, *cap = *cp;
if (which != postorder && which != leaf)
return;
qsort(cap->modv, cap->modc, sizeof (*cap->modv), vlc_module_cmp);
(void) depth;
}
static struct
{
vlc_mutex_t lock;
block_t *caches;
void *caps_tree;
unsigned usage;
} modules = { VLC_STATIC_MUTEX, NULL, NULL, 0 };
vlc_plugin_t *vlc_plugins = NULL;
static int vlc_module_store(module_t *mod)
{
const char *name = module_get_capability(mod);
vlc_modcap_t *cap = malloc(sizeof (*cap));
if (unlikely(cap == NULL))
return -1;
cap->name = strdup(name);
cap->modv = NULL;
cap->modc = 0;
if (unlikely(cap->name == NULL))
goto error;
void **cp = tsearch(cap, &modules.caps_tree, vlc_modcap_cmp);
if (unlikely(cp == NULL))
goto error;
if (*cp != cap)
{
vlc_modcap_free(cap);
cap = *cp;
}
module_t **modv = realloc(cap->modv, sizeof (*modv) * (cap->modc + 1));
if (unlikely(modv == NULL))
return -1;
cap->modv = modv;
cap->modv[cap->modc] = mod;
cap->modc++;
return 0;
error:
vlc_modcap_free(cap);
return -1;
}
static void vlc_plugin_store(vlc_plugin_t *lib)
{
vlc_mutex_assert(&modules.lock);
lib->next = vlc_plugins;
vlc_plugins = lib;
for (module_t *m = lib->module; m != NULL; m = m->next)
vlc_module_store(m);
}
static vlc_plugin_t *module_InitStatic(vlc_plugin_cb entry)
{
vlc_plugin_t *lib = vlc_plugin_describe (entry);
if (unlikely(lib == NULL))
return NULL;
#if defined(HAVE_DYNAMIC_PLUGINS)
atomic_init(&lib->handle, 0);
lib->unloadable = false;
#endif
return lib;
}
#if defined(__ELF__) || !HAVE_DYNAMIC_PLUGINS
VLC_WEAK
extern vlc_plugin_cb vlc_static_modules[];
static void module_InitStaticModules(void)
{
if (!vlc_static_modules)
return;
for (unsigned i = 0; vlc_static_modules[i]; i++)
{
vlc_plugin_t *lib = module_InitStatic(vlc_static_modules[i]);
if (likely(lib != NULL))
vlc_plugin_store(lib);
}
}
#else
static void module_InitStaticModules(void) { }
#endif
#if defined(HAVE_DYNAMIC_PLUGINS)
static const char *module_GetVersion(void *handle)
{
const char *(*get_api_version)(void);
get_api_version = vlc_dlsym(handle, "vlc_entry_api_version");
if (get_api_version == NULL)
return NULL;
return get_api_version();
}
static void *module_Open(struct vlc_logger *log,
const char *path, bool fast)
{
void *handle = vlc_dlopen(path, fast);
if (handle == NULL)
{
char *errmsg = vlc_dlerror();
vlc_error(log, "cannot load plug-in %s: %s", path,
errmsg ? errmsg : "unknown error");
free(errmsg);
return NULL;
}
const char *str = module_GetVersion(handle);
if (str == NULL) {
vlc_error(log, "cannot load plug-in %s: %s", path,
"unknown version or not a plug-in");
error:
vlc_dlclose(handle);
return NULL;
}
if (strcmp(str, VLC_API_VERSION_STRING)) {
vlc_error(log, "cannot load plug-in %s: unsupported version %s", path,
str);
goto error;
}
return handle;
}
static vlc_plugin_t *module_InitDynamic(vlc_object_t *obj, const char *path,
bool fast)
{
void *handle = module_Open(obj->logger, path, fast);
if (handle == NULL)
return NULL;
vlc_plugin_cb entry = vlc_dlsym(handle, "vlc_entry");
if (entry == NULL)
{
msg_Warn (obj, "cannot find plug-in entry point in %s", path);
goto error;
}
vlc_plugin_t *plugin = vlc_plugin_describe(entry);
if (unlikely(plugin == NULL))
{
msg_Err (obj, "cannot initialize plug-in %s", path);
goto error;
}
atomic_init(&plugin->handle, (uintptr_t)handle);
return plugin;
error:
vlc_dlclose(handle);
return NULL;
}
typedef enum
{
CACHE_READ_FILE = 0x1,
CACHE_SCAN_DIR = 0x2,
CACHE_WRITE_FILE = 0x4,
} cache_mode_t;
typedef struct module_bank
{
vlc_object_t *obj;
const char *base;
cache_mode_t mode;
size_t size;
vlc_plugin_t **plugins;
vlc_plugin_t *cache;
} module_bank_t;
static int AllocatePluginFile (module_bank_t *bank, const char *abspath,
const char *relpath, const struct stat *st)
{
vlc_plugin_t *plugin = NULL;
if (bank->mode & CACHE_READ_FILE)
{
plugin = vlc_cache_lookup(&bank->cache, relpath);
if (plugin != NULL
&& (plugin->mtime != (int64_t)st->st_mtime
|| plugin->size != (uint64_t)st->st_size))
{
msg_Err(bank->obj, "stale plugins cache: modified %s",
plugin->abspath);
vlc_plugin_destroy(plugin);
plugin = NULL;
}
}
if (plugin == NULL)
{
char *path = strdup(relpath);
if (path == NULL)
return -1;
plugin = module_InitDynamic(bank->obj, abspath, true);
if (plugin != NULL)
{
plugin->path = path;
plugin->mtime = st->st_mtime;
plugin->size = st->st_size;
}
else free(path);
}
if (plugin == NULL)
return -1;
vlc_plugin_store(plugin);
if (bank->mode & CACHE_WRITE_FILE) 
{
bank->plugins = xrealloc(bank->plugins,
(bank->size + 1) * sizeof (vlc_plugin_t *));
bank->plugins[bank->size] = plugin;
bank->size++;
}
return 0;
}
static void AllocatePluginDir (module_bank_t *bank, unsigned maxdepth,
const char *absdir, const char *reldir)
{
if (maxdepth == 0)
return;
maxdepth--;
DIR *dh = vlc_opendir (absdir);
if (dh == NULL)
return;
for (;;)
{
char *relpath = NULL, *abspath = NULL;
const char *file = vlc_readdir (dh);
if (file == NULL)
break;
if (!strcmp (file, ".") || !strcmp (file, ".."))
continue;
if (reldir != NULL)
{
if (asprintf (&relpath, "%s"DIR_SEP"%s", reldir, file) == -1)
relpath = NULL;
}
else
relpath = strdup (file);
if (unlikely(relpath == NULL))
continue;
if (asprintf (&abspath, "%s"DIR_SEP"%s", bank->base, relpath) == -1)
{
abspath = NULL;
goto skip;
}
struct stat st;
if (vlc_stat (abspath, &st) == -1)
goto skip;
if (S_ISREG (st.st_mode))
{
static const char prefix[] = "lib";
static const char suffix[] = "_plugin"LIBEXT;
size_t len = strlen (file);
#if !defined(__OS2__)
if (len > strlen (suffix)
&& !strncmp (file, prefix, strlen (prefix))
&& !strcmp (file + len - strlen (suffix), suffix))
#else
if (len > strlen (LIBEXT)
&& !strcasecmp (file + len - strlen (LIBEXT), LIBEXT))
#endif
AllocatePluginFile (bank, abspath, relpath, &st);
}
else if (S_ISDIR (st.st_mode))
AllocatePluginDir (bank, maxdepth, abspath, relpath);
skip:
free (relpath);
free (abspath);
}
closedir (dh);
}
static void AllocatePluginPath(vlc_object_t *obj, const char *path,
cache_mode_t mode)
{
module_bank_t bank =
{
.obj = obj,
.base = path,
.mode = mode,
};
if (mode & CACHE_READ_FILE)
bank.cache = vlc_cache_load(obj, path, &modules.caches);
else
msg_Dbg(bank.obj, "ignoring plugins cache file");
if (mode & CACHE_SCAN_DIR)
{
msg_Dbg(obj, "recursively browsing `%s'", bank.base);
AllocatePluginDir(&bank, 5, path, NULL);
}
while (bank.cache != NULL)
{
vlc_plugin_t *plugin = bank.cache;
bank.cache = plugin->next;
if (mode & CACHE_SCAN_DIR)
vlc_plugin_destroy(plugin);
else
vlc_plugin_store(plugin);
}
if (mode & CACHE_WRITE_FILE)
CacheSave(obj, path, bank.plugins, bank.size);
free(bank.plugins);
}
static void AllocateAllPlugins (vlc_object_t *p_this)
{
char *paths;
cache_mode_t mode = 0;
if (var_InheritBool(p_this, "plugins-cache"))
mode |= CACHE_READ_FILE;
if (var_InheritBool(p_this, "plugins-scan"))
mode |= CACHE_SCAN_DIR;
if (var_InheritBool(p_this, "reset-plugins-cache"))
mode = (mode | CACHE_WRITE_FILE) & ~CACHE_READ_FILE;
#if VLC_WINSTORE_APP
AllocatePluginPath (p_this, "plugins", mode);
#else
char *vlcpath = config_GetSysPath(VLC_PKG_LIB_DIR, "plugins");
if (likely(vlcpath != NULL))
{
AllocatePluginPath(p_this, vlcpath, mode);
free(vlcpath);
}
#endif 
paths = getenv( "VLC_PLUGIN_PATH" );
if( paths == NULL )
return;
#if defined(_WIN32)
paths = realpath( paths, NULL );
#else
paths = strdup( paths ); 
#endif
if( unlikely(paths == NULL) )
return;
for( char *buf, *path = strtok_r( paths, PATH_SEP, &buf );
path != NULL;
path = strtok_r( NULL, PATH_SEP, &buf ) )
AllocatePluginPath (p_this, path, mode);
free( paths );
}
int module_Map(struct vlc_logger *log, vlc_plugin_t *plugin)
{
static vlc_mutex_t lock = VLC_STATIC_MUTEX;
if (plugin->abspath == NULL)
return 0; 
if (atomic_load_explicit(&plugin->handle, memory_order_acquire))
return 0; 
assert(plugin->abspath != NULL);
void *handle = module_Open(log, plugin->abspath, false);
if (handle == NULL)
return -1;
vlc_plugin_cb entry = vlc_dlsym(handle, "vlc_entry");
if (entry == NULL)
{
vlc_error(log, "cannot find plug-in entry point in %s",
plugin->abspath);
goto error;
}
vlc_mutex_lock(&lock);
if (atomic_load_explicit(&plugin->handle, memory_order_relaxed) == 0)
{ 
if (vlc_plugin_resolve(plugin, entry))
{
vlc_mutex_unlock(&lock);
goto error;
}
atomic_store_explicit(&plugin->handle, (uintptr_t)handle,
memory_order_release);
}
else 
vlc_dlclose(handle);
vlc_mutex_unlock(&lock);
return 0;
error:
vlc_dlclose(handle);
return -1;
}
static void module_Unmap(vlc_plugin_t *plugin)
{
if (!plugin->unloadable)
return;
void *handle = (void *)atomic_exchange_explicit(&plugin->handle, 0,
memory_order_acquire);
if (handle != NULL)
vlc_dlclose(handle);
}
void *module_Symbol(struct vlc_logger *log,
vlc_plugin_t *plugin, const char *name)
{
if (plugin->abspath == NULL || module_Map(log, plugin))
return NULL;
void *handle = (void *)atomic_load_explicit(&plugin->handle,
memory_order_relaxed);
assert(handle != NULL);
return vlc_dlsym(handle, name);
}
#else
int module_Map(struct vlc_logger *log, vlc_plugin_t *plugin)
{
(void) log; (void) plugin;
return 0;
}
static void module_Unmap(vlc_plugin_t *plugin)
{
(void) plugin;
}
void *module_Symbol(struct vlc_logger *log,
vlc_plugin_t *plugin, const char *name)
{
(void) log; (void) plugin; (void) name;
return NULL;
}
#endif 
void module_InitBank (void)
{
vlc_mutex_lock (&modules.lock);
if (modules.usage == 0)
{
vlc_plugin_t *plugin = module_InitStatic(vlc_entry__core);
if (likely(plugin != NULL))
vlc_plugin_store(plugin);
config_SortConfig ();
}
modules.usage++;
}
void module_EndBank (bool b_plugins)
{
vlc_plugin_t *libs = NULL;
block_t *caches = NULL;
void *caps_tree = NULL;
if( b_plugins )
vlc_mutex_lock (&modules.lock);
else
vlc_mutex_assert(&modules.lock);
assert (modules.usage > 0);
if (--modules.usage == 0)
{
config_UnsortConfig ();
libs = vlc_plugins;
caches = modules.caches;
caps_tree = modules.caps_tree;
vlc_plugins = NULL;
modules.caches = NULL;
modules.caps_tree = NULL;
}
vlc_mutex_unlock (&modules.lock);
tdestroy(caps_tree, vlc_modcap_free);
while (libs != NULL)
{
vlc_plugin_t *lib = libs;
libs = lib->next;
module_Unmap(lib);
vlc_plugin_destroy(lib);
}
block_ChainRelease(caches);
}
#undef module_LoadPlugins
void module_LoadPlugins(vlc_object_t *obj)
{
if (modules.usage == 1)
{
module_InitStaticModules ();
#if defined(HAVE_DYNAMIC_PLUGINS)
msg_Dbg (obj, "searching plug-in modules");
AllocateAllPlugins (obj);
#endif
config_UnsortConfig ();
config_SortConfig ();
twalk(modules.caps_tree, vlc_modcap_sort);
}
vlc_mutex_unlock (&modules.lock);
size_t count;
module_t **list = module_list_get (&count);
module_list_free (list);
msg_Dbg (obj, "plug-ins loaded: %zu modules", count);
}
void module_list_free (module_t **list)
{
free (list);
}
module_t **module_list_get (size_t *n)
{
module_t **tab = NULL;
size_t i = 0;
assert (n != NULL);
for (vlc_plugin_t *lib = vlc_plugins; lib != NULL; lib = lib->next)
{
module_t **nt = realloc(tab, (i + lib->modules_count) * sizeof (*tab));
if (unlikely(nt == NULL))
{
free (tab);
*n = 0;
return NULL;
}
tab = nt;
for (module_t *m = lib->module; m != NULL; m = m->next)
tab[i++] = m;
}
*n = i;
return tab;
}
ssize_t module_list_cap (module_t ***restrict list, const char *name)
{
const void **cp = tfind(&name, &modules.caps_tree, vlc_modcap_cmp);
if (cp == NULL)
{
*list = NULL;
return 0;
}
const vlc_modcap_t *cap = *cp;
size_t n = cap->modc;
module_t **tab = vlc_alloc (n, sizeof (*tab));
*list = tab;
if (unlikely(tab == NULL))
return -1;
memcpy(tab, cap->modv, sizeof (*tab) * n);
return n;
}
