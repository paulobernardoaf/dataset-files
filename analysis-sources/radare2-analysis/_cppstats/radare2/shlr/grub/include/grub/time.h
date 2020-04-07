#include <grub/types.h>
#include <grub/symbol.h>
#include <grub/cpu/time.h>
#if 0
#if defined (GRUB_MACHINE_EMU) || defined (GRUB_UTIL)
#define GRUB_TICKS_PER_SECOND 100000
grub_uint32_t EXPORT_FUNC (grub_get_rtc) (void);
#else
#include <grub/machine/time.h>
#endif
#endif
void grub_millisleep (grub_uint32_t ms);
grub_uint64_t grub_get_time_ms (void);
grub_uint64_t grub_rtc_get_time_ms (void);
static __inline void
grub_sleep (grub_uint32_t s)
{
grub_millisleep (1000 * s);
}
void grub_install_get_time_ms (grub_uint64_t (*get_time_ms_func) (void));
