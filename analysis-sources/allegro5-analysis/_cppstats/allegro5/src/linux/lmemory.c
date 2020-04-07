#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/platform/aintunix.h"
#if !defined(_POSIX_MAPPED_FILES) || !defined(ALLEGRO_HAVE_MMAP)
#error "Sorry, mapped files are required for Linux console Allegro to work!"
#endif
#include <unistd.h>
#include <sys/mman.h>
static int mem_fd = -1; 
int __al_linux_init_memory (void)
{
mem_fd = open ("/dev/mem", O_RDWR);
if (mem_fd < 0) return 1;
mprotect ((void *)&mem_fd, sizeof mem_fd, PROT_READ);
return 0;
}
int __al_linux_shutdown_memory (void)
{
if (mem_fd < 0) return 1;
mprotect ((void *)&mem_fd, sizeof mem_fd, PROT_READ | PROT_WRITE);
close (mem_fd);
mem_fd = -1;
return 0;
}
int __al_linux_map_memory (struct MAPPED_MEMORY *info)
{
ASSERT(info);
info->data = mmap (0, info->size, info->perms, MAP_SHARED, mem_fd, info->base);
if (info->data == MAP_FAILED) {
info->data = NULL;
return 1;
}
return 0;
}
int __al_linux_unmap_memory (struct MAPPED_MEMORY *info)
{
ASSERT(info);
if (info->data == NULL)
return 0;
if (!munmap (info->data, info->size)) {
info->data = NULL;
return 0;
}
return 1;
}
