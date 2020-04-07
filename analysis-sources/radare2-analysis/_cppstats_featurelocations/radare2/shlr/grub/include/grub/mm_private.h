

















#if !defined(GRUB_MM_PRIVATE_H)
#define GRUB_MM_PRIVATE_H 1

#include <grub/mm.h>


#define GRUB_MM_FREE_MAGIC 0x2d3c2808
#define GRUB_MM_ALLOC_MAGIC 0x6db08fa4

typedef struct grub_mm_header
{
struct grub_mm_header *next;
grub_size_t size;
grub_size_t magic;
#if GRUB_CPU_SIZEOF_VOID_P == 4
char padding[4];
#elif GRUB_CPU_SIZEOF_VOID_P == 8
char padding[8];
#else
#error "unknown word size"
#endif
}
*grub_mm_header_t;

#if GRUB_CPU_SIZEOF_VOID_P == 4
#define GRUB_MM_ALIGN_LOG2 4
#elif GRUB_CPU_SIZEOF_VOID_P == 8
#define GRUB_MM_ALIGN_LOG2 5
#endif

#define GRUB_MM_ALIGN (1 << GRUB_MM_ALIGN_LOG2)

typedef struct grub_mm_region
{
struct grub_mm_header *first;
struct grub_mm_region *next;
grub_size_t pre_size;
grub_size_t size;
}
*grub_mm_region_t;

#if !defined(GRUB_MACHINE_EMU)
extern grub_mm_region_t EXPORT_VAR (grub_mm_base);
#endif

#endif
