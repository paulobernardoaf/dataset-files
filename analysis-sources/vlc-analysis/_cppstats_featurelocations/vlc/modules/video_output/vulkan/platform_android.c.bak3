





















#include "platform.h"
#include "../android/utils.h"

int vlc_vk_InitPlatform(vlc_vk_t *vk)
{
if (vk->window->type != VOUT_WINDOW_TYPE_ANDROID_NATIVE)
return VLC_EGENERIC;

return VLC_SUCCESS;
}

void vlc_vk_ClosePlatform(vlc_vk_t *vk)
{
AWindowHandler_releaseANativeWindow(vk->window->handle.anativewindow,
AWindow_Video);
}

const char * const vlc_vk_PlatformExt = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;

int vlc_vk_CreateSurface(vlc_vk_t *vk)
{
VkInstance vkinst = vk->instance->instance;

ANativeWindow *anw =
AWindowHandler_getANativeWindow(vk->window->handle.anativewindow,
AWindow_Video);

VkAndroidSurfaceCreateInfoKHR ainfo = {
.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
.pNext = NULL,
.flags = 0,
.window = anw,
};

VkResult res = vkCreateAndroidSurfaceKHR(vkinst, &ainfo, NULL, &vk->surface);
if (res != VK_SUCCESS) {
msg_Err(vk, "Failed creating Android surface");
return VLC_EGENERIC;
}

return VLC_SUCCESS;
}
