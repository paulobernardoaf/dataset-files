#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "platform.h"
int vlc_vk_InitPlatform(vlc_vk_t *vk)
{
if (vk->window->type != VOUT_WINDOW_TYPE_HWND)
return VLC_EGENERIC;
return VLC_SUCCESS;
}
void vlc_vk_ClosePlatform(vlc_vk_t *vk)
{
VLC_UNUSED(vk);
}
const char * const vlc_vk_PlatformExt = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
int vlc_vk_CreateSurface(vlc_vk_t *vk)
{
VkInstance vkinst = vk->instance->instance;
HINSTANCE hInst = GetModuleHandle(NULL);
VkWin32SurfaceCreateInfoKHR winfo = {
.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
.hinstance = hInst,
.hwnd = (HWND) vk->window->handle.hwnd,
};
VkResult res = vkCreateWin32SurfaceKHR(vkinst, &winfo, NULL, &vk->surface);
if (res != VK_SUCCESS) {
msg_Err(vk, "Failed creating Win32 surface");
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
