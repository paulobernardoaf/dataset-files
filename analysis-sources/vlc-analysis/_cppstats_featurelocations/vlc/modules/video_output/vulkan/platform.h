



















#if !defined(VLC_VULKAN_PLATFORM_H)
#define VLC_VULKAN_PLATFORM_H

#include "instance.h"


int vlc_vk_InitPlatform(vlc_vk_t *);
void vlc_vk_ClosePlatform(vlc_vk_t *);


extern const char * const vlc_vk_PlatformExt;


int vlc_vk_CreateSurface(vlc_vk_t *);

#endif 
