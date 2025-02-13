#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "../libvlc.h"
#include <vlc_aout.h>
#include "audio_output/aout_internal.h"
#include "vlc_interface.h"
#include "vlc_codec.h"
#include "variables.h"
#if defined(HAVE_SEARCH_H)
#include <search.h>
#endif
#include <limits.h>
#include <assert.h>
#define vlc_children_foreach(pos, priv) while (((void)(pos), (void)(priv), 0))
int vlc_object_init(vlc_object_t *restrict obj, vlc_object_t *parent,
const char *typename)
{
vlc_object_internals_t *priv = malloc(sizeof (*priv));
if (unlikely(priv == NULL))
return -1;
priv->parent = parent;
priv->typename = typename;
priv->var_root = NULL;
vlc_mutex_init (&priv->var_lock);
vlc_cond_init (&priv->var_wait);
priv->resources = NULL;
obj->priv = priv;
obj->force = false;
if (likely(parent != NULL))
{
obj->logger = parent->logger;
obj->no_interact = parent->no_interact;
}
else
{
obj->logger = NULL;
obj->no_interact = false;
}
return 0;
}
void *(vlc_custom_create)(vlc_object_t *parent, size_t length,
const char *typename)
{
assert(length >= sizeof (vlc_object_t));
vlc_object_t *obj = calloc(length, 1);
if (unlikely(obj == NULL || vlc_object_init(obj, parent, typename))) {
free(obj);
obj = NULL;
}
return obj;
}
void *(vlc_object_create)(vlc_object_t *p_this, size_t i_size)
{
return vlc_custom_create( p_this, i_size, "generic" );
}
const char *vlc_object_typename(const vlc_object_t *obj)
{
return vlc_internals(obj)->typename;
}
vlc_object_t *(vlc_object_parent)(vlc_object_t *obj)
{
return vlc_internals(obj)->parent;
}
void vlc_object_deinit(vlc_object_t *obj)
{
vlc_object_internals_t *priv = vlc_internals(obj);
assert(priv->resources == NULL);
int canc = vlc_savecancel();
var_DestroyAll(obj);
vlc_restorecancel(canc);
free(priv);
}
void (vlc_object_delete)(vlc_object_t *obj)
{
vlc_object_deinit(obj);
free(obj);
}
void vlc_object_vaLog(vlc_object_t *obj, int prio, const char *module,
const char *file, unsigned line, const char *func,
const char *format, va_list ap)
{
if (obj == NULL)
return;
const char *typename = vlc_object_typename(obj);
if (typename == NULL)
typename = "generic";
vlc_vaLog(&obj->logger, prio, typename, module, file, line, func,
format, ap);
}
void vlc_object_Log(vlc_object_t *obj, int prio, const char *module,
const char *file, unsigned line, const char *func,
const char *format, ...)
{
va_list ap;
va_start(ap, format);
vlc_object_vaLog(obj, prio, module, file, line, func, format, ap);
va_end(ap);
}
size_t vlc_list_children(vlc_object_t *obj, vlc_object_t **restrict tab,
size_t max)
{
vlc_object_internals_t *priv;
size_t count = 0;
vlc_children_foreach(priv, vlc_internals(obj))
{
if (count < max)
tab[count] = vlc_object_hold(vlc_externals(priv));
count++;
}
return count;
}
