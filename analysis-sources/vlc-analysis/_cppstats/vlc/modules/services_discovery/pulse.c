#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <search.h>
#include <assert.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_services_discovery.h>
#include <pulse/pulseaudio.h>
#include "audio_output/vlcpulse.h"
static int Open (vlc_object_t *);
static void Close (vlc_object_t *);
VLC_SD_PROBE_HELPER("pulse", N_("Audio capture"), SD_CAT_DEVICES);
vlc_module_begin ()
set_shortname (N_("Audio capture"))
set_description (N_("Audio capture (PulseAudio)"))
set_category (CAT_PLAYLIST)
set_subcategory (SUBCAT_PLAYLIST_SD)
set_capability ("services_discovery", 0)
set_callbacks (Open, Close)
add_shortcut ("pulse", "pa", "pulseaudio", "audio")
VLC_SD_PROBE_SUBMODULE
vlc_module_end ()
typedef struct
{
void *root;
pa_context *context;
pa_threaded_mainloop *mainloop;
} services_discovery_sys_t;
static void SourceCallback(pa_context *, const pa_source_info *, int, void *);
static void ContextCallback(pa_context *, pa_subscription_event_type_t,
uint32_t, void *);
static int Open (vlc_object_t *obj)
{
services_discovery_t *sd = (services_discovery_t *)obj;
pa_operation *op;
pa_context *ctx;
services_discovery_sys_t *sys = malloc (sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
ctx = vlc_pa_connect (obj, &sys->mainloop);
if (ctx == NULL)
{
free (sys);
return VLC_EGENERIC;
}
sd->p_sys = sys;
sd->description = _("Audio capture");
sys->context = ctx;
sys->root = NULL;
const pa_subscription_mask_t mask = PA_SUBSCRIPTION_MASK_SOURCE;
pa_threaded_mainloop_lock (sys->mainloop);
pa_context_set_subscribe_callback (ctx, ContextCallback, sd);
op = pa_context_subscribe (ctx, mask, NULL, NULL);
if (likely(op != NULL))
pa_operation_unref (op);
op = pa_context_get_source_info_list (ctx, SourceCallback, sd);
if (likely(op != NULL))
{
pa_operation_unref (op);
}
pa_threaded_mainloop_unlock (sys->mainloop);
return VLC_SUCCESS;
}
struct device
{
uint32_t index;
input_item_t *item;
services_discovery_t *sd;
};
static void DestroySource (void *data)
{
struct device *d = data;
services_discovery_RemoveItem (d->sd, d->item);
input_item_Release (d->item);
free (d);
}
static int cmpsrc (const void *a, const void *b)
{
const uint32_t *pa = a, *pb = b;
uint32_t idxa = *pa, idxb = *pb;
return (idxa != idxb) ? ((idxa < idxb) ? -1 : +1) : 0;
}
static int AddSource (services_discovery_t *sd, const pa_source_info *info)
{
services_discovery_sys_t *sys = sd->p_sys;
msg_Dbg (sd, "adding %s (%s)", info->name, info->description);
char *mrl;
if (unlikely(asprintf (&mrl, "pulse://%s", info->name) == -1))
return -1;
input_item_t *item = input_item_NewCard (mrl, info->description);
free (mrl);
if (unlikely(item == NULL))
return -1;
struct device *d = malloc (sizeof (*d));
if (unlikely(d == NULL))
{
input_item_Release (item);
return -1;
}
d->index = info->index;
d->item = item;
void **dp = tsearch (d, &sys->root, cmpsrc);
if (dp == NULL) 
{
free (d);
input_item_Release (item);
return -1;
}
if (*dp != d) 
{
free (d);
d = *dp;
input_item_SetURI (d->item, item->psz_uri);
input_item_SetName (d->item, item->psz_name);
input_item_Release (item);
return 0;
}
const char *card = pa_proplist_gets(info->proplist, "device.product.name");
services_discovery_AddItemCat(sd, item,
(card != NULL) ? card : N_("Generic"));
d->sd = sd;
return 0;
}
static void SourceCallback(pa_context *ctx, const pa_source_info *i, int eol,
void *userdata)
{
services_discovery_t *sd = userdata;
if (eol)
return;
AddSource (sd, i);
(void) ctx;
}
static void RemoveSource (services_discovery_t *sd, uint32_t idx)
{
services_discovery_sys_t *sys = sd->p_sys;
void **dp = tfind (&idx, &sys->root, cmpsrc);
if (dp == NULL)
return;
struct device *d = *dp;
tdelete (d, &sys->root, cmpsrc);
DestroySource (d);
}
static void ContextCallback(pa_context *ctx, pa_subscription_event_type_t type,
uint32_t idx, void *userdata)
{
services_discovery_t *sd = userdata;
pa_operation *op;
assert ((type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
== PA_SUBSCRIPTION_EVENT_SOURCE);
switch (type & PA_SUBSCRIPTION_EVENT_TYPE_MASK)
{
case PA_SUBSCRIPTION_EVENT_NEW:
case PA_SUBSCRIPTION_EVENT_CHANGE:
op = pa_context_get_source_info_by_index(ctx, idx, SourceCallback, sd);
if (likely(op != NULL))
pa_operation_unref(op);
break;
case PA_SUBSCRIPTION_EVENT_REMOVE:
RemoveSource (sd, idx);
break;
}
}
static void Close (vlc_object_t *obj)
{
services_discovery_t *sd = (services_discovery_t *)obj;
services_discovery_sys_t *sys = sd->p_sys;
vlc_pa_disconnect (obj, sys->context, sys->mainloop);
tdestroy (sys->root, DestroySource);
free (sys);
}
