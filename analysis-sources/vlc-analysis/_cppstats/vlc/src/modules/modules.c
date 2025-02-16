#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdlib.h>
#include <string.h>
#if defined(ENABLE_NLS)
#include <libintl.h>
#endif
#include <assert.h>
#include <vlc_common.h>
#include <vlc_modules.h>
#include "libvlc.h"
#include "config/configuration.h"
#include "vlc_arrays.h"
#include "modules/modules.h"
bool module_provides (const module_t *m, const char *cap)
{
return !strcmp (module_get_capability (m), cap);
}
const char *module_get_object( const module_t *m )
{
if (unlikely(m->i_shortcuts == 0))
return "unnamed";
return m->pp_shortcuts[0];
}
const char *module_get_name( const module_t *m, bool long_name )
{
if( long_name && ( m->psz_longname != NULL) )
return m->psz_longname;
if (m->psz_shortname != NULL)
return m->psz_shortname;
return module_get_object (m);
}
const char *module_get_help( const module_t *m )
{
return m->psz_help;
}
const char *module_get_capability (const module_t *m)
{
return (m->psz_capability != NULL) ? m->psz_capability : "none";
}
int module_get_score( const module_t *m )
{
return m->i_score;
}
const char *module_gettext (const module_t *m, const char *str)
{
if (unlikely(str == NULL || *str == '\0'))
return "";
#if defined(ENABLE_NLS)
const char *domain = m->plugin->textdomain;
return dgettext ((domain != NULL) ? domain : PACKAGE_NAME, str);
#else
(void)m;
return str;
#endif
}
static bool module_match_name(const module_t *m, const char *name, size_t len)
{
if (len == 3 && strncasecmp("any", name, len) == 0)
return m->i_score > 0;
for (size_t i = 0; i < m->i_shortcuts; i++)
if (strncasecmp(m->pp_shortcuts[i], name, len) == 0
&& m->pp_shortcuts[i][len] == '\0')
return true;
return false;
}
static int module_load(vlc_logger_t *log, module_t *m,
vlc_activate_t init, bool forced, va_list args)
{
int ret = VLC_SUCCESS;
if (module_Map(log, m->plugin))
return VLC_EGENERIC;
if (m->pf_activate != NULL)
{
va_list ap;
va_copy (ap, args);
ret = init(m->pf_activate, forced, ap);
va_end (ap);
}
return ret;
}
module_t *(vlc_module_load)(struct vlc_logger *log, const char *capability,
const char *name, bool strict,
vlc_activate_t probe, ...)
{
if (name == NULL || name[0] == '\0')
name = "any";
module_t **mods;
ssize_t total = module_list_cap (&mods, capability);
vlc_debug(log, "looking for %s module matching \"%s\": %zd candidates",
capability, name, total);
if (total <= 0)
{
module_list_free (mods);
vlc_debug(log, "no %s modules", capability);
return NULL;
}
module_t *module = NULL;
va_list args;
va_start(args, probe);
while (*name)
{
const char *shortcut = name;
size_t slen = strcspn (name, ",");
name += slen;
name += strspn (name, ",");
if (!strcasecmp ("none", shortcut))
goto done;
bool force = strict && strcasecmp ("any", shortcut);
for (ssize_t i = 0; i < total; i++)
{
module_t *cand = mods[i];
if (cand == NULL)
continue; 
if (!module_match_name(cand, shortcut, slen))
continue;
mods[i] = NULL; 
int ret = module_load(log, cand, probe, force, args);
switch (ret)
{
case VLC_SUCCESS:
module = cand;
case VLC_ETIMEOUT:
goto done;
}
}
}
if (!strict)
{
for (ssize_t i = 0; i < total; i++)
{
module_t *cand = mods[i];
if (cand == NULL || module_get_score (cand) <= 0)
continue;
int ret = module_load(log, cand, probe, false, args);
switch (ret)
{
case VLC_SUCCESS:
module = cand;
case VLC_ETIMEOUT:
goto done;
}
}
}
done:
va_end (args);
module_list_free (mods);
if (module != NULL)
vlc_debug(log, "using %s module \"%s\"", capability,
module_get_object (module));
else
vlc_debug(log, "no %s modules matched", capability);
return module;
}
static int generic_start(void *func, bool forced, va_list ap)
{
vlc_object_t *obj = va_arg(ap, vlc_object_t *);
int (*activate)(vlc_object_t *) = func;
int ret;
obj->force = forced;
ret = activate(obj);
if (ret != VLC_SUCCESS)
vlc_objres_clear(obj);
return ret;
}
#undef module_need
module_t *module_need(vlc_object_t *obj, const char *cap, const char *name,
bool strict)
{
const bool b_force_backup = obj->force; 
module_t *module = vlc_module_load(obj->logger, cap, name, strict,
generic_start, obj);
if (module != NULL) {
var_Create(obj, "module-name", VLC_VAR_STRING);
var_SetString(obj, "module-name", module_get_object(module));
}
obj->force = b_force_backup;
return module;
}
#undef module_unneed
void module_unneed(vlc_object_t *obj, module_t *module)
{
msg_Dbg(obj, "removing module \"%s\"", module_get_object(module));
var_Destroy(obj, "module-name");
if (module->deactivate != NULL)
module->deactivate(obj);
vlc_objres_clear(obj);
}
module_t *module_find (const char *name)
{
size_t count;
module_t **list = module_list_get (&count);
assert (name != NULL);
for (size_t i = 0; i < count; i++)
{
module_t *module = list[i];
if (unlikely(module->i_shortcuts == 0))
continue;
if (!strcmp (module->pp_shortcuts[0], name))
{
module_list_free (list);
return module;
}
}
module_list_free (list);
return NULL;
}
bool module_exists (const char * psz_name)
{
return module_find (psz_name) != NULL;
}
module_config_t *module_config_get( const module_t *module, unsigned *restrict psize )
{
const vlc_plugin_t *plugin = module->plugin;
if (plugin->module != module)
{ 
*psize = 0;
return NULL;
}
unsigned i,j;
size_t size = plugin->conf.size;
module_config_t *config = vlc_alloc( size, sizeof( *config ) );
assert( psize != NULL );
*psize = 0;
if( !config )
return NULL;
for( i = 0, j = 0; i < size; i++ )
{
const module_config_t *item = plugin->conf.items + i;
if( item->b_internal 
|| item->b_removed )
continue;
memcpy( config + j, item, sizeof( *config ) );
j++;
}
*psize = j;
return config;
}
void module_config_free( module_config_t *config )
{
free( config );
}
