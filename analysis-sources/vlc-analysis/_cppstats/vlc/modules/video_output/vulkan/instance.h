#include <vlc_common.h>
#include <vlc_atomic.h>
#include <vlc_vout_window.h>
#include <vulkan/vulkan.h>
#include <libplacebo/vulkan.h>
struct vout_window_t;
struct vout_window_cfg_t;
typedef struct vlc_vk_t
{
struct vlc_object_t obj;
module_t *module;
vlc_atomic_rc_t ref_count;
void *platform_sys;
struct pl_context *ctx;
const struct pl_vk_inst *instance;
const struct pl_vulkan *vulkan;
const struct pl_swapchain *swapchain;
VkSurfaceKHR surface;
struct vout_window_t *window;
} vlc_vk_t;
vlc_vk_t *vlc_vk_Create(struct vout_window_t *, const char *) VLC_USED;
void vlc_vk_Release(vlc_vk_t *);
void vlc_vk_Hold(vlc_vk_t *);
