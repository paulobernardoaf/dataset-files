#include <uv.h>
#include "nvim/os/os.h"
uint64_t os_get_total_mem_kib(void)
{
return uv_get_total_memory() / 1024;
}
