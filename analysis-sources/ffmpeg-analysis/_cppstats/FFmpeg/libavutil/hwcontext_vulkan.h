#include <vulkan/vulkan.h>
typedef struct AVVulkanDeviceContext {
const VkAllocationCallbacks *alloc;
VkInstance inst;
VkPhysicalDevice phys_dev;
VkDevice act_dev;
int queue_family_index;
int queue_family_tx_index;
int queue_family_comp_index;
} AVVulkanDeviceContext;
typedef struct AVVulkanFramesContext {
VkImageTiling tiling;
VkImageUsageFlagBits usage;
void *create_pnext;
void *alloc_pnext[AV_NUM_DATA_POINTERS];
} AVVulkanFramesContext;
typedef struct AVVkFrame {
VkImage img[AV_NUM_DATA_POINTERS];
VkImageTiling tiling;
VkDeviceMemory mem[AV_NUM_DATA_POINTERS];
size_t size[AV_NUM_DATA_POINTERS];
VkMemoryPropertyFlagBits flags;
VkAccessFlagBits access[AV_NUM_DATA_POINTERS];
VkImageLayout layout[AV_NUM_DATA_POINTERS];
VkSemaphore sem[AV_NUM_DATA_POINTERS];
struct AVVkFrameInternal *internal;
} AVVkFrame;
AVVkFrame *av_vk_frame_alloc(void);
const VkFormat *av_vkfmt_from_pixfmt(enum AVPixelFormat p);
