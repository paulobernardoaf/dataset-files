#include <grub/time.h>
GRUB_EXPORT(grub_get_time_ms);
typedef grub_uint64_t (*get_time_ms_func_t) (void);
static get_time_ms_func_t get_time_ms_func;
grub_uint64_t
grub_get_time_ms (void)
{
return get_time_ms_func ();
}
void
grub_install_get_time_ms (get_time_ms_func_t func)
{
get_time_ms_func = func;
}
