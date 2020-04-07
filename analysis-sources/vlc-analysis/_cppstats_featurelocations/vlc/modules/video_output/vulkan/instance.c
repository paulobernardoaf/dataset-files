



















#include <assert.h>
#include <stdlib.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_modules.h>
#include <vlc_atomic.h>

#include "instance.h"








vlc_vk_t *vlc_vk_Create(struct vout_window_t *wnd, const char *name)
{
vlc_object_t *parent = (vlc_object_t *) wnd;
struct vlc_vk_t *vk;

vk = vlc_object_create(parent, sizeof (*vk));
if (unlikely(vk == NULL))
return NULL;

vk->ctx = NULL;
vk->instance = NULL;
vk->surface = (VkSurfaceKHR) NULL;

vk->window = wnd;
vk->module = module_need(vk, "vulkan", name, true);
if (vk->module == NULL)
{
vlc_object_delete(vk);
return NULL;
}
vlc_atomic_rc_init(&vk->ref_count);

return vk;
}

void vlc_vk_Hold(vlc_vk_t *vk)
{
vlc_atomic_rc_inc(&vk->ref_count);
}

void vlc_vk_Release(vlc_vk_t *vk)
{
if (!vlc_atomic_rc_dec(&vk->ref_count))
return;
module_unneed(vk, vk->module);
vlc_object_delete(vk);
}
