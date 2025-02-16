


















#if !defined(GRUB_MEMORY_HEADER)
#define GRUB_MEMORY_HEADER 1

#include <grub/types.h>
#include <grub/err.h>
#include <grub/machine/memory.h>

grub_err_t grub_mmap_iterate (int (*hook) (grub_uint64_t, grub_uint64_t,
grub_uint32_t, void *), void *);
int grub_mmap_register (grub_uint64_t start, grub_uint64_t size, int type);
grub_err_t grub_mmap_unregister (int handle);

void *grub_mmap_malign_and_register (grub_uint64_t align, grub_uint64_t size,
int *handle, int type, int flags);

void grub_mmap_free_and_unregister (int handle);

#if !defined(GRUB_MMAP_REGISTER_BY_FIRMWARE)

struct grub_mmap_region
{
struct grub_mmap_region *next;
grub_uint64_t start;
grub_uint64_t end;
int type;
int handle;
};

extern struct grub_mmap_region *grub_mmap_overlays;
#endif

#endif 
