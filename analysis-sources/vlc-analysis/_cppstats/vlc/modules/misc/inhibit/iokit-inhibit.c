#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_inhibit.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
struct vlc_inhibit_sys
{
IOPMAssertionID act_assertion_id;
IOPMAssertionID inh_assertion_id;
};
static void UpdateInhibit(vlc_inhibit_t *ih, unsigned mask)
{
vlc_inhibit_sys_t* sys = ih->p_sys;
if (sys->inh_assertion_id != kIOPMNullAssertionID) {
msg_Dbg(ih, "Releasing previous IOPMAssertion");
if (IOPMAssertionRelease(sys->inh_assertion_id) != kIOReturnSuccess) {
msg_Err(ih, "Failed releasing previous IOPMAssertion, "
"not acquiring new one!");
}
sys->inh_assertion_id = kIOPMNullAssertionID;
}
IOReturn ret;
if ((mask & VLC_INHIBIT_DISPLAY) == VLC_INHIBIT_DISPLAY) {
CFStringRef activity_reason = CFSTR("VLC video playback");
msg_Dbg(ih, "Inhibiting display sleep");
ret = IOPMAssertionDeclareUserActivity(activity_reason,
kIOPMUserActiveLocal,
&(sys->act_assertion_id));
if (ret != kIOReturnSuccess) {
msg_Warn(ih, "Failed to declare user activity (%i)", ret);
}
ret = IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleDisplaySleep,
kIOPMAssertionLevelOn,
activity_reason,
&(sys->inh_assertion_id));
} else if ((mask & VLC_INHIBIT_SUSPEND) == VLC_INHIBIT_SUSPEND) {
CFStringRef activity_reason = CFSTR("VLC audio playback");
msg_Dbg(ih, "Inhibiting idle sleep");
ret = IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleSystemSleep,
kIOPMAssertionLevelOn,
activity_reason,
&(sys->inh_assertion_id));
} else if ((mask & VLC_INHIBIT_NONE) == VLC_INHIBIT_NONE) {
msg_Dbg(ih, "Removed previous inhibition");
return;
} else {
msg_Warn(ih, "Unhandled inhibiton mask (%i)", mask);
return;
}
if (ret != kIOReturnSuccess) {
msg_Err(ih, "Failed creating IOPMAssertion (%i)", ret);
return;
}
}
static int OpenInhibit(vlc_object_t *obj)
{
vlc_inhibit_t *ih = (vlc_inhibit_t *)obj;
vlc_inhibit_sys_t *sys = ih->p_sys =
vlc_obj_malloc(obj, sizeof(vlc_inhibit_sys_t));
if (unlikely(ih->p_sys == NULL))
return VLC_ENOMEM;
sys->act_assertion_id = kIOPMNullAssertionID;
sys->inh_assertion_id = kIOPMNullAssertionID;
ih->inhibit = UpdateInhibit;
return VLC_SUCCESS;
}
static void CloseInhibit(vlc_object_t *obj)
{
vlc_inhibit_t *ih = (vlc_inhibit_t*)obj;
vlc_inhibit_sys_t* sys = ih->p_sys;
if (sys->inh_assertion_id != kIOPMNullAssertionID) {
msg_Dbg(ih, "Releasing remaining IOPMAssertion (inhibition)");
if (IOPMAssertionRelease(sys->inh_assertion_id) != kIOReturnSuccess) {
msg_Warn(ih, "Failed releasing IOPMAssertion on termination");
}
sys->inh_assertion_id = kIOPMNullAssertionID;
}
if (sys->act_assertion_id != kIOPMNullAssertionID) {
msg_Dbg(ih, "Releasing remaining IOPMAssertion (activity)");
if (IOPMAssertionRelease(sys->act_assertion_id) != kIOReturnSuccess) {
msg_Warn(ih, "Failed releasing IOPMAssertion on termination");
}
sys->act_assertion_id = kIOPMNullAssertionID;
}
}
vlc_module_begin()
set_shortname(N_("macOS sleep inhibition"))
set_description(N_("macOS screen and idle sleep inhibition"))
set_category(CAT_ADVANCED)
set_subcategory(SUBCAT_ADVANCED_MISC)
set_capability("inhibit", 10)
set_callbacks(OpenInhibit, CloseInhibit)
vlc_module_end()
