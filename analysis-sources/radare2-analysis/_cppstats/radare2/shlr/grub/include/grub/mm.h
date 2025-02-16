#include <grub/types.h>
#include <grub/symbol.h>
#if !defined(NULL)
#define NULL ((void *) 0)
#endif
void grub_mm_init_region (void *addr, grub_size_t size);
void *grub_malloc (grub_size_t size);
void *grub_zalloc (grub_size_t size);
void grub_free (void *ptr);
void *grub_realloc (void *ptr, grub_size_t size);
void *grub_memalign (grub_size_t align, grub_size_t size);
grub_size_t grub_mm_get_free (void);
#if defined(MM_DEBUG) && !defined(GRUB_UTIL) && !defined (GRUB_MACHINE_EMU)
extern int grub_mm_debug;
void grub_mm_dump_free (void);
void grub_mm_dump (unsigned lineno);
#define grub_malloc(size) grub_debug_malloc (GRUB_FILE, __LINE__, size)
#define grub_zalloc(size) grub_debug_zalloc (GRUB_FILE, __LINE__, size)
#define grub_realloc(ptr,size) grub_debug_realloc (GRUB_FILE, __LINE__, ptr, size)
#define grub_memalign(align,size) grub_debug_memalign (GRUB_FILE, __LINE__, align, size)
#define grub_free(ptr) grub_debug_free (GRUB_FILE, __LINE__, ptr)
void *grub_debug_malloc (const char *file, int line,
grub_size_t size);
void *grub_debug_zalloc (const char *file, int line,
grub_size_t size);
void grub_debug_free (const char *file, int line, void *ptr);
void *grub_debug_realloc (const char *file, int line, void *ptr,
grub_size_t size);
void *grub_debug_memalign (const char *file, int line,
grub_size_t align, grub_size_t size);
#endif 
