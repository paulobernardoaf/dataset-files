#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_inhibit.h>
#include <vlc_plugin.h>
#include <assert.h>
struct vlc_inhibit_sys
{
vlc_mutex_t mutex;
vlc_cond_t cond;
vlc_thread_t thread;
unsigned int mask;
};
static void Inhibit (vlc_inhibit_t *ih, unsigned mask)
{
vlc_inhibit_sys_t *sys = ih->p_sys;
vlc_mutex_lock(&sys->mutex);
sys->mask = mask;
vlc_cond_signal(&sys->cond);
vlc_mutex_unlock(&sys->mutex);
}
static void RestoreStateOnCancel( void* p_opaque )
{
VLC_UNUSED(p_opaque);
SetThreadExecutionState( ES_CONTINUOUS );
}
static void* Run(void* obj)
{
vlc_inhibit_t *ih = (vlc_inhibit_t*)obj;
vlc_inhibit_sys_t *sys = ih->p_sys;
EXECUTION_STATE prev_state = ES_CONTINUOUS;
for (unsigned int mask = 0;;)
{
vlc_mutex_lock(&sys->mutex);
mutex_cleanup_push(&sys->mutex);
vlc_cleanup_push(RestoreStateOnCancel, ih);
while (mask == sys->mask)
vlc_cond_wait(&sys->cond, &sys->mutex);
mask = sys->mask;
vlc_mutex_unlock(&sys->mutex);
vlc_cleanup_pop();
vlc_cleanup_pop();
bool suspend = (mask & VLC_INHIBIT_DISPLAY) != 0;
if (suspend)
prev_state = SetThreadExecutionState( ES_DISPLAY_REQUIRED |
ES_SYSTEM_REQUIRED |
ES_CONTINUOUS );
else
SetThreadExecutionState( prev_state );
}
vlc_assert_unreachable();
}
static void CloseInhibit (vlc_object_t *obj)
{
vlc_inhibit_t *ih = (vlc_inhibit_t*)obj;
vlc_inhibit_sys_t* sys = ih->p_sys;
vlc_cancel(sys->thread);
vlc_join(sys->thread, NULL);
}
static int OpenInhibit (vlc_object_t *obj)
{
vlc_inhibit_t *ih = (vlc_inhibit_t *)obj;
vlc_inhibit_sys_t *sys = ih->p_sys =
vlc_obj_malloc(obj, sizeof(vlc_inhibit_sys_t));
if (unlikely(ih->p_sys == NULL))
return VLC_ENOMEM;
vlc_mutex_init(&sys->mutex);
vlc_cond_init(&sys->cond);
sys->mask = 0;
if (vlc_clone(&sys->thread, Run, ih, VLC_THREAD_PRIORITY_LOW))
return VLC_EGENERIC;
ih->inhibit = Inhibit;
return VLC_SUCCESS;
}
vlc_module_begin ()
set_shortname (N_("Windows screensaver"))
set_description (N_("Windows screen saver inhibition"))
set_category (CAT_ADVANCED)
set_subcategory (SUBCAT_ADVANCED_MISC)
set_capability ("inhibit", 10)
set_callbacks (OpenInhibit, CloseInhibit)
vlc_module_end ()
